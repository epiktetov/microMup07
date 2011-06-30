/*------------------------------------------------------+----------------------
// МикроМир07  Command Codes Definition / transcoding   | (c) Epi MG, 2006-2011
//------------------------------------------------------+--------------------*/
#ifdef MAKE_TRANS_TABLE
//# define k_BEGIN        static micom trans1[] = {
//# define k_(e,k,v)        { e, k, v },
//# define a_(e,k,v)        { e, k, v },
//# define k_END_OF_TABLE };
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
#define mod_HOME  0x00a00000
#define mod_CtrJ  0x00c00000
#define mod_ESC   0x00e00000
#define Mk_IsSHIFT(x) (Qt::Key_Shift <= x && x < Qt::Key_F1)
#define Mk_IsCHAR(x) (' ' <= x && x < 0x10000) /* only 16bit Unicode allowed */
#ifdef MIM_H_INCLUDED /*- - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  int     MkConvertKeyMods(QKeyEvent *event, int &modMask);
  int     MkFromString    (QString keySequence);
  int     MkToDigit (int kcode);
  QString MkToString(int kcode);
  void    MkMimXEQ  (int kcode, int modMask, QString text, wnd *vp);
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
#define CA_CHANGE  004 /* изменяет состояние текста/строки                   */
#define CA_EXT     010 /* добавление строки текста в конец                   */
#define CA_RPT     020 /* сама обрабатывает аргумент (в т.ч. не повторяема)  */
#define CA_NEND   0100 /* указатель не в конце  строки/текста                */
#define CA_NBEG   0200 /* указатель не в начале строки/текста                */
#define CA_LCUT   0400 /* работает с текстом запомненных строк               */
                       /* Keycode attributes:                                */
#define KxTS   0x0e000 /*  move starts/extends "temporary" selection 0xe0000 */
#define KxBLK  0x10000 /*  keeps "permanent" selection               0xd0000 */
#define KxTMP  0x20000 /*  keeps "temporary" selection              not-used */
#define KxSEL  0x30000 /*  keeps any selection                       0xf0000 */
/*---------------------------------------------------------------------------*/
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void key2mimStart();
int key2mimHomeEscMask();
enum micom_enum key2mimCheckPrefix(int TK_HomeEscCtrJ);
struct micom {
  int ev; /* keyboard event                    */
//+  enum micom_enum ev; /* keyboard event                    */
  int kcode;          /* key code as reported by Qt        */
  int attr;           /* attributes KxBLK / KxTMP / KxSEL  */
};
struct micom *key2mimCmd(int qtKode); /* NIL if not a valid microMir command */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
extern int KbCode;                    /* Запрос клавиатуры: код запроса (ev) */
extern int KbCount;                   /*  повторитель, если не вводился то 1 */
extern int KbRadix;                   /*  основание,   если не вводился то 0 */
typedef struct {
  enum micom_enum mi_ev;  /* код команды */
  void  (*cfunc)(void);   /* подпрограмма, выполняющая команду */
  int attr;               /* атрибуты команды (CA_xxx below)   */
} comdesc;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#ifdef Q_OS_MAC_modifier_removed //+
#  define Mk_UP    (Qt::Key_Up   |Qt::KeypadModifier)
#  define Mk_DOWN  (Qt::Key_Down |Qt::KeypadModifier)
#  define Mk_LEFT  (Qt::Key_Left |Qt::KeypadModifier)
#  define Mk_RIGHT (Qt::Key_Right|Qt::KeypadModifier)
#else
#  define Mk_UP     Qt::Key_Up
#  define Mk_DOWN   Qt::Key_Down
#  define Mk_LEFT   Qt::Key_Left
#  define Mk_RIGHT  Qt::Key_Right
#endif
#define Mk_HOME     Qt::Key_Home
#define Mk_END      Qt::Key_End
#define Mk_PAGEUP   Qt::Key_PageUp
#define Mk_PAGEDOWN Qt::Key_PageDown

#define Mk_PAD_PLUS  (Qt::KeypadModifier|Qt::Key_Plus)
#define Mk_PAD_MINUS (Qt::KeypadModifier|Qt::Key_Minus)
#define Mk_PAD_SLASH (Qt::KeypadModifier|Qt::Key_Slash)
#define Mk_PAD_STAR  (Qt::KeypadModifier|Qt::Key_Asterisk)
#define Mk_PAD_ENTER (Qt::KeypadModifier|Qt::Key_Return)
#define Mk_PAD_UP    (Qt::KeypadModifier|'8') //
#define Mk_PAD_DOWN  (Qt::KeypadModifier|'2') // cannot use Qt::Key_Up etc here
#define Mk_PAD_LEFT  (Qt::KeypadModifier|'4') // because arrow keys from Mac KB
#define Mk_PAD_RIGHT (Qt::KeypadModifier|'6') // already have KeypadModifier

#define Mk_ESCAPE Qt::Key_Escape
#define Mk_ENTER  Qt::Key_Return
#define Mk_TAB    Qt::Key_Tab
#define Mk_BkTAB  Qt::Key_Backtab
#define Mk_BACK   Qt::Key_Backspace
#define Mk_CLEAR  Qt::Key_Clear

#define Mk_McENTER Qt::Key_Enter // Mac-Enter is not "Enter" (that is "Return")
#ifdef Q_OS_MAC                  // Insert key is called "Help" on Mac keyboard
#  define Mk_INSERT Qt::Key_Help // and the name sticks even when Microsoft k/b
#else                            // attached; restore the God-given name here
#  define Mk_INSERT Qt::Key_Insert
#endif
#define Mk_DELETE Qt::Key_Delete
#define Mk_F1     Qt::Key_F1
#define Mk_F2     Qt::Key_F2
#define Mk_F3     Qt::Key_F3
#define Mk_F4     Qt::Key_F4
#define Mk_F5     Qt::Key_F5
#define Mk_F6     Qt::Key_F6
#define Mk_F7     Qt::Key_F7
#define Mk_F8     Qt::Key_F8
#define Mk_F9     Qt::Key_F9
#define Mk_F10    Qt::Key_F10
#define Mk_F11    Qt::Key_F11
#define Mk_F12    Qt::Key_F12
#define Mk_F13    Qt::Key_F13
#define Mk_F14    Qt::Key_F14
#define Mk_F15    Qt::Key_F15
/*---------------------------------------------------------------------------*/
#endif                                                     /* CCD_H_INCLUDED */
