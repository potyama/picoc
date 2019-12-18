#include	<stdio.h>
#include	<stdlib.h>
#include	"define.h"
#include	"scanner.h"
#include	"parser.h"
#include	"error.h"

/* 次の字句を先読みするための変数 */
static token_t	nextsym;

static int
parse_expression(void);

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

/* S ::= E ; */
int
parser_parse_statement(void)
{
	int	r;
	r = parse_expression();
	if (nextsym.sym != SYM_SEMICOLON){
		ERROR("';' が必要です");
	}
	return (r);
}

/* E ::= [+ | -] T { (+ | -) T } */
int
parse_expression(void)
{
	int	r;
	int	s;

	s = 1;
	if (nextsym.sym == SYM_PLUS || nextsym.sym == SYM_MINUS){
		if (nextsym.sym == SYM_PLUS){
			s = 1;
		}else if (nextsym.sym == SYM_MINUS){
			s = -1;
		}
		nextsym = scanner_get_next_sym();
	}
	r = s * parse_term();
	while (nextsym.sym == SYM_PLUS || nextsym.sym == SYM_MINUS){
		if (nextsym.sym == SYM_PLUS){
			nextsym = scanner_get_next_sym();
			s = 1;
		}else if (nextsym.sym == SYM_MINUS){
			nextsym = scanner_get_next_sym();
			s = -1;
		}else{
			ERROR("'+' または '-' が必要です");
		}
		r = r + s * parse_term();
	}
	return (r);
}

/* T ::= F { (* | -) F } */
int
parse_term(void)
{
	int	r;
	int	m;
	
	r = parse_factor();
	while (nextsym.sym == SYM_ASTERISK || nextsym.sym == SYM_SLASH){
		if (nextsym.sym == SYM_ASTERISK){
			nextsym = scanner_get_next_sym();
			m = 1;
		}else if (nextsym.sym == SYM_SLASH){
			nextsym = scanner_get_next_sym();
			m = 0;
		}else{
			ERROR("'*' または '/' が必要です");
		}
		if (m == 1){
			r = r * parse_term();
		}else{
			r = r / parse_term();
		}
	}
	return (r);
}

/* F ::= N | ( E ) */
int
parse_factor(void)
{
	int	r;

	if (nextsym.sym == SYM_CONSTANT_INT){
		r = parse_number();
	}else if (nextsym.sym == SYM_LPAREN){
		nextsym = scanner_get_next_sym();
		r = parse_expression();
		if (nextsym.sym != SYM_RPAREN){
			ERROR("')' が必要です");
		}
		nextsym = scanner_get_next_sym();
	}else{
		ERROR("<N> または '(' が必要です");
	}
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
