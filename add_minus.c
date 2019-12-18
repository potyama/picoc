#include	<stdio.h>
#include	<stdlib.h>
#include	"define.h"
#include	"scanner.h"
#include	"parser.h"
#include	"error.h"

/* 次の字句を先読みするための変数 */
static token_t	nextsym;

static int
parse_term(void);

static int
parse_number(void);

void
parser_init(void)
{
	nextsym = scanner_get_next_sym();
}

/* E ::= T { + T } ; */
int
parser_parse_expression(void)
{
	int	cp,r;
	
	if(nextsym.sym == SYM_PLUS || nextsym.sym == SYM_MINUS){
		cp = nextsym.sym;
	}
	nextsym = scanner_get_next_sym();
	r = parse_term();
	if(cp == SYM_MINUS){
		r = r - (r*2);
      }
	//nextsym = scanner_get_next_sym();
	while (nextsym.sym != SYM_SEMICOLON){
		if (nextsym.sym == SYM_PLUS){
			nextsym = scanner_get_next_sym();
			r += parse_term();
		}else if(nextsym.sym == SYM_MINUS){
			nextsym = scanner_get_next_sym();
			r -= parse_term();
            }else if(nextsym.sym == SYM_ASTERISK){
                  nextsym = scanner_get_next_sym();
                  r *= parse_term();
            }else if(nextsym.sym == SYM_SLASH){
                  nextsym = scanner_get_next_sym();
                  r /= parse_term();
		}else{
			ERROR("'+'か'-'か'*'か'/'が必要です");
		}
	}
	if (nextsym.sym != SYM_SEMICOLON){
		ERROR("';' が必要です");
	}
	/* ';' は式の終わりの記号なので，それ以上先読みをしない */
	/*nextsym = scanner_get_next_sym();*/

	return (r);
}

/* T ::= N */
int
parse_term(void)
{
	int	r;

	r = parse_number();
	return (r);
}

int
parse_number(void)
{
	int	r;

	if (nextsym.sym == SYM_CONSTANT_INT){
		r = nextsym.integer;
		nextsym = scanner_get_next_sym();
	}else{
		ERROR("<N> が必要です");
	}
	return (r);
}

