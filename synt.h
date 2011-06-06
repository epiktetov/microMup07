/*------------------------------------------------------+----------------------
// МикроМир07      Syntax checker / text colorizer      | (c) Epi MG, 2011
//------------------------------------------------------+--------------------*/
#ifndef SYNT_H_INCLUDED
#define SYNT_H_INCLUDED
#ifdef __cplusplus
short SyntKnownLang(QString filename); // returns lang code (or 0 if not known)
short SyntSniffText(txt *text);
enum SyntSupportedLangs {
  CLangNONE = 0, //                  not known / no syntax check & colorization
  CLangGEN  = 1, // generic (eol comment is '#', brackets checked, no keywords)
  CLangSH   = 2, // [ba]sh shell script
  CLangCPP  = 3, // C/C++
  CLangPERL = 4, // Perl   (not implemented yet)
  CLangPYTH = 5, // Python (not implemented yet)
  CLangMAX,
  CLangDISABLED = 8
};
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
// Colorize the line from given text (the line assumed to be located in tcbuf),
// may change txt->thisSynts (in which case it empties the entire txt->cldstk),
// and may add something past the end of the supplied text, updating len param:
//
void SyntColorize (txt *t, tchar *tcp, int& len);

extern "C" {
#endif
/*  Parse the (packed) line, calculating Synts array and return its length   */
/*  (provided text used only to get prevSynts, no changes to text is made):  */
/*                                                                           */
int SyntParse(txt *t, char *str, int len, int *out); /* called from TxDown() */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
char SyntType(tchar tc); /* syntactic type of the given character: `(0x) etc */
void SyntLangOn();
void SyntLangOff();
void SyntBrakToggle();
#ifdef __cplusplus
}
#endif
/*---------------------------------------------------------------------------*/
#endif                                                    /* SYNT_H_INCLUDED */
