

#ifndef AT_TOK_H
#define AT_TOK_H 1

#define ril_in_range(c, lo, up)  ((UINT8)c >= lo && (UINT8)c <= up)
#define ril_isprint(c)           ril_in_range(c, 0x20, 0x7f)
#define ril_isdigit(c)           ril_in_range(c, '0', '9')
#define ril_isxdigit(c)          (ril_isdigit(c) || ril_in_range(c, 'a', 'f') || ril_in_range(c, 'A', 'F'))
#define ril_islower(c)           ril_in_range(c, 'a', 'z')
#define ril_isspace(c)           (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')

int at_tok_start(char **p_cur);
int at_tok_nextint(char **p_cur, int *p_out);
int at_tok_nexthexint(char **p_cur, int *p_out);

int at_tok_nextbool(char **p_cur, char *p_out);
int at_tok_nextstr(char **p_cur, char **out);

int at_tok_hasmore(char **p_cur);

int at_tok_nextparenthes(char **p_cur,char **p_out);


#endif /*AT_TOK_H */
