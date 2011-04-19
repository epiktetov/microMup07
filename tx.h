/*------------------------------------------------------+----------------------
// МикроМир07          Texts - Тексты (header)          | (c) Epi MG, 2007-2011
//------------------------------------------------------+--------------------*/
#ifndef TX_H_INCLUDED        /* Old tx.h (c) Attic 1989, (c) EpiMG 1998,2001 */
#define TX_H_INCLUDED

txt *TxNew(BOOL qundo); void TxInit(),                       TxMarks0(txt *t);
BOOL qTxUp  (txt *t);   void TxDel (txt *t), TxUp  (txt *t), TxTop   (txt *t);
BOOL qTxDown(txt *t);   void TxEmpt(txt *t), TxDown(txt *t), TxBottom(txt *t);
/*
 * Conversions between Ascii file string (UTF-8 encoding) and tchar string 
 *                                 returns destination size, in char/bytes
 */
small aftotc (const char *orig, int len, tchar *dest_buf);
small tctoaf (tchar      *orig, int len, char  *dest_buf);
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
small TxRead (txt *t,  char *p);     /* read text line into specified buffer */
small TxTRead(txt *t, tchar *p);     /*                                      */
small TxFRead(txt *t, tchar *p);     /* <- fill with spaces up to MAXLPAC    */

void TxRep (txt *t,  char *a, small len); /* replace */
void TxTRep(txt *t, tchar *a, small len);
void TxFRep(txt *t, tchar *p);
void TxIL  (txt *t,  char *line, small len), TxDL(txt *t), TxDEL_beg(txt *t);
void TxTIL (txt *t, tchar *line, small len),               TxDEL_end(txt *t);
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#define STKEXT    4096                    /* Оптимальный экстент для стеков  */
#define UNDOEXT   4096                    /* Оптимальный экстент для откатки */
#define MSG_EOF "^^ end of "

extern char    *afbuf;
extern char    *txbuf;
extern small tab_size;
extern txt     *texts;
/*---------------------------------------------------------------------------*/
#endif                                                      /* TX_H_INCLUDED */
