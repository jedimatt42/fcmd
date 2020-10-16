#ifndef _FC_API_H
#define _FC_API_H 1

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
  Peripheral Access Block
*/
struct __attribute__((__packed__)) PAB {
    unsigned char OpCode;			// see DSR_xxx list above
    unsigned char Status;			// file type and error code (DSR_ERR_xxx and DSR_TYPE_xxx)
    unsigned int  VDPBuffer;		// address of the data buffer in VDP memory
    unsigned char RecordLength;	// size of records. Not used for PROGRAM type. >00 on open means autodetect
    unsigned char CharCount;		// number of bytes read or number of bytes to write
    unsigned int  RecordNumber;		// record number for normal files, available bytes (LOAD or SAVE) for PROGRAM type
    unsigned char ScreenOffset;	// Used in BASIC for screen BIAS. Also returns file status on Status call. (DSR_STATUS_xxx)
    unsigned char NameLength;		// for this implementation only, set to zero to read the length from the string
    unsigned char *pName;			// for this implementation only, must be a valid C String even if length is set
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

#define FC_TPUTC 0x6082
#define FC_TPUTS 0x6086
#define FC_GETSTR 0x608a
#define FC_SAMS_MAP_PAGE 0x608e
#define FC_SAMS_ALLOC_PAGES 0x6092
#define FC_SAMS_FREE_PAGES 0x6096
#define FC_DSR_EA5LOAD 0x609a
#define FC_DSR_OPEN 0x609e
#define FC_DSR_CLOSE 0x60a2
#define FC_DSR_READ 0x60a6
#define FC_DSR_WRITE 0x60aa
#define FC_DSR_STATUS 0x60ae
#define FC_DSR_RESET 0x60b2
#define FC_DSR_DELETE 0x60b6
#define FC_LVL2_INPUT 0x60ba
#define FC_LVL2_OUTPUT 0x60be
#define FC_LVL2_PROTECT 0x60c2
#define FC_LVL2_RENAME 0x60c6
#define FC_LVL2_SETDIR 0x60ca
#define FC_LVL2_MKDIR 0x60ce
#define FC_LVL2_RMDIR 0x60d2
#define FC_LVL2_RENDIR 0x60d6
#define FC_VARS_GET 0x60da
#define FC_VARS_SET 0x60de
#define FC_TCP_CONNECT 0x60e2
#define FC_TCP_CLOSE 0x60e6
#define FC_TCP_READ_SOCKET 0x60ea
#define FC_TCP_SEND_CHARS 0x60ee
#define FC_TIPI_ON 0x60f2
#define FC_TIPI_OFF 0x60f6
#define FC_TIPI_SENDMSG 0x60fa
#define FC_TIPI_RECVMSG 0x60fe

// function: void fc_tputc(int c)
DECL_FC_API_CALL(FC_TPUTC, fc_tputc, void, (int c), (c))

// function: void fc_tputs(const char* str)
DECL_FC_API_CALL(FC_TPUTS, fc_tputs, void, (const char* str), (str))

// function: void fc_getstr(char* var, int limit, int backspace)
DECL_FC_API_CALL(FC_GETSTR, fc_getstr, void, (char* var, int limit, int backspace), (var, limit, backspace))

// function: void fc_sams_map_page(int page, int addr)
DECL_FC_API_CALL(FC_SAMS_MAP_PAGE, fc_sams_map_page, void, (int page, int addr), (page, addr))

// function: int fc_sams_alloc_pages(int count)
DECL_FC_API_CALL(FC_SAMS_ALLOC_PAGES, fc_sams_alloc_pages, int, (int count), (count))

// function: void fc_sams_free_pages(int count)
DECL_FC_API_CALL(FC_SAMS_FREE_PAGES, fc_sams_free_pages, void, (int count), (count))

// function: unsigned int fc_dsr_ea5load(struct DeviceServiceRoutine * dsr, const char *fname)
DECL_FC_API_CALL(FC_DSR_EA5LOAD, fc_dsr_ea5load, unsigned int, (struct DeviceServiceRoutine * dsr, const char *fname), (dsr, fname))

// function: unsigned int fc_dsr_open(struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int flags, int reclen)
DECL_FC_API_CALL(FC_DSR_OPEN, fc_dsr_open, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int flags, int reclen), (dsr, pab, fname, flags, reclen))

// function: unsigned int fc_dsr_close(struct DeviceServiceRoutine* dsr, struct PAB* pab)
DECL_FC_API_CALL(FC_DSR_CLOSE, fc_dsr_close, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab), (dsr, pab))

// function: unsigned int fc_dsr_read(struct DeviceServiceRoutine* dsr, struct PAB* pab, int recordNumber)
DECL_FC_API_CALL(FC_DSR_READ, fc_dsr_read, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab, int recordNumber), (dsr, pab, recordNumber))

// function: unsigned int fc_dsr_write(struct DeviceServiceRoutine* dsr, struct PAB* pab, unsigned char* record, int reclen)
DECL_FC_API_CALL(FC_DSR_WRITE, fc_dsr_write, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab, unsigned char* record, int reclen), (dsr, pab, record, reclen))

// function: unsigned int fc_dsr_status(struct DeviceServiceRoutine* dsr, struct PAB* pab)
DECL_FC_API_CALL(FC_DSR_STATUS, fc_dsr_status, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab), (dsr, pab))

// function: unsigned int fc_dsr_reset(struct DeviceServiceRoutine* dsr, struct PAB* pab, int record)
DECL_FC_API_CALL(FC_DSR_RESET, fc_dsr_reset, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab, int record), (dsr, pab, record))

// function: unsigned int fc_dsr_delete(struct DeviceServiceRoutine* dsr, struct PAB* pab)
DECL_FC_API_CALL(FC_DSR_DELETE, fc_dsr_delete, unsigned int, (struct DeviceServiceRoutine* dsr, struct PAB* pab), (dsr, pab))

// function: unsigned int fc_lvl2_input(int crubase, unsigned int unit, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr)
DECL_FC_API_CALL(FC_LVL2_INPUT, fc_lvl2_input, unsigned int, (int crubase, unsigned int unit, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr), (crubase, unit, filename, blockcount, addInfoPtr))

// function: unsigned int fc_lvl2_output(int crubase, unsigned int unit, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr)
DECL_FC_API_CALL(FC_LVL2_OUTPUT, fc_lvl2_output, unsigned int, (int crubase, unsigned int unit, char *filename, unsigned int blockcount, struct AddInfo *addInfoPtr), (crubase, unit, filename, blockcount, addInfoPtr))

// function: unsigned int fc_lvl2_protect(int crubase, unsigned int unit, char *filename, int protect)
DECL_FC_API_CALL(FC_LVL2_PROTECT, fc_lvl2_protect, unsigned int, (int crubase, unsigned int unit, char *filename, int protect), (crubase, unit, filename, protect))

// function: unsigned int fc_lvl2_rename(int crubase, unsigned int unit, char *oldname, char *newname)
DECL_FC_API_CALL(FC_LVL2_RENAME, fc_lvl2_rename, unsigned int, (int crubase, unsigned int unit, char *oldname, char *newname), (crubase, unit, oldname, newname))

// function: unsigned int fc_lvl2_setdir(int crubase, unsigned int unit, char* path)
DECL_FC_API_CALL(FC_LVL2_SETDIR, fc_lvl2_setdir, unsigned int, (int crubase, unsigned int unit, char* path), (crubase, unit, path))

// function: unsigned int fc_lvl2_mkdir(int crubase, unsigned int unit, char *dirname)
DECL_FC_API_CALL(FC_LVL2_MKDIR, fc_lvl2_mkdir, unsigned int, (int crubase, unsigned int unit, char *dirname), (crubase, unit, dirname))

// function: unsigned int fc_lvl2_rmdir(int crubase, unsigned int unit, char *dirname)
DECL_FC_API_CALL(FC_LVL2_RMDIR, fc_lvl2_rmdir, unsigned int, (int crubase, unsigned int unit, char *dirname), (crubase, unit, dirname))

// function: unsigned int fc_lvl2_rendir(int crubase, unsigned int unit, char *oldname, char *newname)
DECL_FC_API_CALL(FC_LVL2_RENDIR, fc_lvl2_rendir, unsigned int, (int crubase, unsigned int unit, char *oldname, char *newname), (crubase, unit, oldname, newname))

// function: char* fc_vars_get(char* name)
DECL_FC_API_CALL(FC_VARS_GET, fc_vars_get, char*, (char* name), (name))

// function: void fc_vars_set(char* name, char* value)
DECL_FC_API_CALL(FC_VARS_SET, fc_vars_set, void, (char* name, char* value), (name, value))

// function: unsigned int fc_tcp_connect(unsigned int socketId, unsigned char* hostname, unsigned char* port)
DECL_FC_API_CALL(FC_TCP_CONNECT, fc_tcp_connect, unsigned int, (unsigned int socketId, unsigned char* hostname, unsigned char* port), (socketId, hostname, port))

// function: unsigned int fc_tcp_close(unsigned int socketId)
DECL_FC_API_CALL(FC_TCP_CLOSE, fc_tcp_close, unsigned int, (unsigned int socketId), (socketId))

// function: int fc_tcp_read_socket(unsigned int socketId, unsigned char* buf, int bufsize)
DECL_FC_API_CALL(FC_TCP_READ_SOCKET, fc_tcp_read_socket, int, (unsigned int socketId, unsigned char* buf, int bufsize), (socketId, buf, bufsize))

// function: int fc_tcp_send_chars(unsigned int socketId, unsigned char* buf, int size)
DECL_FC_API_CALL(FC_TCP_SEND_CHARS, fc_tcp_send_chars, int, (unsigned int socketId, unsigned char* buf, int size), (socketId, buf, size))

// function: void fc_tipi_on()
DECL_FC_API_CALL(FC_TIPI_ON, fc_tipi_on, void, (), ())

// function: void fc_tipi_off()
DECL_FC_API_CALL(FC_TIPI_OFF, fc_tipi_off, void, (), ())

// function: void fc_tipi_sendmsg(unsigned int len, const unsigned char *buf)
DECL_FC_API_CALL(FC_TIPI_SENDMSG, fc_tipi_sendmsg, void, (unsigned int len, const unsigned char *buf), (len, buf))

// function: void fc_tipi_recvmsg(unsigned int *len, unsigned char *buf)
DECL_FC_API_CALL(FC_TIPI_RECVMSG, fc_tipi_recvmsg, void, (unsigned int *len, unsigned char *buf), (len, buf))

#endif
