#ifndef _FC_API_H
#define _FC_API_H 1

/*
 * Expected signature of main function 
 * non-zero return indicates error to ForceCommand.
 *   This may trigger additional environment restoration upon return.
 * command line parameters are passed as a single un-parsed buffer.
 */
int main(char* args);

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

#define FC_TERM_PUTC 0x0
#define FC_TERM_PUTS 0x1
#define FC_TERM_GETS 0x2
#define FC_TERM_KSCAN 0x3
#define FC_SAMS_MAP_PAGE 0x4
#define FC_SAMS_ALLOC_PAGES 0x5
#define FC_SAMS_FREE_PAGES 0x6
#define FC_SAMS_READ_PAGE 0x7
#define FC_SYS_INFO 0x8
#define FC_SYS_DISPLAY_INFO 0x9
#define FC_SYS_SAMS_INFO 0xa
#define FC_VAR_GET 0xb
#define FC_VAR_SET 0xc
#define FC_EXEC_CMD 0xd
#define FC_DSR_EA5_LOAD 0xe
#define FC_DSR_OPEN 0xf
#define FC_DSR_CLOSE 0x10
#define FC_DSR_READ 0x11
#define FC_DSR_READ_CPU 0x12
#define FC_DSR_WRITE 0x13
#define FC_DSR_STATUS 0x14
#define FC_DSR_RESET 0x15
#define FC_DSR_DELETE 0x16
#define FC_DSR_PRG_LOAD 0x17
#define FC_DSR_PRG_SAVE 0x18
#define FC_DSR_SCRATCH 0x19
#define FC_DSR_CATALOG 0x1a
#define FC_PATH_TO_IOCODE 0x1b
#define FC_LVL2_INPUT 0x1c
#define FC_LVL2_INPUT_CPU 0x1d
#define FC_LVL2_OUTPUT 0x1e
#define FC_LVL2_OUTPUT_CPU 0x1f
#define FC_LVL2_PROTECT 0x20
#define FC_LVL2_RENAME 0x21
#define FC_LVL2_SETDIR 0x22
#define FC_LVL2_MKDIR 0x23
#define FC_LVL2_RMDIR 0x24
#define FC_LVL2_RENDIR 0x25
#define FC_LVL2_SECTOR_READ 0x26
#define FC_LVL2_SECTOR_WRITE 0x27
#define FC_LVL2_FORMAT 0x28
#define FC_TCP_CONNECT 0x29
#define FC_TCP_CLOSE 0x2a
#define FC_TCP_READ_SOCKET 0x2b
#define FC_TCP_SEND_CHARS 0x2c
#define FC_TIPI_ON 0x2d
#define FC_TIPI_OFF 0x2e
#define FC_TIPI_SENDMSG 0x2f
#define FC_TIPI_RECVMSG 0x30
#define FC_TIME_GET 0x31
#define FC_DSR_FIND 0x32
#define FC_PATH_PARSE 0x33
#define FC_STR_FROM_UINT 0x34
#define FC_HEX_FROM_UINT 0x35
#define FC_STR_TO_INT 0x36
#define FC_HEX_TO_INT 0x37
#define FC_STR_FROM_FLOAT 0x38
#define FC_STR_COPY 0x39
#define FC_STR_NCOPY 0x3a
#define FC_STR_CAT 0x3b
#define FC_STR_TOKEN 0x3c
#define FC_STR_TOKEN_PEEK 0x3d
#define FC_STR_LEN 0x3e
#define FC_STR_CMP 0x3f
#define FC_STR_CMP_ICASE 0x40
#define FC_STR_INDEX_OF 0x41
#define FC_STR_LAST_INDEX_OF 0x42
#define FC_STR_STARTSWITH 0x43
#define FC_STR_ENDSWITH 0x44
#define FC_STR_FROM_BASIC 0x45
#define FC_STR_SET 0x46
#define FC_STR_TOKEN_NEXT 0x47
#define FC_AUDIO_BEEP 0x48
#define FC_AUDIO_HONK 0x49
#define FC_TERM_SET_IDENTIFY_HOOK 0x4a
#define FC_VDP_SETCHAR 0x4b
#define FC_VDP_CURSOR_ADDR 0x4c
#define FC_UI_DROP_DOWN 0x4d
#define FC_UI_GOTOXY 0x4e
#define FC_SPEECH_RESET 0x4f
#define FC_SPEECH_DETECT 0x50
#define FC_SPEECH_SAY_VOCAB 0x51
#define FC_SPEECH_SAY_DATA 0x52
#define FC_SPEECH_START 0x53
#define FC_SPEECH_CONTINUE 0x54
#define FC_SPEECH_WAIT 0x55
#define FC_MOUSE_READ 0x56
#define FC_MOUSE_MOVE 0x57
#define FC_MOUSE_SHOW 0x58
#define FC_MOUSE_HIDE 0x59
#define FC_TLS_CONNECT 0x5a
#define FC_TLS_CLOSE 0x5b
#define FC_TLS_READ_SOCKET 0x5c
#define FC_TLS_SEND_CHARS 0x5d
#define FC_SOCKBUF_INIT 0x5e
#define FC_SOCKBUF_READLINE 0x5f
#define FC_SOCKBUF_READSTREAM 0x60
#define FC_LIST_INIT 0x61
#define FC_LIST_PUSH 0x62
#define FC_LIST_POP 0x63
#define FC_LIST_GET 0x64
#define FC_COLOR_TEXT 0x65
#define FC_COLOR_BG 0x66
#define FC_COLOR_BORDER 0x67
#define FC_TIPI_LOG 0x68
#define FC_SND_START 0x69
#define FC_SND_TICK 0x6a
#define FC_SND_PLAYING 0x6b
#define FC_SND_STOP 0x6c
#define FC_SND_PLAY 0x6d

// function: void term_putc(int c)
DECL_FC_API_CALL(FC_TERM_PUTC, term_putc, void, (int c), (c))

// function: void term_puts(const char* str)
DECL_FC_API_CALL(FC_TERM_PUTS, term_puts, void, (const char* str), (str))

// function: void term_gets(char* var, int limit, int backspace)
DECL_FC_API_CALL(FC_TERM_GETS, term_gets, void, (char* var, int limit, int backspace), (var, limit, backspace))

// function: unsigned int term_kscan(unsigned int mode)
DECL_FC_API_CALL(FC_TERM_KSCAN, term_kscan, unsigned int, (unsigned int mode), (mode))

// function: void sams_map_page(int page, int addr)
DECL_FC_API_CALL(FC_SAMS_MAP_PAGE, sams_map_page, void, (int page, int addr), (page, addr))

// function: int sams_alloc_pages(int count)
DECL_FC_API_CALL(FC_SAMS_ALLOC_PAGES, sams_alloc_pages, int, (int count), (count))

// function: int sams_free_pages(int count)
DECL_FC_API_CALL(FC_SAMS_FREE_PAGES, sams_free_pages, int, (int count), (count))

// function: int sams_read_page(int slot)
DECL_FC_API_CALL(FC_SAMS_READ_PAGE, sams_read_page, int, (int slot), (slot))

// function: void sys_info(struct SystemInformation* info)
DECL_FC_API_CALL(FC_SYS_INFO, sys_info, void, (struct SystemInformation* info), (info))

// function: void sys_display_info(struct DisplayInformation * info)
DECL_FC_API_CALL(FC_SYS_DISPLAY_INFO, sys_display_info, void, (struct DisplayInformation * info), (info))

// function: void sys_sams_info(struct SamsInformation* info)
DECL_FC_API_CALL(FC_SYS_SAMS_INFO, sys_sams_info, void, (struct SamsInformation* info), (info))

// function: char* var_get(char* name)
DECL_FC_API_CALL(FC_VAR_GET, var_get, char*, (char* name), (name))

// function: void var_set(char* name, char* value)
DECL_FC_API_CALL(FC_VAR_SET, var_set, void, (char* name, char* value), (name, value))

// function: int exec_cmd(char* command)
DECL_FC_API_CALL(FC_EXEC_CMD, exec_cmd, int, (char* command), (command))

// function: void dsr_ea5_load(struct DeviceServiceRoutine * dsr, const char *fname)
DECL_FC_API_CALL(FC_DSR_EA5_LOAD, dsr_ea5_load, void, (struct DeviceServiceRoutine * dsr, const char *fname), (dsr, fname))

// function: unsigned int dsr_open(struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int flags, int reclen)
DECL_FC_API_CALL(FC_DSR_OPEN, dsr_open, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int flags, int reclen), (dsr, pab, fname, flags, reclen))

// function: unsigned int dsr_close(struct DeviceServiceRoutine* dsr, struct PAB* pab)
DECL_FC_API_CALL(FC_DSR_CLOSE, dsr_close, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab), (dsr, pab))

// function: unsigned int dsr_read(struct DeviceServiceRoutine* dsr, struct PAB* pab, int recordNumber)
DECL_FC_API_CALL(FC_DSR_READ, dsr_read, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab, int recordNumber), (dsr, pab, recordNumber))

// function: unsigned int dsr_read_cpu(struct DeviceServiceRoutine* dsr, struct PAB* pab, int recordNumber, char* recordBuf)
DECL_FC_API_CALL(FC_DSR_READ_CPU, dsr_read_cpu, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab, int recordNumber, char* recordBuf), (dsr, pab, recordNumber, recordBuf))

// function: unsigned int dsr_write(struct DeviceServiceRoutine* dsr, struct PAB* pab, char* record, int reclen)
DECL_FC_API_CALL(FC_DSR_WRITE, dsr_write, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab, char* record, int reclen), (dsr, pab, record, reclen))

// function: unsigned int dsr_status(struct DeviceServiceRoutine* dsr, struct PAB* pab)
DECL_FC_API_CALL(FC_DSR_STATUS, dsr_status, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab), (dsr, pab))

// function: unsigned int dsr_reset(struct DeviceServiceRoutine* dsr, struct PAB* pab, int record)
DECL_FC_API_CALL(FC_DSR_RESET, dsr_reset, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab, int record), (dsr, pab, record))

// function: unsigned int dsr_delete(struct DeviceServiceRoutine* dsr, struct PAB* pab)
DECL_FC_API_CALL(FC_DSR_DELETE, dsr_delete, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab), (dsr, pab))

// function: unsigned int dsr_prg_load(struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int vdpaddr, int maxsize)
DECL_FC_API_CALL(FC_DSR_PRG_LOAD, dsr_prg_load, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int vdpaddr, int maxsize), (dsr, pab, fname, vdpaddr, maxsize))

// function: unsigned int dsr_prg_save(struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int vdpaddr, int count)
DECL_FC_API_CALL(FC_DSR_PRG_SAVE, dsr_prg_save, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int vdpaddr, int count), (dsr, pab, fname, vdpaddr, count))

// function: unsigned int dsr_scratch(struct DeviceServiceRoutine* dsr, struct PAB* pab, int record)
DECL_FC_API_CALL(FC_DSR_SCRATCH, dsr_scratch, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab, int record), (dsr, pab, record))

// function: unsigned int dsr_catalog(struct DeviceServiceRoutine* dsr, const char* pathname, vol_entry_cb vol_cb, dir_entry_cb dir_cb)
DECL_FC_API_CALL(FC_DSR_CATALOG, dsr_catalog, unsigned int, (struct DeviceServiceRoutine* dsr, const char* pathname, vol_entry_cb vol_cb, dir_entry_cb dir_cb), (dsr, pathname, vol_cb, dir_cb))

// function: unsigned int path_to_iocode(const char* currentPath)
DECL_FC_API_CALL(FC_PATH_TO_IOCODE, path_to_iocode, unsigned int, (const char* currentPath), (currentPath))

// function: unsigned int lvl2_input(int crubase, unsigned int iocode, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr)
DECL_FC_API_CALL(FC_LVL2_INPUT, lvl2_input, unsigned int, (int crubase, unsigned int iocode, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr), (crubase, iocode, filename, blockcount, addInfoPtr))

// function: unsigned int lvl2_input_cpu(int crubase, unsigned int iocode, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr)
DECL_FC_API_CALL(FC_LVL2_INPUT_CPU, lvl2_input_cpu, unsigned int, (int crubase, unsigned int iocode, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr), (crubase, iocode, filename, blockcount, addInfoPtr))

// function: unsigned int lvl2_output(int crubase, unsigned int iocode, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr)
DECL_FC_API_CALL(FC_LVL2_OUTPUT, lvl2_output, unsigned int, (int crubase, unsigned int iocode, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr), (crubase, iocode, filename, blockcount, addInfoPtr))

// function: unsigned int lvl2_output_cpu(int crubase, unsigned int iocode, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr)
DECL_FC_API_CALL(FC_LVL2_OUTPUT_CPU, lvl2_output_cpu, unsigned int, (int crubase, unsigned int iocode, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr), (crubase, iocode, filename, blockcount, addInfoPtr))

// function: unsigned int lvl2_protect(int crubase, unsigned int iocode, char *filename, int protect)
DECL_FC_API_CALL(FC_LVL2_PROTECT, lvl2_protect, unsigned int, (int crubase, unsigned int iocode, char *filename, int protect), (crubase, iocode, filename, protect))

// function: unsigned int lvl2_rename(int crubase, unsigned int iocode, char *oldname, char *newname)
DECL_FC_API_CALL(FC_LVL2_RENAME, lvl2_rename, unsigned int, (int crubase, unsigned int iocode, char *oldname, char *newname), (crubase, iocode, oldname, newname))

// function: unsigned int lvl2_setdir(int crubase, unsigned int iocode, char* path)
DECL_FC_API_CALL(FC_LVL2_SETDIR, lvl2_setdir, unsigned int, (int crubase, unsigned int iocode, char* path), (crubase, iocode, path))

// function: unsigned int lvl2_mkdir(int crubase, unsigned int iocode, char *dirname)
DECL_FC_API_CALL(FC_LVL2_MKDIR, lvl2_mkdir, unsigned int, (int crubase, unsigned int iocode, char *dirname), (crubase, iocode, dirname))

// function: unsigned int lvl2_rmdir(int crubase, unsigned int iocode, char *dirname)
DECL_FC_API_CALL(FC_LVL2_RMDIR, lvl2_rmdir, unsigned int, (int crubase, unsigned int iocode, char *dirname), (crubase, iocode, dirname))

// function: unsigned int lvl2_rendir(int crubase, unsigned int iocode, char *oldname, char *newname)
DECL_FC_API_CALL(FC_LVL2_RENDIR, lvl2_rendir, unsigned int, (int crubase, unsigned int iocode, char *oldname, char *newname), (crubase, iocode, oldname, newname))

// function: unsigned int lvl2_sector_read(int crubase, unsigned int iocode, unsigned int sector, char* bufaddr)
DECL_FC_API_CALL(FC_LVL2_SECTOR_READ, lvl2_sector_read, unsigned int, (int crubase, unsigned int iocode, unsigned int sector, char* bufaddr), (crubase, iocode, sector, bufaddr))

// function: unsigned int lvl2_sector_write(int crubase, unsigned int iocode, unsigned int sector, char*bufaddr)
DECL_FC_API_CALL(FC_LVL2_SECTOR_WRITE, lvl2_sector_write, unsigned int, (int crubase, unsigned int iocode, unsigned int sector, char*bufaddr), (crubase, iocode, sector, bufaddr))

// function: unsigned int lvl2_format(int crubase, unsigned int iocode, unsigned int tracks, unsigned int density, unsigned int sides, unsigned int interleave)
DECL_FC_API_CALL(FC_LVL2_FORMAT, lvl2_format, unsigned int, (int crubase, unsigned int iocode, unsigned int tracks, unsigned int density, unsigned int sides, unsigned int interleave), (crubase, iocode, tracks, density, sides, interleave))

// function: unsigned int tcp_connect(unsigned int socketId, char* hostname, char* port)
DECL_FC_API_CALL(FC_TCP_CONNECT, tcp_connect, unsigned int, (unsigned int socketId, char* hostname, char* port), (socketId, hostname, port))

// function: unsigned int tcp_close(unsigned int socketId)
DECL_FC_API_CALL(FC_TCP_CLOSE, tcp_close, unsigned int, (unsigned int socketId), (socketId))

// function: int tcp_read_socket(unsigned int socketId, char* buf, int bufsize)
DECL_FC_API_CALL(FC_TCP_READ_SOCKET, tcp_read_socket, int, (unsigned int socketId, char* buf, int bufsize), (socketId, buf, bufsize))

// function: int tcp_send_chars(unsigned int socketId, char* buf, int size)
DECL_FC_API_CALL(FC_TCP_SEND_CHARS, tcp_send_chars, int, (unsigned int socketId, char* buf, int size), (socketId, buf, size))

// function: int tipi_on()
DECL_FC_API_CALL(FC_TIPI_ON, tipi_on, int, (), ())

// function: void tipi_off()
DECL_FC_API_CALL(FC_TIPI_OFF, tipi_off, void, (), ())

// function: void tipi_sendmsg(unsigned int len, const char* buf)
DECL_FC_API_CALL(FC_TIPI_SENDMSG, tipi_sendmsg, void, (unsigned int len, const char* buf), (len, buf))

// function: void tipi_recvmsg(unsigned int* len, char* buf)
DECL_FC_API_CALL(FC_TIPI_RECVMSG, tipi_recvmsg, void, (unsigned int* len, char* buf), (len, buf))

// function: void time_get(struct DateTime* dt)
DECL_FC_API_CALL(FC_TIME_GET, time_get, void, (struct DateTime* dt), (dt))

// function: struct DeviceServiceRoutine* dsr_find(char* devicename, int crubase)
DECL_FC_API_CALL(FC_DSR_FIND, dsr_find, struct DeviceServiceRoutine*, (char* devicename, int crubase), (devicename, crubase))

// function: void path_parse(char* str_in, struct DeviceServiceRoutine** dsr, char* buffer, int requirements)
DECL_FC_API_CALL(FC_PATH_PARSE, path_parse, void, (char* str_in, struct DeviceServiceRoutine** dsr, char* buffer, int requirements), (str_in, dsr, buffer, requirements))

// function: char * str_from_uint(unsigned int x)
DECL_FC_API_CALL(FC_STR_FROM_UINT, str_from_uint, char *, (unsigned int x), (x))

// function: char * hex_from_uint(unsigned int x)
DECL_FC_API_CALL(FC_HEX_FROM_UINT, hex_from_uint, char *, (unsigned int x), (x))

// function: int str_to_int(char *s)
DECL_FC_API_CALL(FC_STR_TO_INT, str_to_int, int, (char *s), (s))

// function: int hex_to_int(char* s)
DECL_FC_API_CALL(FC_HEX_TO_INT, hex_to_int, int, (char* s), (s))

// function: char* str_from_float(double a)
DECL_FC_API_CALL(FC_STR_FROM_FLOAT, str_from_float, char*, (double a), (a))

// function: char * str_copy(char *d, const char *s)
DECL_FC_API_CALL(FC_STR_COPY, str_copy, char *, (char *d, const char *s), (d, s))

// function: char * str_ncopy(char *dest, char *src, int limit)
DECL_FC_API_CALL(FC_STR_NCOPY, str_ncopy, char *, (char *dest, char *src, int limit), (dest, src, limit))

// function: char * str_cat(char *dst, const char *add)
DECL_FC_API_CALL(FC_STR_CAT, str_cat, char *, (char *dst, const char *add), (dst, add))

// function: char * str_token(char *str, int delim)
DECL_FC_API_CALL(FC_STR_TOKEN, str_token, char *, (char *str, int delim), (str, delim))

// function: char * str_token_peek(char *str, int delim)
DECL_FC_API_CALL(FC_STR_TOKEN_PEEK, str_token_peek, char *, (char *str, int delim), (str, delim))

// function: int str_len(const char *s)
DECL_FC_API_CALL(FC_STR_LEN, str_len, int, (const char *s), (s))

// function: int str_cmp(const char *a, const char *b)
DECL_FC_API_CALL(FC_STR_CMP, str_cmp, int, (const char *a, const char *b), (a, b))

// function: int str_cmp_icase(const char *a, const char *b)
DECL_FC_API_CALL(FC_STR_CMP_ICASE, str_cmp_icase, int, (const char *a, const char *b), (a, b))

// function: int str_index_of(const char *str, int c)
DECL_FC_API_CALL(FC_STR_INDEX_OF, str_index_of, int, (const char *str, int c), (str, c))

// function: int str_last_index_of(const char *str, int c, int start)
DECL_FC_API_CALL(FC_STR_LAST_INDEX_OF, str_last_index_of, int, (const char *str, int c, int start), (str, c, start))

// function: int str_startswith(const char *str, const char *prefix)
DECL_FC_API_CALL(FC_STR_STARTSWITH, str_startswith, int, (const char *str, const char *prefix), (str, prefix))

// function: int str_endswith(const char *str, const char *suffix)
DECL_FC_API_CALL(FC_STR_ENDSWITH, str_endswith, int, (const char *str, const char *suffix), (str, suffix))

// function: int str_from_basic(const char *str, char *buf)
DECL_FC_API_CALL(FC_STR_FROM_BASIC, str_from_basic, int, (const char *str, char *buf), (str, buf))

// function: void str_set(char *buffer, int value, int limit)
DECL_FC_API_CALL(FC_STR_SET, str_set, void, (char *buffer, int value, int limit), (buffer, value, limit))

// function: char* str_token_next(char* dst, char* str, int delim)
DECL_FC_API_CALL(FC_STR_TOKEN_NEXT, str_token_next, char*, (char* dst, char* str, int delim), (dst, str, delim))

// function: void audio_beep()
DECL_FC_API_CALL(FC_AUDIO_BEEP, audio_beep, void, (), ())

// function: void audio_honk()
DECL_FC_API_CALL(FC_AUDIO_HONK, audio_honk, void, (), ())

// function: void term_set_identify_hook(identify_callback cb)
DECL_FC_API_CALL(FC_TERM_SET_IDENTIFY_HOOK, term_set_identify_hook, void, (identify_callback cb), (cb))

// function: void vdp_setchar(int pAddr, int ch)
DECL_FC_API_CALL(FC_VDP_SETCHAR, vdp_setchar, void, (int pAddr, int ch), (pAddr, ch))

// function: unsigned int vdp_cursor_addr()
DECL_FC_API_CALL(FC_VDP_CURSOR_ADDR, vdp_cursor_addr, unsigned int, (), ())

// function: void ui_drop_down(int linecount)
DECL_FC_API_CALL(FC_UI_DROP_DOWN, ui_drop_down, void, (int linecount), (linecount))

// function: void ui_gotoxy(int x, int y)
DECL_FC_API_CALL(FC_UI_GOTOXY, ui_gotoxy, void, (int x, int y), (x, y))

// function: void speech_reset()
DECL_FC_API_CALL(FC_SPEECH_RESET, speech_reset, void, (), ())

// function: int speech_detect()
DECL_FC_API_CALL(FC_SPEECH_DETECT, speech_detect, int, (), ())

// function: void speech_say_vocab(int phrase_addr)
DECL_FC_API_CALL(FC_SPEECH_SAY_VOCAB, speech_say_vocab, void, (int phrase_addr), (phrase_addr))

// function: void speech_say_data(const char* addr, int len)
DECL_FC_API_CALL(FC_SPEECH_SAY_DATA, speech_say_data, void, (const char* addr, int len), (addr, len))

// function: void speech_start(struct LpcPlaybackCtx* ctx)
DECL_FC_API_CALL(FC_SPEECH_START, speech_start, void, (struct LpcPlaybackCtx* ctx), (ctx))

// function: void speech_continue(struct LpcPlaybackCtx* ctx)
DECL_FC_API_CALL(FC_SPEECH_CONTINUE, speech_continue, void, (struct LpcPlaybackCtx* ctx), (ctx))

// function: void speech_wait()
DECL_FC_API_CALL(FC_SPEECH_WAIT, speech_wait, void, (), ())

// function: void mouse_read(struct MouseData* mouseData)
DECL_FC_API_CALL(FC_MOUSE_READ, mouse_read, void, (struct MouseData* mouseData), (mouseData))

// function: void mouse_move(struct MouseData* mouseData)
DECL_FC_API_CALL(FC_MOUSE_MOVE, mouse_move, void, (struct MouseData* mouseData), (mouseData))

// function: void mouse_show(struct MouseData* mouseData)
DECL_FC_API_CALL(FC_MOUSE_SHOW, mouse_show, void, (struct MouseData* mouseData), (mouseData))

// function: void mouse_hide()
DECL_FC_API_CALL(FC_MOUSE_HIDE, mouse_hide, void, (), ())

// function: unsigned int tls_connect(unsigned int socketId, char* hostname, char* port)
DECL_FC_API_CALL(FC_TLS_CONNECT, tls_connect, unsigned int, (unsigned int socketId, char* hostname, char* port), (socketId, hostname, port))

// function: unsigned int tls_close(unsigned int socketId)
DECL_FC_API_CALL(FC_TLS_CLOSE, tls_close, unsigned int, (unsigned int socketId), (socketId))

// function: int tls_read_socket(unsigned int socketId, char* buf, int bufsize)
DECL_FC_API_CALL(FC_TLS_READ_SOCKET, tls_read_socket, int, (unsigned int socketId, char* buf, int bufsize), (socketId, buf, bufsize))

// function: int tls_send_chars(unsigned int socketId, char* buf, int size)
DECL_FC_API_CALL(FC_TLS_SEND_CHARS, tls_send_chars, int, (unsigned int socketId, char* buf, int size), (socketId, buf, size))

// function: void sockbuf_init(struct SocketBuffer* socket_buf, int tls, unsigned int socketId)
DECL_FC_API_CALL(FC_SOCKBUF_INIT, sockbuf_init, void, (struct SocketBuffer* socket_buf, int tls, unsigned int socketId), (socket_buf, tls, socketId))

// function: char* sockbuf_readline(struct SocketBuffer* socket_buf)
DECL_FC_API_CALL(FC_SOCKBUF_READLINE, sockbuf_readline, char*, (struct SocketBuffer* socket_buf), (socket_buf))

// function: int sockbuf_readstream(struct SocketBuffer* socket_buf, char* block, int limit)
DECL_FC_API_CALL(FC_SOCKBUF_READSTREAM, sockbuf_readstream, int, (struct SocketBuffer* socket_buf, char* block, int limit), (socket_buf, block, limit))

// function: void list_init(struct List* list, void* addr, void* ceiling)
DECL_FC_API_CALL(FC_LIST_INIT, list_init, void, (struct List* list, void* addr, void* ceiling), (list, addr, ceiling))

// function: void list_push(struct List* list, char* buffer, int length)
DECL_FC_API_CALL(FC_LIST_PUSH, list_push, void, (struct List* list, char* buffer, int length), (list, buffer, length))

// function: void list_pop(struct List* list, char* buffer, int limit)
DECL_FC_API_CALL(FC_LIST_POP, list_pop, void, (struct List* list, char* buffer, int limit), (list, buffer, limit))

// function: struct ListEntry* list_get(struct List* list, int index)
DECL_FC_API_CALL(FC_LIST_GET, list_get, struct ListEntry*, (struct List* list, int index), (list, index))

// function: unsigned int color_text(unsigned int color)
DECL_FC_API_CALL(FC_COLOR_TEXT, color_text, unsigned int, (unsigned int color), (color))

// function: unsigned int color_bg(unsigned int color)
DECL_FC_API_CALL(FC_COLOR_BG, color_bg, unsigned int, (unsigned int color), (color))

// function: unsigned int color_border(unsigned int x)
DECL_FC_API_CALL(FC_COLOR_BORDER, color_border, unsigned int, (unsigned int x), (x))

// function: void tipi_log(char* msg)
DECL_FC_API_CALL(FC_TIPI_LOG, tipi_log, void, (char* msg), (msg))

// function: void snd_start(const unsigned char *list)
DECL_FC_API_CALL(FC_SND_START, snd_start, void, (const unsigned char *list), (list))

// function: void snd_tick()
DECL_FC_API_CALL(FC_SND_TICK, snd_tick, void, (), ())

// function: int snd_playing()
DECL_FC_API_CALL(FC_SND_PLAYING, snd_playing, int, (), ())

// function: void snd_stop()
DECL_FC_API_CALL(FC_SND_STOP, snd_stop, void, (), ())

// function: void snd_play(const unsigned char *list)
DECL_FC_API_CALL(FC_SND_PLAY, snd_play, void, (const unsigned char *list), (list))

#endif
