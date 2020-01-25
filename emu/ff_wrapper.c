#include "ff_wrapper.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *syscall_dup_mem(uint32_t addr, uint32_t size);
void *syscall_dup_str(uint32_t addr);

static inline char *gen_path(uint32_t addr)
{
  char *p = syscall_dup_str(addr);
  const char *q = "build/";
  char *r = malloc(strlen(p) + strlen(q) + 1);
  strcpy(r, q);
  strcat(r, p);
  free(p);
  return r;
}

FRESULT f_open (FIL* fp, guestptr_t path, BYTE mode)
{
  char *p = gen_path(path);
  fp->f = fopen(p, "w");
  free(p);
  if (fp->f == NULL) return errno;
  return 0;
}

FRESULT f_close (FIL* fp)
{
  if (fclose(fp->f) != 0) return errno;
  return 0;
}

FRESULT f_read (FIL* fp, guestptr_t buff, UINT btr, UINT* br)
{
}

FRESULT f_write (FIL* fp, guestptr_t buff, UINT btw, UINT* bw)
{
}

FRESULT f_lseek (FIL* fp, FSIZE_t ofs)
{
}

FRESULT f_truncate (FIL* fp)
{
}

FRESULT f_sync (FIL* fp)
{
}

FRESULT f_opendir (DIR_* dp, guestptr_t path)
{
}

FRESULT f_closedir (DIR_* dp)
{
}

FRESULT f_readdir (DIR_* dp, FILINFO* fno)
{
}

FRESULT f_mkdir (guestptr_t path)
{
}

FRESULT f_unlink (guestptr_t path)
{
}

FRESULT f_rename (guestptr_t path_old, guestptr_t path_new)
{
}

FRESULT f_stat (guestptr_t path, FILINFO* fno)
{
}

int f_eof(FIL* fp)
{
}

int f_error(FIL* fp)
{
}

FSIZE_t f_tell(FIL* fp)
{
}

FSIZE_t f_size(FIL* fp)
{
}

const char *f_strerr(FRESULT result)
{
  return strerror(result);
}
