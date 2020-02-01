#ifndef _Mikabox_input_h_
#define _Mikabox_input_h_

#define BTN_U     (1 << 0)
#define BTN_D     (1 << 1)
#define BTN_L     (1 << 2)
#define BTN_R     (1 << 3)
#define BTN_A     (1 << 4)
#define BTN_B     (1 << 5)
#define BTN_X     (1 << 6)
#define BTN_Y     (1 << 7)
#define BTN_L1    (1 << 8)
#define BTN_R1    (1 << 9)
#define BTN_L2    (1 << 10)
#define BTN_R2    (1 << 11)
#define BTN_L3    (1 << 12)
#define BTN_R3    (1 << 13)
#define BTN_START (1 << 14)
#define BTN_OPTN  (1 << 15)
#define BTN_META  (1 << 16)
#define BTN_AUX   (1 << 17)

#define BTN_CRO   BTN_A
#define BTN_CIR   BTN_B
#define BTN_SQR   BTN_X
#define BTN_TRI   BTN_Y

#define BTN_BIT(_name, _val, _bit)  (((_val) & (1 << (_bit))) ? BTN_##_name : 0)

#endif
