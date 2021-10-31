#ifndef _KSCAN_H
#define _KSCAN_H 1

// Address to set the scan mode (see KSCAN_MODE_xxx defines)
#define KSCAN_MODE	*((volatile unsigned char*)0x8374)

// Address to read back the detected key. 0xFF if no key was pressed.
#define KSCAN_KEY	*((volatile unsigned char*)0x8375)

// Address to read back the joystick X axis (scan modes 1 and 2 only)
#define KSCAN_JOYY	*((volatile unsigned char*)0x8376)

// Address to read back the joystick Y axis (scan modes 1 and 2 only)
#define KSCAN_JOYX	*((volatile unsigned char*)0x8377)

// Address to check the status byte. KSCAN_MASK is set if a key was pressed
#define KSCAN_STATUS *((volatile unsigned char*)0x837c)
// STATUS mask for new key press (vs held key)
#define KSCAN_MASK	0x20

//*********************
// KSCAN modes
//*********************

#define KSCAN_MODE_LAST		0		// last mode scanned
#define KSCAN_MODE_LEFT		1		// left side of keyboard and joystick 1 (fire is a key of 18)
#define KSCAN_MODE_RIGHT	2		// right side of keyboard and joystick 2 (fire is a key of 18)
#define KSCAN_MODE_994		3		// upper-case only, 99/4 compatible results
#define KSCAN_MODE_PASCAL	4		// PASCAL mapping, different control keys
#define KSCAN_MODE_BASIC	5		// Normal 99/4A BASIC mode


//*********************
// Joystick return values
//*********************

#define JOY_LEFT	0xfc
#define JOY_RIGHT	0x04
#define JOY_UP		0x04
#define JOY_DOWN	0xfc

// BASIC mode keyboard codes
#define KEY_CTRL_A 129
#define KEY_CTRL_B 130
#define KEY_CTRL_C 131
#define KEY_CTRL_D 132
#define KEY_CTRL_E 133
#define KEY_CTRL_F 134
#define KEY_CTRL_G 135
#define KEY_CTRL_H 136
#define KEY_CTRL_I 137
#define KEY_CTRL_J 138
#define KEY_CTRL_K 139
#define KEY_CTRL_L 140
#define KEY_CTRL_M 141
#define KEY_CTRL_N 142
#define KEY_CTRL_O 143
#define KEY_CTRL_P 144
#define KEY_CTRL_Q 145
#define KEY_CTRL_R 146
#define KEY_CTRL_S 147
#define KEY_CTRL_T 148
#define KEY_CTRL_U 149
#define KEY_CTRL_V 150
#define KEY_CTRL_W 151
#define KEY_CTRL_X 152
#define KEY_CTRL_Y 153
#define KEY_CTRL_Z 154
#define KEY_LEFT 8
#define KEY_RIGHT 9
#define KEY_DOWN 10
#define KEY_UP 11
#define KEY_DELETE 3
#define KEY_INSERT 4
#define KEY_ERASE 7
#define KEY_BEGIN 14
#define KEY_PROCD 12
#define KEY_AID 1
#define KEY_REDO 6
#define KEY_BACK 15
#define KEY_SPACE 0x20
#define KEY_TILDE 0x7E
#define KEY_ENTER 13
#define KEY_QUIT KEY_CTRL_Q
#define KEY_TAB KEY_CTRL_I
#define KEY_ESC KEY_BACK


#endif
