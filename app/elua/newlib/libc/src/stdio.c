/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    stdio.c
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2012/9/15
 *
 * Description:
 * 
 **************************************************************************/
#include <unistd.h>
#include "stdio.h"
#include "string.h"
#include "fcntl.h"
#include "errno.h"
#include "assert.h"
#include "local.h"
#include "reent.h"
#include "limits.h"
#include "btea.h"

#define	POS_ERR	(-(fpos_t)1)


static int __sread(void *cookie, char *buf, int n);
static int __swrite(void *cookie, const char *buf, int n);

static fpos_t __sseek(void *cookie, fpos_t offset, int whence);

static int __sclose(void *cookie);


#define	std(flags, file) {flags,file,__sF+file,__sclose,__sread,__sseek,__swrite,0}

FILE __sF[3] = {
    std(__SRD, STDIN_FILENO),          /* stdin */
    std(__SWR, STDOUT_FILENO),         /* stdout */
    std(__SWR|__SNBF, STDERR_FILENO)          /* stderr */
};


static char iobuf_temp[2048];
int lualibc_vfprintf(FILE *fp, const char *fmt, va_list ap)
{
    int len;

    len = vsnprintf(iobuf_temp, sizeof(iobuf_temp), fmt, ap);

    ASSERT(len < sizeof(iobuf_temp));

    return (*fp->_write)(fp->_cookie, iobuf_temp, len);
}

int lualibc_sprintf(char * buf,const char * fmt,...){
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = vsnprintf(buf, INT_MAX, fmt, ap);
    va_end(ap);

    return ret;
}

int
lualibc_printf(const char *fmt, ...)
{
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = lualibc_vfprintf(stdout, fmt, ap);
    va_end(ap);

    return ret;
}

int
lualibc_putc(int c, FILE *fp)
{
    return (*fp->_write)(fp->_cookie, (const char *)&c, 1);
}

int
lualibc_fprintf(FILE *fp, const char *fmt, ...)
{
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = lualibc_vfprintf(fp, fmt, ap);
    va_end(ap);

    return ret;
}

int
lualibc_fputs(const char *s, FILE *fp)
{
    return (*fp->_write)(fp->_cookie, s, strlen(s));
}

char *lualibc_fgets(char *buf, int n, FILE *fp)
{
    int character = 0;
    int idx = 0;
    
    if((NULL == buf) || (n <= 1) || (NULL == fp))
    {
        return NULL;
    }

    if(LUA_UNCOMPRESS_FILE & fp->_type)
    {
        return fgets_ext(buf,n,fp);
    }

    character = getc(fp);

    if(EOF == character)
    {
        return NULL;
    }
    
    while(EOF != character)
    {
        if(idx >= (n-1))
        {
            break;
        }

        buf[idx] = character;
        if(0x0A == character)
        {
            buf[idx+1] = 0;
            break;
        }
        
        idx++;
        character = getc(fp);
    }

    if(EOF == character)
    {
        buf[idx] = 0;
    }
    
    buf[n-1] = 0;
    return buf;
}

int lualibc_fflush(FILE *fp)
{       
    //return FS_Commit(fp);
    return 0;
}

/*
 * Return the (stdio) flags for a given mode.  Store the flags
 * to be passed to an open() syscall through *optr.
 * Return 0 on error.
 */
int __sflags(const char *mode, int *optr)
{
	int ret, m, o;

	switch (*mode++) {

	case 'r':	/* open for reading */
		ret = __SRD;
		m = O_RDONLY;
		o = 0;
		break;

	case 'w':	/* open for writing */
		ret = __SWR;
		m = O_WRONLY;
		o = O_CREAT | O_TRUNC;
		break;

	case 'a':	/* open for appending */
		ret = __SWR;
		m = O_WRONLY;
		o = O_CREAT | O_APPEND;
		break;

	default:	/* illegal mode */
		errno = EINVAL;
		return (0);
	}

	/* [rwa]\+ or [rwa]b\+ means read and write */
	if (*mode == '+' || (*mode == 'b' && mode[1] == '+')) {
		ret = __SRW;
		m = O_RDWR;
	}
	*optr = m | o;
	return (ret);
}

FILE *lualibc_fopen(const char *file, const char *mode)
{
    int f;
    FILE *fp;
    int flags, oflags;
    int fileNameLen = strlen(file);


    
	
    FILE *pFile = fopen_ext(file, mode);
    if((pFile != NULL) /*&& (pFile->_flags == 0)*/) /*LUA_RES_TABLE_FLASH_SECTION*/
    {
        return pFile;
    }

    
    if((flags = __sflags(mode, &oflags)) == 0)
        return (NULL);

	
	/*+\NEW\zhuwangbin\2020.3.29\ lua解析没有使用自带的__sfp接口，使用了8910 newlib中的接口导致内存泄漏*/
    if((fp = LUA__sfp()) == NULL)
        return (NULL);
	/*-\NEW\zhuwangbin\2020.3.29\ lua解析没有使用自带的__sfp接口，使用了8910 newlib中的接口导致内存泄漏*/
	
    if((f = _open_r(file, oflags, 0)) < 0){
        fp->_flags = 0;         /* release */
        printf("lualibc_fopen fail %d,%d,%s\r\n", f, oflags,file);
        return pFile;
    }

    fp->_file = f;
    fp->_flags = flags;
    fp->_cookie = fp;
    fp->_read = __sread;
    fp->_write = __swrite;
    fp->_seek = __sseek;
    fp->_close = __sclose;

#ifdef AM_LUA_CRYPTO_SUPPORT
    if(strncmp(&file[fileNameLen - 5],".luac", 5) == 0)
    {
        fp->_type = ENC_FILE;
    }
    else
#endif
    {
        fp->_type = COMMON_FILE;
    } 
    
    return fp;
}

int lualibc_fclose(FILE *fp)
{
    int r = 0;

    if(LUA_UNCOMPRESS_FILE & fp->_type)
    {
        return fclose_ext(fp);
    }
    
    if(fp->_flags == 0){    /* not open! */
        errno = EBADF;
        return (EOF);
    }

    //r = fp->_flags & __SWR ? __sflush(fp) : 0; 由于不需要清除缓存故直接返回0
    
    if(fp->_close != NULL && (*fp->_close)(fp->_cookie)){
        r = EOF;
    }
    
    fp->_flags = 0;         /* release */
    
    return r;
}

FILE *lualibc_freopen(const char *file, const char *mode, FILE *fp)
{
    lualibc_fclose(fp);
    return lualibc_fopen(file,  mode);
}

int lualibc_getc(FILE *fp)
{
    char c;

    if(LUA_UNCOMPRESS_FILE & fp->_type)
    {
        return getc_ext(fp);
    }

    if(fread((void *)&c, 1, 1, fp) != 1){
        return (EOF);
    }

    return c;
}

int lualibc_ungetc(int c, FILE *fp)
{
    // 由于没有实现完整的stdio 故暂不支持stdin ungetc
    ASSERT(fp != stdin);


    if(LUA_UNCOMPRESS_FILE & fp->_type)
    {
        return ungetc_ext(c,fp);
    }


    fseek(fp, -1, SEEK_CUR);

    return 0;
}

const uint32_t enc_code[4] = 
{
    0x982131a3,
    0x89765fad,
    0xa3b57aec,
    0x875ed2a5,
};

#define DEC_BUFF_SIZE 512

//#define CRYPTO_DEBUG

size_t lualibc_fread(void *buf, size_t size, size_t count, FILE *fp)
{
    size_t resid;
    int len;
	
    unsigned int* data = NULL;
    unsigned char* temp = buf;
    unsigned int offset  = ftell(fp);
    unsigned int act_low_boundary = (offset & 0xFFFFFE00); /*以512对齐的读取文件的起始位置*/
    unsigned int read_count;        /*需要从文件中读取的长度*/
    unsigned int act_up_boundary;   /*以512对齐的读取文件的结束位置*/
    unsigned int act_count;         /*读取到的有效数据长度*/

    if(LUA_UNCOMPRESS_FILE & fp->_type)
    {
        return fread_ext(buf,size,count,fp);
    }


    if((resid = count * size) == 0)
        return (0);

#ifdef AM_LUA_CRYPTO_SUPPORT

    if(ENC_FILE == fp->_type)
    {
        act_up_boundary = ((offset + resid + DEC_BUFF_SIZE - 1) & 0xFFFFFE00);
        read_count = act_up_boundary - act_low_boundary; 

        /*多申请8个字节的内存，以保证能4字节对齐*/
        data = (unsigned int*)L_MALLOC(4 + read_count + 4);
        
        /*保证4字节对齐*/
        temp = (unsigned char*)((((unsigned int)data + 3) >> 2) << 2);

        /*把文件指针移到以512对齐的位置*/
        lualibc_fseek(fp, act_low_boundary, SEEK_SET);
        
    }
    else
#endif

    {
        read_count = resid;
    }

    len = (*fp->_read)(fp->_cookie, temp, read_count);

    if(len <= 0){
        if(len == 0)
            fp->_flags |= __SEOF;
        else
            fp->_flags |= __SERR;

#ifdef AM_LUA_CRYPTO_SUPPORT
        if(ENC_FILE == fp->_type)
        {
            L_FREE(data);
        }
#endif

        return (0);
    }
    else
    {
#ifdef AM_LUA_CRYPTO_SUPPORT
        if(ENC_FILE == fp->_type)
        {
            int decCount = len / DEC_BUFF_SIZE;
            int i = 0;
            int real_size;
            
            act_count = resid;

            /*如果没有读到足够多的数据，意味着快到文件的末尾了*/
            if(read_count > len)
            {
                lualibc_fseek(fp, 0, SEEK_END);
                real_size = lualibc_ftell(fp);

                if(real_size - offset < resid)
                {
                    act_count = real_size - offset;
                }
            }
            
            /*把文件指针移到真实的位置*/
            lualibc_fseek(fp, offset + act_count, SEEK_SET);

#ifdef CRYPTO_DEBUG
            printf("liulean info  %d %d %d %d %d %d\r\n", 
                act_low_boundary, 
                act_up_boundary, 
                offset + count,
                offset - act_low_boundary,
                count,
                decCount);
#endif

            while(i < decCount)
            {
                btea((unsigned int*)(temp + DEC_BUFF_SIZE * i), -((DEC_BUFF_SIZE) / 4), enc_code);
                i++;
            }

            btea((unsigned int*)(temp + DEC_BUFF_SIZE * i), -((len % DEC_BUFF_SIZE) / 4), enc_code);
            
            memcpy(buf, &temp[offset - act_low_boundary], act_count);
            L_FREE(data);
        }
        else
#endif
        {
            act_count = len;
        }
    }
    
    return (act_count / size);
}

size_t lualibc_fwrite(const void *buf, size_t size, size_t count, FILE *fp)
{
    size_t len;
    int w;
    
    if(cantwrite(fp)){
        errno = EBADF;
        return (EOF);
    }

    len = size*count;

    w = (*fp->_write)(fp->_cookie, buf, len);

    if(w <= 0){
        fp->_flags |= __SERR;
        return (EOF);
    }
    
    return w/size;
}

int lualibc_fseek(FILE *fp, long offset, int whence)
{

    if(LUA_UNCOMPRESS_FILE & fp->_type)
    {
        return fseek_ext(fp,offset,whence);
    }

    if(fp->_seek == NULL){
        errno = ESPIPE;
        return (EOF);
    }

    if((*fp->_seek)(fp->_cookie, offset, whence) == POS_ERR){
        fp->_flags &= ~__SEOF;
        return (EOF);
    }

    return (0);
}

FILE *lualibc_tmpfile(void)
{
    errno = ENOTSUP;
    
    return (FILE*)0;
}

int lualibc_fscanf(FILE *fp, const char *fmt, ...)
{
    errno = ENOTSUP;
    
    return (-1);
}

long lualibc_ftell(FILE *fp)
{
	fpos_t pos;

    if(LUA_UNCOMPRESS_FILE & fp->_type)
    {
        return ftell_ext(fp);
    }

	if (fp->_seek == NULL) {
		errno = ESPIPE;			/* historic practice */
		return ((off_t)-1);
	}

	if (fp->_flags & __SOFF)
		pos = fp->_offset;
	else {
		pos = (*fp->_seek)(fp->_cookie, (fpos_t)0, SEEK_CUR);
	}

    return pos;
}

int lualibc_setvbuf(FILE *fp, char *buf, int mode, size_t size)
{
    errno = ENOTSUP;
    
    return (-1);
}

/*+\NEW\liweiqiang\2013.5.11\增加remove接口*/
int lualibc_remove(const char *filename)
{
    if(_unlink_r(filename) == -1)
        return -1;

    return 0;
}
/*-\NEW\liweiqiang\2013.5.11\增加remove接口*/

int lualibc_rename(const char *old, const char *new)
{
	if(_rename_r(old, new) < 0)
	      return -1;

	  return 0;
}

char* lualibc_tmpnam(char *s)
{
    errno = ENOSYS;
    return NULL;
}

/*
 * Small standard I/O/seek/close functions.
 * These maintain the `known seek offset' for seek optimisation.
 */
static int
__sread(void *cookie, char *buf, int n)
{
	FILE *fp = cookie;
	int ret;
	
	ret = _read_r(fp->_file, buf, n);
	/* if the read succeeded, update the current offset */
	if (ret >= 0)
		fp->_offset += ret;
	else
		fp->_flags &= ~__SOFF;	/* paranoia */
	return (ret);
}

static int
__swrite(void *cookie, const char *buf, int n)
{
	FILE *fp = cookie;

	if (fp->_flags & O_APPEND)
		(void) _lseek_r(fp->_file, (off_t)0, SEEK_END);
	fp->_flags &= ~__SOFF;	/* in case FAPPEND mode is set */
	return (_write_r(fp->_file, buf, n));
}

static fpos_t
__sseek(void *cookie, fpos_t offset, int whence)
{
	FILE *fp = cookie;
	off_t ret;
	
	ret = _lseek_r(fp->_file, (off_t)offset, whence);
	if (ret == (off_t)-1)
		fp->_flags &= ~__SOFF;
	else {
		fp->_flags |= __SOFF;
		fp->_offset = ret;
	}
	return (ret);
}

static int
__sclose(void *cookie)
{
	return (_close_r(((FILE *)cookie)->_file));
}


