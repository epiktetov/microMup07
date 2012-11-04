/*------------------------------------------------------+----------------------
// МикроМир07  Command Codes Definition / transcoding   | (c) Epi MG, 2006-2012
//------------------------------------------------------+--------------------*/
#ifdef MAKE_TRANS_TABLE
# define k_BEGIN        static microCCD CCD[] = {
# define k_(e,k,v)        { e, k, v },
# define a_(e,k,v)        { e, k, v },
# define k_END_OF_TABLE };
#else
# ifdef CCD_H_INCLUDED
#  define k_BEGIN
#  define k_(e,k,v)
#  define a_(e,k,v)
#  define k_END_OF_TABLE
# else
#  define k_BEGIN        enum micom_enum {
#  define k_(e,k,v)        e = v,
#  define a_(e,k,v)
#  define k_END_OF_TABLE };
# endif
#endif
#include "micro.keys"
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#undef k_BEGIN
#undef k_
#undef a_
#undef k_END_OF_TABLE
/*---------------------------------------------------------------------------*/
#ifndef CCD_H_INCLUDED
#define CCD_H_INCLUDED
#define mod_SHIFT 0x02000000 /* Mostly equivalent to Qt::SHIFT/CTRL/ALT/META */
#define mod_CTRL  0x04000000 /* except mod_CTRL == Ctrl even on Mac keyboard */
#define mod_ALT   0x08000000
#define mod_META  0x10000000
#define mod_KyPAD 0x20000000
#define Mk_IsSHIFT(x) (Qt::Key_Shift <= x && x < Qt::Key_F1)
#define Mk_IsCHAR(x) (' ' <= x && x < 0x10000) /* only 16bit Unicode allowed */
#ifdef MIM_H_INCLUDED /*- - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  int     MkConvertKeyMods(QKeyEvent *event, int &modMask);
  int     MkFromString    (QString keySequence);
  int     MkToDigit (int kcode);
  QString MkToString(int kcode);
  void    MkMimXEQ  (int kcode, int modMask, QString text, wnd *vp);
  void    MkStrXEQ                          (QString text, wnd *vp);
  void    MkLuaXEQ                          (QString text, wnd *vp);
#endif
struct microCCD {
  int ev;             /* keyboard event (usually micom_enum)          */
  const char *kseq;   /* key sequence (text format) as reported by Qt */
  int hexcode;        /* hex code with KxBLK/KxTMP/KxSEL attributes   */
};
void MkInitCCD(void);
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#define CA_BLOCK   001 /* block command (работает по другому если есть блок) */
#define CA_LEARG   002 /* работает иначе внутри LenterARG                    */
#define CA_MOD     004 /* изменяет состояние текста/строки                   */
#define CA_EXT     010 /* добавление строки текста в конец                   */
#define CA_RPT     020 /* сама обрабатывает аргумент (в т.ч. не повторяема)  */
#define CA_LINEAR  040 /* "линейная" команда - может выполнятхся над многими */
#define CA_LinMOD  044 /*       строками сразу (tall cursor), изменяет текст */
#define CA_NEND   0100 /* указатель не в конце  строки/текста                */
#define CA_NBEG   0200 /* указатель не в начале строки/текста                */
#define CA_LCUT   0400 /* работает с текстом запомненных строк               */
                       /* Keycode attributes:                                */
#define KxTS   0x0e000 /*  move starts/extends "temporary" selection 0xe0000 */
#define KxBLK  0x10000 /*  keeps "permanent" selection               0xd0000 */
#define KxTMP  0x20000 /*  keeps "temporary" selection              not-used */
#define KxSEL  0x30000 /*  keeps any selection                       0xf0000 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
extern int KbCode;                    /* Запрос клавиатуры: код запроса (ev) */
extern int KbCount;                   /*  повторитель, если не вводился то 1 */
extern int KbRadix;                   /*  основание,   если не вводился то 0 */
typedef struct {
  enum micom_enum mi_ev;  /* код команды */
  void  (*cfunc)(void);   /* подпрограмма, выполняющая команду */
  int attr;               /* атрибуты команды (CA_xxx above)   */
} comdesc;
/*---------------------------------------------------------------------------*/
#endif                                                     /* CCD_H_INCLUDED */
