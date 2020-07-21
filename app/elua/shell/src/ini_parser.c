/*c语言实现，可在linux平台上用了，在Windows下可以用GetPrivateProfileString或GetPrivateProfileInt方便读取.ini配置文件内容，
但是在Linux平台上就一筹莫展了。为了解决该问题，打算用C来读取.ini，即可不受平台的限制了*/
#include <string.h>
#ifdef WIN32
#include <Windows.h>
#include <stdio.h>
#else
#define  MAX_PATH 260
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#endif
char g_szConfigPath[MAX_PATH];

#include "am_openat.h"

#define fgetc getc

//从INI文件读取字符串类型数据
char *GetIniKeyString(char *title,char *key,char *filename) 
{ 
	FILE *fp; 
	char szLine[256];
	static char tmpstr[256];
	int rtnval;
	int i = 0; 
	int flag = 0; 
	char *tmp;
	char* strend;
 
	if((fp = fopen(filename, "r")) == NULL) 
	{ 
		printf("have   no   such   file \n");
		return ""; 
	}
	while(!feof(fp)) 
	{ 
		rtnval = fgetc(fp); 
		if(rtnval == EOF) 
		{ 
			break; 
		} 
		else 
		{ 
			szLine[i++] = rtnval; 
		} 
		if(rtnval == '\n') 
		{ 
#ifndef WIN32
			i--;
#endif	
			szLine[--i] = '\0';
			i = 0; 
			tmp = strchr(szLine, '='); 

 
			if(( tmp != NULL )&&(flag == 1)) 
			{ 
				if(strstr(szLine,key)!=NULL) 
				{ 
					//注释行
					if ('#' == szLine[0])
					{
					}
					else if ( '/' == szLine[0] && '/' == szLine[1] )
					{
						
					}
					else
					{
						//找打key对应变量
						tmp++;
						while(*tmp && (*tmp == ' ' || *tmp == '\t')) tmp++;
						strend = tmp;
						while(*strend && *strend != ' ' && *strend != '\t') strend++;
						strncpy(tmpstr,tmp, strend - tmp); 
						tmpstr[strend - tmp] = 0;
						fclose(fp);
						return tmpstr; 
					}
				} 
			}
			else 
			{ 
				strcpy(tmpstr,"["); 
				strcat(tmpstr,title); 
				strcat(tmpstr,"]");
				if( strncmp(tmpstr,szLine,strlen(tmpstr)) == 0 ) 
				{
					//找到title
					flag = 1; 
				}
			}
		}
	}
	fclose(fp); 
	return ""; 
}
 
//从INI文件读取整类型数据
int GetIniKeyInt(char *title,char *key,char *filename)
{
	return atoi(GetIniKeyString(title,key,filename));
}
 


