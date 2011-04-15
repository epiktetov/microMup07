/*------------------------------------------------------+----------------------
// МикроМир07         Самый главный header файл         | (c) Epi MG, 2006-2011
//------------------------------------------------------+--------------------*/
#ifndef MIC_H_INCLUDED  /* Old "nm.h" (c) Attic 1989-90, (c) EpiMG 1998-2001 */
#define MIC_H_INCLUDED
#define microVERSION "4.4.14" // released Thu Apr 14 23:30 PDT 2011
#ifdef __cplusplus
extern "C" {
#endif
#ifdef small /*----------- Определим свои типы (for legacy C code) ----------*/
#undef small
#endif
typedef short small; /* (at least) 16-bit signed integer      */
typedef long  large; /* (at least) 32-bit integer             */
typedef long  addr;  /* Целое достаточного размера для адреса */
typedef int   BOOL;  
#ifndef FALSE
# define TRUE  ((BOOL)1)
# define FALSE ((BOOL)0)
#endif
#define CR      '\015'
#define LF      '\012'
#define TAB     '\011'
#define EOFchar '\032' /* 'EOF' is reserved somewhere */
#define ACPR    '\xCA'
#define ACPR2   '\x80'
#define local static                           /* Локальная (static) функция */
#define ctotc(c) ((tchar)(unsigned char)(c))   /* <- без расширения знака    */
#define NIL      ((void*)0) /* указатель (same as standard NULL but shorter) */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
typedef long tchar;                           /* Unicode символ с атрибутами */
#define AT_ALL     0x03FF0000
#define AT_CHAR    0x0000FFFF /* собственно Unicode символ (use only 16-bit) */
#define AT_BOLD    0x00010000 /* Bold                               == KxTS  */
#define AT_LIGHT   0x00020000 /* light (blue), used for prompts     == KxBLK */
#define AT_ITALIC  0x00040000 /* italic (red), incorrect Unicode    == KxTMP */
#define AT_SUPER   0x00080000 /* sky blue (special chars) forces Insert mode */
#define AT_UNDERL  0x00100000 /* reserved for underline                      */
#define AT_COMMAND 0x00200000 /* rounded negative background (macro)         */
#define AT_IN_FILE 0x003F0000 /* <---- mask of attributes saved in file ---- */
#define AT_BG_CLR  0x00c00000 /* background color:                           */
#define AT_BG_RED  0x00400000 /*  = red (cannot edit / "temporary" block)    */
#define AT_BG_GRN  0x00800000 /*  = green (can edit, text unchanged)         */
#define AT_BG_BLU  0x00c00000 /*  = blue (text changed / regular block mark) */
#define AT_INVERT  0x01000000 /* when added to AT_BG_CLR => indicates cursor */
#define AT_TAB     0x02000000 /* special attribute to preserve TABs in files */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#define AF_NONE    "\xCA\x80" /*   file representation for some attributes   */
#define AF_LIGHT   "\xCA\x82" /*   (used to set nice prompt for LenterARG)   */
#define AF_SUPER   "\xCA\x88"
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#define CpSCH_NEW_LINE  '\f'          /* Beginning of new line in CP buffer  */
#define LeSCH_REPL_BEG  AT_SUPER+'>'  /* Replace substring begin             */
#define LeSCH_REPL_END  AT_SUPER+'/'  /* Replace substring end / files start */
#define TeSCH_CONTINUE  AT_SUPER+0xBB /* Line continuation mark (TE_FORMAT)  */
/*---------------------------------------------------------------------------*/
extern int TABsize; /* Настраиваемые параметры - табуляция, 4 или 8 символов */
extern BOOL dosEOL; /* - DOS/Windows style for end-of-line (CR/LF вместо CR) */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#define MAXPATH    400                     /* Некоторые магические константы */
#define MAXDIRSTK   12
#define UBUFSIZ  32768 /* Буфер откатки                */
#define UDHQUOTA 16384 /* Квота откатки (на один файл) */
#define UDLQUOTA  8192
#define MAXFILES    64
#define MAXDEQS    200 /*                    at least (MAXFILES*3+something) */
#define MAXTXRM    360 /* Максимальная ширина текта в редакторе              */
#define MAXLUP   32768 /* Максимальная длина строки в файле                  */
#define MAXLPAC  16380 /* Максимальная длина строки в редакторе (le.c, te.c) */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#define Mk_IsCHAR(x) (' ' <= x && x < 0x10000) /* only 16bit Unicode allowed */
#define Mk_IsSHIFT(x) (Qt::Key_Shift <= x && x < Qt::Key_F1)
#define mod_SHIFT 0x02000000
#define mod_CTRL  0x04000000
#define mod_ALT   0x08000000
#define mod_META  0x10000000   /* NOTE: 0x20000000 is Qt::KeypadModifier */
#define mod_ESC   0x40000000
#define mod_HOME  0x80000000
/*---------------------------------------- Коды ошибок (или событий) --------*/
#define E_OK         0   /* Все в порядке                           -- Общие */
#define E_NOCOM      1   /* Неопознанная команда для данного уровня          */
#define E_CHANGE     2   /* Запрещены изменения текста                       */
#define E_KBREAK     3   /* Выполнение команды прервано с клавиатуры         */
#define E_UP         4   /* Откатка переходит на внешний уровень             */
#define E_DOWN       5   /* Откатка переходит на внутренний уровень          */
#define E_NOUNDO     6   /* Откатка не поддерживается                        */
#define E_LENTER     7   /* Started Argument Entering mode                   */
#define E_LEXIT      8   /* Argument Entering mode completed                 */
                         /*------------------------ События редактора строки */
#define E_MOVBEG     9   /* Перемещение за левую границу                     */
#define E_MOVEND    10   /* Перемещение за правую границу                    */
#define E_EDTBEG    11   /* Изменение за левой границей                      */
#define E_EDTEND    12   /* Изменение за правой зраницей                     */
#define E_ATTR      13   /* Запрещена работа с атрибутами                    */
#define E_ILCHAR    14   /* совсем непечатный символ                         */
#define E_CCFUL     15   /* Буфер запомненных символов полон                 */
                         /*--------------- События редактора текста и прочее */
#define E_NOOP      16   /* Нечего делать (команда неприминима в этом месте) */
#define E_BADPRM    17   /* Недопустимое значение параметра / повторителя    */
#define E_LCUT      18   /* Запоминание строк в файле NDTSAV                 */
#define E_MOVUP     19   /* Верхняя граница текста                           */
#define E_MOVDOWN   20   /* Нижняя зраница текста                            */
#define E_SETY      21   /* Позиционирование за границы текста               */
#define E_FINISHED  22   /* Выполнение операции завершилось                  */
#define E_LPASTE    23
#define E_NOBLOCK   24
#define E_BLOCKOUT  25
#define E_SFAIL     26
#define E_NOSPAT    27
#define E_NORPAT    28
#define E_ROUT      29
#define E_FORMAT    30
#define E_CPFER     31
#define E_PRINT     32
#define E_FINDER    33
#define E_NOMEM     34  /* Нет памяти - могут быть  фатальные последствия    */
#define E_CB        35  /* Clipboard error (Windows only)                    */
#define E_MAX       36
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
        struct deq_tag;         /* Forward declarations of some C structures */                  
typedef struct deq_tag deq;     /*  dq.h */
        struct qfile_tag;
typedef struct qfile_tag qfile; /* qfs.h */
        struct txt_tag;
typedef struct txt_tag txt;     /* vip.h */
        struct wnd_tag;
typedef struct wnd_tag wnd;     /* vip.h */
/*---------------------------------------------------------------------------*/
extern char *GetMain(long n); /* =malloc() but always returns non-NULL, rt.c */
/*
 * Инициализация "большого" сегмента памяти -- platform-specific manipulations
 * with VirtualAlloc() etc are removed, now it just malloc()'s a bug chunk and
 * let virtual memory subsystem do optimization by itself
 */
char *MemInit(long *allocated_size);
/*
 * Часто употребляемые функции:
 */
extern char 
  *sncpy(const char *from, char *to, int how_many), /* copy functions return */
  *scpy (const char *from, char *to),               /* pointer to the end of */
  *scpyx(const char *from, char *to, int limit),    /* destination string    */
  *dtol(char *pn, large *dest),
  *dtos(char *pn, small *dest),
  *ltod(char *buffer, large l), /* return pointer to end-of-num   */
  *stod(char *buffer, small s); 

char *lblkmov(char *From, char *To, long len); /* returns ptr after the dest */
void * blkmov(void *From, void *To, long len); /*        block (i.e. To+len) */
tchar *blktmov(tchar *From, tchar *To, long len);
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void exc(int code);                                                  /* rt.c */
#include <setjmp.h>
extern jmp_buf *excptr;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#ifdef __cplusplus           /* QString <-> tchar string conversion, vip.cpp */
}
#ifdef QSTRING_H
  QString tcs2qstr(tchar *tcs, int len);
  int  qstr2tcs (QString qstr, tchar *buffer, tchar attr = 0); // returns len
#endif                                                         // of t-string
#endif /* __cplusplus */
/*---------------------------------------------------------------------------*/
#define cStr() toLocal8Bit().data() /* to pass wxString value to legacy code */
#define uStr()      toUtf8().data() /* the same, but convert value to UTF-8  */

#define my_max(a, b) ((a)>(b) ? (a) : (b))
#define my_min(a, b) ((a)<(b) ? (a) : (b))
#define my_abs(a)    ((a)>=0 ? (a) : -(a))
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#define L2_INSERT(_head,_elem,_prev,_next)   \
  _elem->_prev = NULL; _elem->_next = _head; \
  if (_head)           _head->_prev = _elem; _head = _elem;

#define L2_DELETE(_head,_elem,_prev,_next) \
  if (_elem->_prev) _elem->_prev->_next = _elem->_next; \
  if (_elem->_next) _elem->_next->_prev = _elem->_prev; \
  if (_head == _elem) _head = _elem->_next;

#define L2C_IS_EMPTY(_elem,_prev,_next) (_elem->_next == _elem)
#define L2C_SET_EMPTY(_elem,_prev,_next) \
   _elem->_next = _elem; _elem->_prev = _elem;

#define L2C_INSERT(_peer,_elem,_prev,_next) \
  _elem->_prev = _peer; _elem->_next = _peer->_next; \
  _peer->_next->_prev = _elem; _peer->_next = _elem;

#define L2C_DELETE(_elem,_prev,_next) \
  _elem->_next->_prev = _elem->_prev; _elem->_prev->_next = _elem->_next;
/*---------------------------------------------------------------------------*/
#endif                                                     /* MIC_H_INCLUDED */
