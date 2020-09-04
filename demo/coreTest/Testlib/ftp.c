/***************************************************************************/
/*									   */
/* qftp.c - command line driven ftp file transfer program		   */
/* Copyright (C) 1996-2001, 2013 Thomas Pfau, tfpfau@gmail.com		   */
/*	1407 Thomas Ave, North Brunswick, NJ, 08902			   */
/*									   */
/* This library is free software.  You can redistribute it and/or	   */
/* modify it under the terms of the Artistic License 2.0.		   */
/* 									   */
/* This library is distributed in the hope that it will be useful,	   */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of	   */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	   */
/* Artistic License 2.0 for more details.				   */
/* 									   */
/* See the file LICENSE or 						   */
/* http://www.perlfoundation.org/artistic_license_2_0			   */
/*									   */
/***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iot_network.h"
#include "ftppal.h"
#include "ftplib.h"
#include "iot_fs.h"

/* exit values */
#define EX_SYNTAX 2	 /* command syntax errors */
#define EX_NETDB 3	 /* network database errors */
#define EX_CONNECT 4 /* network connect errors */
#define EX_LOGIN 5	 /* remote login errors */
#define EX_REMCMD 6	 /* remote command errors */
#define EX_SYSERR 7	 /* system call errors */

#define FTP_SEND 1 /* send files */
#define FTP_GET 2  /* retreive files */
#define FTP_DIR 3  /* verbose directory */
#define FTP_RM 4   /* delete files */
#define FTP_LIST 5 /* terse directory */

#define DIRBUF_SIZE 1024 /* for wildcard processing */

#define PRIFSZ "u"

#define APP_FTP_SERVER_IP "36.7.87.100"
#define APP_FTP_USR "user"
#define APP_FTP_PWD "123456"
#define APP_FTP_GET_FILE "1.txt"
#define APP_FTP_MSG_NETWORK_READY (0)
#define APP_FTP_MSG_NETWORK_LINKED (1)

static int logged_in = 0;
static char *host = APP_FTP_SERVER_IP;
static char *user = APP_FTP_USR;
static char *pass = APP_FTP_PWD;
static char mode = 'I';
static int action = 0;
static netbuf *conn = NULL;
static int wildcard = 0;

static HANDLE g_s_ftp_task;

void ftp_connect(void)
{
	if (conn)
		return;
	if (!logged_in)
	{
		if (!FtpConnect(host, &conn))
		{
			iot_debug_print("[coreTest-False-ftp]: Unable to connect to node %s\n", host);
			exit(EX_CONNECT);
		}
		if (!FtpLogin(user, pass, conn))
		{
			iot_debug_print("[coreTest-False-ftp]: Login failure\n%s", FtpLastResponse(conn));
			exit(EX_LOGIN);
		}
		logged_in++;
	}
}

void change_directory(char *root)
{
	ftp_connect();
	if (!FtpChdir(root, conn))
	{
		iot_debug_print("[coreTest-False-ftp]: Change directory failed\n%s", FtpLastResponse(conn));
		exit(EX_REMCMD);
	}
}

void site_cmd(char *cmd)
{
	ftp_connect();
	if (!FtpSite(cmd, conn))
	{
		iot_debug_print("[coreTest-False-ftp]: SITE command failed\n%s", FtpLastResponse(conn));
		exit(EX_REMCMD);
	}
}

struct REMFILE
{
	struct REMFILE *next;
	fsz_t fsz;
	char *fnm;
};

static int log_progress(netbuf *ctl, fsz_t xfered, void *arg)
{
	struct REMFILE *f = (struct REMFILE *)arg;
	if (f->fsz)
	{
		int pct = (xfered * 100.0) / f->fsz;
		iot_debug_print("[coreTest-ftp]: %s %d%% %" PRIFSZ "\r", f->fnm, pct, xfered);
	}
	else
	{
		iot_debug_print("[coreTest-ftp]: %s %" PRIFSZ "\r", f->fnm, xfered);
	}
	return 1;
}

void process_file(char *fnm)
{
	int sts = 0;
	fsz_t fsz;
	struct REMFILE *filelist = NULL;
	struct REMFILE rem;

	ftp_connect();
	FtpClearCallback(conn);
	if ((action == FTP_SEND) || (action == FTP_GET) || (action == FTP_RM))
	{
		if (action == FTP_SEND)
		{

			if (!FtpMkdir(fnm, conn))
				iot_debug_print("[coreTest-False-ftp]: mkdir %s failed\n%s", fnm, FtpLastResponse(conn));
			else if (ftplib_debug)
				iot_debug_print("[coreTest-ftp]: Directory %s created\n", fnm);
			return;
		}
		else
		{
			if (!wildcard)
			{
				struct REMFILE *f;
				f = (struct REMFILE *)malloc(sizeof(struct REMFILE));
				memset(f, 0, sizeof(struct REMFILE));
				f->next = filelist;
				filelist = f;
				f->fnm = strdup(fnm);
			}
			else
			{
				netbuf *dir;
				char *buf;
				if (!FtpAccess(fnm, FTPLIB_DIR, FTPLIB_ASCII, conn, &dir))
				{
					iot_debug_print("[coreTest-False-ftp]: error requesting directory of %s\n%s\n",fnm, FtpLastResponse(conn));
					return;
				}
				buf = malloc(DIRBUF_SIZE);
				while (FtpRead(buf, DIRBUF_SIZE, dir) > 0)
				{
					struct REMFILE *f;
					char *p;
					f = (struct REMFILE *)malloc(sizeof(struct REMFILE));
					memset(f, 0, sizeof(struct REMFILE));
					f->next = filelist;
					p = strchr(buf, '\n');
					if (p)
						*p = '\0';
					f->fnm = strdup(buf);
					filelist = f;
				}
				free(buf);
				FtpClose(dir);
			}
		}
	}
	switch (action)
	{
	case FTP_DIR:
		sts = FtpDir(NULL, fnm, conn);
		break;
	case FTP_LIST:
		sts = FtpNlst(NULL, fnm, conn);
		break;
	case FTP_SEND:
		rem.next = NULL;
		rem.fnm = fnm;
		rem.fsz = fsz;
		fsz /= 100;
		if (fsz > 100000)
			fsz = 100000;
		if (ftplib_debug && fsz)
		{
			FtpCallbackOptions opt;
			opt.cbFunc = log_progress;
			opt.cbArg = &rem;
			opt.idleTime = 1000;
			opt.bytesXferred = fsz;
			FtpSetCallback(&opt, conn);
		}
		sts = FtpPut(fnm, fnm, mode, conn);
		if (ftplib_debug && sts)
			iot_debug_print("[coreTest-ftp]: %s sent\n", fnm);
		break;
	case FTP_GET:
		while (filelist)
		{
			struct REMFILE *f = filelist;
			filelist = f->next;
#if defined(__UINT64_MAX)
			if (!FtpSizeLong(f->fnm, &fsz, mode, conn))
#else
			if (!FtpSize(f->fnm, (unsigned int *)&fsz, mode, conn))
#endif
				fsz = 0;
			f->fsz = fsz;
			fsz /= 100;
			if (fsz > 100000)
				fsz = 100000;
			if (fsz == 0)
				fsz = 32768;
			if (ftplib_debug)
			{
				FtpCallbackOptions opt;
				opt.cbFunc = log_progress;
				opt.cbArg = f;
				opt.idleTime = 1000;
				opt.bytesXferred = fsz;
				FtpSetCallback(&opt, conn);
			}
			sts = FtpGet(f->fnm, APP_FTP_GET_FILE, mode, conn);
			if (ftplib_debug && sts)
				iot_debug_print("[coreTest-ftp]: %s retrieved\n", f->fnm);
			free(f->fnm);
			free(f);
		}
		break;
	case FTP_RM:
		while (filelist)
		{
			struct REMFILE *f = filelist;
			filelist = f->next;
			sts = FtpDelete(f->fnm, conn);
			if (ftplib_debug && sts)
				iot_debug_print("[coreTest-ftp]: %s deleted\n", f->fnm);
			free(f->fnm);
			free(f);
		}
		break;
	}
	if (!sts)
		iot_debug_print("[coreTest-False-ftp]: error\n%s\n", FtpLastResponse(conn));
	return;
}

void ftp_main()
{
	action = FTP_GET;
	FtpInit();
	{
		ftp_connect();
		if ((action == FTP_DIR) || (action == FTP_LIST))
			process_file(NULL);
		else
			process_file(APP_FTP_GET_FILE);
	}
	if (conn)
		FtpClose(conn);
}

bool ftpTest(void)
{
	extern bool networkstatus;
	if (networkstatus == FALSE)
		return FALSE;
	ftp_main();
	iot_fs_delete_file(APP_FTP_GET_FILE);
}
