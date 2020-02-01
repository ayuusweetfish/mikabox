#ifndef _Mikabox_ff_wrapper_h_
#define _Mikabox_ff_wrapper_h_

#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>

extern const char *fs_root;

typedef int FRESULT;
typedef char TCHAR;
typedef uint32_t UINT;
typedef uint8_t BYTE;
typedef long FSIZE_t;
typedef uint32_t guestptr_t;

#define FR_OK           0
#define FR_NO_FILE      ENOENT
#define FR_NO_PATH      ENOTDIR
#define FR_INVALID_NAME ENOENT

#define AM_DIR  1

#define	FA_READ				0x01
#define	FA_WRITE			0x02
#define	FA_OPEN_EXISTING	0x00
#define	FA_CREATE_NEW		0x04
#define	FA_CREATE_ALWAYS	0x08
#define	FA_OPEN_ALWAYS		0x10
#define	FA_OPEN_APPEND		0x30

typedef struct {
  FILE *f;
} FIL;

typedef struct {
  DIR *d;
} DIR_;

typedef struct {
  BYTE fattrib;
  TCHAR fname[257];
} FILINFO;

FRESULT f_open (FIL* fp, guestptr_t path, BYTE mode);
FRESULT f_close (FIL* fp);
FRESULT f_read (FIL* fp, guestptr_t buff, UINT btr, UINT* br);
FRESULT f_write (FIL* fp, guestptr_t buff, UINT btw, UINT* bw);
FRESULT f_lseek (FIL* fp, FSIZE_t ofs);
FRESULT f_truncate (FIL* fp);
FRESULT f_sync (FIL* fp);
FRESULT f_opendir (DIR_* dp, guestptr_t path);
FRESULT f_closedir (DIR_* dp);
FRESULT f_readdir (DIR_* dp, FILINFO* fno);
FRESULT f_mkdir (guestptr_t path);
FRESULT f_unlink (guestptr_t path);
FRESULT f_rename (guestptr_t path_old, guestptr_t path_new);
FRESULT f_stat (guestptr_t path, FILINFO* fno);

int f_eof(FIL* fp);
int f_error(FIL* fp);
FSIZE_t f_tell(FIL* fp);
FSIZE_t f_size(FIL* fp);

const char *f_strerr(FRESULT result);

#endif
