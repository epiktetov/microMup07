/*------------------------------------------------------+----------------------
// МикроМир07      Syntax checker / text colorizer      | (c) Epi MG, 2011
//------------------------------------------------------+--------------------*/
#ifndef SYNT_H_INCLUDED
#define SYNT_H_INCLUDED
#ifdef __cplusplus
small SyntKnownLang(QString filename); // returns lang code (or 0 if not known)
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
// Colorize the line from given text (the line assumed to be located in tcbuf),
// may change txt->thisSynts (in which case it empties the entire txt->cldstk):
//
void SyntColorize (txt *t, tchar *tcp, int len);

extern "C" {
#endif
/*  Parse the (packed) line, calculating Synts array and return its length   */
/*  (provided text used only to get prevSynts, no changes to text is made):  */
/*                                                                           */
int SyntParse(txt *t, char *str, int len, int *out); /* called from TxDown() */
#ifdef __cplusplus
}
#endif
/*---------------------------------------------------------------------------*/
#endif                                                    /* SYNT_H_INCLUDED */
