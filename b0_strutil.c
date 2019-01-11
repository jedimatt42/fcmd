#include "strutil.h"

#include <conio.h>
#include <string.h>


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

char lowerchar(char c) {
  if (c >= 'A' && c <= 'Z') {
    return c - 'A' + 'a';
  }
  return c;
}

int strcmpi(const char* a, const char* b) {
  int i=0;
  char ch = lowerchar(a[i]);
  do {
    if (ch == '\0') {
      return ch - lowerchar(b[i]);
    }
    i++;
    ch = lowerchar(a[i]);
  } while(ch == lowerchar(b[i]));
  return ch - lowerchar(b[i]);
}

int indexof(const char* str, char c) {
  int i=0;
  while(str[i] != 0) {
    if (str[i] == c) {
      return i;
    }
    i++;
  }
  return -1;
}

int lindexof(const char* str, char c, int start) {
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

char* strtok(char* str, char* delim) {
  int ch;

  if (str == 0)
	  str = lasts;
  do {
	  if ((ch = *str++) == '\0')
	    return 0;
  } while (strchr(delim, ch));
  --str;
  lasts = str + strcspn(str, delim);
  if (*lasts != 0)
	  *lasts++ = 0;
  return str;
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

void strset(char* buffer, char value, int limit) {
  for(int i=0; i<limit; i++) {
    buffer[i] = value;
  }
}

char* strcat(char* dest, char* add) {
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
  while(src[i] != 0 && i < (limit -1)) {
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

void strpad(char* dest, int limit, char pad) {
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