/*------------------------------------------------------+----------------------
// МикроМир07   runtime (memory and string operations)  | (c) Epi MG, 2007
//------------------------------------------------------+--------------------*/
#include "mic.h" /* Old me.c - Память (c) Attic 1989-90, (c) EpiMG 1998-2003 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *GetMain (long n)
{
  char *p;          /*  some systems do not like to give zero length blocks  */
  if (n == 0) n++;  /* if we don't need anything, we'll be happy with 1 byte */

  if ((p = (char*)malloc(n)) == NULL) { printf("OutOfMemory"); abort(); }
  return p;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#define BIG_MEM_CHUNK 20*1024*1024
/*
 * Инициализация "большого" сегмента памяти -- platform-specific manipulations
 * with VirtualAlloc() etc are removed, now it just malloc()'s a bug chunk and
 * let virtual memory subsystem do optimization by itself
 */
char *MemInit (long *allocated_size)
{
  return GetMain(*allocated_size = BIG_MEM_CHUNK);
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Переслать блок из <len> байт начиная с адреса <from> по адресу <to>.
 * Возвращает <to+len>.     Допускается любое перекрытие <from> и <to>.
 * Пересылка (особенно больших) блоков должна осуществляться максимально быстро
 */
char *lblkmov(char *from, char *to, long len)
{
  return (char*)memmove(to, from, len) + len;
}
void *blkmov(void *from, void *to, long len) // to be used with non-char types
{
  return (void*)((char*)memmove(to, from, len) + len);
}
tchar *blktmov(tchar *from, tchar *to, long len)
{
  return (tchar*)memmove(to, from, len*sizeof(tchar)) + len;
}
/*---------------------------------------------------------------------------*/
/* nanoMir 2000 $Id: rt.c,v 1.4 2003/04/04 00:57:00 epi Exp $
 *-----------------------------------------------------------------------------
 * (c) Attic 1989                             Runtime - сервисные подпрограммы
 * (c) EpiMG 1998-99, 2007
 */
char *sncpy (const char *from, char *to, int n)
{
  while (n--) *to++ = *from++; return to;
}
char *scpy (const char *from, char *to)
{
  while (*from) *to++ = *from++; return to;
}
char *scpyx (const char *from, char *to, int limit)
{
  while (*from && limit--) *to++ = *from++; return to;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
char *ltod(char *buffer, large l) /* long to decimal (returns end-of-number) */
{
  char *p1, lbuf[12];
  small i;
  if (l < 0) { l= -l; *buffer++ = '-'; }

  p1 = lbuf+12; *(--p1) = 0;  i = 11;
  do {          *(--p1) = l%10 + '0'; } while((l = l/10) != 0 && --i);

  return scpy(p1, buffer);
}
char *stod(char *p, small s) { return ltod(p, (large)s); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
char *dtol(char *p, large *dest) 
{
  small i, is_negative = 0; 
  large l = 0;
  if (*p == '-') { p++; is_negative++; }

  while ((i = *p++ - '0') >= 0 && i < 10) l = l*10 + i;

  if (is_negative) l = -l;
  *dest = l;   return --p;
}
char *dtos(char *p, small *dest) { large l; p = dtol(p, &l);
                                           *dest = (small)l; return p; }
/*---------------------------------------------------------------------------*/
jmp_buf *excptr = 0;

void otkaz (char *p)
{
  fprintf(stderr, "Otkaz - %s\n", p); abort();
}
void exc (int code) 
{
  if (excptr == NIL) otkaz("EXC");
  longjmp(*excptr, code);
}
/*---------------------------------------------------------------------------*/
