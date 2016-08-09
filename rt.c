/*------------------------------------------------------+----------------------
// МикроМир07   runtime (memory and string operations)  | (c) Epi MG, 2007-2016
//------------------------------------------------------+--------------------*/
#include "mic.h" /* Old me.c - Память (c) Attic 1989-90, (c) EpiMG 1998-2003 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "version.h"

void xfree (void *mem) { free(mem); }
char *xmalloc (long n)
{                                   // some systems don't like to provide zero
  char *p = (char*)malloc(n?n:n+1); // length blocks => ask for 1 byte instead
  if  (!p) exc(E_NOMEM);  return p; // (if caller does not need anything, they
}                                   // will be happy with that byte anyway)
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#define BIG_MEM_CHUNK 60*1024*1024
/*
 * Инициализация "большого" сегмента памяти -- platform-specific manipulations
 * with VirtualAlloc() etc are removed, now it just malloc()'s a bug chunk and
 * let virtual memory subsystem do optimization by itself
 */
char *MemInit (long *allocated_size)
{
  return xmalloc(*allocated_size = BIG_MEM_CHUNK);
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
char *xstrndup (const char *orig, int limit)
{
  const char *p = orig; int N = 0;
  while     (*p++ && limit--) N++;
  char *dup = xmalloc(N);
  lblkmov((char*)orig, dup, N); return dup;
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
char *ltod (char *buffer, long l) /* long to decimal (returns end-of-number) */
{
  char *p1, lbuf[12];
  short i;
  if (l < 0) { l= -l; *buffer++ = '-'; }

  p1 = lbuf+12; *(--p1) = 0;  i = 11;
  do {          *(--p1) = l%10 + '0'; } while((l = l/10) != 0 && --i);

  return scpy(p1, buffer);
}
char *stod(char *p, short s) { return ltod(p, (long)s); }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
char *dtol (char *p, long *dest)
{
  short i, is_negative = 0;
  long l = 0;
  if (*p == '-') { p++; is_negative++; }

  while ((i = *p++ - '0') >= 0 && i < 10) l = l*10 + i;

  if (is_negative) l = -l;
  *dest = l;   return --p;
}
char *dtos(char *p, short *dest) { long l; p = dtol(p, &l);
                                          *dest = (short)l; return p; }
/*---------------------------------------------------------------------------*/
void exc (int code) 
{
  if (excptr)                 longjmp(*excptr, code);
  fprintf(stderr, "Unhandled excepition %d\n", code); abort();
}
jmp_buf *excptr = 0;
/*---------------------------------------------------------------------------*/
