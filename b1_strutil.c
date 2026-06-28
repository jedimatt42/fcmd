#include "banks.h"
#define MYBANK BANK(1)

#include "b1_strutil.h"

#include <string.h>

char str2ram_buf[128]; // for the inlined str2ram function

int str_cmp(const char* a, const char* b) {
  int i=0;
  do {
    if (a[i] == '\0') {
      return a[i] - b[i];
    }
    i++;
  } while(a[i] == b[i]);
  return a[i] - b[i];
}

int str_equals(char* a, char* b) {
  int l = str_len(a);
  if (str_len(b) != l) {
    return 0;
  }
  for(int i=0; i<l; i++) {
    if (a[i] != b[i]) {
      return 0;
    }
  }
  return 1;
}

char lowerchar(unsigned char c) {
  if (c >= 'A' && c <= 'Z') {
    return c + ('a' - 'A');
  }
  return c;
}

int str_startswith(const char* str, const char* prefix) {
  if (str == 0 || str_len(str) < str_len(prefix)) {
    return 0;
  }
  while(*prefix != 0) {
    if (*prefix != *str) {
      return 0;
    }
    str++;
    prefix++;
  }
  return 1;
}

int str_endswith(const char* str, const char* suffix) {
  int istr = str_len(str);
  int isuf = str_len(suffix);
  if (str == 0 || istr < isuf) {
    return 0;
  }
  while(isuf > 0) {
    istr--;
    isuf--;
    if (str[istr] != suffix[isuf]) {
      return 0;
    }
  }
  return 1;
}

int str_cmp_icase(const char *a, const char *b) {
  while(*a) { // a is not 0 termination character
    if (lowerchar(*a) != lowerchar(*b)) {
      break;
    }
    a++;
    b++;
  }
  return lowerchar(*(const unsigned char*)a) - lowerchar(*(const unsigned char*)b);
}

int str_index_of(const char* str, int c) {
  int i=0;
  while(str[i] != 0) {
    if (str[i] == c) {
      return i;
    }
    i++;
  }
  return -1;
}

int str_last_index_of(const char* str, int c, int start) {
  int i = start;
  while(i != -1) {
    if (str[i] == c) {
      return i;
    }
    i--;
  }
  return -1;
}

int str_from_basic(const char* str, char* buf) {
  int len = (int) str[0];
  for(int i=0; i<len; i++) {
    buf[i] = str[i+1];
  }
  buf[len] = 0;
  return len;
}

char *lasts;
int strcspn(char *string, char *chars);
char *strchr(char *str, int delim);

char* str_token(char* str, int delim) {
  int ch;
  char delim_str[2];
  delim_str[0] = delim;
  delim_str[1] = 0;

  if (str == 0)
	  str = lasts;
  do {
	  if ((ch = *str++) == '\0')
	    return 0;
  } while (strchr(delim_str, ch));
  --str;
  lasts = str + strcspn(str, delim_str);
  if (*lasts != 0)
	  *lasts++ = 0;
  return str;
}

char peekbuf[256];

char* str_token_peek(char* str, int delim) {
  int ch;
  char delim_str[2];
  delim_str[0] = delim;
  delim_str[1] = 0;

  if (str == 0)
	  str = lasts;
  do {
	  if ((ch = *str++) == '\0')
	    return 0;
  } while (strchr(delim_str, ch));
  --str;
  char* plasts = str + strcspn(str, delim_str);

  str_ncopy(peekbuf, str, plasts - str);
  return peekbuf;
}

// searches for the first occurrence of a specified character in a string and returns a pointer to it, or NULL if the character is not found.
char* strchr(char* str, int delim)
{
  int x;

  while (1) {
	  x = *str++;
	  if (x == delim) {
	    return str - 1;
	  }
	  if (x == 0) {
	    return (char *) 0;
	  }
  }
}

// calculates the length of the initial segment of string that consists entirely of characters not found in chars. 
int strcspn(char* string, char* chars) {
  char c, *p, *s;

  for (s = string, c = *s; c != 0; s++, c = *s) {
	  for (p = chars; *p != 0; p++) {
	    if (c == *p) {
		    return s-string;
	    }
	  }
  }
  return s-string;
}

void str_set(char* buffer, int value, int limit) {
  for(int i=0; i<limit; i++) {
    buffer[i] = value;
  }
}

char* str_cat(char* dest, const char* add) {
  int x = str_len(dest);
  char* s = dest + x;
  while(*add != 0) {
    *s++ = *add++;
  }
  *s = 0;
  return dest;
}

char* str_ncopy(char* dest, char* src, int limit) {
  int i = 0;
  while(src[i] != 0 && i < limit) {
    dest[i] = src[i];
    i++;
  }
  dest[i] = 0;
  return dest;
}

int hex_to_int(char* s) {
  unsigned int old;
  unsigned int out = 0;
  int neg = 0;

  // skip whitespace
  while ((*s <= ' ') && (*s != '\0')) ++s;

  if (*s == '-') {
    ++s;
    neg = 1;
  }
  if (*s == '+') ++s;

  // check whole digits
  while (((*s >= '0') && (*s <= '9')) || ((*s >= 'A') && (*s <= 'F'))) {
    old = out;
    out *= 16;
    if (*s >= 'A') {
      out += (*s)-'A' + 10;
    } else {
      out += (*s)-'0';
    }
    ++s;
    if (out < old) {
      // too big, wrapped around
      return 0;
    }
  }

  if (neg) out = 0-out;
  return out;
}

void strpad(char* dest, int limit, int pad) {
  int dopad = 0;
  for(int i = 0; i < limit; i++) {
    if (dest[i] == 0) {
      dopad = 1;
    }
    if (dopad) {
      dest[i] = pad;
    }
  }
  dest[limit] = 0;
}

