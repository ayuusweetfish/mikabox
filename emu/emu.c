#include "elf/elf.h"
#include <unicorn/unicorn.h>
#include <stdio.h>

static inline uint32_t align(uint32_t addr, uint32_t align)
{
  return (addr + align - 1) & ~(align - 1);
}

static uc_engine *uc;

void *elf_alloc(elf_word vaddr, elf_word memsz, elf_word flags)
{
  return malloc(memsz);
}

void elf_alloc_post(elf_word vaddr, elf_word memsz, elf_word flags, void *buf)
{
  uint32_t perms = 0;
  if (flags & 4) perms |= UC_PROT_READ;
  if (flags & 2) perms |= UC_PROT_WRITE;
  if (flags & 1) perms |= UC_PROT_EXEC;

  uc_err err;
  if ((err = uc_mem_map(uc, vaddr, align(memsz, 0x100000), perms)) != UC_ERR_OK) {
    printf("uc_mem_map() returned error %u (%s)\n", err, uc_strerror(err));
    exit(1);
  }
  if ((err = uc_mem_write(uc, vaddr, buf, memsz)) != UC_ERR_OK) {
    printf("uc_mem_write() returned error %u (%s)\n", err, uc_strerror(err));
    exit(1);
  }

  free(buf);
  printf("Mapped memory region 0x%08x, size %6d, perms %c%c%c\n",
    (uint32_t)vaddr, (uint32_t)memsz,
    (perms & UC_PROT_READ) ? 'R' : '.',
    (perms & UC_PROT_WRITE) ? 'W' : '.',
    (perms & UC_PROT_EXEC) ? 'X' : '.');
}

static void fp_get(void *user, void *dest, uint32_t offs, uint32_t len)
{
  FILE *fp = user;
  fseek(fp, offs, SEEK_SET);
  fread(dest, len, 1, user);
}

static inline const char *mem_type_str(uc_mem_type t)
{
  switch (t) {
    case UC_MEM_READ: return "read";
    case UC_MEM_WRITE: return "write";
    case UC_MEM_FETCH: return "fetch";
    case UC_MEM_READ_UNMAPPED: return "read unmapped";
    case UC_MEM_WRITE_UNMAPPED: return "write unmapped";
    case UC_MEM_FETCH_UNMAPPED: return "fetch unmapped";
    case UC_MEM_READ_PROT: return "read protected";
    case UC_MEM_WRITE_PROT: return "write protected";
    case UC_MEM_FETCH_PROT: return "fetch protected";
    default: return "--";
  }
}

static void handler_unmapped(
  uc_engine *uc, uc_mem_type type,
  uint64_t address, int size, int64_t value, void *user_data)
{
  if (type == UC_MEM_FETCH_UNMAPPED && address == 0) {
    printf("Done!\n");
    return;
  }
  printf("Invalid memory access 0x%08x (type = %s, value = 0x%08x)\n",
    (uint32_t)address, mem_type_str(type), (uint32_t)value);
}

static void handler_syscall(
  uc_engine *uc, uint32_t intno, void *user_data)
{
  uint32_t r0, r1;
  uc_reg_read(uc, UC_ARM_REG_R0, &r0);
  uc_reg_read(uc, UC_ARM_REG_R1, &r1);
  printf("Syscall #%u (r0 = 0x%08x, r1 = 0x%08x)\n", intno, r0, r1);

  char ch;
  uc_err err;
  while (1) {
    if ((err = uc_mem_read(uc, r0++, &ch, 1)) != UC_ERR_OK) {
      printf("uc_mem_read() returned error %u (%s)\n", err, uc_strerror(err));
      uc_emu_stop(uc);
      return;
    }
    if (ch == '\0') break;
    else putchar(ch);
  }
  putchar('\n');
}

void emu()
{
  uc_err err;

  // Initialize Unicorn
  if ((err = uc_open(UC_ARCH_ARM, UC_MODE_ARM, &uc)) != UC_ERR_OK) {
    printf("uc_open() returned error %u (%s)\n", err, uc_strerror(err));
    return;
  }

  // Parse and load ELF
  FILE *fp = fopen("user/a.out", "r");
  if (fp == NULL) {
    printf("Unable to open file\n");
    return;
  }
  elf_addr entry;
  uint8_t elfret = elf_load(fp_get, fp, &entry);
  fclose(fp);
  if (elfret != 0) {
    printf("elf_load() returned error %u\n", elfret);
    return;
  }

  // Add hooks
  uc_hook hook_mem, hook_syscall;
  uc_hook_add(uc, &hook_mem, UC_HOOK_MEM_INVALID, handler_unmapped, NULL, 1, 0);
  uc_hook_add(uc, &hook_syscall, UC_HOOK_INTR, handler_syscall, NULL, 1, 0);

  // Initialize stack pointer
  uint32_t initial_sp = 0x80120000;
  uint32_t initial_lr = 0x0;
  uc_reg_write(uc, UC_ARM_REG_SP, &initial_sp);
  uc_reg_write(uc, UC_ARM_REG_LR, &initial_lr);

  // Execute
  printf("Entry 0x%08x\n", entry);
  if ((err = uc_emu_start(uc, entry, 0, 1000000, 0)) != UC_ERR_OK) {
    printf("uc_emu_start() returned error %u (%s)\n", err, uc_strerror(err));
    return;
  }
}

int main()
{
  emu();
  return 0;
}
