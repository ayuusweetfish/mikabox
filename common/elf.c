#include "elf.h"
#include <stddef.h>
#include <stdlib.h>

// Reference document
// http://infocenter.arm.com/help/topic/com.arm.doc.ihi0044f/IHI0044F_aaelf.pdf

// ELF headers start from p. 17

static uint8_t check_ehdr(const elf_ehdr *ehdr, elf_addr *entry)
{
  if (ehdr->ident[0] != 0x7f ||
    ehdr->ident[1] != 'E' ||
    ehdr->ident[2] != 'L' ||
    ehdr->ident[3] != 'F')
  {
    return ELF_E_INVALID;
  }
  if (ehdr->ident[4] != 1 ||    // class 32-bit ELFCLASS32
    ehdr->ident[5] != 1 ||      // data LSB     ELFDATA2LSB
    ehdr->ident[6] != 1 ||      // ELF version  EV_CURRENT
    ehdr->type != 2 ||          // type         ET_EXEC
    ehdr->machine != 40 ||      // machine      EM_ARM
    (ehdr->flags >> 24) != 5 || // ABI version  EF_ARM_ABIMASK
    !(ehdr->flags & 0x400))     // hard float   EF_ARM_ABI_FLOAT_HARD
  {
    return ELF_E_UNSUPPORT;
  }

  *entry = ehdr->entry;

  return ELF_E_NONE;
}

uint8_t elf_load(elf_fread *reader, elf_alloc *alloc, void *user, elf_addr *entry)
{
  elf_ehdr ehdr;
  reader(user, &ehdr, 0, sizeof(elf_ehdr));

  uint8_t ehdr_result = check_ehdr(&ehdr, entry);
  if (ehdr_result != ELF_E_NONE) return ehdr_result;

#ifdef ELF_STRTAB
  elf_shdr strtabhdr;
  reader(user, &strtabhdr,
    ehdr.shoffs + ehdr.shstrndx * sizeof(elf_shdr), sizeof(elf_shdr));

  char *strtab = (char *)malloc(strtabhdr.size);
  if (strtab == NULL) return ELF_E_MEM;
  reader(user, strtab, strtabhdr.offs, strtabhdr.size);
#endif

  elf_shdr shdr;

  ELF_LOG("%-16s %8s %3s %8s %6s %6s %6s\n",
    "section", "type", "flg", "addr", "aln", "offset", "size");
  for (uint32_t i = 0; i < ehdr.shnum; i++) {
    reader(user, &shdr, ehdr.shoffs + i * sizeof(elf_shdr), sizeof(elf_shdr));
    ELF_LOG("%-16s %8x %c%c%c %8x %6x %6x %6x\n",
#ifdef ELF_STRTAB
      strtab + shdr.name,
#else
      "<unknown>",
#endif
      shdr.type,
      (shdr.flags & 1) ? 'W' : '.',
      (shdr.flags & 2) ? 'A' : '.',
      (shdr.flags & 4) ? 'X' : '.',
      shdr.addr, shdr.addralign, shdr.offs, shdr.size);
  }

  elf_phdr phdr;

  ELF_LOG("\n");
  ELF_LOG("%8s %6s %8s %8s %6s %6s %3s %6s\n",
    "progtype", "offset", "vaddr", "paddr", "filesz", "memsz", "flg", "aln");
  for (uint32_t i = 0; i < ehdr.phnum; i++) {
    reader(user, &phdr, ehdr.phoffs + i * sizeof(elf_phdr), sizeof(elf_phdr));
    ELF_LOG("%8x %6x %8x %8x %6x %6x %c%c%c %6x\n",
      phdr.type, phdr.offs,
      phdr.vaddr, phdr.paddr,
      phdr.filesz, phdr.memsz,
      (phdr.flags & 4) ? 'R' : '.',
      (phdr.flags & 2) ? 'W' : '.',
      (phdr.flags & 1) ? 'X' : '.',
      phdr.align);
    void *p = alloc(phdr.vaddr, phdr.memsz, phdr.flags);
    reader(user, p, phdr.offs, phdr.filesz);
  }

  return ELF_E_NONE;
}


#ifdef ELF_TEST
#include <stdio.h>  // Ahhhhhhh~~~!!!
#include <stdlib.h>
#include <string.h>

#include "elf.h"

void *p80000000 = NULL;

void *alloc(elf_word vaddr, elf_word memsz, elf_word flags)
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
  uint8_t ret = elf_load(buf_get, alloc, buf, &entry);
  printf("load_elf returns %d\n", ret);
  if (ret != ELF_E_NONE) return 4;

  printf("Entry 0x%08x\n", entry);
  if (p80000000 == NULL) return 5;

  uint32_t *p = p80000000;
  for (int i = 0; i < 8; i++) printf("%08x\n", p[i]);

  return 0;
}
#endif
