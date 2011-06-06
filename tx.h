/*------------------------------------------------------+----------------------
// МикроМир07          Texts - Тексты (header)          | (c) Epi MG, 2007-2011
//------------------------------------------------------+--------------------*/
#ifndef TX_H_INCLUDED        /* Old tx.h (c) Attic 1989, (c) EpiMG 1998,2001 */
#define TX_H_INCLUDED

txt *TxNew(bool qundo); void TxInit(),                       TxMarks0(txt *t);
bool qTxTop   (txt *t); void TxDel (txt *t), TxUp  (txt *t), TxTop   (txt *t);
bool qTxBottom(txt *t); void TxEmpt(txt *t), TxDown(txt *t), TxBottom(txt *t);
void TxEnableSynt(txt *t, short clang);
/*
 * Conversions between Ascii file string (UTF-8 encoding) and tchar string 
 *                                 returns destination size, in char/bytes
 */
short aftotc (const char *orig, int len, tchar *dest_buf);
short tctoaf (tchar      *orig, int len, char  *dest_buf);
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
char *TxGetLn(txt *t, int *len);     /* get top (raw) line from upper stack  */
short TxRead (txt *t,  char *p);     /* read text line into specified buffer */
short TxTRead(txt *t, tchar *p);     /*                                      */
short TxFRead(txt *t, tchar *p);     /* <- fill with spaces up to MAXLPAC    */

void TxRep (txt *t,  char *a, short len); /* replace */
void TxTRep(txt *t, tchar *a, short len);
void TxFRep(txt *t, tchar *p);
void TxIL  (txt *t,  char *line, short len), TxDL(txt *t), TxDEL_beg(txt *t);
void TxTIL (txt *t, tchar *line, short len),               TxDEL_end(txt *t);
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
extern char   afbuf[]; extern short tab_size;
extern char   txbuf[]; extern txt     *texts;
/*---------------------------------------------------------------------------*/
#endif                                                      /* TX_H_INCLUDED */
