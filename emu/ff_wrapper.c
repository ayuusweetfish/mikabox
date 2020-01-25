#include "ff_wrapper.h"
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void *syscall_dup_mem(uint32_t addr, uint32_t size);
void *syscall_dup_str(uint32_t addr);
void syscall_write_mem(uint32_t addr, uint32_t size, void *buf);

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
  const char *flags;
  switch (mode) {
    case FA_READ: flags = "r"; break;
    case FA_READ | FA_WRITE: flags = "r+"; break;
    case FA_CREATE_ALWAYS | FA_WRITE: flags = "w"; break;
    case FA_CREATE_ALWAYS | FA_WRITE | FA_READ: flags = "w+"; break;
    case FA_OPEN_APPEND | FA_WRITE: flags = "a"; break;
    case FA_OPEN_APPEND | FA_WRITE | FA_READ: flags = "a+"; break;
    case FA_CREATE_NEW | FA_WRITE: flags = "wx"; break;
    case FA_CREATE_NEW | FA_WRITE | FA_READ: flags = "w+x"; break;
    default:
      printf("Invalid mode argument: %u\n", (unsigned)mode);
      flags = "a+";
  }
  fp->f = fopen(p, flags);
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
  void *p = malloc(btr);
  *br = fread(p, 1, btr, fp->f);
  syscall_write_mem(buff, btr, p);
  free(p);
  return ferror(fp->f);
}

FRESULT f_write (FIL* fp, guestptr_t buff, UINT btw, UINT* bw)
{
  void *p = syscall_dup_mem(buff, btw);
  *bw = fwrite(p, 1, btw, fp->f);
  free(p);
  return ferror(fp->f);
}

FRESULT f_lseek (FIL* fp, FSIZE_t ofs)
{
  if (fseek(fp->f, ofs, SEEK_SET) == -1) return errno;
  return 0;
}

FRESULT f_truncate (FIL* fp)
{
  if (fflush(fp->f) == -1) return errno;
  if (ftruncate(fileno(fp->f), ftello(fp->f)) == -1) return errno;
  return 0;
}

FRESULT f_sync (FIL* fp)
{
  if (fflush(fp->f) == -1) return errno;
  return 0;
}

FRESULT f_opendir (DIR_* dp, guestptr_t path)
{
  char *p = gen_path(path);
  dp->d = opendir(p);
  free(p);
  if (dp->d == NULL) return errno;
  return 0;
}

FRESULT f_closedir (DIR_* dp)
{
  if (closedir(dp->d) != 0) return errno;
  return 0;
}

FRESULT f_readdir (DIR_* dp, FILINFO* fno)
{
  struct dirent *ent;
  while (1) {
    errno = 0;
    if ((ent = readdir(dp->d)) == NULL) {
      fno->fname[0] = '\0';
      return errno;
    }
    if (strcmp(ent->d_name, ".") == 0 ||
      strcmp(ent->d_name, "..") == 0) continue;
    // Set fname and fattrib (AM_DIR)
    fno->fattrib = ((ent->d_type == DT_DIR) ? AM_DIR : 0);
    strlcpy(fno->fname, ent->d_name, sizeof fno->fname);
    break;
  }
  return 0;
}

FRESULT f_mkdir (guestptr_t path)
{
  char *p = gen_path(path);
  if (mkdir(p, 0777) == -1 && errno != EEXIST) {
    free(p);
    return errno;
  }
  free(p);
  return 0;
}

FRESULT f_unlink (guestptr_t path)
{
  char *p = gen_path(path);
  if (unlink(p) == -1 && rmdir(p) == -1 && errno != ENOENT) {
    free(p);
    return errno;
  }
  free(p);
  return 0;
}

FRESULT f_rename (guestptr_t path_old, guestptr_t path_new)
{
  char *p1 = gen_path(path_old);
  char *p2 = gen_path(path_new);
  if (rename(p1, p2) == -1) {
    free(p1); free(p2);
    return errno;
  }
  free(p1); free(p2);
  return 0;
}

FRESULT f_stat (guestptr_t path, FILINFO* fno)
{
  char *p = gen_path(path);
  struct stat s;
  if (stat(p, &s) == -1) {
    free(p);
    return errno;
  }
  fno->fattrib = (S_ISDIR(s.st_mode) ? AM_DIR : 0);
  return 0;
}

int f_eof(FIL* fp)
{
  return feof(fp->f);
}

int f_error(FIL* fp)
{
  return ferror(fp->f);
}

FSIZE_t f_tell(FIL* fp)
{
  return ftell(fp->f);
}

FSIZE_t f_size(FIL* fp)
{
  long p = ftell(fp->f);
  if (fseek(fp->f, 0, SEEK_END) == -1) {
    printf("fseek() returned error: %d (%s)\n", errno, strerror(errno));
    return -1;
  }
  long sz = ftell(fp->f);
  if (fseek(fp->f, p, SEEK_SET) == -1) {
    printf("fseek() returned error: %d (%s)\n", errno, strerror(errno));
    return -1;
  }
  return sz;
}

const char *f_strerr(FRESULT result)
{
  return strerror(result);
}
