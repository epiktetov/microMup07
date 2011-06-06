/*------------------------------------------------------+----------------------
// МикроМир07         Самый главный header файл         | (c) Epi MG, 2006-2011
//------------------------------------------------------+--------------------*/
#ifndef MIC_H_INCLUDED  /* Old "nm.h" (c) Attic 1989-90, (c) EpiMG 1998-2001 */
#define MIC_H_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#include <stdbool.h>
#ifndef FALSE
# define TRUE  ((bool)1)
# define FALSE ((bool)0)
#endif
#define CR      '\015'
#define LF      '\012'
#define clipEOL '\012'
#define TAB     '\011'
#define EOFchar '\032' /* 'EOF' is reserved somewhere */
#define ACPR    '\xCA'
#define ACPR2   '\x80'
#define ctotc(c) ((tchar)(unsigned char)(c))   /* <- без расширения знака    */
#define NIL      ((void*)0) /* указатель (same as standard NULL but shorter) */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
typedef long tchar;                           /* Unicode символ с атрибутами */
#define tATTR(x)(x&0x7FFF0000)
#define AT_CHAR    0x0000FFFF /* собственно Unicode символ (use only 16-bit) */
#define AT_BOLD    0x00010000 /* Bold                               == KxTS  */
#define AT_PROMPT  0x00020000 /* prompts text (def. dark blue text) == KxBLK */
#define AT_REGEX   0x00040000 /* indicates regex search (dark red)  == KxTMP */
#define AT_SUPER   0x00080000 /* sky blue (special chars) forces Insert mode */
#define AT_DIRTY   0x00100000 /* "dirty" mark  (was: reserved for underline) */
#define AT_IN_FILE 0x001F0000 /* <---- mask of attributes saved in file ---- */
#define AT_INVERT  0x00200000 /* when added to AT_BG_CLR => indicates cursor */
#define AT_BG_CLR  0x00c00000 /* background color:                           */
#define AT_BG_RED  0x00400000 /*  = red (cannot edit / "temporary" block)    */
#define AT_BG_GRN  0x00800000 /*  = green (can edit, text unchanged)         */
#define AT_BG_BLU  0x00c00000 /*  = blue (text changed / regular block mark) */
#define AT_MARKFLG 0x01000000 /* numbered marker flag (solid color gradient) */
#define AT_BRIGHT  0x02000000 /* bright background (for error/open brackets) */
#define AT_ERROR   0x02400000 /* - error mark                                */
#define AT_MARKOK  0x02800000 /* - regular (ok) mark                         */
#define AT_QUOTE   0x04000000 /* "smart" quotes, shown as ‘x’ and “string”   */
#define AT_QOPEN   0x04000000 /* - open             (kept in text as typed,  */
#define AT_QCLOSE  0x06000000 /* - close             no changes made without */
#define AT_KEYWORD 0x08000000 /* keywords                  user's permission)*/
#define AT_COMMENT 0x10000000 /* comments                                    */
#define AT_TAB     0x20000000 /* special attribute to preserve TABs in files */
#define AT_BADCHAR 0x40000000 /* incorrect Unicode char (moved +0x60/+0x350) */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#define AF_NONE    "\xCA\x80" /*   file representation for some attributes   */
#define AF_PROMPT  "\xCA\x82" /*   (used to set nice prompt for LenterARG)   */
#define AF_SUPER   "\xCA\x88"
#define AF_DIRTY   "\xCA\x90"
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#define CpSCH_NEW_LINE  '\f'          /* Beginning of new line in CP buffer  */
#define LeSCH_REPL_BEG  AT_SUPER+'>'  /* Replace substring begin             */
#define LeSCH_REPL_END  AT_SUPER+'/'  /* Replace substring end / files start */
#define TmSCH_THIS_OBJ     0x00082022 /* Reference to this file: • (tmshell) */
#define TeSCH_CONTINUE  AT_SUPER+0xBB /* Line continuation mark (TE_FORMAT)  */
/*---------------------------------------------------------------------------*/
extern int TABsize; /* Настраиваемые параметры - табуляция, 4 или 8 символов */
extern bool dosEOL; /* - DOS/Windows style for end-of-line (CR/LF вместо CR) */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#define MAXPATH    400                     /* Некоторые магические константы */
#define MAXDIRSTK   12
#define MAXTXRM    360 /* Максимальная ширина текта в редакторе (default)    */
#define MAXLUP   16384 /* Максимальная длина строки в файле                  */
#define MAXLPAC   4096 /* Максимальная длина строки в редакторе (le.c, te.c) */
#define MAXSYNTBUF  42 /* Буфер для synt checker                             */
#define UBUFSIZ  32780 /* Буфер откатки >= max(2*MAXLUP,2*tchar*MAXLPAC)+eps */
#define UDHQUOTA 24576 /* Квота откатки (на один файл) только если нет места */
#define UDLQUOTA  8192
/*---------------------------------------- Коды ошибок (или событий) --------*/
#define E_OK         0   /* Все в порядке                           -- Общие */
#define E_NOCOM      1   /* Неопознанная команда для данного уровня          */
#define E_CHANGE     2   /* Запрещены изменения текста                       */
#define E_KBREAK     3   /* Выполнение команды прервано с клавиатуры         */
#define E_NOUNDO     4   /* Откатка не поддерживается                        */
#define E_LENTER     5   /* Started Argument Entering mode                   */
#define E_LEXIT      6   /* Argument Entering mode completed                 */
                         /*------------------------ События редактора строки */
#define E_MOVBEG     7   /* Перемещение за левую границу                     */
#define E_MOVEND     8   /* Перемещение за правую границу                    */
#define E_EDTBEG     9   /* Изменение за левой границей                      */
#define E_EDTEND    10   /* Изменение за правой границей                     */
#define E_ATTR      11   /* Запрещена работа с атрибутами                    */
#define E_ILCHAR    12   /* совсем непечатный символ                         */
#define E_CCFUL     13   /* Буфер запомненных символов полон                 */
                         /*--------------- События редактора текста и прочее */
#define E_NOOP      14   /* Нечего делать (команда неприминима в этом месте) */
#define E_BADPRM    15   /* Недопустимое значение параметра / повторителя    */
#define E_LCUT      16   /* Запоминание строк в файле NDTSAV или после chars */
#define E_MOVUP     17   /* Верхняя граница текста                           */
#define E_MOVDOWN   18   /* Нижняя зраница текста                            */
#define E_SETY      19   /* Позиционирование за границы текста               */
#define E_FINISHED  20   /* Выполнение операции завершилось                  */
#define E_LPASTE    21
#define E_NOBLOCK   22
#define E_BLOCKOUT  23
#define E_SFAIL     24
#define E_NOSPAT    25
#define E_NORPAT    26
#define E_ROUT      27
#define E_FORMAT    28
#define E_CPFER     39
#define E_NOMEM     30  /* Нет памяти - могут быть  фатальные последствия    */
#define E_CB        31  /* Clipboard error (Windows only)                    */
#define E_MAX       32
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
        struct deq_tag;         /* Forward declarations of some C structures */                  
typedef struct deq_tag deq;     /*  dq.h */
        struct qfile_tag;
typedef struct qfile_tag qfile; /* qfs.h */
        struct txt_tag;
typedef struct txt_tag txt;     /* twm.h */
        struct wnd_tag;
typedef struct wnd_tag wnd;     /* twm.h */
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
  *dtol(char *pn, long  *dest),
  *dtos(char *pn, short *dest),
  *ltod(char *buffer, long  l), /* return pointer to end-of-num   */
  *stod(char *buffer, short s);

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
#define cStr()  toLocal8Bit().data() /* to pass QString value to legacy code */
#define uStr()       toUtf8().data() /* the same, but convert value to UTF-8 */
#define Utf8(x) QString::fromUtf8(x) /* just a shorthand to save some space  */

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
