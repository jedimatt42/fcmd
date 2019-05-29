#ifndef _terminal_h
#define _terminal_h 1

void initTerminal();

#define tputc(x) terminalDisplay(x)
void terminalDisplay(unsigned char c);
void tputs(const char* str);

extern int termWidth;
extern unsigned char foreground;
extern unsigned char background;
extern unsigned char colors[];
extern unsigned char isBold;

void resetState();
int getParamA(int def);
int getParamB(int def);
void cursorUp(int lines);
void cursorDown(int lines);
void cursorRight(int cols);
void cursorLeft(int cols);
void cursorGoto(int x, int y);
void eraseDisplay(int opt);
void eraseLine(int opt);
void scrollUp(int lc);
void sendTermCoord();
void sendTermType();
void setColors();
void doSGRCommand();
void doCsiCommand(unsigned char c);
int doEscCommand(unsigned char c);
void charout(unsigned char ch);
void enable_more();
void disable_more();

#endif
