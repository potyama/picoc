/* 課題								*/
/*								*/
/* (1) 組込み関数 read, write を追加				*/
/* (2) % 演算子を追加						*/
/* (3) parse_factor() の <変数> に対応するコード生成を追加	*/
/* (4) RETURN 文のコード生成を追加				*/
/* (5) WHILE 文のコード生成を追加				*/
/* (6) IF 文に ELSE を追加					*/
/* (7) DO-WHILE 文を追加					*/
/* (8) += 演算子の追加						*/
/* (9) FOR 文を追加						*/
/* (A) グローバル変数の導入					*/
/* (B) x86 ネイティブコードコンパイラの作成			*/
/*     picoc-vm の命令を x86 に変換するのが簡単			*/

#include	<stdio.h>
#include	<stdlib.h>
#include	"define.h"
#include	"scanner.h"
#include	"parser.h"
#include	"id_table_stack.h"
#include	"id_table.h"
#include	"id_info.h"
#include	"codegen.h"

static token_t	nextsym;
static id_table_stack_t	*id_table_stack;
static int	variable_offset;
static int	label_counter;

static void
parse_program(void);

static void
parse_global_definition(int x);

static void
parse_func_var(void);

static void
parse_function_definition(void);

static void
parse_parameter_declaration(void);

static void
parse_parameter_list(void);

static void
parse_parameter(void);

static void
parse_complex_statement(void);

static void
parse_variable_declaration_list(void);

static void
parse_variable_declaration(void);

static void
parse_variable_list(void);

static void
parse_statement_list(void);

static void
parse_statement(void);

static void
parse_expression_statement(void);

static void
parse_if_statement(void);

static void
parse_while_statement(void);

static void
parse_do_statement(void);

static void
parse_for_statement(void);

static void
parse_return_statement(void);

static void
parse_expression(void);

static void
parse_assign_expression(void);

static void
parse_equality_expression(void);

static void
parse_relational_expression(void);

static void
parse_additive_expression(void);

static void
parse_multiplicative_expression(void);

static void
parse_monomial_expression(void);

static void
parse_factor(void);

static void
parse_argument_list(int *n_args);

static id_info_t *
search_for_identifier(char *name);

static char *
format_label(int l,char *lstr);

static void
error(char *str,char *func,int line);

static void
put_startup(void);

#define	MAX_BLOCK_NEST		100
#define MAX_IDS_PER_BLOCK	100

#define	LABEL_LEN			50

/* 一番右の引数の FP からの変位 */
#define	PARAMETER_OFFSET	3

/* 戻り値の FP からの変位 */
#define	RETURN_OFFSET		2

/* 最初のローカル変数の FP からの変位 */
#define	VARIABLE_OFFSET		(-1)

/* エラーメッセージを表示する */
#define	ERROR(m)	error((m),((char *)__FUNCTION__),(__LINE__))

void
parser_init(void)
{
	nextsym = scanner_get_next_sym();
	id_table_stack = id_table_stack_new(MAX_BLOCK_NEST);
	id_table_stack_push(id_table_stack, id_table_new(MAX_IDS_PER_BLOCK));
	label_counter = 0;
}

void
parser_parse_program(void)
{
	put_startup();
	parse_program();
}

/*プログラム ::= 関数定義 { 関数定義 } . */
void
parse_program(void)
{

	while(nextsym.sym != SYM_INT){
		parse_func_var();
	}

	while (nextsym.sym != SYM_EOF){
		parse_function_definition();
	}
}

void
parse_func_var(void){
	if (nextsym.sym != SYM_INT){
		ERROR("Parser error");
	}
	nextsym = scanner_get_next_sym();
	int x = nextsym;
	nextsym = scanner_get_next_sym();
	if(nextsym.sym == SYM_LPAREN){
		parse_function_definition();
	}else if(nextsym.sym == SYM_SEMICOLON){
		parse_global_definition(x);
	}
}

void
parse_global_definition(int x){

	id_info_t	*info;

	if ((info = search_for_identifier(x.identifier)) == NULL){
		/* 変数が未定義 */
		ERROR("Parser error");
	}
	if(nextsym.sym == SYM_EQUAL){
		nextsym = scanner_get_next_sym();
		parse_expression();
		codegen_put_code_num("storeg",info->variable.offset);
}

/*関数定義 ::= 型名 関数名 仮引数宣言 複文 . */
void
parse_function_definition(void){

	id_info_t	info;

	if (nextsym.sym != SYM_INT){
		ERROR("Parser error");
	}
	nextsym = scanner_get_next_sym();
	if (nextsym.sym != SYM_IDENTIFIER){
		ERROR("Parser error");
	}

	/* ここで関数名を識別子表に登録する(衝突のチェックも) */
	info.name = nextsym.identifier;
	info.id_type = ID_FUNCTION;
	info.function.n_params = -1; /* 引数の数．いまのところ未使用 */
	if (!id_table_add(id_table_stack_peek(id_table_stack,0), info)){
		/* 名前の衝突 */
		ERROR("Parser error");
	}

	//codegen_put_comment("関数定義",0);
	codegen_put_label(nextsym.identifier);

	/* この関数のための新しいレベルの識別子表を作る */
	id_table_stack_push(id_table_stack, id_table_new(MAX_IDS_PER_BLOCK));

	nextsym = scanner_get_next_sym();
	parse_parameter_declaration();

	codegen_put_code("enter");

	variable_offset = VARIABLE_OFFSET;
	parse_complex_statement();

	codegen_put_code("leave");
	codegen_put_code("ret");

	/* 用済みなので識別子表を捨てる */
	id_table_delete(id_table_stack_pop(id_table_stack));
}

/* 仮引数宣言 ::= "(" [ 仮引数並び ] ")" . */
/* 配布した BNF を少し変更                 */
void
parse_parameter_declaration(void)
{
	if (nextsym.sym != SYM_LPAREN){
		ERROR("Parser error");
	}
	nextsym = scanner_get_next_sym();
	if (nextsym.sym != SYM_RPAREN){
		parse_parameter_list();
	}
	if (nextsym.sym != SYM_RPAREN){
		ERROR("Parser error");
	}
	nextsym = scanner_get_next_sym();
}

/*仮引数並び ::= 仮引数 { "," 仮引数 } . */
/* 配布した BNF を少し変更               */
void
parse_parameter_list(void)
{
	int	n,i,param_offset;
	id_info_t	*info;

	n = 0;
	parse_parameter();
	n++;
	while (nextsym.sym == SYM_COMMA){
		nextsym = scanner_get_next_sym();
		parse_parameter();
		n++;
	}
	/* 引数の数がわかってから各引数のオフセットを計算する	*/
	/* 最後(最右)の引数のオフセットは PARAMETER_OFFSET		*/
	/* 最後から 2 番目の引数は PARAMETER_OFFSET + 1			*/
	/* 以下, 1 番地ずつ増えていく							*/
	/* 最初の引数のオフセットは PARAMETER_OFFSET + n - 1	*/
	param_offset = PARAMETER_OFFSET;
	for (i = n - 1;i >= 0;i--){
		info = id_table_get_id_info(id_table_stack_peek(id_table_stack,0),i);
		info->variable.offset = param_offset;
		param_offset++;
	}
}

/*仮引数 ::= 型名 引数名 . */
void
parse_parameter(void)
{
	id_info_t	info;

	if (nextsym.sym != SYM_INT){
		ERROR("Parser error");
	}
	nextsym = scanner_get_next_sym();
	if (nextsym.sym != SYM_IDENTIFIER){
		ERROR("Parser error");
	}

	info.name = nextsym.identifier;
	info.id_type = ID_VARIABLE;
	info.variable.offset = 0; /* オフセット．この時点では値が決まらない */
	if (!id_table_add(id_table_stack_peek(id_table_stack,0), info)){
		/* 名前の衝突 */
		ERROR("Parser error");
	}

	nextsym = scanner_get_next_sym();
}

/* 複文 ::= "{" 変数宣言並び 文並び "}" . */
void
parse_complex_statement(void)
{
	int	voffset_base,voffset_diff;

	voffset_base = variable_offset;
	if (nextsym.sym != SYM_LBRACE){
		ERROR("Parser error");
	}
	nextsym = scanner_get_next_sym();
	parse_variable_declaration_list();
	voffset_diff = variable_offset - voffset_base;
	//codegen_put_comment("複文",1);
	if (voffset_diff < 0){
		codegen_put_code_num("mvsp", voffset_diff);
	}
	parse_statement_list();
	if (nextsym.sym != SYM_RBRACE){
		ERROR("Parser error");
	}
	if (voffset_diff < 0){
		codegen_put_code_num("mvsp", -voffset_diff);
	}
	//codegen_put_comment("複文終わり",1);
	/* 2010/12/10 修正 */
	variable_offset = voffset_base;
	nextsym = scanner_get_next_sym();
}

/* 変数宣言並び ::= { 変数宣言 } . */
void
parse_variable_declaration_list(void)
{
	while (nextsym.sym == SYM_INT){
		parse_variable_declaration();
	}
}

/* 変数宣言 ::= 型名 変数名並び ";" . */
void
parse_variable_declaration(void)
{
	if (nextsym.sym != SYM_INT){
		ERROR("Parser error");
	}
	nextsym = scanner_get_next_sym();
	parse_variable_list();
	if (nextsym.sym != SYM_SEMICOLON){
		ERROR("Parser error");
	}
	nextsym = scanner_get_next_sym();
}


/* 変数名並び ::= 変数名 { "," 変数名 } . */
void
parse_variable_list(void)
{
	id_info_t	info;

	if (nextsym.sym != SYM_IDENTIFIER){
		ERROR("Parser error");
	}
	info.name = nextsym.identifier;
	info.id_type = ID_VARIABLE;
	info.variable.offset = variable_offset;
	variable_offset--;
	if (!id_table_add(id_table_stack_peek(id_table_stack,0), info)){
		/* 名前の衝突 */
		ERROR("Parser error");
	}
	nextsym = scanner_get_next_sym();
	while (nextsym.sym == SYM_COMMA){
		nextsym = scanner_get_next_sym();
		if (nextsym.sym != SYM_IDENTIFIER){
			ERROR("Parser error");
		}
		info.name = nextsym.identifier;
		info.id_type = ID_VARIABLE;
		info.variable.offset = variable_offset;
		variable_offset--;
		if (!id_table_add(id_table_stack_peek(id_table_stack,0), info)){
			/* 名前の衝突 */
			ERROR("Parser error");
		}
		nextsym = scanner_get_next_sym();
	}
}

/* 文並び ::= { 文 } . */
void
parse_statement_list(void)
{
	while (nextsym.sym != SYM_RBRACE){
		parse_statement();
	}
}

/* 文 ::= 式文 | 複文 | 選択文 | 反復文 | リターン文. */
void
parse_statement(void)
{
	if (nextsym.sym == SYM_LBRACE){
		/* 複文のために新しいレベルの識別子表を作る */
		id_table_stack_push(id_table_stack, id_table_new(MAX_IDS_PER_BLOCK));

		parse_complex_statement();

		/* 用済みなので作成した識別子表を捨てる */
		id_table_delete(id_table_stack_pop(id_table_stack));
	}else if (nextsym.sym == SYM_IF){
		parse_if_statement();
	}else if (nextsym.sym == SYM_WHILE){
		parse_while_statement();
	}else if (nextsym.sym == SYM_DO){
		parse_do_statement();
	}else if (nextsym.sym == SYM_FOR){
		parse_for_statement();
	}else if (nextsym.sym == SYM_RETURN){
		parse_return_statement();
	}else{
		parse_expression_statement();
	}
}

/* 式文 ::= [ 式 ] ";" . */
void
parse_expression_statement(void)
{
	//codegen_put_comment("式文",1);
	if (nextsym.sym == SYM_SEMICOLON){
		/* 空の式文 */
		nextsym = scanner_get_next_sym();
	}else{
		parse_expression();
		if (nextsym.sym == SYM_SEMICOLON){
			nextsym = scanner_get_next_sym();
		}else{
			ERROR("Parser error");
		}
		codegen_put_code_num("mvsp",1);
	}
	//codegen_put_comment("式文終わり",1);
}

/* 選択文 ::= "if" "(" 式 ")" 文 [ "else" 文 ] . */
void
parse_if_statement(void)
{
	char	l1[LABEL_LEN], l2[LABEL_LEN];

	nextsym = scanner_get_next_sym();
	if (nextsym.sym != SYM_LPAREN){
		ERROR("Parser error");
	}
	nextsym = scanner_get_next_sym();
	parse_expression();
	if (nextsym.sym != SYM_RPAREN){
		ERROR("Parser error");
	}
	format_label(label_counter, l1);
	label_counter++;
	format_label(label_counter, l2);
	label_counter++;
	//codegen_put_comment("if文",1);
	codegen_put_code_str("jf",l1);
	nextsym = scanner_get_next_sym();
	//codegen_put_comment("条件成立時",1);
	parse_statement();
	codegen_put_code_str("jp",l2);

	codegen_put_label(l1);

	//codegen_put_comment("if文終わり",1);
	if(nextsym.sym == SYM_ELSE){
		nextsym = scanner_get_next_sym();
		parse_statement();
	}
	codegen_put_label(l2);
}


/* 反復文 ::= "while" "( 式 ")" 文 . */
void
parse_while_statement(void)
{
	char	l1[LABEL_LEN], l2[LABEL_LEN];

	format_label(label_counter, l1);
	label_counter++;

	format_label(label_counter, l2);
	label_counter++;

	codegen_put_label(l1);

	nextsym = scanner_get_next_sym();
	if (nextsym.sym != SYM_LPAREN){
		ERROR("Parser error");
	}
	nextsym = scanner_get_next_sym();
	parse_expression();
	if (nextsym.sym != SYM_RPAREN){
		ERROR("Parser error");
	}

	codegen_put_code_str("jf", l2);

	nextsym = scanner_get_next_sym();
	parse_statement();

	codegen_put_code_str("jp", l1);

	codegen_put_label(l2);
}

/* DO文 ::= "do" 文 "while" "( 式 ")" ; . */
void
parse_do_statement(void)
{

	/* DO 文に対応しなさい */

	char	l1[LABEL_LEN];

	format_label(label_counter, l1);
	label_counter++;
	nextsym = scanner_get_next_sym();

	codegen_put_label(l1);

	parse_statement();

	if (nextsym.sym != SYM_WHILE){
		ERROR("Parser error");
	}
	nextsym = scanner_get_next_sym();

	if (nextsym.sym != SYM_LPAREN){
		ERROR("Parser error");
	}
	nextsym = scanner_get_next_sym();

	parse_expression();
	if (nextsym.sym != SYM_RPAREN){
		ERROR("Parser error");
	}
	nextsym = scanner_get_next_sym();

	codegen_put_code_str("jt", l1);

}

/* 反複文 ::= "for" "(" 式1 ";" 式2 ";" 式3")" 文 .*/

void
parse_for_statement(void){
	char start[LABEL_LEN]; //start
	char prog[LABEL_LEN]; //program
	char count[LABEL_LEN]; //count
	char end[LABEL_LEN]; //end

	format_label(label_counter, start);
	label_counter++;
	format_label(label_counter, prog);
	label_counter++;
	format_label(label_counter, count);
	label_counter++;
	format_label(label_counter, end);
	label_counter++;

	nextsym = scanner_get_next_sym();

	if(nextsym.sym != SYM_LPAREN){
		ERROR("Parser error");
	}
	nextsym = scanner_get_next_sym();

	//式1
	parse_expression();

	if(nextsym.sym != SYM_SEMICOLON){
		ERROR("Parser error");
	}

	codegen_put_label(start);

	//式2
	nextsym = scanner_get_next_sym();
	parse_expression();
	codegen_put_code_str("jf", end);
	codegen_put_code_str("jp", prog);

	if(nextsym.sym != SYM_SEMICOLON){
		ERROR("Parser error");
	}

	//式3
	codegen_put_label(count);
	nextsym = scanner_get_next_sym();
	parse_expression();
	codegen_put_code_str("jp", start);

	if(nextsym.sym != SYM_RPAREN){
		ERROR("Parser error");
	}

	//文
	codegen_put_label(prog);

	nextsym = scanner_get_next_sym();
	parse_statement();
	codegen_put_code_str("jp", count);

	codegen_put_label(end);
}
/* リターン文 ::= "return" [ 式 ] ";" .		*/
/* 2010/12/06 式を [ ] に入れてオプションにした	*/
void
parse_return_statement(void)
{
	nextsym = scanner_get_next_sym();
	if (nextsym.sym != SYM_SEMICOLON){
		parse_expression();
		/* ここでparse_expression() が計算した値を	*/
		/* 戻り値領域にストアするコードを生成		*/
		codegen_put_code_num("storel", RETURN_OFFSET);
	}
	if (nextsym.sym != SYM_SEMICOLON){
		ERROR("Parser error");
	}
	/* ここで関数終了の処理のコードを生成			*/
	nextsym = scanner_get_next_sym();
}

/* 式 ::= 等価式 | 代入式 . */
void
parse_expression(void)
{
	token_t	t;

	/* nextsym のさらに一つ先の記号を t に読み込んで調べる	*/
	/* nextsym が識別子，t が = なら代入式である			*/
	/* t に先読みした記号は scanner_push_back_sym() で，	*/
	/* scanner に返しておく									*/
	/* 2 文字先読みは反則気味だが，処理がすっきりする		*/

	t = scanner_get_next_sym();
	scanner_push_back_sym(t);
	if (nextsym.sym == SYM_IDENTIFIER && (t.sym == SYM_EQUAL || t.sym == SYM_PLUSEQUAL)){
		parse_assign_expression();
	}else{
		parse_equality_expression();
	}
}

/* 代入式 ::= 変数名 代入演算子 式 . */
void
parse_assign_expression(void)
{
	id_info_t	*info;

	if ((info = search_for_identifier(nextsym.identifier)) == NULL){
		/* 変数が未定義 */
		ERROR("Parser error");
	}
	if (info->id_type != ID_VARIABLE){
		/* この名前は変数名でない */
		ERROR("Parser error");
	}
	nextsym = scanner_get_next_sym();
	if (nextsym.sym != SYM_EQUAL && nextsym.sym != SYM_PLUSEQUAL){
		ERROR("Parser error");
	}
	if(nextsym.sym == SYM_EQUAL){
		nextsym = scanner_get_next_sym();
		parse_expression();
		codegen_put_code_num("storel",info->variable.offset);
	}else if(nextsym.sym == SYM_PLUSEQUAL){
		nextsym = scanner_get_next_sym();
		parse_expression();
		codegen_put_code("add");
		codegen_put_code_num("storel", info->variable.offset);
	}
}

/* 等価式 ::= 関係式 { ( "==" | "!=" ) 関係式 } . */
void
parse_equality_expression(void)
{
	char	*opcode;

	parse_relational_expression();
	while (nextsym.sym == SYM_EQ || nextsym.sym == SYM_NE){
		if (nextsym.sym == SYM_EQ){
			opcode = "eq";
		}else{
			opcode = "ne";
		}
		nextsym = scanner_get_next_sym();
		parse_relational_expression();
		codegen_put_code(opcode);
	}
}

/* 関係式 ::= 加算式 { ( "<" | ">" | "<=" | ">=" ) 加算式 } . */
void
parse_relational_expression(void)
{
	char	*opcode;

	parse_additive_expression();
	while (nextsym.sym == SYM_LT || nextsym.sym == SYM_GT|| nextsym.sym == SYM_LE|| nextsym.sym == SYM_GE){
		if (nextsym.sym == SYM_LT){
			opcode = "lt";
		}else if (nextsym.sym == SYM_GT){
			opcode = "gt";
		}else if (nextsym.sym == SYM_LE){
			opcode = "le";
		}else if (nextsym.sym == SYM_GE){
			opcode = "ge";
		}
		nextsym = scanner_get_next_sym();
		parse_additive_expression();
		codegen_put_code(opcode);
	}
}

/* 加算式 ::= 乗算式 { ( "+" | "-" ) 乗算式 } . */
void
parse_additive_expression(void)
{
	char	*opcode;

	parse_multiplicative_expression();
	while (nextsym.sym == SYM_PLUS || nextsym.sym == SYM_MINUS){
		if (nextsym.sym == SYM_PLUS){
			opcode = "add";
		}else if (nextsym.sym == SYM_MINUS){
			opcode = "sub";
		}
		nextsym = scanner_get_next_sym();
		parse_multiplicative_expression();
		codegen_put_code(opcode);
	}
}

/* 乗算式 ::= 単項式 { ( "*" | "/" | "%" ) 単項式 } . */
void
parse_multiplicative_expression(void)
{
	char	*opcode;

	parse_monomial_expression();
	while (nextsym.sym == SYM_ASTERISK || nextsym.sym == SYM_SLASH || nextsym.sym == SYM_PERCENT){
		if (nextsym.sym == SYM_ASTERISK){
			opcode = "mul";
		}else if (nextsym.sym == SYM_SLASH){
			opcode = "div";
		}else if (nextsym.sym == SYM_PERCENT){
                 	opcode = "mod";
		}
            	nextsym = scanner_get_next_sym();
		parse_monomial_expression();
		codegen_put_code(opcode);
	}
}

/* 単項式 ::= [ 単項演算子 ] 単項式 | 素式 . */
void
parse_monomial_expression(void)
{
	token_t	t;
	if (nextsym.sym == SYM_PLUS || nextsym.sym == SYM_MINUS){
		t = nextsym;
		nextsym = scanner_get_next_sym();
		parse_monomial_expression();
		if (t.sym == SYM_MINUS){
			codegen_put_code_num("pushi",-1);
			codegen_put_code("mul");
		}
	}else{
		parse_factor();
	}
}

/* 素式 ::= 変数名 | 関数名 "(" [ 実引数並び ] ")" | 定数 | "(" 式 ")" . */
void
parse_factor(void)
{
	token_t	t;
	id_info_t	*info;
	int	n_args;

	if (nextsym.sym == SYM_IDENTIFIER){
		/* 変数か関数 */
		info = search_for_identifier(nextsym.identifier);
		t = scanner_get_next_sym();
		if (t.sym != SYM_LPAREN){
			/* 次の文字が ( でないので，変数のはず */
			if (info == NULL){
				/* 変数が未定義 */
				ERROR("Parser error");
			}else if (info->id_type != ID_VARIABLE){
				/* 変数名ではない */
				ERROR("Parser error");
			}
			/* ここで変数を push する命令を生成						*/
			/* codegen_put_code_num() は 1 個の数値オペランドを持つ	*/
			/* 命令を生成する										*/
			codegen_put_code_num("pushl",info->variable.offset);
			nextsym = t;
		}else{
			/* 次の文字が ( なので，関数のはず */
			if (info == NULL){
				/* 未定義の関数．                                   */
				/* これから定義されるかもしれないのでエラーではない */
				/* 前方参照のマークをつけて id_table に登録すべき？ */
				/* いまのところ，なにもしない						*/
			}else if (info->id_type != ID_FUNCTION){
				/* 関数ではない */
				ERROR("Parser error");
			}
			t = nextsym;
			nextsym = scanner_get_next_sym();
			if (nextsym.sym != SYM_RPAREN){
				parse_argument_list(&n_args);
			}else{
				n_args = 0;
			}
			if (nextsym.sym != SYM_RPAREN){
				ERROR("Parser error");
			}
			codegen_put_code_num("mvsp",-1);
			codegen_put_code_str("call",t.identifier);

			/* ここで引数を捨てて戻り値をスタックトップに移動しなくてはならない */
			if (n_args > 0){
				codegen_put_code_num("storet",n_args);
				codegen_put_code_num("mvsp",n_args);
			}
			nextsym = scanner_get_next_sym();
		}
	}else if (nextsym.sym == SYM_CONSTANT_INT){
		codegen_put_code_num("pushi",nextsym.integer);
		nextsym = scanner_get_next_sym();
	}else if (nextsym.sym == SYM_LPAREN){
		nextsym = scanner_get_next_sym();
		parse_expression();
		if (nextsym.sym != SYM_RPAREN){
			ERROR("Parser error");
		}
		nextsym = scanner_get_next_sym();
	}
}

/* 実引数並び ::= 式 { "," 式 } . */
void
parse_argument_list(int *n_args)
{
	*n_args = 0;
	parse_expression();
	(*n_args)++;
	while (nextsym.sym == SYM_COMMA){
		nextsym = scanner_get_next_sym();
		parse_expression();
		(*n_args)++;
	}
}

id_info_t *
search_for_identifier(char *name)
{
	int		i, n;
	id_info_t	*info;

	n = id_table_stack_get_size(id_table_stack);
	for (i = 0;i < n;i++){
		info = id_table_search(id_table_stack_peek(id_table_stack, i), name);
		if (info != NULL){
			return (info);
		}
	}
	return (NULL);
}

char *
format_label(int l,char *lstr)
{
	sprintf(lstr,".L%d",l);
	return (lstr);
}

void
put_startup(void)
{
	codegen_put_comment("Picoc09 2011-12-13",0);
	codegen_put_label("__start__");
	codegen_put_code_str("call","main");
	codegen_put_code("halt");

	/* 組込み関数 read() を追加 */
	codegen_put_comment("組込み関数 read()",0);
	codegen_put_comment("ここに read() の機械語を書く",0);
      codegen_put_label("read");
      codegen_put_code("enter");
      codegen_put_code("rd");
      codegen_put_code_num("storel", 2);
      codegen_put_code("leave");
      codegen_put_code("ret");

	/* 組込み関数 write() を追加 */
	codegen_put_comment("組込み関数 write()",0);
	codegen_put_comment("ここに write() の機械語を書く",0);
      codegen_put_label("write");
      codegen_put_code("enter");
      codegen_put_code_num("pushl", PARAMETER_OFFSET);
      codegen_put_code("wr");
      codegen_put_code("leave");
      codegen_put_code("ret");

	codegen_put_comment("組込み関数 writeln()",0);
	codegen_put_label("writeln");
	codegen_put_code("enter");
	codegen_put_code("wrln");
	codegen_put_code("leave");
	codegen_put_code("ret");
}

void
error(char *str,char *func,int line)
{
	fprintf(
		stderr,"Error: at %d: %s(%s:%d)\n",
		scanner_get_current_line_number(),str,func,line
	);
	exit(1);
}
