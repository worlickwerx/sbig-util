/*

	compatibility.h

*/
#ifndef _COMPATIBILITY_
#define _COMPATIBILITY_

typedef enum { SCREEN_OFF, SCREEN_UPDATE } SCREEN_CONTROL;

extern void term_start(void);
extern void term_stop(void);
extern int mykbhit(void);
extern void ansigotoxy(int x, int y);
extern void gotoxy(int x, int y);
extern void clreol(void);
extern void clreols(void);
extern void delay(long ms);
extern void clrscr(void);
extern int ccprintf( char *fs, ...);
extern void screenControl(SCREEN_CONTROL sc);
extern void mygets(char *s);
extern int mygetch(void);

#if TARGET == ENV_MACOSX || TARGET == ENV_LINUX
 extern int kbhit(void);
#endif

#endif

