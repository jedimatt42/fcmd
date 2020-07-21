#include "banks.h"
#define MYBANK BANK(1)

#include "b1_strutil.h"

#include <string.h>

char str2ram_buf[128]; // for the inlined str2ram function

int strcmp(const char* a, const char* b) {
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
  int l = strlen(a);
  if (strlen(b) != l) {
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
  if (str == 0 || strlen(str) < strlen(prefix)) {
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
  int istr = strlen(str);
  int isuf = strlen(suffix);
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

int strcmpi(const char *a, const char *b) {
  while(*a) { // a is not 0 termination character
    if (lowerchar(*a) != lowerchar(*b)) {
      break;
    }
    a++;
    b++;
  }
  return lowerchar(*(const unsigned char*)a) - lowerchar(*(const unsigned char*)b);
}

int indexof(const char* str, int c) {
  int i=0;
  while(str[i] != 0) {
    if (str[i] == c) {
      return i;
    }
    i++;
  }
  return -1;
}

int lindexof(const char* str, int c, int start) {
  int i = start;
  while(i != -1) {
    if (str[i] == c) {
      return i;
    }
    i--;
  }
  return -1;
}

int basic_strcmp(const char* basstr, const char* cstr) {
  int i = 0;
  do {
    if (basstr[0] == i) {
      return 0 - cstr[i];
    }
    i++;
  } while(basstr[i+1] == cstr[i]);
  return basstr[i+1] - cstr[i];
}

int basicToCstr(const char* str, char* buf) {
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

char* strtok(char* str, int delim) {
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

char* strtokpeek(char* str, int delim) {
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

  strncpy(peekbuf, str, plasts - str);
  return peekbuf;
}

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

void strset(char* buffer, int value, int limit) {
  for(int i=0; i<limit; i++) {
    buffer[i] = value;
  }
}

char* strcat(char* dest, const char* add) {
  int x = strlen(dest);
  char* s = dest + x;
  while(*add != 0) {
    *s++ = *add++;
  }
  *s = 0;
  return dest;
}

char* strncpy(char* dest, char* src, int limit) {
  int i = 0;
  while(src[i] != 0 && i < (limit)) {
    dest[i] = src[i];
    i++;
  }
  dest[i] = 0;
  return dest;
}

int htoi(char* s) {
  int old;
  int out = 0;
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

