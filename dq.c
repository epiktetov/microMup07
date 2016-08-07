/*------------------------------------------------------+----------------------
// МикроМир07       Деки                                | (c) Epi MG, 2007-2016
//------------------------------------------------------+----------------------
* Original "dq.c" (c) Attic 1989-91
*                 (c) EpiMG 1997-2001
*
   Исполнитель "Деки" предназначен для хранения потенциально больших объемов
информации: редактируемых файлов, файлов откатки и т.п. Каждый дек представляет
собой классический дек текстовых строк - внутренняя структура может (но не
обязана) совпадать со структурой текстового файла используемой ОС. Реализованы
два типа деков - текстовый и двоичный. Текстовый дек (как и текстовый файл в
большинстве ОС) - последовательность байтов, разбитая на записи разделителями
CR LF (или только LF). Таким образом, появление символов CR, LF, EOF в записях
текстового дека настоятельно не рекомендуется. Каждая из записей двоичного дека
начинается с четного  адреса, выровнена с конца до границы слова и окаймлена
с обеих концов 16 разрядными словами, содержащими длину записи. 

   Параметры "размер оптимального экстента  начала/конца  дека" добавлены для
оптимизации непрерывной реализации и характеризуют потенциальные апетиты на
добавление записей в начало и конец дека соответственно. Если дек используется
как стек, следует задать экстент ноль для неактивного конца.

   Эта реализация деков для компьютера с "большой" памятью - деки непрерывно
расположены в памяти. При нехватке места для добавления записи деки перемещают-
ся по памяти, расширяя просвет до размера "оптимального экстента".

   Записи текстового дека разделяются LF или CR LF, при чтении запись выдается
без терминаторов. Добавляемая запись терминируется CR LF (если dosEOL истинно)
или одним LF (в противном случае).  При загрузке из конца дека удаляется EOF 
(если такой имеется) и добавляются (при отсутствии) LF.
*/
#include "mic.h"
#include "qfs.h"
#include "vip.h"
#include "dq.h"

static deq deq0, *freedeqs = NIL, *gapdeq, *lockdeq;
static bool DqReserveExt  = TRUE;
static long tmswap(long req_mem); /* выкидывает заменимое, return: available */
/*---------------------------------------------------------------------------*/
void DqInit(char *membuf, long bufsize)     /* create "anti-deq" that covers */
{                                           /* memory that other deqs cannot */
  deq0.dbext = 0; gapdeq = NIL;             /* use (it has reverse beg/end)  */
  deq0.deext = 0;
  deq0.dprev = &deq0;  deq0.dend = membuf;
  deq0.dnext = &deq0;  deq0.dbeg = membuf+bufsize;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
deq *DqNew (short typ, short bext, short eext)
{
  deq *d; if (freedeqs) { d = freedeqs; freedeqs = d->dnext; }
          else            d = (deq*)GetMain(sizeof(deq));
  deq *dn = deq0.dprev;
  d->dprev = dn;
  dn->dnext = d;                    d->dtyp  = typ;
  d->dnext = &deq0; deq0.dprev = d; d->dbext = bext;
  d->dbeg = d->dend = dn->dend;     d->deext = eext; return d;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
bool DqDel (deq *d)
{
  if (d == lockdeq) return FALSE;
  else {
    (d->dnext)->dprev = d->dprev;
    (d->dprev)->dnext = d->dnext;
    if (d == gapdeq) gapdeq = d->dprev;
    d->dnext = freedeqs;  freedeqs = d; return TRUE;
} }
/*---------------------------------------------------------------------------*/
long DqFree()                         /* calculate total free memory in deqs */
{                                     /*   if DqReserveExt => don't consider */
  deq *d = &deq0;                     /*   memory under extents to be "free" */
  long f = 0;
  do {
    long len = cpdiff(d->dnext->dbeg, d->dend);
    if (DqReserveExt) {
      long maxext = my_max(d->deext, d->dnext->dbext);
      if (len > maxext) len -= maxext;
      else              len  =      0;
    }
    f += (d->dextra = len & (~1));
  }
  while ((d = d->dnext) != &deq0); return f;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#include <stdio.h>
void DqPrintDeqs()
{
  long gap, len;
  deq *d;
  if (deq0.dnext != &deq0) {
    gap = cpdiff(deq0.dnext->dbeg, deq0.dend);
    if (gap > 0) fprintf(stderr, "%ld-", gap);
  }
  for (d = deq0.dnext; d != &deq0; d = d->dnext) {
    fprintf(stderr, "%c%c", d->dbext? '{' : '[', d->dtyp);
         if (d == lockdeq) fprintf(stderr, "*");
    else if (d ==  gapdeq) fprintf(stderr, "+");
    len = DqLen(d);
         if (len > 1048576) fprintf(stderr, "%.1fM", (double)len/1048576);
    else if (len > 1024)    fprintf(stderr, "%.1fk", (double)len/1024);
    else                    fprintf(stderr, "%ld",           len);
    fprintf(stderr, "%c", d->deext ? '}' : ']');
    gap = cpdiff(d->dnext->dbeg, d->dend);
         if (gap > 1048576) fprintf(stderr, "-%.1fM-", (double)gap/1048576);
    else if (gap > 1024)    fprintf(stderr, "-%.1fk-", (double)gap/1024);
    else if (gap > 0)       fprintf(stderr, "-%ld-",           gap);
  }
  fprintf(stderr, "\n");
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void extgap (deq *d, long len, bool move_previous)
{
  deq *d1st, *d2nd; /* нужна память в промежутке между [d1st}-..gap..-{d2nd] */
  long delta;
  if (move_previous) { d1st = d->dprev; d2nd = d;        }
  else               { d1st = d;        d2nd = d->dnext; }

  if (cpdiff(d2nd->dbeg, d1st->dend) >= len) return; /* уже есть! */
  lockdeq = d;
  gapdeq = d1st;
  if (debugDQ) { fprintf(stderr,"extgap(len=%ld%s):\n",
                    len, move_previous ? ",prev" : ""); DqPrintDeqs(); }
  if (d1st->deext > len) len = d1st->deext;
  if (d2nd->dbext > len) len = d2nd->dbext;
  if (DqFree() < len && tmswap(len) < len) { /* Если нет места: swap smthng */
    DqReserveExt = FALSE;                    /* still fail => ignore extent */
    if (DqFree() < len) {
      vipError( "Нет памяти (out ot memory)"); exc(E_NOMEM);
  } }
  for (delta = deq0.dextra, d = deq0.dnext; d != d2nd; d = d->dnext) {
    if (delta > 0) {
      lblkmov(d->dbeg, d->dbeg - delta, DqLen(d)); d->dbeg -= delta;
                                                   d->dend -= delta;
    }
    delta += d->dextra;
  }
  for (delta = 0, d = deq0.dprev; d != d1st; d = d->dprev) {
    delta += d->dextra;
    if (delta > 0) {
      lblkmov(d->dbeg, d->dbeg + delta, DqLen(d)); d->dbeg += delta;
                                                   d->dend += delta;
  } }
  DqReserveExt = TRUE;
  lockdeq = gapdeq = NIL; if (debugDQ) DqPrintDeqs();
}
/*---------------------------------------------------------------------------*/
void DqAddB (deq *d, char *data, int len)
{
  char *pb;
  if (DT_IsBIN(d->dtyp)) {
    int real_len = (len+1) & (~1); 
    extgap (d, real_len+4,  TRUE);
    d->dbeg -= real_len+4;   pb  = d->dbeg;    *(short*)pb = len;
    blkmov(data, pb+2, len); pb += real_len+2; *(short*)pb = len;
  }
  else if (dosEOL) {
    extgap(d, len+2, TRUE);                 d->dbeg -= len+2;
    pb = lblkmov(data, d->dbeg, len); *pb++ = CR; *pb++ = LF;
  }
  else { extgap(d, len+1, TRUE);     d->dbeg -= len+1;
         pb = lblkmov(data, d->dbeg, len); *pb++ = LF; }
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void DqAddE (deq *d, char *data, int len)
{
  char *pb;
  if (DT_IsBIN(d->dtyp)) {
    int real_len = (len+1) & (~1);
    extgap (d, real_len+4, FALSE); 
    pb = d->dend;       d->dend += real_len+4; *(short*)pb = len;
    blkmov(data, pb+2, len); pb += real_len+2; *(short*)pb = len;
  }
  else {
    if (dosEOL) { extgap(d, len+2, FALSE);
                  pb = lblkmov(data, d->dend, len); *pb++ = CR; *pb++ = LF; }
    else { 
      extgap(d, len+1, FALSE);
      pb = lblkmov(data, d->dend, len); *pb++ = LF;
    }
    d->dend = pb;
} }
/*---------------------------------------------------------------------------*/
char *DqLookupForw(deq *d, int pos, int *len_out, int *real_len_out)
{
  char *pb, *pbeg; short *pi;
  int len, real_len;
  if (DT_IsBIN(d->dtyp)) {
    pi = (short*)(d->dbeg + pos);         len = *pi++;
    pbeg = (char*)pi; real_len = ((len+1) & (~1)) + 4;
  } 
  else {
    pbeg = d->dbeg + pos;
    for (pb = pbeg+1; pb < d->dend && pb[-1] != LF; pb++);
    len = (real_len = pb - pbeg) - 1;
    if (len < 0) exc(E_MOVDOWN);
    if (len > 0 && pb[-2] == CR) len--;
  }
  if (len > MAXLUP-1) len = MAXLUP-1;
  if (len_out) *len_out = len;
  if (real_len_out) *real_len_out = real_len; return pbeg;
}
int DqCopyForward(deq *d, int pos, char *out, int *real_len_out)
{
  int len; char *pbeg = DqLookupForw(d, pos, &len, real_len_out);
  if (out) lblkmov(pbeg, out, len);                   return len;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
char *DqLookupBack(deq *d, int pos, int *len_out, int *real_len_out)
{
  char *pb, *pend; short *pi;
  long len, real_len;
  if (DT_IsBIN(d->dtyp)) {
    pi = (short*)(pend = d->dbeg + pos);    len = *(--pi);
    real_len = ((len+1) & (~1)) + 4; pb = pend-real_len+2;
  } 
  else {
    pend = d->dbeg + pos;
    for (pb = pend-1; pb > d->dbeg && pb[-1] != LF; pb--);
    len = (real_len = pend - pb) - 1;
    if (len < 0) exc(E_MOVUP);
    if (len > 0 && pend[-2] == CR) len--;
  }
  if (len > MAXLUP-1) len = MAXLUP-1;
  if (len_out) *len_out = len;
  if (real_len_out) *real_len_out = real_len; return pb;
}
int DqCopyBackward (deq *d, int pos, char *out, int *real_len_out)
{
  int len; char *pb = DqLookupBack(d, pos, &len, real_len_out);
  if (out) lblkmov(pb, out, len);                   return len;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
int DqGetB (deq *d, char *out)
{ 
  int real_len, len = DqCopyForward(d, 0, out, &real_len);
  d->dbeg += real_len;                         return len; 
}                                                                      
int DqGetE (deq *d, char *out)
{
  int real_len, len = DqCopyBackward(d, DqLen(d), out, &real_len);
  d->dend -= real_len;                                 return len;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void DqEmpt (deq *d) { if (d->dbext < d->deext) d->dend = d->dbeg;
                       else                     d->dbeg = d->dend; 
}                                                                   
void DqCutB_byX (deq *d, long  dx) { d->dbeg += dx;           }
void DqCutE_toX (deq *d, long len) { d->dend = d->dbeg + len; }
/*---------------------------------------------------------------------------*/
int DqLoad (deq *d, qfile *f, long size)
{
  long actual_size;  int rc = 2; DqEmpt(d);
  long mem_available = DqFree();
  if (mem_available < size && (mem_available = tmswap(size)) < size) {
    vipFileTooBigError(f, size); if (DT_IsBIN(d->dtyp)) return -1;
    size = mem_available - MAXLUP - MAXLPAC;               rc = 1;
  }
  extgap (d, size+10, FALSE); actual_size = QfsRead(f, size, d->dbeg);
  if (actual_size < 0) return -2;
  else {
    char *p = d->dbeg + actual_size;
    if (p[-1] != LF) *p++ = LF; 
    if (rc == 1) p = scpy("«...»\n",p); d->dend = p; return rc;
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
bool DqSave (deq *d, qfile *f)
{                               
  long len = DqLen(d);
  return (QfsWrite(f, len, d->dbeg) == len);
}
/*---------------------------------------------------------------------------*/
#include "twm.h" 
#include "tx.h"
#include "ud.h"
/*
 * Приоритет выбрасывания файлов обратный LRU внутри группы (тексты с хотя бы
 * одним окном никогда не выбрасываются -- в любой момент может потребоваться
 * переизобразить это окно на экране):
 *  0) pseudo|file[changed]      - throw-away text, выкинем без сожаления
 *  1) file|undo|dirlst[changed] - выкинем файл, откатку, освободим дескриптор
 *  2) file|undo         - выкинем только файл (если не поможет -> шаг 4)
 *  3) *                 - обрежем откатку (у ВСЕХ файлов) до положенной квоты
 *  4) undo              - выкинем откатку, освободим дескриптор
 *  5) file|undo|changed - сохраним и выкинем файл
 */
#define S_SFILE  001 /* save file  = сохраним файл        */
#define S_EFILE  002 /* erase file = выкинем файл         */
#define S_EUNDO  004 /* erase undo = выкинем откатку      */
#define S_EDESC  010 /* erase desc = освободим дескриптор */
#define S_UDCUT  020 /* cut undo (up to UDLQUOTA == 8192) */

static short mem_swap_rules[] =
{
  TS_BUSY|TS_PSEUDO|TS_FILE|TS_CHANGED,         S_EFILE|S_EDESC,
  TS_BUSY|TS_PSEUDO|TS_FILE,                    S_EFILE|S_EDESC,
  TS_BUSY|TS_FILE|TS_UNDO|TS_DIRLST|TS_CHANGED, S_EFILE|S_EUNDO|S_EDESC,
  TS_BUSY|TS_FILE|TS_UNDO|TS_DIRLST,            S_EFILE|S_EUNDO|S_EDESC,
  TS_BUSY|TS_FILE|TS_UNDO,                      S_EFILE,
  TS_DqSWAP,            /* applied to ALL -> */ S_UDCUT,
  TS_BUSY|TS_UNDO,                              S_EUNDO|S_EDESC,
  TS_BUSY|TS_FILE|TS_UNDO|TS_CHANGED,           S_SFILE|S_EFILE, 0
};
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
long tmswap (long required)
{
  short *pcode, mask, lrum, op; long available;
  txt *t, *tm;
  for (pcode = mem_swap_rules; (mask = *pcode++); pcode++) {
    if ((op = *pcode) & S_UDCUT) {
      for (t = texts; t; t = t->txnext)
        if (t->txstat && t->txudeq != NIL) udcut(t);
    //
      if ((available = DqFree()) < required) continue;
      else                           return available;
    }
    else for (;;) {
      for (t = texts, tm = NIL, lrum = 0; t; t = t->txnext) {
        if ((t->txstat & TS_DqSWAP) == mask && t->txlructr > lrum) {
          tm = t; lrum = t->txlructr;
      } }
      if (tm == NIL) break;  /* самого подходящего не нашлось */
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
      if (op & S_SFILE) {
        if (!tmsave(tm, FALSE)) { tm->txstat |= TS_SAVERR; continue; }
      }
      if (op & S_EFILE) {
        if (tm->clustk) DqEmpt(tm->clustk);
        if (tm->cldstk) DqEmpt(tm->cldstk);
        DqEmpt(tm->txustk);
        DqEmpt(tm->txdstk); tm->txy = 0; tm->txstat &= ~TS_FILE;
      }
      if (op & S_EUNDO) udclear(tm);
      if (op & S_EDESC) TxDel(tm);
      if ((available = DqFree()) >= required) return available;
  } }
/* Не удалось. В памяти остались: текущий файл, файл сохранения,
                         откатка текущего файла, урезанная до лимита */
  return DqFree();
}
/*---------------------------------------------------------------------------*/
