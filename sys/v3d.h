#ifndef _Mikabox_v3d_h_
#define _Mikabox_v3d_h_

#define V3D_IDENT0  (volatile uint32_t *)(PERI_BASE + 0xc00000 + 0x000)

void v3d_init();

#endif
