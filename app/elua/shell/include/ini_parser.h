#ifndef INI_PARSER_H
#define INI_PARSER_H
#ifdef __cplusplus
extern "C" {
#endif

int GetIniKeyInt(char *title,char *key,char *filename);
char *GetIniKeyString(char *title,char *key,char *filename);

 
#ifdef __cplusplus
}
#endif
#endif // INI_PARSER_H
