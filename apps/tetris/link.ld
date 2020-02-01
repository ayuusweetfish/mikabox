ENTRY(main)

SECTIONS
{
  . = 0x40000000;

  /DISCARD/ : { *(.ARM.* .debug_frame) }

  .text ALIGN(1048576) : { *(.text .text.*) }
  .rodata ALIGN(1048576) : { *(.rodata .rodata.*) }
  .data ALIGN(1048576) : { *(.data .data.*) }
  . = ALIGN(0x8);
  _bss_begin = .;
  .bss : { *(.bss) }
  _bss_end = .;
}
