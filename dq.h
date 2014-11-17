/*------------------------------------------------------+----------------------
// МикроМир07       Деки (deques) для Tx и Undo         | (c) Epi MG, 2007-2014
//------------------------------------------------------+--------------------*/
#ifndef DQ_H_INCLUDED                           /* Old "dq.h" (c) Attic 1989 */
#define DQ_H_INCLUDED

void DqInit(char *membuf, long bufsize);
long DqFree();                               /* <- total free memory in deqs */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
struct deq_tag
{
  char     *dbeg, *dend;   /* указатель на начало / за конец дека            */
  short    dbext, deext;   /* оптимальный размер экстента для начала / конца */
  struct deq_tag *dnext,   /* следующий дек                                  */
                 *dprev;   /* предыдущий дек                                 */
  long           dextra;   /* промежуток свыше квоты                         */
  short            dtyp;   /* тип дека: 't'ext, 'u'ndo, 's'ynt-info          */
};
/* Типы деков - поле dtyp (младший бит означает двоичный дек:  */
#define DT_TEXT 't' /* верхний/нижний стек для текстов (ascii) */
#define DT_UNDO 'u' /* буфер откатки                  (binary) */
#define DT_SYNT 's' /* буфер для Syntax checker       (binary) */
/*---------------------------------------------------------------------------*/
deq *DqNew(short typ, short bext, short eext);
bool DqDel(deq *d);

#define DT_IsBIN(dtyp) (dtyp & 1)
#define            cpdiff(p1, p2) (p1 - p2)
#define DqLen(d)   cpdiff(d->dend, d->dbeg)
#define qDqEmpt(d) ((d)->dbeg == (d)->dend)

int  DqLoad(deq *d, qfile *f, long size); /* -1:fail, 0:empty, 1:trunc, 2:ok */
void DqEmpt(deq *d);
bool DqSave(deq *d, qfile *f);                 /* if move_prev => extend gap */
void extgap(deq *d, long len, bool move_prev); /* for previous (upper) deque */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void DqAddB(deq *d, char *data, int len);  /* add to Beginning (at dbeg ptr) */
void DqAddE(deq *d, char *data, int len);  /* add to End       (at dend ptr) */

char *DqLookupForw(deq *d, int pos, int *len_out, int *real_len_out);
char *DqLookupBack(deq *d, int pos, int *len_out, int *real_len_out);
int DqCopyForward (deq *d, int pos, char *pb_out, int *real_len_out);
int DqCopyBackward(deq *d, int pos, char *pb_out, int *real_len_out);

int DqGetB(deq *d, char *out);
int DqGetE(deq *d, char *out);

void DqMoveBtoE (deq *from, deq *to);  /* move one record from.Beg -> to.End */
void DqMoveEtoB (deq *from, deq *to);  /* move one record from.End -> to.Beg */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void DqCutB_byX (deq *d, long  dx);                     /* used only in ud.c */
void DqCutE_toX (deq *d, long len);
/*---------------------------------------------------------------------------*/
#endif                                                      /* DQ_H_INCLUDED */
