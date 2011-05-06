/*------------------------------------------------------+----------------------
// МикроМир07       Деки (deques) для Tx и Undo         | (c) Epi MG, 2007,2011
//------------------------------------------------------+--------------------*/
#ifndef DQ_H_INCLUDED                           /* Old "dq.h" (c) Attic 1989 */
#define DQ_H_INCLUDED

void DqInit(char *membuf, long bufsize);
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
struct deq_tag
{
  char     *dbeg, *dend;   /* указатель на начало / за конец дека            */
  small    dbext, deext;   /* оптимальный размер экстента для начала / конца */
  struct deq_tag *dnext,   /* следующий дек                                  */
                 *dprev;   /* предыдущий дек                                 */
  small            dtyp;   /* тип дека                                       */
  addr           dextra;   /* промежуток свыше квоты                         */
};
/* Типы деков - dtyp:          */
#define DT_ASC  0 /* текстовый */
#define DT_BIN  1 /* двоичный  */
//+fine DT_NONE 2 /* никакого неестественного интеллекта */
/*---------------------------------------------------------------------------*/
deq *DqNew(small typ, small bext, small eext);
BOOL DqDel(deq *d);

#define            cpdiff(p1, p2) (p1 - p2)
#define DqLen(d)   cpdiff(d->dend, d->dbeg)
#define qDqEmpt(d) ((d)->dbeg == (d)->dend)

void DqEmpt(deq *d);
BOOL DqLoad(deq *d, qfile *f, large size);
BOOL DqSave(deq *d, qfile *f);                 /* if move_prev => extend gap */
void extgap(deq *d, long len, BOOL move_prev); /* for previous (upper) deque */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void DqAddB(deq *d, char *data, int len);  /* add to Beginning (at dbeg ptr) */
void DqAddE(deq *d, char *data, int len);  /* add to End       (at dend ptr) */

char *DqLookupForw(deq *d, int pos, int *len_out, int *real_len_out);
char *DqLookupBack(deq *d, int pos, int *len_out, int *real_len_out);
int DqCopyForward (deq *d, int pos, char *pb_out, int *real_len_out);
int DqCopyBackward(deq *d, int pos, char *pb_out, int *real_len_out);

//+#define DqCopyB(d,out) DqCopyForward (d,0,       out,0)
//+#define DqCopyE(d,out) DqCopyBackward(d,DqLen(d),out,0)

int DqGetB(deq *d, char *out);
int DqGetE(deq *d, char *out);

void DqMoveBtoE (deq *from, deq *to);  /* move one record from.Beg -> to.End */
void DqMoveEtoB (deq *from, deq *to);  /* move one record from.End -> to.Beg */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void DqCutB_byX (deq *d, long  dx);                     /* used only in ud.c */
void DqCutE_toX (deq *d, long len);
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
BOOL qFilSwap();
BOOL tmswap (BOOL freedesc);
/*---------------------------------------------------------------------------*/
#endif                                                      /* DQ_H_INCLUDED */
