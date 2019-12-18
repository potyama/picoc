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
parse_factor(void);

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
	int s,r;

	if(nextsym.sym == SYM_MINUS){
            s = -1;
	}else if(nextsym.sym ==SYM_PLUS){
            s = 1;
      }else{
            s = 1;
      }
      r = s * parse_term();

	nextsym = scanner_get_next_sym();
	while (nextsym.sym == SYM_PLUS || nextsym.sym == SYM_MINUS){
		if (nextsym.sym == SYM_PLUS){
			nextsym = scanner_get_next_sym();
			r += parse_term();
		}else if(nextsym.sym == SYM_MINUS){
			nextsym = scanner_get_next_sym();
			r -= parse_term();
		}else{
			ERROR("'+'か'-' が必要です");
		}
      }
	return (r);
}

int 
parse_term(void){
      int r;
      
      r = parse_factor();

      while(nextsym.sym == SYM_ASTERISK || nextsym.sym == SYM_SLASH){
           if(nextsym.sym == SYM_ASTERISK){
                  nextsym = scanner_get_next_sym();
                  r *= parse_factor();
           }else if(nextsym.sym = SYM_SLASH){
                  nextsym = scanner_get_next_sym();
                  r /= parse_factor();
           }
      }
      return (r);
}

/* T ::= N */
int
parse_factor(void)
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

