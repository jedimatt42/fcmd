#ifndef _FC_API_H
#define _FC_API_H 1

#include <types.h>

/*
 * Expected signature of main function 
 * non-zero return indicates error to ForceCommand.
 *   This may trigger additional environment restoration upon return.
 * command line parameters are passed as a single un-parsed buffer.
 */
int fc_main(char* args);

#define FC_SYS *(int *)0x6080

#define DECL_FC_API_CALL(index, func, return_type, arg_sig, args)     \
    static inline return_type func arg_sig                            \
    {                                                                 \
        __asm__("li r0,%0"                                            \
                :                                                     \
                : "i"(index));                                        \
        return_type(*tramp) arg_sig = (return_type(*) arg_sig)FC_SYS; \
        return tramp args;                                            \
    }

/*
  Device Service Routine pre-loaded entry
*/
struct __attribute__((__packed__)) DeviceServiceRoutine {
    char name[8];
    int crubase;
    unsigned int addr;
    char unit;
};

/*
  System information structure
*/
struct __attribute__((__packed__)) SystemInformation {
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

/*
  Display parameter structure
*/
struct __attribute__((__packed__)) DisplayInformation {
  int isPal;
  int vdp_type;
  int displayWidth;
  int displayHeight;
  int imageAddr;
  int patternAddr;
  int colorAddr;
  int spritePatternAddr;
};

struct __attribute__((__packed__)) SamsInformation {
  int next_page;
  int total_pages;
};

/*
  Peripheral Access Block
*/
struct __attribute__((__packed__)) PAB {
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
struct __attribute__((__packed__)) AddInfo {
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
struct __attribute__((__packed__)) DirEntry {
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

struct __attribute__((__packed__)) VolInfo {
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
struct __attribute__((__packed__)) DateTime {
  unsigned char dayOfWeek;
  unsigned int year;
  unsigned char month;
  unsigned char day;
  unsigned char hours;
  unsigned char minutes;
  unsigned char seconds;
  unsigned char pm;
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
struct __attribute__((__packed__)) LpcPlaybackCtx {
    char* addr;
    int remaining;
};

/*
  TIPI Mouse return data and butten masks
 */
struct __attribute__((__packed__)) MouseData {
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
struct __attribute__((__packed__)) SocketBuffer {
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
  Rom address tables
*/

// memcpy is special, it is in all bank of the cartridge
extern void* memcpy(void* dest, const void* src, int count);

#define FC_TPUTC 0x6082
#define FC_TPUTS 0x6086
#define FC_GETSTR 0x608a
#define FC_KSCAN 0x608e
#define FC_SAMS_MAP_PAGE 0x6092
#define FC_SAMS_ALLOC_PAGES 0x6096
#define FC_SAMS_FREE_PAGES 0x609a
#define FC_SYS_INFO 0x609e
#define FC_DISPLAY_INFO 0x60a2
#define FC_SAMS_INFO 0x60a6
#define FC_VARS_GET 0x60aa
#define FC_VARS_SET 0x60ae
#define FC_EXEC 0x60b2
#define FC_DSR_EA5LOAD 0x60b6
#define FC_DSR_OPEN 0x60ba
#define FC_DSR_CLOSE 0x60be
#define FC_DSR_READ 0x60c2
#define FC_DSR_WRITE 0x60c6
#define FC_DSR_STATUS 0x60ca
#define FC_DSR_RESET 0x60ce
#define FC_DSR_DELETE 0x60d2
#define FC_DSR_CATALOG 0x60d6
#define FC_PATH2IOCODE 0x60da
#define FC_LVL2_INPUT 0x60de
#define FC_LVL2_OUTPUT 0x60e2
#define FC_LVL2_PROTECT 0x60e6
#define FC_LVL2_RENAME 0x60ea
#define FC_LVL2_SETDIR 0x60ee
#define FC_LVL2_MKDIR 0x60f2
#define FC_LVL2_RMDIR 0x60f6
#define FC_LVL2_RENDIR 0x60fa
#define FC_TCP_CONNECT 0x60fe
#define FC_TCP_CLOSE 0x6102
#define FC_TCP_READ_SOCKET 0x6106
#define FC_TCP_SEND_CHARS 0x610a
#define FC_TIPI_ON 0x610e
#define FC_TIPI_OFF 0x6112
#define FC_TIPI_SENDMSG 0x6116
#define FC_TIPI_RECVMSG 0x611a
#define FC_DATETIME 0x611e
#define FC_FIND_DSR 0x6122
#define FC_PARSE_PATH_PARAM 0x6126
#define FC_UINT2STR 0x612a
#define FC_UINT2HEX 0x612e
#define FC_ATOI 0x6132
#define FC_HTOI 0x6136
#define FC_STRCPY 0x613a
#define FC_STRNCPY 0x613e
#define FC_STRCAT 0x6142
#define FC_STRTOK 0x6146
#define FC_STRTOKPEEK 0x614a
#define FC_STRLEN 0x614e
#define FC_STRCMP 0x6152
#define FC_STRCMPI 0x6156
#define FC_INDEXOF 0x615a
#define FC_LINDEXOF 0x615e
#define FC_STR_STARTSWITH 0x6162
#define FC_STR_ENDSWITH 0x6166
#define FC_BASICTOCSTR 0x616a
#define FC_STRSET 0x616e
#define FC_NEXT_TOKEN 0x6172
#define FC_BEEP 0x6176
#define FC_HONK 0x617a
#define FC_SET_IDENTIFY_HOOK 0x617e
#define FC_VDP_SETCHAR 0x6182
#define FC_VDP_GET_CURSOR_ADDR 0x6186
#define FC_UI_DROP_DOWN 0x618a
#define FC_UI_GOTOXY 0x618e
#define FC_SPEECH_RESET 0x6192
#define FC_DETECT_SPEECH 0x6196
#define FC_SAY_VOCAB 0x619a
#define FC_SAY_DATA 0x619e
#define FC_SPEECH_START 0x61a2
#define FC_SPEECH_CONTINUE 0x61a6
#define FC_SPEECH_WAIT 0x61aa
#define FC_TIPI_MOUSE 0x61ae
#define FC_TIPI_MOUSE_MOVE 0x61b2
#define FC_TIPI_MOUSE_ENABLE 0x61b6
#define FC_TIPI_MOUSE_DISABLE 0x61ba
#define FC_TLS_CONNECT 0x61be
#define FC_TLS_CLOSE 0x61c2
#define FC_TLS_READ_SOCKET 0x61c6
#define FC_TLS_SEND_CHARS 0x61ca
#define FC_INIT_SOCKET_BUFFER 0x61ce
#define FC_READLINE 0x61d2
#define FC_READSTREAM 0x61d6

// function: void fc_tputc(int c)
DECL_FC_API_CALL(FC_TPUTC, fc_tputc, void, (int c), (c))

// function: void fc_tputs(const char* str)
DECL_FC_API_CALL(FC_TPUTS, fc_tputs, void, (const char* str), (str))

// function: void fc_getstr(char* var, int limit, int backspace)
DECL_FC_API_CALL(FC_GETSTR, fc_getstr, void, (char* var, int limit, int backspace), (var, limit, backspace))

// function: unsigned int fc_kscan(unsigned int mode)
DECL_FC_API_CALL(FC_KSCAN, fc_kscan, unsigned int, (unsigned int mode), (mode))

// function: void fc_sams_map_page(int page, int addr)
DECL_FC_API_CALL(FC_SAMS_MAP_PAGE, fc_sams_map_page, void, (int page, int addr), (page, addr))

// function: int fc_sams_alloc_pages(int count)
DECL_FC_API_CALL(FC_SAMS_ALLOC_PAGES, fc_sams_alloc_pages, int, (int count), (count))

// function: int fc_sams_free_pages(int count)
DECL_FC_API_CALL(FC_SAMS_FREE_PAGES, fc_sams_free_pages, int, (int count), (count))

// function: void fc_sys_info(struct SystemInformation* info)
DECL_FC_API_CALL(FC_SYS_INFO, fc_sys_info, void, (struct SystemInformation* info), (info))

// function: void fc_display_info(struct DisplayInformation * info)
DECL_FC_API_CALL(FC_DISPLAY_INFO, fc_display_info, void, (struct DisplayInformation * info), (info))

// function: void fc_sams_info(struct SamsInformation* info)
DECL_FC_API_CALL(FC_SAMS_INFO, fc_sams_info, void, (struct SamsInformation* info), (info))

// function: char* fc_vars_get(char* name)
DECL_FC_API_CALL(FC_VARS_GET, fc_vars_get, char*, (char* name), (name))

// function: void fc_vars_set(char* name, char* value)
DECL_FC_API_CALL(FC_VARS_SET, fc_vars_set, void, (char* name, char* value), (name, value))

// function: int16_t fc_exec(char* command)
DECL_FC_API_CALL(FC_EXEC, fc_exec, int16_t, (char* command), (command))

// function: unsigned int fc_dsr_ea5load(struct DeviceServiceRoutine * dsr, const char *fname)
DECL_FC_API_CALL(FC_DSR_EA5LOAD, fc_dsr_ea5load, unsigned int, (struct DeviceServiceRoutine * dsr, const char *fname), (dsr, fname))

// function: unsigned int fc_dsr_open(struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int flags, int reclen)
DECL_FC_API_CALL(FC_DSR_OPEN, fc_dsr_open, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int flags, int reclen), (dsr, pab, fname, flags, reclen))

// function: unsigned int fc_dsr_close(struct DeviceServiceRoutine* dsr, struct PAB* pab)
DECL_FC_API_CALL(FC_DSR_CLOSE, fc_dsr_close, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab), (dsr, pab))

// function: unsigned int fc_dsr_read(struct DeviceServiceRoutine* dsr, struct PAB* pab, int recordNumber)
DECL_FC_API_CALL(FC_DSR_READ, fc_dsr_read, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab, int recordNumber), (dsr, pab, recordNumber))

// function: unsigned int fc_dsr_write(struct DeviceServiceRoutine* dsr, struct PAB* pab, char* record, int reclen)
DECL_FC_API_CALL(FC_DSR_WRITE, fc_dsr_write, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab, char* record, int reclen), (dsr, pab, record, reclen))

// function: unsigned int fc_dsr_status(struct DeviceServiceRoutine* dsr, struct PAB* pab)
DECL_FC_API_CALL(FC_DSR_STATUS, fc_dsr_status, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab), (dsr, pab))

// function: unsigned int fc_dsr_reset(struct DeviceServiceRoutine* dsr, struct PAB* pab, int record)
DECL_FC_API_CALL(FC_DSR_RESET, fc_dsr_reset, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab, int record), (dsr, pab, record))

// function: unsigned int fc_dsr_delete(struct DeviceServiceRoutine* dsr, struct PAB* pab)
DECL_FC_API_CALL(FC_DSR_DELETE, fc_dsr_delete, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab), (dsr, pab))

// function: unsigned int fc_dsr_catalog(struct DeviceServiceRoutine* dsr, const char* pathname, vol_entry_cb vol_cb, dir_entry_cb dir_cb)
DECL_FC_API_CALL(FC_DSR_CATALOG, fc_dsr_catalog, unsigned int, (struct DeviceServiceRoutine* dsr, const char* pathname, vol_entry_cb vol_cb, dir_entry_cb dir_cb), (dsr, pathname, vol_cb, dir_cb))

// function: unsigned int fc_path2iocode(const char* currentPath)
DECL_FC_API_CALL(FC_PATH2IOCODE, fc_path2iocode, unsigned int, (const char* currentPath), (currentPath))

// function: unsigned int fc_lvl2_input(int crubase, unsigned int iocode, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr)
DECL_FC_API_CALL(FC_LVL2_INPUT, fc_lvl2_input, unsigned int, (int crubase, unsigned int iocode, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr), (crubase, iocode, filename, blockcount, addInfoPtr))

// function: unsigned int fc_lvl2_output(int crubase, unsigned int iocode, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr)
DECL_FC_API_CALL(FC_LVL2_OUTPUT, fc_lvl2_output, unsigned int, (int crubase, unsigned int iocode, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr), (crubase, iocode, filename, blockcount, addInfoPtr))

// function: unsigned int fc_lvl2_protect(int crubase, unsigned int iocode, char *filename, int protect)
DECL_FC_API_CALL(FC_LVL2_PROTECT, fc_lvl2_protect, unsigned int, (int crubase, unsigned int iocode, char *filename, int protect), (crubase, iocode, filename, protect))

// function: unsigned int fc_lvl2_rename(int crubase, unsigned int iocode, char *oldname, char *newname)
DECL_FC_API_CALL(FC_LVL2_RENAME, fc_lvl2_rename, unsigned int, (int crubase, unsigned int iocode, char *oldname, char *newname), (crubase, iocode, oldname, newname))

// function: unsigned int fc_lvl2_setdir(int crubase, unsigned int iocode, char* path)
DECL_FC_API_CALL(FC_LVL2_SETDIR, fc_lvl2_setdir, unsigned int, (int crubase, unsigned int iocode, char* path), (crubase, iocode, path))

// function: unsigned int fc_lvl2_mkdir(int crubase, unsigned int iocode, char *dirname)
DECL_FC_API_CALL(FC_LVL2_MKDIR, fc_lvl2_mkdir, unsigned int, (int crubase, unsigned int iocode, char *dirname), (crubase, iocode, dirname))

// function: unsigned int fc_lvl2_rmdir(int crubase, unsigned int iocode, char *dirname)
DECL_FC_API_CALL(FC_LVL2_RMDIR, fc_lvl2_rmdir, unsigned int, (int crubase, unsigned int iocode, char *dirname), (crubase, iocode, dirname))

// function: unsigned int fc_lvl2_rendir(int crubase, unsigned int iocode, char *oldname, char *newname)
DECL_FC_API_CALL(FC_LVL2_RENDIR, fc_lvl2_rendir, unsigned int, (int crubase, unsigned int iocode, char *oldname, char *newname), (crubase, iocode, oldname, newname))

// function: unsigned int fc_tcp_connect(unsigned int socketId, char* hostname, char* port)
DECL_FC_API_CALL(FC_TCP_CONNECT, fc_tcp_connect, unsigned int, (unsigned int socketId, char* hostname, char* port), (socketId, hostname, port))

// function: unsigned int fc_tcp_close(unsigned int socketId)
DECL_FC_API_CALL(FC_TCP_CLOSE, fc_tcp_close, unsigned int, (unsigned int socketId), (socketId))

// function: int fc_tcp_read_socket(unsigned int socketId, char* buf, int bufsize)
DECL_FC_API_CALL(FC_TCP_READ_SOCKET, fc_tcp_read_socket, int, (unsigned int socketId, char* buf, int bufsize), (socketId, buf, bufsize))

// function: int fc_tcp_send_chars(unsigned int socketId, char* buf, int size)
DECL_FC_API_CALL(FC_TCP_SEND_CHARS, fc_tcp_send_chars, int, (unsigned int socketId, char* buf, int size), (socketId, buf, size))

// function: int fc_tipi_on()
DECL_FC_API_CALL(FC_TIPI_ON, fc_tipi_on, int, (), ())

// function: void fc_tipi_off()
DECL_FC_API_CALL(FC_TIPI_OFF, fc_tipi_off, void, (), ())

// function: void fc_tipi_sendmsg(unsigned int len, const char* buf)
DECL_FC_API_CALL(FC_TIPI_SENDMSG, fc_tipi_sendmsg, void, (unsigned int len, const char* buf), (len, buf))

// function: void fc_tipi_recvmsg(unsigned int* len, char* buf)
DECL_FC_API_CALL(FC_TIPI_RECVMSG, fc_tipi_recvmsg, void, (unsigned int* len, char* buf), (len, buf))

// function: void fc_datetime(struct DateTime* dt)
DECL_FC_API_CALL(FC_DATETIME, fc_datetime, void, (struct DateTime* dt), (dt))

// function: struct DeviceServiceRoutine* fc_find_dsr(char* devicename, int crubase)
DECL_FC_API_CALL(FC_FIND_DSR, fc_find_dsr, struct DeviceServiceRoutine*, (char* devicename, int crubase), (devicename, crubase))

// function: void fc_parse_path_param(char* str_in, struct DeviceServiceRoutine** dsr, char* buffer, int requirements)
DECL_FC_API_CALL(FC_PARSE_PATH_PARAM, fc_parse_path_param, void, (char* str_in, struct DeviceServiceRoutine** dsr, char* buffer, int requirements), (str_in, dsr, buffer, requirements))

// function: char * fc_uint2str(unsigned int x)
DECL_FC_API_CALL(FC_UINT2STR, fc_uint2str, char *, (unsigned int x), (x))

// function: char * fc_uint2hex(unsigned int x)
DECL_FC_API_CALL(FC_UINT2HEX, fc_uint2hex, char *, (unsigned int x), (x))

// function: int fc_atoi(char *s)
DECL_FC_API_CALL(FC_ATOI, fc_atoi, int, (char *s), (s))

// function: int fc_htoi(char* s)
DECL_FC_API_CALL(FC_HTOI, fc_htoi, int, (char* s), (s))

// function: char * fc_strcpy(char *d, const char *s)
DECL_FC_API_CALL(FC_STRCPY, fc_strcpy, char *, (char *d, const char *s), (d, s))

// function: char * fc_strncpy(char *dest, char *src, int limit)
DECL_FC_API_CALL(FC_STRNCPY, fc_strncpy, char *, (char *dest, char *src, int limit), (dest, src, limit))

// function: char * fc_strcat(char *dst, const char *add)
DECL_FC_API_CALL(FC_STRCAT, fc_strcat, char *, (char *dst, const char *add), (dst, add))

// function: char * fc_strtok(char *str, int delim)
DECL_FC_API_CALL(FC_STRTOK, fc_strtok, char *, (char *str, int delim), (str, delim))

// function: char * fc_strtokpeek(char *str, int delim)
DECL_FC_API_CALL(FC_STRTOKPEEK, fc_strtokpeek, char *, (char *str, int delim), (str, delim))

// function: int fc_strlen(const char *s)
DECL_FC_API_CALL(FC_STRLEN, fc_strlen, int, (const char *s), (s))

// function: int fc_strcmp(const char *a, const char *b)
DECL_FC_API_CALL(FC_STRCMP, fc_strcmp, int, (const char *a, const char *b), (a, b))

// function: int fc_strcmpi(const char *a, const char *b)
DECL_FC_API_CALL(FC_STRCMPI, fc_strcmpi, int, (const char *a, const char *b), (a, b))

// function: int fc_indexof(const char *str, int c)
DECL_FC_API_CALL(FC_INDEXOF, fc_indexof, int, (const char *str, int c), (str, c))

// function: int fc_lindexof(const char *str, int c, int start)
DECL_FC_API_CALL(FC_LINDEXOF, fc_lindexof, int, (const char *str, int c, int start), (str, c, start))

// function: int fc_str_startswith(const char *str, const char *prefix)
DECL_FC_API_CALL(FC_STR_STARTSWITH, fc_str_startswith, int, (const char *str, const char *prefix), (str, prefix))

// function: int fc_str_endswith(const char *str, const char *suffix)
DECL_FC_API_CALL(FC_STR_ENDSWITH, fc_str_endswith, int, (const char *str, const char *suffix), (str, suffix))

// function: int fc_basicToCstr(const char *str, char *buf)
DECL_FC_API_CALL(FC_BASICTOCSTR, fc_basicToCstr, int, (const char *str, char *buf), (str, buf))

// function: void fc_strset(char *buffer, int value, int limit)
DECL_FC_API_CALL(FC_STRSET, fc_strset, void, (char *buffer, int value, int limit), (buffer, value, limit))

// function: char* fc_next_token(char* dst, char* str, int delim)
DECL_FC_API_CALL(FC_NEXT_TOKEN, fc_next_token, char*, (char* dst, char* str, int delim), (dst, str, delim))

// function: void fc_beep()
DECL_FC_API_CALL(FC_BEEP, fc_beep, void, (), ())

// function: void fc_honk()
DECL_FC_API_CALL(FC_HONK, fc_honk, void, (), ())

// function: void fc_set_identify_hook(identify_callback cb)
DECL_FC_API_CALL(FC_SET_IDENTIFY_HOOK, fc_set_identify_hook, void, (identify_callback cb), (cb))

// function: void fc_vdp_setchar(int pAddr, int ch)
DECL_FC_API_CALL(FC_VDP_SETCHAR, fc_vdp_setchar, void, (int pAddr, int ch), (pAddr, ch))

// function: unsigned int fc_vdp_get_cursor_addr()
DECL_FC_API_CALL(FC_VDP_GET_CURSOR_ADDR, fc_vdp_get_cursor_addr, unsigned int, (), ())

// function: void fc_ui_drop_down(int linecount)
DECL_FC_API_CALL(FC_UI_DROP_DOWN, fc_ui_drop_down, void, (int linecount), (linecount))

// function: void fc_ui_gotoxy(int x, int y)
DECL_FC_API_CALL(FC_UI_GOTOXY, fc_ui_gotoxy, void, (int x, int y), (x, y))

// function: void fc_speech_reset()
DECL_FC_API_CALL(FC_SPEECH_RESET, fc_speech_reset, void, (), ())

// function: int fc_detect_speech()
DECL_FC_API_CALL(FC_DETECT_SPEECH, fc_detect_speech, int, (), ())

// function: void fc_say_vocab(int phrase_addr)
DECL_FC_API_CALL(FC_SAY_VOCAB, fc_say_vocab, void, (int phrase_addr), (phrase_addr))

// function: void fc_say_data(const char* addr, int len)
DECL_FC_API_CALL(FC_SAY_DATA, fc_say_data, void, (const char* addr, int len), (addr, len))

// function: void fc_speech_start(struct LpcPlaybackCtx* ctx)
DECL_FC_API_CALL(FC_SPEECH_START, fc_speech_start, void, (struct LpcPlaybackCtx* ctx), (ctx))

// function: void fc_speech_continue(struct LpcPlaybackCtx* ctx)
DECL_FC_API_CALL(FC_SPEECH_CONTINUE, fc_speech_continue, void, (struct LpcPlaybackCtx* ctx), (ctx))

// function: void fc_speech_wait()
DECL_FC_API_CALL(FC_SPEECH_WAIT, fc_speech_wait, void, (), ())

// function: void fc_tipi_mouse(struct MouseData* mouseData)
DECL_FC_API_CALL(FC_TIPI_MOUSE, fc_tipi_mouse, void, (struct MouseData* mouseData), (mouseData))

// function: void fc_tipi_mouse_move(struct MouseData* mouseData)
DECL_FC_API_CALL(FC_TIPI_MOUSE_MOVE, fc_tipi_mouse_move, void, (struct MouseData* mouseData), (mouseData))

// function: void fc_tipi_mouse_enable(struct MouseData* mouseData)
DECL_FC_API_CALL(FC_TIPI_MOUSE_ENABLE, fc_tipi_mouse_enable, void, (struct MouseData* mouseData), (mouseData))

// function: void fc_tipi_mouse_disable()
DECL_FC_API_CALL(FC_TIPI_MOUSE_DISABLE, fc_tipi_mouse_disable, void, (), ())

// function: unsigned int fc_tls_connect(unsigned int socketId, char* hostname, char* port)
DECL_FC_API_CALL(FC_TLS_CONNECT, fc_tls_connect, unsigned int, (unsigned int socketId, char* hostname, char* port), (socketId, hostname, port))

// function: unsigned int fc_tls_close(unsigned int socketId)
DECL_FC_API_CALL(FC_TLS_CLOSE, fc_tls_close, unsigned int, (unsigned int socketId), (socketId))

// function: int fc_tls_read_socket(unsigned int socketId, char* buf, int bufsize)
DECL_FC_API_CALL(FC_TLS_READ_SOCKET, fc_tls_read_socket, int, (unsigned int socketId, char* buf, int bufsize), (socketId, buf, bufsize))

// function: int fc_tls_send_chars(unsigned int socketId, char* buf, int size)
DECL_FC_API_CALL(FC_TLS_SEND_CHARS, fc_tls_send_chars, int, (unsigned int socketId, char* buf, int size), (socketId, buf, size))

// function: void fc_init_socket_buffer(struct SocketBuffer* socket_buf, int tls, unsigned int socketId)
DECL_FC_API_CALL(FC_INIT_SOCKET_BUFFER, fc_init_socket_buffer, void, (struct SocketBuffer* socket_buf, int tls, unsigned int socketId), (socket_buf, tls, socketId))

// function: char* fc_readline(struct SocketBuffer* socket_buf)
DECL_FC_API_CALL(FC_READLINE, fc_readline, char*, (struct SocketBuffer* socket_buf), (socket_buf))

// function: int fc_readstream(struct SocketBuffer* socket_buf, char* block, int limit)
DECL_FC_API_CALL(FC_READSTREAM, fc_readstream, int, (struct SocketBuffer* socket_buf, char* block, int limit), (socket_buf, block, limit))

#endif
