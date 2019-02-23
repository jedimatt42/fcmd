#ifndef _STRUTIL_H
#define _STRUTIL_H 1

int strcmp(const char* a, const char* b);
int strcmpi(const char* a, const char* b);
int indexof(const char* str, char c);
int lindexof(const char* str, char c, int start);
int basicToCstr(const char* str, char* buf);
int basic_strcmp(const char* basstr, const char* cstr);
char* strtok(char* str, char* delim);
char* strchr(char* str, int delim);
int strcspn(char* string, char* chars);
void strset(char* buffer, char value, int limit);
char* strcat(char* dst, char* add);
char* strncpy(char* dest, char* src, int limit);
void strpad(char* dest, int limit, char pad);
int htoi(char* s);

#endif