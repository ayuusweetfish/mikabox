#include "elf.h"
#include <stddef.h>

// Reference document
// http://infocenter.arm.com/help/topic/com.arm.doc.ihi0044f/IHI0044F_aaelf.pdf

// ELF headers start from p. 17

static uint8_t check_ehdr(const elf_ehdr *ehdr)
{
  if (ehdr->ident[0] != 0x7f ||
    ehdr->ident[1] != 'E' ||
    ehdr->ident[2] != 'L' ||
    ehdr->ident[3] != 'F')
  {
    return ELF_E_INVALID;
  }
  if (ehdr->ident[4] != 1 ||    // class 32-bit ELFCLASS32
    ehdr->ident[5] != 1 ||      // data LSB   ELFDATA2LSB
    ehdr->ident[6] != 1 ||      // ELF version  EV_CURRENT
    ehdr->type != 2 ||          // type     ET_EXEC
    ehdr->machine != 40 ||      // machine    EM_ARM
    (ehdr->flags >> 24) != 5 || // ABI version  EF_ARM_ABIMASK
    !(ehdr->flags & 0x400))     // hard float   EF_ARM_ABI_FLOAT_HARD
  {
    return ELF_E_UNSUPPORT;
  }

  ELF_LOG("entry 0x%x\n", ehdr->entry);

  return ELF_E_NONE;
}

static inline const elf_shdr *get_shdr(const elf_ehdr *ehdr)
{
  return (const elf_shdr *)((const char *)ehdr + ehdr->shoffs);
}

static inline const elf_phdr *get_phdr(const elf_ehdr *ehdr)
{
  return (const elf_phdr *)((const char *)ehdr + ehdr->phoffs);
}

static inline const char *get_strtab(const elf_ehdr *ehdr)
{
  return (ehdr->shstrndx == 0) ? NULL :
    (const char *)ehdr + get_shdr(ehdr)[ehdr->shstrndx].offs;
}

static inline const char *get_strtab_ent(const elf_ehdr *ehdr, uint32_t offs)
{
  const char *table = get_strtab(ehdr);
  return table ? table + offs : NULL;
}

uint8_t load_elf(const char *buf)
{
  const elf_ehdr *ehdr = (elf_ehdr *)buf;

  uint8_t ehdr_result = check_ehdr(ehdr);
  if (ehdr_result != ELF_E_NONE) return ehdr_result;

  printf("%-16s %8s %3s %8s %6s %6s %6s\n",
    "section", "type", "flg", "addr", "aln", "offset", "size");
  const elf_shdr *shdr = get_shdr(ehdr);
  for (uint32_t i = 0; i < ehdr->shnum; i++) {
    const elf_shdr *section = shdr + i;
    ELF_LOG("%-16s %8x %c%c%c %8x %6x %6x %6x\n",
      get_strtab_ent(ehdr, section->name), section->type,
      (section->flags & 1) ? 'W' : '.',
      (section->flags & 2) ? 'A' : '.',
      (section->flags & 4) ? 'X' : '.',
      section->addr, section->addralign, section->offs, section->size);
  }

  printf("\n");
  printf("%8s %6s %8s %8s %6s %6s %3s %6s\n",
    "progtype", "offset", "vaddr", "paddr", "filesz", "memsz", "flg", "aln");
  const elf_phdr *phdr = get_phdr(ehdr);
  for (uint32_t i = 0; i < ehdr->phnum; i++) {
    const elf_phdr *program = phdr + i;
    ELF_LOG("%8x %6x %8x %8x %6x %6x %c%c%c %6x\n",
      program->type, program->offs,
      program->vaddr, program->paddr,
      program->filesz, program->memsz,
      (program->flags & 4) ? 'R' : ' ',
      (program->flags & 2) ? 'W' : ' ',
      (program->flags & 1) ? 'X' : ' ',
      program->align);
    load_program(ehdr, program);
  }

  return ELF_E_NONE;
}
