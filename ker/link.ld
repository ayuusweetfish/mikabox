SECTIONS
{
  . = 0x8000;
  PROVIDE(__executable_start = 0x8000);

  .text.startup : { *(.text.startup) }
  .text : { EXCLUDE_FILE(*user.o) *(.text .text.*) }
  .rodata : { *(.rodata .rodata.*) }
  .data : { *(.data .data.*) }

  . = ALIGN(0x8);
  _bss_begin = .;
  .bss : { *(.bss) }
  _bss_end = .;

  . = ALIGN(0x100000);
  _bss_ord_begin = .;
  .bss.ord : { *(.bss.ord) }
  _bss_ord_end = .;

  . = ALIGN(0x100000);
  _text_user_begin = .;
  .text.user : { *user.o(.text) }
  _text_user_end = .;
}
