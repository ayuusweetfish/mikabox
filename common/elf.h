#ifndef __MIKAN__ELF_H__
#define __MIKAN__ELF_H__

#include <stdint.h>

typedef uint16_t elf_half;
typedef uint32_t elf_offs;
typedef uint32_t elf_addr;
typedef uint32_t elf_word;
typedef int32_t elf_sword;

typedef struct {
  uint8_t ident[16];
  elf_half type;
  elf_half machine;
  elf_word version;
  elf_addr entry;
  elf_offs phoffs;
  elf_offs shoffs;
  elf_word flags;
  elf_half ehsize;
  elf_half phentsize;
  elf_half phnum;
  elf_half shentsize;
  elf_half shnum;
  elf_half shstrndx;
} elf_ehdr;

typedef struct {
  elf_word name;
  elf_word type;
  elf_word flags;
  elf_addr addr;
  elf_offs offs;
  elf_word size;
  elf_word link;
  elf_word info;
  elf_word addralign;
  elf_word entsize;
} elf_shdr;

typedef struct {
  elf_word type;
  elf_offs offs;
  elf_addr vaddr;
  elf_addr paddr;
  elf_word filesz;
  elf_word memsz;
  elf_word flags;
  elf_word align;
} elf_phdr;

#define ELF_E_NONE      0
#define ELF_E_INVALID   1
#define ELF_E_UNSUPPORT 2
#define ELF_E_MEM       3

typedef void elf_fread(void *user, void *buf, uint32_t offs, uint32_t len);
typedef void *elf_alloc(elf_word vaddr, elf_word memsz, elf_word flags);
uint8_t elf_load(elf_fread *reader, elf_alloc *alloc, void *user, elf_addr *entry);

#ifdef ELF_TEST
#include <stdio.h>
#define ELF_LOG printf
#else
#define ELF_LOG(...)
#endif

#endif
