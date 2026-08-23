#ifndef _FC_API_H
#define _FC_API_H 1

/*
 * Expected signature of main function 
 * non-zero return indicates error to ForceCommand.
 *   This may trigger additional environment restoration upon return.
 * command line parameters are passed as a single un-parsed buffer.
 */
int main(char* args);

#define exit(x) fc_exit((x))

#define FC_SYS *(int *)0x2000

#define DECL_FC_API_CALL(index, func, return_type, arg_sig, args)     \
    static inline return_type func arg_sig                            \
    {                                                                 \
        __asm__("li r0,%0 ; " #func                                   \
                :                                                     \
                : "i"(index));                                        \
        return_type(*tramp) arg_sig = (return_type(*) arg_sig)FC_SYS; \
        return tramp args;                                            \
    }

#define FC_SAMS_TRAMP_DATA *((int*)0x2002)
#define FC_SAMS_TRAMP *(int *)0x2004

/*
 * Use FC_SAMS_BANKED to declare (prototype) a non-void function that can be called 
 * with SAMS bank switching.
 * 
 * Define the function in the module (.c) with FC_SAMS(function_name) 
 */
#define FC_SAMS_BANKED(bank_id, return_type, function_name, param_signature, param_list)          \
    static inline return_type function_name param_signature                                       \
    {                                                                                             \
      return_type b##bank_id##_##function_name param_signature ;                                  \
      if (bank_id == SAMS_CURRENT_BANK) {                                                         \
        return b##bank_id##_##function_name param_list ;                                          \
      }                                                                                           \
      static const int fcsams_data_##function_name[] = {                                          \
        (int)bank_id,                                                                             \
        (int)SAMS_CURRENT_BANK,                                                                   \
        (int)b##bank_id##_##function_name                                                         \
      };                                                                                          \
      FC_SAMS_TRAMP_DATA = (int) fcsams_data_##function_name;                                     \
      return_type (*fcstramp) param_signature = (return_type (*) param_signature) FC_SAMS_TRAMP;  \
      return fcstramp param_list;                                                                 \
    }                                                                                             \

/*
 * Use FC_SAMS_VOIDBANKED to declare (prototype) a void function that can be called 
 * with SAMS bank switching.
 * 
 * Define the function in the module (.c) with FC_SAMS(function_name) 
 */
#define FC_SAMS_VOIDBANKED(bank_id, function_name, param_signature, param_list)          \
    static inline void function_name param_signature                                     \
    {                                                                                    \
      void b##bank_id##_##function_name param_signature ;                                \
      if (bank_id == SAMS_CURRENT_BANK) {                                                \
        b##bank_id##_##function_name param_list ;                                        \
        return;                                                                          \
      }                                                                                  \
      static const int fcsams_data_##function_name[] = {                                 \
        (int)bank_id,                                                                    \
        (int)SAMS_CURRENT_BANK,                                                          \
        (int)b##bank_id##_##function_name                                                \
      };                                                                                 \
      FC_SAMS_TRAMP_DATA = (int) fcsams_data_##function_name;                            \
      void (*fcstramp) param_signature = (void (*) param_signature) FC_SAMS_TRAMP;       \
      fcstramp param_list;                                                               \
    }                                                                                    \

/*
 * add bank identifier to function_name when defining for SAMS banking in module (.c)
 * files.
 */
#define FC_SAMS(bank_id, function_name) b##bank_id##_##function_name

/*
  Device Service Routine pre-loaded entry
*/
struct DeviceServiceRoutine {
    char name[8];
    int crubase;
    unsigned int addr;
    char unit;
    int cpuSup;
};

/*
  System information structure
*/
struct SystemInformation {
  struct DeviceServiceRoutine* dsrList; // points to array of 20 dsrs
  struct DeviceServiceRoutine* currentDsr;
  const char* currentPath;
  unsigned int vdp_io_buf;
};

// Values for vdp_type in DisplayInformation
#define VDP_F18A 0xF18A
#define VDP_9938 0x9938
#define VDP_9958 0x9958
#define VDP_9918 0x9918

// Screen mode constants for vdp_screenmode
// GRAPHICS: 32x24, 2 colors per 8 chars, sprites.
//   SIT >0000(768B) | SprAttr >0300(128B) | Color >0380(32B)
//   Pattern >0800(2048B, shared with sprite patterns)
#define VDP_SCREENMODE_GRAPHICS    0

// TEXT: 40x24, 2 colors global, no sprites.
//   SIT >0000(960B) | Pattern >0800(2048B)
//   Color table unused, sprite tables unused
#define VDP_SCREENMODE_TEXT        1

// TEXT80: 80x24/26, 2 colors global, no sprites. Requires 9938/9958/F18A.
//   SIT >0000(1920B) | Pattern >1000(2048B)
#define VDP_SCREENMODE_TEXT80      2

// TEXT80X30: 80x30, per-char colors, sprites. F18A only.
//   SIT >0000(2400B) | SprAttr >0A00(128B) | Pattern >1000(2048B)
//   Color-attrib >1800(2400B, one byte per screen pos)
#define VDP_SCREENMODE_TEXT80X30   3

// Color attribute values
#define COLOR_TRANS 0x00
#define COLOR_BLACK 0x01
#define COLOR_MEDGREEN 0x02
#define COLOR_LTGREEN 0x03
#define COLOR_DKBLUE 0x04
#define COLOR_LTBLUE 0x05
#define COLOR_DKRED 0x06
#define COLOR_CYAN 0x07
#define COLOR_MEDRED 0x08
#define COLOR_LTRED 0x09
#define COLOR_DKYELLOW 0x0A
#define COLOR_LTYELLOW 0x0B
#define COLOR_DKGREEN 0x0C
#define COLOR_MAGENTA 0x0D
#define COLOR_GRAY 0x0E
#define COLOR_WHITE 0x0F

/*
  Display parameter structure
*/
struct DisplayInformation {
  int isPal;
  int vdp_type;
  int displayWidth;
  int displayHeight;
  int imageAddr;
  int patternAddr;
  int colorAddr;
  int spritePatternAddr;
};

/* High-level graphics modes. */
#define GFX_MODE_GRAPHICS1       0
#define GFX_MODE_TEXT40          1
#define GFX_MODE_GRAPHICS2       2
#define GFX_MODE_BITMAP          GFX_MODE_GRAPHICS2
#define GFX_MODE_MULTICOLOR      3
#define GFX_MODE_GRAPHICS3       4
#define GFX_MODE_GRAPHICS4       5
#define GFX_MODE_GRAPHICS5       6
#define GFX_MODE_GRAPHICS6       7
#define GFX_MODE_GRAPHICS7       8
#define GFX_MODE_TEXT80          9
#define GFX_MODE_YJK_YAE         10
#define GFX_MODE_YJK_RGB         11
#define GFX_MODE_YJK             12
#define GFX_MODE_F18A_TEXT80X30  13

#define GFX_SCREEN_INTERLACED    0x0001
#define GFX_SPRITE_8X8           0x00
#define GFX_SPRITE_8X8_MAG       0x01
#define GFX_SPRITE_16X16         0x02
#define GFX_SPRITE_16X16_MAG     0x03

#define GFX_OK                   0
#define GFX_ERR_INVALID          -1
#define GFX_ERR_UNSUPPORTED      -2
#define GFX_ERR_WRONG_MODE       -3
#define GFX_ERR_RANGE            -4
#define GFX_ERR_BUSY             -5
#define GFX_ERR_NO_VRAM          -6

#define GFX_CAP_TEXT             0x0001
#define GFX_CAP_TILES            0x0002
#define GFX_CAP_PIXELS           0x0004
#define GFX_CAP_LINES            0x0008
#define GFX_CAP_CIRCLES          0x0010
#define GFX_CAP_PAINT            0x0020
#define GFX_CAP_COPY             0x0040
#define GFX_CAP_PAGES            0x0080
#define GFX_CAP_PALETTE          0x0100
#define GFX_CAP_SPRITES          0x0200
#define GFX_CAP_SPRITE_STATUS    0x0400
#define GFX_CAP_YJK              0x0800
#define GFX_CAP_ATTRIBUTES       0x1000

#define GFX_COLOR_INDEXED        0
#define GFX_COLOR_DIRECT         1
#define GFX_COLOR_YJK            2
#define GFX_COLOR_ATTRIBUTES     3

#define GFX_OP_PSET              0
#define GFX_OP_PRESET            1
#define GFX_OP_AND               2
#define GFX_OP_OR                3
#define GFX_OP_XOR               4
#define GFX_LINE_NORMAL          0x00
#define GFX_LINE_BOX             0x01
#define GFX_LINE_FILL            0x02
#define GFX_COLOR_DEFAULT        -1

#define GFX_RGB(r, g, b) \
    ((((r) & 0x0f) << 8) | (((g) & 0x0f) << 4) | ((b) & 0x0f))

struct GfxInformation {
    int vdp_type;
    int mode;
    int width;
    int height;
    int physical_width;
    int physical_height;
    int colors;
    int color_model;
    int sprite_count;
    int display_page;
    int draw_page;
    int page_count;
    int capabilities;
    unsigned int image_addr;
    unsigned int pattern_addr;
    unsigned int color_addr;
    unsigned int sprite_addr;
    unsigned int sprite_pattern_addr;
};

struct GfxSpriteStatus {
    int collision;
    int fifth_sprite;
    int overflow;
};

struct SamsInformation {
  int next_page;
  int total_pages;
};

/*
  Peripheral Access Block
*/
struct PAB {
    unsigned char OpCode;		// see DSR_xxx list above
    unsigned char Status;		// file type and error code (DSR_ERR_xxx and DSR_TYPE_xxx)
    unsigned int  VDPBuffer;		// address of the data buffer in VDP memory
    unsigned char RecordLength;		// size of records. Not used for PROGRAM type. >00 on open means autodetect
    unsigned char CharCount;		// number of bytes read or number of bytes to write
    unsigned int  RecordNumber;		// record number for normal files, available bytes (LOAD or SAVE) for PROGRAM type
    unsigned char ScreenOffset;		// Used in BASIC for screen BIAS. Also returns file status on Status call. (DSR_STATUS_xxx)
    unsigned char NameLength;		// for this implementation only, set to zero to read the length from the string
    char *pName;			// for this implementation only, must be a valid C String even if length is set
};

/*
  For direct input and direct file meta data is represented as AddInfo ( or additional information )
*/
struct AddInfo {
  unsigned int buffer;
  unsigned int first_sector;
  unsigned char flags;
  unsigned char recs_per_sec;
  unsigned char eof_offset;
  unsigned char rec_length;
  unsigned int records; // swizzled
};

/*
  File open mode types, default DISPLAY, FIXED, UPDATE, SEQUENTIAL
*/
#define DSR_TYPE_VARIABLE	0x10
#define DSR_TYPE_INTERNAL	0x08
#define DSR_TYPE_OUTPUT		0x02
#define DSR_TYPE_INPUT		0x04
#define DSR_TYPE_APPEND		0x06
#define DSR_TYPE_RELATIVE	0x01

/*
  catalog call back functions
*/
struct DirEntry {
  char name[11];
  int type;
  int sectors;
  int reclen;
  char ts_hour;
  char ts_min;
  char ts_second;
  int ts_year;
  char ts_month;
  char ts_day;
};

struct VolInfo {
  char volname[11];
  int total;
  int available;
  char timestamps;
  struct DeviceServiceRoutine* dsr;
};

typedef void (*vol_entry_cb)(struct VolInfo*);
typedef void (*dir_entry_cb)(struct DirEntry*);

/*
  clock DateTime structure
*/
struct DateTime {
  unsigned int year;
  unsigned char month;
  unsigned char day;
  unsigned char hours;
  unsigned char minutes;
  unsigned char seconds;
  unsigned char pm;
  unsigned char dayOfWeek;
};

/*
  fc_parse_path_param option constants
 */
#define PR_OPTIONAL 0x0000
#define PR_REQUIRED 0x0001
#define PR_WILDCARD 0x0002

/*
  response hook for terminal CSI commands that
  transmit back to server. For use fc_set_identify_hook.
  If flag is 1, send ANSI identify code, if 52, send vt52 code
 */
typedef void (*identify_callback)(int flag);

/*
  speech LPC code tracking cursor structure
 */
struct LpcPlaybackCtx {
    char* addr;
    int remaining;
};

/*
  TIPI Mouse return data and butten masks
 */
struct MouseData {
  char mx;
  char my;
  char buttons;
  int pointerx;
  int pointery;
};

#define MB_LEFT 0x01
#define MB_RIGHT 0x02
#define MB_MID 0x04

/*
  State for reentrant bufferedio socket api
 */
struct SocketBuffer {
  unsigned int socket_id;
  char buffer[256];
  int available;
  char lastline[256];
  int loaded;
  int tls;
};

#define TLS 1
#define TCP 0

/*
 Linked List stack that shoves oldest items off to make room if necessary.
 Use with fc_list_ functions.
 */
struct List {
  char* addr;
  char* end;
  char* ceiling;
};
  
struct ListEntry {
  int length;
  char data[];
};



/*
  Rom address tables
*/

// memcpy is special, it is in all bank of the cartridge
extern void* memcpy(void* dest, const void* src, int count);

/*
  TUI widget types
*/
enum tui_widget_type {
    TUI_WIDGET_BUTTON,
    TUI_WIDGET_LABEL,
    TUI_WIDGET_LIST,
    TUI_WIDGET_TEXTFIELD,
    TUI_WIDGET_CHECKBOX,
    TUI_WIDGET_PROGRESSBAR,
};

/*
  TUI event types
*/
enum tui_event_type {
    TUI_EV_NONE = 0,
    TUI_EV_KEY,
    TUI_EV_MOUSE_CLICK,
    TUI_EV_MOUSE_MOVE,
    TUI_EV_WIDGET_ACTIVATED,
};

/*
  TUI event structure
*/
struct tui_event {
    enum tui_event_type type;
    struct tui_widget* widget;
    int key;
    int mx;
    int my;
    int buttons;
};

/*
  TUI widget base structure
*/
struct tui_widget {
    enum tui_widget_type type;
    int x;
    int y;
    int w;
    int h;
    unsigned char flags;
    struct tui_window* parent;
    struct tui_widget* next;
};

/*
  TUI window structure
*/

#define TUI_BF_TOP    0x01
#define TUI_BF_BOTTOM 0x02
#define TUI_BF_SIDES  0x04
#define TUI_BF_ALL    0x07

struct tui_window {
    int x;
    int y;
    int w;
    int h;
    int cx;
    int cy;
    int cw;
    int ch;
    int fg;
    int bg;
    unsigned char border_flags;
    char title[41];
    struct tui_widget* children;
    struct tui_widget* focus;
    struct tui_window* next;
};

typedef struct tui_window tui_win_t;
typedef struct tui_widget tui_widget_t;
#define FC_AUDIO_BEEP 0x0
#define FC_AUDIO_HONK 0x1
#define FC_DSR_CATALOG 0x2
#define FC_DSR_CLOSE 0x3
#define FC_DSR_DELETE 0x4
#define FC_DSR_EA5_LOAD 0x5
#define FC_DSR_FIND 0x6
#define FC_DSR_OPEN 0x7
#define FC_DSR_PRG_LOAD 0x8
#define FC_DSR_PRG_SAVE 0x9
#define FC_DSR_READ 0xa
#define FC_DSR_READ_CPU 0xb
#define FC_DSR_RESET 0xc
#define FC_DSR_SCRATCH 0xd
#define FC_DSR_STATUS 0xe
#define FC_DSR_WRITE 0xf
#define FC_EXEC_CMD 0x10
#define FC_HEX_FROM_UINT 0x11
#define FC_HEX_TO_INT 0x12
#define FC_LIST_GET 0x13
#define FC_LIST_INIT 0x14
#define FC_LIST_POP 0x15
#define FC_LIST_PUSH 0x16
#define FC_LVL2_FORMAT 0x17
#define FC_LVL2_INPUT 0x18
#define FC_LVL2_INPUT_CPU 0x19
#define FC_LVL2_MKDIR 0x1a
#define FC_LVL2_OUTPUT 0x1b
#define FC_LVL2_OUTPUT_CPU 0x1c
#define FC_LVL2_PROTECT 0x1d
#define FC_LVL2_RENAME 0x1e
#define FC_LVL2_RENDIR 0x1f
#define FC_LVL2_RMDIR 0x20
#define FC_LVL2_SECTOR_READ 0x21
#define FC_LVL2_SECTOR_WRITE 0x22
#define FC_LVL2_SETDIR 0x23
#define FC_MOUSE_HIDE 0x24
#define FC_MOUSE_MOVE 0x25
#define FC_MOUSE_READ 0x26
#define FC_MOUSE_SET_POINTER 0x27
#define FC_MOUSE_SHOW 0x28
#define FC_PATH_PARSE 0x29
#define FC_PATH_TO_IOCODE 0x2a
#define FC_SAMS_ALLOC_PAGES 0x2b
#define FC_SAMS_FREE_PAGES 0x2c
#define FC_SAMS_MAP_PAGE 0x2d
#define FC_SAMS_READ_PAGE 0x2e
#define FC_SND_PLAY 0x2f
#define FC_SND_PLAYING 0x30
#define FC_SND_START 0x31
#define FC_SND_STOP 0x32
#define FC_SND_TICK 0x33
#define FC_SOCKBUF_INIT 0x34
#define FC_SOCKBUF_READLINE 0x35
#define FC_SOCKBUF_READSTREAM 0x36
#define FC_SPEECH_CONTINUE 0x37
#define FC_SPEECH_DETECT 0x38
#define FC_SPEECH_RESET 0x39
#define FC_SPEECH_SAY_DATA 0x3a
#define FC_SPEECH_SAY_VOCAB 0x3b
#define FC_SPEECH_START 0x3c
#define FC_SPEECH_WAIT 0x3d
#define FC_STR_CAT 0x3e
#define FC_STR_CMP 0x3f
#define FC_STR_CMP_ICASE 0x40
#define FC_STR_COPY 0x41
#define FC_STR_ENDSWITH 0x42
#define FC_STR_FROM_BASIC 0x43
#define FC_STR_FROM_FLOAT 0x44
#define FC_STR_FROM_UINT 0x45
#define FC_STR_INDEX_OF 0x46
#define FC_STR_LAST_INDEX_OF 0x47
#define FC_STR_LEN 0x48
#define FC_STR_NCOPY 0x49
#define FC_STR_SET 0x4a
#define FC_STR_STARTSWITH 0x4b
#define FC_STR_TO_INT 0x4c
#define FC_STR_TOKEN 0x4d
#define FC_STR_TOKEN_NEXT 0x4e
#define FC_STR_TOKEN_PEEK 0x4f
#define FC_SYS_DISPLAY_INFO 0x50
#define FC_SYS_INFO 0x51
#define FC_SYS_SAMS_INFO 0x52
#define FC_TCP_CLOSE 0x53
#define FC_TCP_CONNECT 0x54
#define FC_TCP_READ_SOCKET 0x55
#define FC_TCP_SEND_CHARS 0x56
#define FC_TERM_CLS 0x57
#define FC_TERM_DROP_DOWN 0x58
#define FC_TERM_GETS 0x59
#define FC_TERM_GOTOXY 0x5a
#define FC_TERM_KSCAN 0x5b
#define FC_TERM_PUTC 0x5c
#define FC_TERM_PUTS 0x5d
#define FC_TERM_SET_BG_COLOR 0x5e
#define FC_TERM_SET_BORDER_COLOR 0x5f
#define FC_TERM_SET_IDENTIFY_HOOK 0x60
#define FC_TERM_SET_TEXT_COLOR 0x61
#define FC_TIME_GET 0x62
#define FC_TIPI_LOG 0x63
#define FC_TIPI_OFF 0x64
#define FC_TIPI_ON 0x65
#define FC_TIPI_RECVMSG 0x66
#define FC_TIPI_SENDMSG 0x67
#define FC_TLS_CLOSE 0x68
#define FC_TLS_CONNECT 0x69
#define FC_TLS_READ_SOCKET 0x6a
#define FC_TLS_SEND_CHARS 0x6b
#define FC_VAR_GET 0x6c
#define FC_VAR_SET 0x6d
#define FC_VDP_CURSOR_ADDR 0x6e
#define FC_VDP_SCREENMODE 0x6f
#define FC_VDP_SETCHAR 0x70
#define FC_GFX_SCREEN 0x71
#define FC_GFX_GET_INFO 0x72
#define FC_GFX_COLOR 0x73
#define FC_GFX_SET_PAGE 0x74
#define FC_GFX_SET_CURSOR 0x75
#define FC_GFX_GET_CURSOR 0x76
#define FC_GFX_CLEAR 0x77
#define FC_GFX_PSET 0x78
#define FC_GFX_PRESET 0x79
#define FC_GFX_POINT 0x7a
#define FC_GFX_LINE 0x7b
#define FC_GFX_CIRCLE 0x7c
#define FC_GFX_PAINT 0x7d
#define FC_GFX_DRAW 0x7e
#define FC_GFX_COPY 0x7f
#define FC_GFX_PATTERN_DEFINE 0x80
#define FC_GFX_TILE 0x81
#define FC_GFX_SPRITE_PATTERN 0x82
#define FC_GFX_SPRITE 0x83
#define FC_GFX_SPRITE_LOC 0x84
#define FC_GFX_SPRITE_HIDE 0x85
#define FC_GFX_SPRITE_ENABLE 0x86
#define FC_GFX_SPRITE_STATUS 0x87
#define FC_GFX_PALETTE_SET 0x88
#define FC_TUI_BOX 0x89
#define FC_TUI_BOX_TITLE 0x8a
#define FC_TUI_BUTTON_CREATE 0x8b
#define FC_TUI_BUTTON_SET_LABEL 0x8c
#define FC_TUI_CHECKBOX_CREATE 0x8d
#define FC_TUI_CHECKBOX_GET 0x8e
#define FC_TUI_CHECKBOX_SET 0x8f
#define FC_TUI_CONFIRM_BOX 0x90
#define FC_TUI_DISPATCH_EVENT 0x91
#define FC_TUI_DONE 0x92
#define FC_TUI_FILL 0x93
#define FC_TUI_GET_EVENT 0x94
#define FC_TUI_GET_FOCUS 0x95
#define FC_TUI_GOTOXY 0x96
#define FC_TUI_HAS_COLOR 0x97
#define FC_TUI_HLINE 0x98
#define FC_TUI_INIT 0x99
#define FC_TUI_INPUT_BOX 0x9a
#define FC_TUI_LABEL_CREATE 0x9b
#define FC_TUI_LABEL_SET_TEXT 0x9c
#define FC_TUI_LIST_ADD 0x9d
#define FC_TUI_LIST_CLEAR 0x9e
#define FC_TUI_LIST_COUNT 0x9f
#define FC_TUI_LIST_CREATE 0xa0
#define FC_TUI_LIST_GET_SELECTED 0xa1
#define FC_TUI_LIST_INSERT 0xa2
#define FC_TUI_LIST_REMOVE 0xa3
#define FC_TUI_LIST_SET_SELECTED 0xa4
#define FC_TUI_MENU_CREATE 0xa5
#define FC_TUI_MENU_DESTROY 0xa6
#define FC_TUI_MENU_GET_SELECTED 0xa7
#define FC_TUI_MESSAGE_BOX 0xa8
#define FC_TUI_PROGRESSBAR_CREATE 0xa9
#define FC_TUI_PROGRESSBAR_SET 0xaa
#define FC_TUI_PUTC 0xab
#define FC_TUI_PUTS 0xac
#define FC_TUI_SCREEN_HEIGHT 0xad
#define FC_TUI_SCREEN_WIDTH 0xae
#define FC_TUI_SET_COLOR 0xaf
#define FC_TUI_SET_FOCUS 0xb0
#define FC_TUI_TEXTFIELD_CREATE 0xb1
#define FC_TUI_TEXTFIELD_GET 0xb2
#define FC_TUI_TEXTFIELD_SET 0xb3
#define FC_TUI_TEXTFIELD_SET_CURSOR 0xb4
#define FC_TUI_VLINE 0xb5
#define FC_TUI_WIDGET_DESTROY 0xb6
#define FC_TUI_WIDGET_DISABLE 0xb7
#define FC_TUI_WIDGET_ENABLE 0xb8
#define FC_TUI_WIDGET_FOCUS 0xb9
#define FC_TUI_WIDGET_HIDE 0xba
#define FC_TUI_WIDGET_SHOW 0xbb
#define FC_TUI_WIN_CLOSE 0xbc
#define FC_TUI_WIN_GOTOXY 0xbd
#define FC_TUI_WIN_MOVE 0xbe
#define FC_TUI_WIN_OPEN 0xbf
#define FC_TUI_WIN_PRINTF 0xc0
#define FC_TUI_WIN_PUTC 0xc1
#define FC_TUI_WIN_PUTS 0xc2
#define FC_TUI_WIN_RESIZE 0xc3
#define FC_TUI_WIN_SCROLL 0xc4
#define FC_TUI_WIN_SET_COLORS 0xc5
#define FC_TUI_WIN_SET_TITLE 0xc6
#define FC_TUI_WIN_SET_BORDER 0xc7
#define FC_FC_EXIT 0xc8

// function: void audio_beep()
DECL_FC_API_CALL(FC_AUDIO_BEEP, audio_beep, void, (), ())

// function: void audio_honk()
DECL_FC_API_CALL(FC_AUDIO_HONK, audio_honk, void, (), ())

// function: unsigned int dsr_catalog(struct DeviceServiceRoutine* dsr, const char* pathname, vol_entry_cb vol_cb, dir_entry_cb dir_cb)
DECL_FC_API_CALL(FC_DSR_CATALOG, dsr_catalog, unsigned int, (struct DeviceServiceRoutine* dsr, const char* pathname, vol_entry_cb vol_cb, dir_entry_cb dir_cb), (dsr, pathname, vol_cb, dir_cb))

// function: unsigned int dsr_close(struct DeviceServiceRoutine* dsr, struct PAB* pab)
DECL_FC_API_CALL(FC_DSR_CLOSE, dsr_close, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab), (dsr, pab))

// function: unsigned int dsr_delete(struct DeviceServiceRoutine* dsr, struct PAB* pab)
DECL_FC_API_CALL(FC_DSR_DELETE, dsr_delete, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab), (dsr, pab))

// function: void dsr_ea5_load(struct DeviceServiceRoutine * dsr, const char *fname)
DECL_FC_API_CALL(FC_DSR_EA5_LOAD, dsr_ea5_load, void, (struct DeviceServiceRoutine * dsr, const char *fname), (dsr, fname))

// function: struct DeviceServiceRoutine* dsr_find(char* devicename, int crubase)
DECL_FC_API_CALL(FC_DSR_FIND, dsr_find, struct DeviceServiceRoutine*, (char* devicename, int crubase), (devicename, crubase))

// function: unsigned int dsr_open(struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int flags, int reclen)
DECL_FC_API_CALL(FC_DSR_OPEN, dsr_open, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int flags, int reclen), (dsr, pab, fname, flags, reclen))

// function: unsigned int dsr_prg_load(struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int vdpaddr, int maxsize)
DECL_FC_API_CALL(FC_DSR_PRG_LOAD, dsr_prg_load, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int vdpaddr, int maxsize), (dsr, pab, fname, vdpaddr, maxsize))

// function: unsigned int dsr_prg_save(struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int vdpaddr, int count)
DECL_FC_API_CALL(FC_DSR_PRG_SAVE, dsr_prg_save, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int vdpaddr, int count), (dsr, pab, fname, vdpaddr, count))

// function: unsigned int dsr_read(struct DeviceServiceRoutine* dsr, struct PAB* pab, int recordNumber)
DECL_FC_API_CALL(FC_DSR_READ, dsr_read, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab, int recordNumber), (dsr, pab, recordNumber))

// function: unsigned int dsr_read_cpu(struct DeviceServiceRoutine* dsr, struct PAB* pab, int recordNumber, char* recordBuf)
DECL_FC_API_CALL(FC_DSR_READ_CPU, dsr_read_cpu, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab, int recordNumber, char* recordBuf), (dsr, pab, recordNumber, recordBuf))

// function: unsigned int dsr_reset(struct DeviceServiceRoutine* dsr, struct PAB* pab, int record)
DECL_FC_API_CALL(FC_DSR_RESET, dsr_reset, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab, int record), (dsr, pab, record))

// function: unsigned int dsr_scratch(struct DeviceServiceRoutine* dsr, struct PAB* pab, int record)
DECL_FC_API_CALL(FC_DSR_SCRATCH, dsr_scratch, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab, int record), (dsr, pab, record))

// function: unsigned int dsr_status(struct DeviceServiceRoutine* dsr, struct PAB* pab)
DECL_FC_API_CALL(FC_DSR_STATUS, dsr_status, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab), (dsr, pab))

// function: unsigned int dsr_write(struct DeviceServiceRoutine* dsr, struct PAB* pab, char* record, int reclen)
DECL_FC_API_CALL(FC_DSR_WRITE, dsr_write, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab, char* record, int reclen), (dsr, pab, record, reclen))

// function: int exec_cmd(char* command)
DECL_FC_API_CALL(FC_EXEC_CMD, exec_cmd, int, (char* command), (command))

// function: char * hex_from_uint(unsigned int x)
DECL_FC_API_CALL(FC_HEX_FROM_UINT, hex_from_uint, char *, (unsigned int x), (x))

// function: int hex_to_int(char* s)
DECL_FC_API_CALL(FC_HEX_TO_INT, hex_to_int, int, (char* s), (s))

// function: struct ListEntry* list_get(struct List* list, int index)
DECL_FC_API_CALL(FC_LIST_GET, list_get, struct ListEntry*, (struct List* list, int index), (list, index))

// function: void list_init(struct List* list, void* addr, void* ceiling)
DECL_FC_API_CALL(FC_LIST_INIT, list_init, void, (struct List* list, void* addr, void* ceiling), (list, addr, ceiling))

// function: void list_pop(struct List* list, char* buffer, int limit)
DECL_FC_API_CALL(FC_LIST_POP, list_pop, void, (struct List* list, char* buffer, int limit), (list, buffer, limit))

// function: void list_push(struct List* list, char* buffer, int length)
DECL_FC_API_CALL(FC_LIST_PUSH, list_push, void, (struct List* list, char* buffer, int length), (list, buffer, length))

// function: unsigned int lvl2_format(int crubase, unsigned int iocode, unsigned int tracks, unsigned int density, unsigned int sides, unsigned int interleave)
DECL_FC_API_CALL(FC_LVL2_FORMAT, lvl2_format, unsigned int, (int crubase, unsigned int iocode, unsigned int tracks, unsigned int density, unsigned int sides, unsigned int interleave), (crubase, iocode, tracks, density, sides, interleave))

// function: unsigned int lvl2_input(int crubase, unsigned int iocode, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr)
DECL_FC_API_CALL(FC_LVL2_INPUT, lvl2_input, unsigned int, (int crubase, unsigned int iocode, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr), (crubase, iocode, filename, blockcount, addInfoPtr))

// function: unsigned int lvl2_input_cpu(int crubase, unsigned int iocode, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr)
DECL_FC_API_CALL(FC_LVL2_INPUT_CPU, lvl2_input_cpu, unsigned int, (int crubase, unsigned int iocode, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr), (crubase, iocode, filename, blockcount, addInfoPtr))

// function: unsigned int lvl2_mkdir(int crubase, unsigned int iocode, char *dirname)
DECL_FC_API_CALL(FC_LVL2_MKDIR, lvl2_mkdir, unsigned int, (int crubase, unsigned int iocode, char *dirname), (crubase, iocode, dirname))

// function: unsigned int lvl2_output(int crubase, unsigned int iocode, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr)
DECL_FC_API_CALL(FC_LVL2_OUTPUT, lvl2_output, unsigned int, (int crubase, unsigned int iocode, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr), (crubase, iocode, filename, blockcount, addInfoPtr))

// function: unsigned int lvl2_output_cpu(int crubase, unsigned int iocode, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr)
DECL_FC_API_CALL(FC_LVL2_OUTPUT_CPU, lvl2_output_cpu, unsigned int, (int crubase, unsigned int iocode, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr), (crubase, iocode, filename, blockcount, addInfoPtr))

// function: unsigned int lvl2_protect(int crubase, unsigned int iocode, char *filename, int protect)
DECL_FC_API_CALL(FC_LVL2_PROTECT, lvl2_protect, unsigned int, (int crubase, unsigned int iocode, char *filename, int protect), (crubase, iocode, filename, protect))

// function: unsigned int lvl2_rename(int crubase, unsigned int iocode, char *oldname, char *newname)
DECL_FC_API_CALL(FC_LVL2_RENAME, lvl2_rename, unsigned int, (int crubase, unsigned int iocode, char *oldname, char *newname), (crubase, iocode, oldname, newname))

// function: unsigned int lvl2_rendir(int crubase, unsigned int iocode, char *oldname, char *newname)
DECL_FC_API_CALL(FC_LVL2_RENDIR, lvl2_rendir, unsigned int, (int crubase, unsigned int iocode, char *oldname, char *newname), (crubase, iocode, oldname, newname))

// function: unsigned int lvl2_rmdir(int crubase, unsigned int iocode, char *dirname)
DECL_FC_API_CALL(FC_LVL2_RMDIR, lvl2_rmdir, unsigned int, (int crubase, unsigned int iocode, char *dirname), (crubase, iocode, dirname))

// function: unsigned int lvl2_sector_read(int crubase, unsigned int iocode, unsigned int sector, char* bufaddr)
DECL_FC_API_CALL(FC_LVL2_SECTOR_READ, lvl2_sector_read, unsigned int, (int crubase, unsigned int iocode, unsigned int sector, char* bufaddr), (crubase, iocode, sector, bufaddr))

// function: unsigned int lvl2_sector_write(int crubase, unsigned int iocode, unsigned int sector, char*bufaddr)
DECL_FC_API_CALL(FC_LVL2_SECTOR_WRITE, lvl2_sector_write, unsigned int, (int crubase, unsigned int iocode, unsigned int sector, char*bufaddr), (crubase, iocode, sector, bufaddr))

// function: unsigned int lvl2_setdir(int crubase, unsigned int iocode, char* path)
DECL_FC_API_CALL(FC_LVL2_SETDIR, lvl2_setdir, unsigned int, (int crubase, unsigned int iocode, char* path), (crubase, iocode, path))

// function: void mouse_hide()
DECL_FC_API_CALL(FC_MOUSE_HIDE, mouse_hide, void, (), ())

// function: void mouse_move(struct MouseData* mouseData)
DECL_FC_API_CALL(FC_MOUSE_MOVE, mouse_move, void, (struct MouseData* mouseData), (mouseData))

// function: void mouse_read(struct MouseData* mouseData)
DECL_FC_API_CALL(FC_MOUSE_READ, mouse_read, void, (struct MouseData* mouseData), (mouseData))

// function: void mouse_set_pointer(int p)
DECL_FC_API_CALL(FC_MOUSE_SET_POINTER, mouse_set_pointer, void, (int p), (p))

// function: void mouse_show(struct MouseData* mouseData)
DECL_FC_API_CALL(FC_MOUSE_SHOW, mouse_show, void, (struct MouseData* mouseData), (mouseData))

// function: void path_parse(char* str_in, struct DeviceServiceRoutine** dsr, char* buffer, int requirements)
DECL_FC_API_CALL(FC_PATH_PARSE, path_parse, void, (char* str_in, struct DeviceServiceRoutine** dsr, char* buffer, int requirements), (str_in, dsr, buffer, requirements))

// function: unsigned int path_to_iocode(const char* currentPath)
DECL_FC_API_CALL(FC_PATH_TO_IOCODE, path_to_iocode, unsigned int, (const char* currentPath), (currentPath))

// function: int sams_alloc_pages(int count)
DECL_FC_API_CALL(FC_SAMS_ALLOC_PAGES, sams_alloc_pages, int, (int count), (count))

// function: int sams_free_pages(int count)
DECL_FC_API_CALL(FC_SAMS_FREE_PAGES, sams_free_pages, int, (int count), (count))

// function: void sams_map_page(int page, int addr)
DECL_FC_API_CALL(FC_SAMS_MAP_PAGE, sams_map_page, void, (int page, int addr), (page, addr))

// function: int sams_read_page(int slot)
DECL_FC_API_CALL(FC_SAMS_READ_PAGE, sams_read_page, int, (int slot), (slot))

// function: void snd_play(const unsigned char *list)
DECL_FC_API_CALL(FC_SND_PLAY, snd_play, void, (const unsigned char *list), (list))

// function: int snd_playing()
DECL_FC_API_CALL(FC_SND_PLAYING, snd_playing, int, (), ())

// function: void snd_start(const unsigned char *list)
DECL_FC_API_CALL(FC_SND_START, snd_start, void, (const unsigned char *list), (list))

// function: void snd_stop()
DECL_FC_API_CALL(FC_SND_STOP, snd_stop, void, (), ())

// function: void snd_tick()
DECL_FC_API_CALL(FC_SND_TICK, snd_tick, void, (), ())

// function: void sockbuf_init(struct SocketBuffer* socket_buf, int tls, unsigned int socketId)
DECL_FC_API_CALL(FC_SOCKBUF_INIT, sockbuf_init, void, (struct SocketBuffer* socket_buf, int tls, unsigned int socketId), (socket_buf, tls, socketId))

// function: char* sockbuf_readline(struct SocketBuffer* socket_buf)
DECL_FC_API_CALL(FC_SOCKBUF_READLINE, sockbuf_readline, char*, (struct SocketBuffer* socket_buf), (socket_buf))

// function: int sockbuf_readstream(struct SocketBuffer* socket_buf, char* block, int limit)
DECL_FC_API_CALL(FC_SOCKBUF_READSTREAM, sockbuf_readstream, int, (struct SocketBuffer* socket_buf, char* block, int limit), (socket_buf, block, limit))

// function: void speech_continue(struct LpcPlaybackCtx* ctx)
DECL_FC_API_CALL(FC_SPEECH_CONTINUE, speech_continue, void, (struct LpcPlaybackCtx* ctx), (ctx))

// function: int speech_detect()
DECL_FC_API_CALL(FC_SPEECH_DETECT, speech_detect, int, (), ())

// function: void speech_reset()
DECL_FC_API_CALL(FC_SPEECH_RESET, speech_reset, void, (), ())

// function: void speech_say_data(const char* addr, int len)
DECL_FC_API_CALL(FC_SPEECH_SAY_DATA, speech_say_data, void, (const char* addr, int len), (addr, len))

// function: void speech_say_vocab(int phrase_addr)
DECL_FC_API_CALL(FC_SPEECH_SAY_VOCAB, speech_say_vocab, void, (int phrase_addr), (phrase_addr))

// function: void speech_start(struct LpcPlaybackCtx* ctx)
DECL_FC_API_CALL(FC_SPEECH_START, speech_start, void, (struct LpcPlaybackCtx* ctx), (ctx))

// function: void speech_wait()
DECL_FC_API_CALL(FC_SPEECH_WAIT, speech_wait, void, (), ())

// function: char * str_cat(char *dst, const char *add)
DECL_FC_API_CALL(FC_STR_CAT, str_cat, char *, (char *dst, const char *add), (dst, add))

// function: int str_cmp(const char *a, const char *b)
DECL_FC_API_CALL(FC_STR_CMP, str_cmp, int, (const char *a, const char *b), (a, b))

// function: int str_cmp_icase(const char *a, const char *b)
DECL_FC_API_CALL(FC_STR_CMP_ICASE, str_cmp_icase, int, (const char *a, const char *b), (a, b))

// function: char * str_copy(char *d, const char *s)
DECL_FC_API_CALL(FC_STR_COPY, str_copy, char *, (char *d, const char *s), (d, s))

// function: int str_endswith(const char *str, const char *suffix)
DECL_FC_API_CALL(FC_STR_ENDSWITH, str_endswith, int, (const char *str, const char *suffix), (str, suffix))

// function: int str_from_basic(const char *str, char *buf)
DECL_FC_API_CALL(FC_STR_FROM_BASIC, str_from_basic, int, (const char *str, char *buf), (str, buf))

// function: char* str_from_float(double a)
DECL_FC_API_CALL(FC_STR_FROM_FLOAT, str_from_float, char*, (double a), (a))

// function: char * str_from_uint(unsigned int x)
DECL_FC_API_CALL(FC_STR_FROM_UINT, str_from_uint, char *, (unsigned int x), (x))

// function: int str_index_of(const char *str, int c)
DECL_FC_API_CALL(FC_STR_INDEX_OF, str_index_of, int, (const char *str, int c), (str, c))

// function: int str_last_index_of(const char *str, int c, int start)
DECL_FC_API_CALL(FC_STR_LAST_INDEX_OF, str_last_index_of, int, (const char *str, int c, int start), (str, c, start))

// function: int str_len(const char *s)
DECL_FC_API_CALL(FC_STR_LEN, str_len, int, (const char *s), (s))

// function: char * str_ncopy(char *dest, char *src, int limit)
DECL_FC_API_CALL(FC_STR_NCOPY, str_ncopy, char *, (char *dest, char *src, int limit), (dest, src, limit))

// function: void str_set(char *buffer, int value, int limit)
DECL_FC_API_CALL(FC_STR_SET, str_set, void, (char *buffer, int value, int limit), (buffer, value, limit))

// function: int str_startswith(const char *str, const char *prefix)
DECL_FC_API_CALL(FC_STR_STARTSWITH, str_startswith, int, (const char *str, const char *prefix), (str, prefix))

// function: int str_to_int(char *s)
DECL_FC_API_CALL(FC_STR_TO_INT, str_to_int, int, (char *s), (s))

// function: char * str_token(char *str, int delim)
DECL_FC_API_CALL(FC_STR_TOKEN, str_token, char *, (char *str, int delim), (str, delim))

// function: char* str_token_next(char* dst, char* str, int delim)
DECL_FC_API_CALL(FC_STR_TOKEN_NEXT, str_token_next, char*, (char* dst, char* str, int delim), (dst, str, delim))

// function: char * str_token_peek(char *str, int delim)
DECL_FC_API_CALL(FC_STR_TOKEN_PEEK, str_token_peek, char *, (char *str, int delim), (str, delim))

// function: void sys_display_info(struct DisplayInformation * info)
DECL_FC_API_CALL(FC_SYS_DISPLAY_INFO, sys_display_info, void, (struct DisplayInformation * info), (info))

// function: void sys_info(struct SystemInformation* info)
DECL_FC_API_CALL(FC_SYS_INFO, sys_info, void, (struct SystemInformation* info), (info))

// function: void sys_sams_info(struct SamsInformation* info)
DECL_FC_API_CALL(FC_SYS_SAMS_INFO, sys_sams_info, void, (struct SamsInformation* info), (info))

// function: unsigned int tcp_close(unsigned int socketId)
DECL_FC_API_CALL(FC_TCP_CLOSE, tcp_close, unsigned int, (unsigned int socketId), (socketId))

// function: unsigned int tcp_connect(unsigned int socketId, char* hostname, char* port)
DECL_FC_API_CALL(FC_TCP_CONNECT, tcp_connect, unsigned int, (unsigned int socketId, char* hostname, char* port), (socketId, hostname, port))

// function: int tcp_read_socket(unsigned int socketId, char* buf, int bufsize)
DECL_FC_API_CALL(FC_TCP_READ_SOCKET, tcp_read_socket, int, (unsigned int socketId, char* buf, int bufsize), (socketId, buf, bufsize))

// function: int tcp_send_chars(unsigned int socketId, char* buf, int size)
DECL_FC_API_CALL(FC_TCP_SEND_CHARS, tcp_send_chars, int, (unsigned int socketId, char* buf, int size), (socketId, buf, size))

// function: void term_cls()
DECL_FC_API_CALL(FC_TERM_CLS, term_cls, void, (), ())

// function: void term_drop_down(int linecount)
DECL_FC_API_CALL(FC_TERM_DROP_DOWN, term_drop_down, void, (int linecount), (linecount))

// function: void term_gets(char* var, int limit, int backspace)
DECL_FC_API_CALL(FC_TERM_GETS, term_gets, void, (char* var, int limit, int backspace), (var, limit, backspace))

// function: void term_gotoxy(int x, int y)
DECL_FC_API_CALL(FC_TERM_GOTOXY, term_gotoxy, void, (int x, int y), (x, y))

// function: unsigned int term_kscan(unsigned int mode)
DECL_FC_API_CALL(FC_TERM_KSCAN, term_kscan, unsigned int, (unsigned int mode), (mode))

// function: void term_putc(int c)
DECL_FC_API_CALL(FC_TERM_PUTC, term_putc, void, (int c), (c))

// function: void term_puts(const char* str)
DECL_FC_API_CALL(FC_TERM_PUTS, term_puts, void, (const char* str), (str))

// function: unsigned int term_set_bg_color(unsigned int color)
DECL_FC_API_CALL(FC_TERM_SET_BG_COLOR, term_set_bg_color, unsigned int, (unsigned int color), (color))

// function: unsigned int term_set_border_color(unsigned int x)
DECL_FC_API_CALL(FC_TERM_SET_BORDER_COLOR, term_set_border_color, unsigned int, (unsigned int x), (x))

// function: void term_set_identify_hook(identify_callback cb)
DECL_FC_API_CALL(FC_TERM_SET_IDENTIFY_HOOK, term_set_identify_hook, void, (identify_callback cb), (cb))

// function: unsigned int term_set_text_color(unsigned int color)
DECL_FC_API_CALL(FC_TERM_SET_TEXT_COLOR, term_set_text_color, unsigned int, (unsigned int color), (color))

// function: void time_get(struct DateTime* dt)
DECL_FC_API_CALL(FC_TIME_GET, time_get, void, (struct DateTime* dt), (dt))

// function: void tipi_log(char* msg)
DECL_FC_API_CALL(FC_TIPI_LOG, tipi_log, void, (char* msg), (msg))

// function: void tipi_off()
DECL_FC_API_CALL(FC_TIPI_OFF, tipi_off, void, (), ())

// function: int tipi_on()
DECL_FC_API_CALL(FC_TIPI_ON, tipi_on, int, (), ())

// function: void tipi_recvmsg(unsigned int* len, char* buf)
DECL_FC_API_CALL(FC_TIPI_RECVMSG, tipi_recvmsg, void, (unsigned int* len, char* buf), (len, buf))

// function: void tipi_sendmsg(unsigned int len, const char* buf)
DECL_FC_API_CALL(FC_TIPI_SENDMSG, tipi_sendmsg, void, (unsigned int len, const char* buf), (len, buf))

// function: unsigned int tls_close(unsigned int socketId)
DECL_FC_API_CALL(FC_TLS_CLOSE, tls_close, unsigned int, (unsigned int socketId), (socketId))

// function: unsigned int tls_connect(unsigned int socketId, char* hostname, char* port)
DECL_FC_API_CALL(FC_TLS_CONNECT, tls_connect, unsigned int, (unsigned int socketId, char* hostname, char* port), (socketId, hostname, port))

// function: int tls_read_socket(unsigned int socketId, char* buf, int bufsize)
DECL_FC_API_CALL(FC_TLS_READ_SOCKET, tls_read_socket, int, (unsigned int socketId, char* buf, int bufsize), (socketId, buf, bufsize))

// function: int tls_send_chars(unsigned int socketId, char* buf, int size)
DECL_FC_API_CALL(FC_TLS_SEND_CHARS, tls_send_chars, int, (unsigned int socketId, char* buf, int size), (socketId, buf, size))

// function: char* var_get(char* name)
DECL_FC_API_CALL(FC_VAR_GET, var_get, char*, (char* name), (name))

// function: void var_set(char* name, char* value)
DECL_FC_API_CALL(FC_VAR_SET, var_set, void, (char* name, char* value), (name, value))

// function: unsigned int vdp_cursor_addr()
DECL_FC_API_CALL(FC_VDP_CURSOR_ADDR, vdp_cursor_addr, unsigned int, (), ())

// function: int vdp_screenmode(int mode)
DECL_FC_API_CALL(FC_VDP_SCREENMODE, vdp_screenmode, int, (int mode), (mode))

// function: void vdp_setchar(int pAddr, int ch)
DECL_FC_API_CALL(FC_VDP_SETCHAR, vdp_setchar, void, (int pAddr, int ch), (pAddr, ch))

// function: int gfx_screen(int mode, int sprite_mode, int flags)
DECL_FC_API_CALL(FC_GFX_SCREEN, gfx_screen, int, (int mode, int sprite_mode, int flags), (mode, sprite_mode, flags))

// function: int gfx_get_info(struct GfxInformation* info)
DECL_FC_API_CALL(FC_GFX_GET_INFO, gfx_get_info, int, (struct GfxInformation* info), (info))

// function: int gfx_color(int foreground, int background, int border)
DECL_FC_API_CALL(FC_GFX_COLOR, gfx_color, int, (int foreground, int background, int border), (foreground, background, border))

// function: int gfx_set_page(int display_page, int draw_page)
DECL_FC_API_CALL(FC_GFX_SET_PAGE, gfx_set_page, int, (int display_page, int draw_page), (display_page, draw_page))

// function: int gfx_set_cursor(int x, int y)
DECL_FC_API_CALL(FC_GFX_SET_CURSOR, gfx_set_cursor, int, (int x, int y), (x, y))

// function: int gfx_get_cursor(int* x, int* y)
DECL_FC_API_CALL(FC_GFX_GET_CURSOR, gfx_get_cursor, int, (int* x, int* y), (x, y))

// function: int gfx_clear(int color)
DECL_FC_API_CALL(FC_GFX_CLEAR, gfx_clear, int, (int color), (color))

// function: int gfx_pset(int x, int y, int color, int op)
DECL_FC_API_CALL(FC_GFX_PSET, gfx_pset, int, (int x, int y, int color, int op), (x, y, color, op))

// function: int gfx_preset(int x, int y, int color, int op)
DECL_FC_API_CALL(FC_GFX_PRESET, gfx_preset, int, (int x, int y, int color, int op), (x, y, color, op))

// function: int gfx_point(int x, int y, int* color)
DECL_FC_API_CALL(FC_GFX_POINT, gfx_point, int, (int x, int y, int* color), (x, y, color))

// function: int gfx_line(int x1, int y1, int x2, int y2, int color, int style, int op)
DECL_FC_API_CALL(FC_GFX_LINE, gfx_line, int, (int x1, int y1, int x2, int y2, int color, int style, int op), (x1, y1, x2, y2, color, style, op))

// function: int gfx_circle(int center_x, int center_y, int radius, int color, int start_angle, int end_angle, int aspect, int op)
DECL_FC_API_CALL(FC_GFX_CIRCLE, gfx_circle, int, (int center_x, int center_y, int radius, int color, int start_angle, int end_angle, int aspect, int op), (center_x, center_y, radius, color, start_angle, end_angle, aspect, op))

// function: int gfx_paint(int x, int y, int color, int border_color, int op)
DECL_FC_API_CALL(FC_GFX_PAINT, gfx_paint, int, (int x, int y, int color, int border_color, int op), (x, y, color, border_color, op))

// function: int gfx_draw(const char* commands, int color, int op)
DECL_FC_API_CALL(FC_GFX_DRAW, gfx_draw, int, (const char* commands, int color, int op), (commands, color, op))

// function: int gfx_copy(int source_page, int destination_page, int x1, int y1, int x2, int y2, int destination_x, int destination_y, int op)
DECL_FC_API_CALL(FC_GFX_COPY, gfx_copy, int, (int source_page, int destination_page, int x1, int y1, int x2, int y2, int destination_x, int destination_y, int op), (source_page, destination_page, x1, y1, x2, y2, destination_x, destination_y, op))

// function: int gfx_pattern_define(int pattern, const unsigned char* data, int bytes)
DECL_FC_API_CALL(FC_GFX_PATTERN_DEFINE, gfx_pattern_define, int, (int pattern, const unsigned char* data, int bytes), (pattern, data, bytes))

// function: int gfx_tile(int x, int y, int pattern, int color)
DECL_FC_API_CALL(FC_GFX_TILE, gfx_tile, int, (int x, int y, int pattern, int color), (x, y, pattern, color))

// function: int gfx_sprite_pattern(int pattern, const unsigned char* data, int bytes)
DECL_FC_API_CALL(FC_GFX_SPRITE_PATTERN, gfx_sprite_pattern, int, (int pattern, const unsigned char* data, int bytes), (pattern, data, bytes))

// function: int gfx_sprite(int number, int pattern, int color, int x, int y)
DECL_FC_API_CALL(FC_GFX_SPRITE, gfx_sprite, int, (int number, int pattern, int color, int x, int y), (number, pattern, color, x, y))

// function: int gfx_sprite_loc(int number, int x, int y)
DECL_FC_API_CALL(FC_GFX_SPRITE_LOC, gfx_sprite_loc, int, (int number, int x, int y), (number, x, y))

// function: int gfx_sprite_hide(int number)
DECL_FC_API_CALL(FC_GFX_SPRITE_HIDE, gfx_sprite_hide, int, (int number), (number))

// function: int gfx_sprite_enable(int enabled)
DECL_FC_API_CALL(FC_GFX_SPRITE_ENABLE, gfx_sprite_enable, int, (int enabled), (enabled))

// function: int gfx_sprite_status(struct GfxSpriteStatus* status)
DECL_FC_API_CALL(FC_GFX_SPRITE_STATUS, gfx_sprite_status, int, (struct GfxSpriteStatus* status), (status))

// function: int gfx_palette_set(int index, int red, int green, int blue)
DECL_FC_API_CALL(FC_GFX_PALETTE_SET, gfx_palette_set, int, (int index, int red, int green, int blue), (index, red, green, blue))

// function: void tui_box(int x, int y, int w, int h)
DECL_FC_API_CALL(FC_TUI_BOX, tui_box, void, (int x, int y, int w, int h), (x, y, w, h))

// function: void tui_box_title(int x, int y, int w, int h, const char* title)
DECL_FC_API_CALL(FC_TUI_BOX_TITLE, tui_box_title, void, (int x, int y, int w, int h, const char* title), (x, y, w, h, title))

// function: tui_widget_t* tui_button_create(tui_win_t* win, int x, int y, int w, const char* label)
DECL_FC_API_CALL(FC_TUI_BUTTON_CREATE, tui_button_create, tui_widget_t*, (tui_win_t* win, int x, int y, int w, const char* label), (win, x, y, w, label))

// function: void tui_button_set_label(tui_widget_t* btn, const char* label)
DECL_FC_API_CALL(FC_TUI_BUTTON_SET_LABEL, tui_button_set_label, void, (tui_widget_t* btn, const char* label), (btn, label))

// function: tui_widget_t* tui_checkbox_create(tui_win_t* win, int x, int y, const char* label)
DECL_FC_API_CALL(FC_TUI_CHECKBOX_CREATE, tui_checkbox_create, tui_widget_t*, (tui_win_t* win, int x, int y, const char* label), (win, x, y, label))

// function: int tui_checkbox_get(tui_widget_t* cb)
DECL_FC_API_CALL(FC_TUI_CHECKBOX_GET, tui_checkbox_get, int, (tui_widget_t* cb), (cb))

// function: void tui_checkbox_set(tui_widget_t* cb, int state)
DECL_FC_API_CALL(FC_TUI_CHECKBOX_SET, tui_checkbox_set, void, (tui_widget_t* cb, int state), (cb, state))

// function: int tui_confirm_box(const char* title, const char* msg)
DECL_FC_API_CALL(FC_TUI_CONFIRM_BOX, tui_confirm_box, int, (const char* title, const char* msg), (title, msg))

// function: int tui_dispatch_event(struct tui_event* ev)
DECL_FC_API_CALL(FC_TUI_DISPATCH_EVENT, tui_dispatch_event, int, (struct tui_event* ev), (ev))

// function: void tui_done()
DECL_FC_API_CALL(FC_TUI_DONE, tui_done, void, (), ())

// function: void tui_fill(int x, int y, int w, int h, int ch)
DECL_FC_API_CALL(FC_TUI_FILL, tui_fill, void, (int x, int y, int w, int h, int ch), (x, y, w, h, ch))

// function: int tui_get_event(struct tui_event* ev)
DECL_FC_API_CALL(FC_TUI_GET_EVENT, tui_get_event, int, (struct tui_event* ev), (ev))

// function: tui_widget_t* tui_get_focus()
DECL_FC_API_CALL(FC_TUI_GET_FOCUS, tui_get_focus, tui_widget_t*, (), ())

// function: void tui_gotoxy(int x, int y)
DECL_FC_API_CALL(FC_TUI_GOTOXY, tui_gotoxy, void, (int x, int y), (x, y))

// function: int tui_has_color()
DECL_FC_API_CALL(FC_TUI_HAS_COLOR, tui_has_color, int, (), ())

// function: void tui_hline(int x, int y, int w)
DECL_FC_API_CALL(FC_TUI_HLINE, tui_hline, void, (int x, int y, int w), (x, y, w))

// function: int tui_init(void* pool, int size)
DECL_FC_API_CALL(FC_TUI_INIT, tui_init, int, (void* pool, int size), (pool, size))

// function: int tui_input_box(const char* title, const char* prompt, char* buf, int len)
DECL_FC_API_CALL(FC_TUI_INPUT_BOX, tui_input_box, int, (const char* title, const char* prompt, char* buf, int len), (title, prompt, buf, len))

// function: tui_widget_t* tui_label_create(tui_win_t* win, int x, int y, const char* text)
DECL_FC_API_CALL(FC_TUI_LABEL_CREATE, tui_label_create, tui_widget_t*, (tui_win_t* win, int x, int y, const char* text), (win, x, y, text))

// function: void tui_label_set_text(tui_widget_t* lbl, const char* text)
DECL_FC_API_CALL(FC_TUI_LABEL_SET_TEXT, tui_label_set_text, void, (tui_widget_t* lbl, const char* text), (lbl, text))

// function: int tui_list_add(tui_widget_t* list, const char* item)
DECL_FC_API_CALL(FC_TUI_LIST_ADD, tui_list_add, int, (tui_widget_t* list, const char* item), (list, item))

// function: void tui_list_clear(tui_widget_t* list)
DECL_FC_API_CALL(FC_TUI_LIST_CLEAR, tui_list_clear, void, (tui_widget_t* list), (list))

// function: int tui_list_count(tui_widget_t* list)
DECL_FC_API_CALL(FC_TUI_LIST_COUNT, tui_list_count, int, (tui_widget_t* list), (list))

// function: tui_widget_t* tui_list_create(tui_win_t* win, int x, int y, int w, int h)
DECL_FC_API_CALL(FC_TUI_LIST_CREATE, tui_list_create, tui_widget_t*, (tui_win_t* win, int x, int y, int w, int h), (win, x, y, w, h))

// function: int tui_list_get_selected(tui_widget_t* list)
DECL_FC_API_CALL(FC_TUI_LIST_GET_SELECTED, tui_list_get_selected, int, (tui_widget_t* list), (list))

// function: int tui_list_insert(tui_widget_t* list, int idx, const char* item)
DECL_FC_API_CALL(FC_TUI_LIST_INSERT, tui_list_insert, int, (tui_widget_t* list, int idx, const char* item), (list, idx, item))

// function: void tui_list_remove(tui_widget_t* list, int idx)
DECL_FC_API_CALL(FC_TUI_LIST_REMOVE, tui_list_remove, void, (tui_widget_t* list, int idx), (list, idx))

// function: void tui_list_set_selected(tui_widget_t* list, int idx)
DECL_FC_API_CALL(FC_TUI_LIST_SET_SELECTED, tui_list_set_selected, void, (tui_widget_t* list, int idx), (list, idx))

// function: tui_win_t* tui_menu_create(int x, int y, const char** items, int count)
DECL_FC_API_CALL(FC_TUI_MENU_CREATE, tui_menu_create, tui_win_t*, (int x, int y, const char** items, int count), (x, y, items, count))

// function: void tui_menu_destroy(tui_win_t* menu)
DECL_FC_API_CALL(FC_TUI_MENU_DESTROY, tui_menu_destroy, void, (tui_win_t* menu), (menu))

// function: int tui_menu_get_selected(tui_win_t* menu)
DECL_FC_API_CALL(FC_TUI_MENU_GET_SELECTED, tui_menu_get_selected, int, (tui_win_t* menu), (menu))

// function: int tui_message_box(const char* title, const char* msg)
DECL_FC_API_CALL(FC_TUI_MESSAGE_BOX, tui_message_box, int, (const char* title, const char* msg), (title, msg))

// function: tui_widget_t* tui_progressbar_create(tui_win_t* win, int x, int y, int w)
DECL_FC_API_CALL(FC_TUI_PROGRESSBAR_CREATE, tui_progressbar_create, tui_widget_t*, (tui_win_t* win, int x, int y, int w), (win, x, y, w))

// function: void tui_progressbar_set(tui_widget_t* pb, int value)
DECL_FC_API_CALL(FC_TUI_PROGRESSBAR_SET, tui_progressbar_set, void, (tui_widget_t* pb, int value), (pb, value))

// function: void tui_putc(int c)
DECL_FC_API_CALL(FC_TUI_PUTC, tui_putc, void, (int c), (c))

// function: void tui_puts(const char* s)
DECL_FC_API_CALL(FC_TUI_PUTS, tui_puts, void, (const char* s), (s))

// function: int tui_screen_height()
DECL_FC_API_CALL(FC_TUI_SCREEN_HEIGHT, tui_screen_height, int, (), ())

// function: int tui_screen_width()
DECL_FC_API_CALL(FC_TUI_SCREEN_WIDTH, tui_screen_width, int, (), ())

// function: void tui_set_color(int fg, int bg)
DECL_FC_API_CALL(FC_TUI_SET_COLOR, tui_set_color, void, (int fg, int bg), (fg, bg))

// function: void tui_set_focus(tui_widget_t* w)
DECL_FC_API_CALL(FC_TUI_SET_FOCUS, tui_set_focus, void, (tui_widget_t* w), (w))

// function: tui_widget_t* tui_textfield_create(tui_win_t* win, int x, int y, int w)
DECL_FC_API_CALL(FC_TUI_TEXTFIELD_CREATE, tui_textfield_create, tui_widget_t*, (tui_win_t* win, int x, int y, int w), (win, x, y, w))

// function: const char* tui_textfield_get(tui_widget_t* tf)
DECL_FC_API_CALL(FC_TUI_TEXTFIELD_GET, tui_textfield_get, const char*, (tui_widget_t* tf), (tf))

// function: void tui_textfield_set(tui_widget_t* tf, const char* text)
DECL_FC_API_CALL(FC_TUI_TEXTFIELD_SET, tui_textfield_set, void, (tui_widget_t* tf, const char* text), (tf, text))

// function: void tui_textfield_set_cursor(tui_widget_t* tf, int pos)
DECL_FC_API_CALL(FC_TUI_TEXTFIELD_SET_CURSOR, tui_textfield_set_cursor, void, (tui_widget_t* tf, int pos), (tf, pos))

// function: void tui_vline(int x, int y, int h)
DECL_FC_API_CALL(FC_TUI_VLINE, tui_vline, void, (int x, int y, int h), (x, y, h))

// function: void tui_widget_destroy(tui_widget_t* w)
DECL_FC_API_CALL(FC_TUI_WIDGET_DESTROY, tui_widget_destroy, void, (tui_widget_t* w), (w))

// function: void tui_widget_disable(tui_widget_t* w)
DECL_FC_API_CALL(FC_TUI_WIDGET_DISABLE, tui_widget_disable, void, (tui_widget_t* w), (w))

// function: void tui_widget_enable(tui_widget_t* w)
DECL_FC_API_CALL(FC_TUI_WIDGET_ENABLE, tui_widget_enable, void, (tui_widget_t* w), (w))

// function: void tui_widget_focus(tui_widget_t* w)
DECL_FC_API_CALL(FC_TUI_WIDGET_FOCUS, tui_widget_focus, void, (tui_widget_t* w), (w))

// function: void tui_widget_hide(tui_widget_t* w)
DECL_FC_API_CALL(FC_TUI_WIDGET_HIDE, tui_widget_hide, void, (tui_widget_t* w), (w))

// function: void tui_widget_show(tui_widget_t* w)
DECL_FC_API_CALL(FC_TUI_WIDGET_SHOW, tui_widget_show, void, (tui_widget_t* w), (w))

// function: void tui_win_close(tui_win_t* win)
DECL_FC_API_CALL(FC_TUI_WIN_CLOSE, tui_win_close, void, (tui_win_t* win), (win))

// function: void tui_win_gotoxy(tui_win_t* win, int x, int y)
DECL_FC_API_CALL(FC_TUI_WIN_GOTOXY, tui_win_gotoxy, void, (tui_win_t* win, int x, int y), (win, x, y))

// function: void tui_win_move(tui_win_t* win, int x, int y)
DECL_FC_API_CALL(FC_TUI_WIN_MOVE, tui_win_move, void, (tui_win_t* win, int x, int y), (win, x, y))

// function: tui_win_t* tui_win_open(int x, int y, int w, int h)
DECL_FC_API_CALL(FC_TUI_WIN_OPEN, tui_win_open, tui_win_t*, (int x, int y, int w, int h), (x, y, w, h))

// function: void tui_win_printf(tui_win_t* win, const char* fmt, ...)
DECL_FC_API_CALL(FC_TUI_WIN_PRINTF, tui_win_printf, void, (tui_win_t* win, const char* fmt, ...), (win, fmt))

// function: void tui_win_putc(tui_win_t* win, int c)
DECL_FC_API_CALL(FC_TUI_WIN_PUTC, tui_win_putc, void, (tui_win_t* win, int c), (win, c))

// function: void tui_win_puts(tui_win_t* win, const char* s)
DECL_FC_API_CALL(FC_TUI_WIN_PUTS, tui_win_puts, void, (tui_win_t* win, const char* s), (win, s))

// function: void tui_win_resize(tui_win_t* win, int w, int h)
DECL_FC_API_CALL(FC_TUI_WIN_RESIZE, tui_win_resize, void, (tui_win_t* win, int w, int h), (win, w, h))

// function: void tui_win_scroll(tui_win_t* win, int lines)
DECL_FC_API_CALL(FC_TUI_WIN_SCROLL, tui_win_scroll, void, (tui_win_t* win, int lines), (win, lines))

// function: void tui_win_set_colors(tui_win_t* win, int fg, int bg)
DECL_FC_API_CALL(FC_TUI_WIN_SET_COLORS, tui_win_set_colors, void, (tui_win_t* win, int fg, int bg), (win, fg, bg))

// function: void tui_win_set_title(tui_win_t* win, const char* title)
DECL_FC_API_CALL(FC_TUI_WIN_SET_TITLE, tui_win_set_title, void, (tui_win_t* win, const char* title), (win, title))

// function: void tui_win_set_border(tui_win_t* win, int flags)
DECL_FC_API_CALL(FC_TUI_WIN_SET_BORDER, tui_win_set_border, void, (tui_win_t* win, int flags), (win, flags))

// function: void fc_exit(int status)
DECL_FC_API_CALL(FC_FC_EXIT, fc_exit, void, (int status), (status))

#endif
