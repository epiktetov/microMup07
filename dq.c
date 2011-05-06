/*------------------------------------------------------+----------------------
// МикроМир07       Деки                                | (c) Epi MG, 2007,2011
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
#include "ccd.h" /* need E_NOMEM for exc() */
#include "qfs.h"
#include "vip.h"
#include "dq.h"

static deq deq0, *freedeqs = NIL, *gapdeq, *lockdeq;
static BOOL  DqReserveExt = TRUE;
static long  DqReqMem; /* required memory for extend_gap(), used by qFilSwap */
/*---------------------------------------------------------------------------*/
void DqInit(char *membuf, long bufsize)     /* create "anti-deq" that covers */
{                                           /* memory that other deqs cannot */
  deq0.dbext = 0; gapdeq = NIL;             /* use (it has reverse beg/end)  */
  deq0.deext = 0;
  deq0.dprev = &deq0;  deq0.dend = membuf;
  deq0.dnext = &deq0;  deq0.dbeg = membuf+bufsize;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
deq *DqNew (small typ, small bext, small eext)
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
BOOL DqDel (deq *d)
{
  if (d == lockdeq) return FALSE;
  else {
    (d->dnext)->dprev = d->dprev;
    (d->dprev)->dnext = d->dnext;
    if (d == gapdeq) gapdeq = d->dprev;
    d->dnext = freedeqs;  freedeqs = d; return TRUE;
} }
/*---------------------------------------------------------------------------*/
static long extragap(deq *d) 
{                             
  deq *d2 = d->dnext;
  long len = cpdiff(d2->dbeg, d->dend);
  if (DqReserveExt) {
    long maxext = my_max(d->deext, d2->dbext);
    if (len > maxext) len -= maxext;
    else              len  =      0;
  }
  return (d->dextra = len & (~1));
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static long DqFree() 
{                     
  deq *d = &deq0;
  long f = 0;
  do { f += extragap(d); } while ((d = d->dnext) != &deq0); return f;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static int extend_gap (deq *d, long len, BOOL move_previous)
{
  deq *d2;
  long  m;                           lockdeq = d;
  if (move_previous) d = d->dprev; d2 = d->dnext;
/*
 * Если промежуток имеет достаточный размер, то ничего не делать
 */
   if (cpdiff (d2->dbeg, d->dend) >= len) { lockdeq = NIL; return 0; }
/*
 * Иначе найдем место для оптимального экстента и освободим необходимую память
 */
  if (d ->deext > len) len = d ->deext;
  if (d2->dbext > len) len = d2->dbext; DqReqMem = len;
  gapdeq = d;
  if (DqFree() < len)  /* нет места => просим диспетчера текстов потесниться */
  {                                             qFilSwap();
    if (DqFree() < len) { DqReserveExt = FALSE; qFilSwap(); }
    if (DqFree() < len) {
      vipError( "Нет памяти (out ot memory)"); return -1;
  } }
/*
 * Соберем ее в нашем промежутке
 */
  for (m = 0, d = &deq0; d2 = d, d = d2->dnext, d2 != gapdeq; ) {
    if (m += d2->dextra) {
      lblkmov(d->dbeg, d->dbeg - m, cpdiff(d->dend, d->dbeg));
      d->dbeg -= m;
      d->dend -= m;
  } }
  for (m = 0, d2 = &deq0; d = d2, d2 = d2->dprev, d2 != gapdeq; ) {
    if (m += d2->dextra) {
      lblkmov(d2->dbeg, d2->dbeg + m, cpdiff(d2->dend, d2->dbeg));
      d2->dbeg += m;
      d2->dend += m;
  } }
  lockdeq = NIL; gapdeq = NIL; DqReserveExt = TRUE; return 0;
}
void extgap (deq *d, long len, BOOL move_previous)
{
  if (extend_gap(d, len, move_previous) < 0) exc(E_NOMEM);
}
/*---------------------------------------------------------------------------*/
void DqAddB (deq *d, char *data, int len)
{
  char *pb;
  if (d->dtyp == DT_ASC) {
    if (dosEOL) {
      extgap(d, len+2, TRUE);                 d->dbeg -= len+2;
      pb = lblkmov(data, d->dbeg, len); *pb++ = CR; *pb++ = LF;
    }
    else { extgap(d, len+1, TRUE);     d->dbeg -= len+1;
           pb = lblkmov(data, d->dbeg, len); *pb++ = LF; }
  }
  else {
    int real_len = (len+1) & (~1); 
    extgap (d, real_len+4,  TRUE);
    d->dbeg -= real_len+4;   pb  = d->dbeg;    *(small*)pb = len;
    blkmov(data, pb+2, len); pb += real_len+2; *(small*)pb = len;
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void DqAddE (deq *d, char *data, int len)
{
  char *pb;
  if (d->dtyp == DT_ASC) {
    if (dosEOL) { extgap(d, len+2, FALSE);
                  pb = lblkmov(data, d->dend, len); *pb++ = CR; *pb++ = LF; }
    else { 
      extgap(d, len+1, FALSE);
      pb = lblkmov(data, d->dend, len); *pb++ = LF;
    }
    d->dend = pb;
  }
  else {
    int real_len = (len+1) & (~1);
    extgap (d, real_len+4, FALSE); 
    pb = d->dend;       d->dend += real_len+4; *(small*)pb = len;
    blkmov(data, pb+2, len); pb += real_len+2; *(small*)pb = len;
} }
/*---------------------------------------------------------------------------*/
char *DqLookupForw(deq *d, int pos, int *len_out, int *real_len_out)
{
  char *pb, *pbeg; small *pi;
  int len, real_len;
  if (d->dtyp == DT_ASC) {
    pbeg = d->dbeg + pos;
    for (pb = pbeg+1; pb < d->dend && pb[-1] != LF; pb++);
    len = (real_len = pb - pbeg) - 1;
    if (len < 0) exc(E_MOVDOWN);
    if (len > 0 && pb[-2] == CR) len--;
  } 
  else {
    pi = (small*)(d->dbeg + pos);         len = *pi++;
    pbeg = (char*)pi; real_len = ((len+1) & (~1)) + 4;
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
  char *pb, *pend; small *pi;
  long len, real_len;
  if (d->dtyp == DT_ASC) {
    pend = d->dbeg + pos;
    for (pb = pend-1; pb > d->dbeg && pb[-1] != LF; pb--);
    len = (real_len = pend - pb) - 1;
    if (len < 0) exc(E_MOVUP);
    if (len > 0 && pend[-2] == CR) len--;
  } 
  else {
    pi = (small*)(pend = d->dbeg + pos);    len = *(--pi);
    real_len = ((len+1) & (~1)) + 4; pb = pend-real_len+2;
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
BOOL DqLoad (deq *d, qfile *f, large size)
{
  large actual_size; char *p;
  DqEmpt(d);
  if (            extend_gap(d, size+2, FALSE)  < 0) return FALSE;
  if ((actual_size = QfsRead(f, size, d->dbeg)) < 0) return FALSE;
  p = d->dbeg + actual_size;
  if (d->dtyp == DT_ASC) { 
    if (p[-1] == EOFchar)  p--;
    if (p[-1] != LF) *p++ = LF; 
  }
  d->dend = p; return TRUE;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
BOOL DqSave (deq *d, qfile *f) 
{                               
  large len = DqLen(d);
  return (QfsWrite(f, len, d->dbeg) == len);
}
/*---------------------------------------------------------------------------*/
#include "twm.h" 
#include "tx.h"
#include "ud.h"
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
BOOL qFilSwap (void)      /* Освободить память, возвращает TRUE если удалось */
{
  txt *t; for (t = texts; t != NIL; t = t->txnext)
            if (t->txstat && t->txudeq != NIL) udcut(t);

  if (DqFree() < DqReqMem) return tmswap(FALSE);
  else                     return TRUE;
}
/* Приоритет выбрасывания файлов обратный LRU внутри группы (тексты с хотя бы
 * одним окном никогда не выбрасываются -- в любой момент может потребоваться
 * переизобразить это окно на экране)
 * Для освобождения памяти (биты TS_NEW и TS_PSEUDO игнорируются):
 *  1) file|undo|dirlst  - выкинем файл, откатку, освободим дескриптор
 *  2) file|undo         - выкинем файл
 *  3) undo              - выкинем откатку, освободим дескриптор
 *  4) file|undo|changed - сохраним и выкинем файл
 * Для освобождения дескрипторов:
 *  1) file|undo|dirlst  - выкинем файл, откатку, освободим дескриптор
 *  2) undo              - выкинем откатку, освободим дескриптор
 *  3) file|undo         - выкинем файл, выкинем откатку, освободим дескриптор
 */
#define S_SFILE  001 /* save file  = сохраним файл        */
#define S_EFILE  002 /* erase file = выкинем файл         */
#define S_EUNDO  004 /* erase undo = выкинем откатку      */
#define S_EDESC  010 /* erase desc = освободим дескриптор */

static small memswr[] = 
{
  TS_BUSY|TS_FILE|TS_UNDO|TS_DIRLST,  S_EFILE|S_EUNDO|S_EDESC,
  TS_BUSY|TS_FILE|TS_UNDO,            S_EFILE,
  TS_BUSY|TS_UNDO,                    S_EUNDO|S_EDESC,
  TS_BUSY|TS_FILE|TS_UNDO|TS_CHANGED, S_SFILE|S_EFILE, 0
};
static small descswr[] = {
  TS_BUSY|TS_FILE|TS_UNDO|TS_DIRLST,  S_EFILE|S_EUNDO|S_EDESC,
  TS_BUSY|TS_UNDO,                    S_EUNDO|S_EDESC,
  TS_BUSY|TS_FILE|TS_UNDO,            S_EFILE|S_EUNDO|S_EDESC, 0
};
BOOL tmswap (BOOL freedesc)
{
  small *pcode = freedesc ? descswr : memswr, lrum, mask;
  txt *t, *tm;
  for (; (mask = *pcode++); pcode++) { /* Применим очередное правило... */
    for (;;) {
      for (t = texts, tm = NIL, lrum = 0; t; t = t->txnext) {
        if ((t->txstat & ~(TS_NEW|TS_PSEUDO)) == mask && t->txlructr > lrum) {
          tm = t; lrum = t->txlructr;
      } }
      if ((t = tm) == NIL) break; /* самого подходящего не нашлось */
      lrum = *pcode;
      if (lrum & S_SFILE) {
        if (!tmsave(t, FALSE)) { t->txstat |= TS_ERR; continue; }
      }
      if (lrum & S_EFILE) {
        DqEmpt(t->txustk); 
        DqEmpt(t->txdstk); t->txy = 0; t->txstat &= ~TS_FILE;
      }
      if (lrum & S_EUNDO) {
        DqEmpt(t->txudeq);
        t->txudcptr = t->txudlptr = 0; /* t->txstat &= ~TS_UNDO; */
      }
      if (lrum & S_EDESC) { 
        TxDel(t); if (freedesc) return TRUE; 
      }
      if (DqFree() >= DqReqMem) return TRUE;
  } }
/* Не удалось. В памяти остались текущий файл, файл сохранения,
                                 откатка текущего файла, урезанная до лимита */
  return FALSE;
}
/*---------------------------------------------------------------------------*/
