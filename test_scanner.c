#include	<stdio.h>
#include	"scanner.h"

char *sym_name[] = {
	"INVALID",
	"IDENTIFIER",
	"CONSTANT_INT",
	"IF",
	"ELSE",
	"WHILE",
	"DO",
	"FOR",
	"RETURN",
	"INT",
	"PLUS",
	"MINUS",
	"ASTERISK",
	"SLASH",
	"PERCENT",
	"LT",
	"LE",
	"GT",
	"GE",
	"EQ",
	"NE",
	"EQUAL",
	"PLUSEQUAL",
	"SEMICOLON",
	"COMMA",
	"LPAREN",
	"RPAREN",
	"LBRACE",
	"RBRACE",
	"EOFILE"
};

int
main()
{
	token_t	sym;

	scanner_init(stdin);
	sym = scanner_get_next_sym();
	while (sym.sym != SYM_EOF){
		printf("%d:%s",scanner_get_current_line_number(),sym_name[sym.sym]);
		if (sym.sym == SYM_IDENTIFIER){
			printf("(%s)",sym.identifier);
		}else if (sym.sym == SYM_CONSTANT_INT){
			printf("(%d)",sym.integer);
		}else if (sym.sym == SYM_INVALID){
			printf("(%c)", sym.invalid_char);
		}
		printf("\n");
		sym = scanner_get_next_sym();
	}
}
