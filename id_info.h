#ifndef ID_INFO_H_INCLUDED

#define ID_INFO_H_INCLUDED

typedef enum{
	ID_VARIABLE,
	ID_FUNCTION
} id_kind_t;

typedef struct{
	char *name;
	id_kind_t	id_type;
	union{
		struct{
			int offset;
		} variable;
		struct{
			/* 現在未使用 */
			int n_params;
		} function;
	};
} id_info_t;

#endif
