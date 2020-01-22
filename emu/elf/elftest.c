#include <stdio.h>  // Ahhhhhhh~~~!!!
#include <stdlib.h>
#include <string.h>

#include "elf.h"

void *p80000000 = NULL;

void *elf_alloc(elf_word vaddr, elf_word memsz, elf_word flags)
{
  void *p = malloc(memsz);
  if (vaddr == 0x80000000) p80000000 = p;
  return p;
}

void buf_get(void *from, void *to, uint32_t offs, uint32_t len)
{
  memcpy(to, (uint8_t *)from + offs, len);
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    printf("Usage: %s <file>\n", argv[0]);
    return 0;
  }

  FILE *f = fopen(argv[1], "r");
  if (!f) {
    printf("Cannot open file %s\n", argv[1]);
    return 1;
  }

  fseek(f, 0, SEEK_END);
  long len = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *buf = (char *)malloc(len);
  if (!buf) {
    printf("Cannot allocate memory for reading the entire file\n");
    return 2;
  }
  if (fread(buf, len, 1, f) != 1) {
    printf("Cannot read from file %s\n", argv[1]);
    return 3;
  }
  fclose(f);

  printf("File size %lu B\n", len);
  elf_addr entry;
  uint8_t ret = elf_load(buf_get, buf, &entry);
  printf("load_elf returns %d\n", ret);
  if (ret != ELF_E_NONE) return 4;

  printf("Entry 0x%08x\n", entry);
  if (p80000000 == NULL) return 5;

  uint32_t *p = p80000000;
  for (int i = 0; i < 8; i++) printf("%08x\n", p[i]);

  return 0;
}
