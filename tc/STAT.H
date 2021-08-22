/*	stat.h

	Definitions used for file status/directory functions.  

	Copyright (c) Borland International 1987
	All Rights Reserved.
*/
#ifdef __STDC__
#define _Cdecl
#else
#define _Cdecl	cdecl
#endif

#ifndef _STAT_H
#define _STAT_H 1

#define S_IFMT		0xF000		/* file type mask */
#define S_IFDIR		0x4000		/* directory */
#define S_IFIFO		0x1000		/* FIFO special */
#define S_IFCHR		0x2000		/* character special */
#define S_IFBLK		0x3000		/* block special */
#define S_IFREG		0x8000		/* or just 0x0000, regular */
#define S_IREAD		0x0100		/* owner may read */
#define S_IWRITE	0x0080		/* owner may write */
#define S_IEXEC		0x0040		/* owner may execute <directory search> */

struct	stat
{
    short   st_dev;
    short   st_ino;
    short   st_mode;
    short   st_nlink;
    int     st_uid;
    int     st_gid;
    short   st_rdev;
    long    st_size;
    long    st_atime;
    long    st_mtime;
    long    st_ctime;
};

int  fstat (int fildes, struct stat *statbuf);
int  stat  (char *path, struct stat *statbuf);

#endif	/* _STAT_H */
