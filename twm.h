//------------------------------------------------------+----------------------
// МикроМир07           Text & Window Manager           | (c) Epi MG, 2004-2020
//------------------------------------------------------+----------------------
#ifndef TWM_H_INCLUDED  /* Old "nm.h" (c) Attic 1989-90, (c) EpiMG 1998-2001 */
#define TWM_H_INCLUDED  /* old "wd.h" (c) Attic 1989, then (c) EpiMG 1996,98 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#define LDS_MCD '|'
#define MCD_LEFT     63     /* первый (основной) разделитель в micros.dir    */
#define MCD_RIGHT    68     /* второй разделитель в micros.dir (optional)    */
#define TXT_MARKS    20     /* макс.количество запомненных маркеров в тексте */
#define TXT_MARK_ERR  4     /* |                                             */
#define TXT_MARK_INFO 5     /* | index in 'tchar txMarkT[TXT_MARKS]' array,  */
#define TXT_MARK_SMTH 6     /* | keeping mark types, see  file te.c, line 49 */
#define TXT_MARK_WARN 7     /* |                                             */
#define DIRLST_FNPOS 36     /* position of a filename in the TS_DIRLST text  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
struct txt_tag 
{
  qfile *file;              /* Файл (or dir/pseudo-name) связанный с текстом */
  deq *txustk, *txdstk;     /* Деки - верхний и нижний стеки                 */
  long txy;                /* Число строк над указателем                    */
  deq  *txudeq;             /* Дек - буфер откатки                           */
  long  txudcptr;           /* - указатель в буфере откатки                  */
  long  txudlptr;           /* - граница строчной откатки                    */
  long  txudfile;           /* - положение откатки равное "текст не менялся" */
  short txredit;            /* Можно ли менять текст:                        */
#define TXED_YES  0         /* - можно                                       */
#define TXED_NO   1         /* - нельзя                                      */
#define TXED_NONO 2         /* - совсем нельзя                               */
  short clang;              /* Язык для раскрашивания текста (см. synt.h)    */
  deq *clustk, *cldstk;     /* - верхний и нижний стеки для раскрашивания    */
  short txlm,     txrm;     /* Left margin / правая граница текста           */
  short txmarkx[TXT_MARKS]; /* Маркеры (установлены в ноль если не занято),  */
  long  txmarky[TXT_MARKS]; /* см. te(c|s)mark в файле te.c и TxMarks0(tx.c) */
  tchar txmarkt[TXT_MARKS]; /* mark type, AT_MARKOK+AT_BG_RED/GRN/BLU+[char] */
  char *txmarks[TXT_MARKS]; /* optional mark string (malloc'ed)              */
  wnd  *txwndptr;           /* Список окон, наложенных на текст              */
  short txlructr;           /* Счетчик для алгоритма выталкивания            */
  short txstat;             /* Биты слова-состояния текста:                  */
#define TS_BUSY     0x01    /* - слот занят                                  */
#define TS_CHANGED  0x02    /* - файл изменялся                              */
#define TS_DIRLST   0x04    /* - это список файлов (временный индекс)        */
#define TS_WND      0x08    /* - есть хотя бы одно окно                      */
#define TS_FILE     0x10    /* - файл в памяти                               */
#define TS_UNDO     0x20    /* - откатка файла в памяти                      */
#define TS_DqSWAP  0x1FF    /* - биты используются для чистки в tmswap |     */
#define TS_PERM     0x40    /* - файл постоянно резидентен в памяти    |dq.c */
#define TS_SAVERR   0x80    /* - в процессе сохранения возникла ошибка |     */
#define TS_PSEUDO  0x100    /* - corresponding file is not real file         */
#define TS_RDONLY  0x200    /* - force read-only (for help, dirlist etc)     */
#define TS_NEW     0x400    /* - новый файл - записывать по FO_NEW           */
#define TS_MCD     0x800    /* - this file == micros.dir                     */
#define TS_GITLOG 0x1000    /* - this file == git/Hg log (or blame/annotate) */
  short vp_ctx, vp_wtx;     /* Позиция курсора в тексте (cursor-in-text-x/y) */
  long  vp_cty, vp_wty;     /* и окна (win-in-text) дла посл. активного окна */
  struct txt_tag *txnext;
  int thisSynts[MAXSYNTBUF];  /* Syntax info for after-line-on-top-of-txdstk */
  int prevSynts[MAXSYNTBUF];  /* Previos syntax info = between txustk/txdstk */
  long maxTy;                 /* max Ty (total number of lines in the text)  */
  int lastSynts[MAXSYNTBUF];  /* Last syntax info (from after the last line) */

  int luaTxid; /* ссылка на элемент в Lua таблицe Txt (0 если нет), уникален */
               /* для данного состояния текста (меняется после перезагрузки) */
};
void wattach(txt *text, wnd *vp);
void wupdate(txt *text, wnd *vp);
void wdetach(txt *text);
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
struct dirstk_tag
{
  qfile *file; short dswtx, dsx;    /* Имя файла и сохраненные wnd->wtx, Tx */
               long  dswty, dsy;    /*                         wnd->wty, Ty */
};
typedef struct dirstk_tag dirstk;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
struct wnd_tag 
{
  short  wtx;   long  wty;  /* координаты левого верхнего угла окна в тексте */
  short  ctx;   long  cty;  /* курсор в связанном с окном тексте (inactive)  */
  int cx, cy;   tchar ctc;  /* - оконные координаты курсора и аттрибуты      */
  short wsh, wsw,  wspace;  /* высота и ширина окна, workspace (Mac only)    */
  txt              *wtext;  /* ссылка на описатель текста                    */
  dirstk stack[MAXDIRSTK];  /* стек файлов, связанный с окном                */
  dirstk *dirsp;            /* - его указатель                               */
  struct wnd_tag   *wnext,  /* следующее окно, прикрепленное к тексту        */
                *wsibling;  /* брат/сестра                                   */
  int sib_pos;              /* <-  0 for upper pane (frame::main)            */
#ifdef MIM_H_INCLUDED       /*    +1 for lower (main::scwin)                 */
  MiScTwin *sctw;           /* reference to corresponding MiScTwin object    */
#else
  void *sctw;
#endif
  struct wnd_tag *wdprev, *wdnext;    /* L2 list of all allocated structures */
};
/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
void tmInitialize();
void tmCheckFiles();
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
bool twStart (QString param);                   /* <-- начать редактирование */
wnd *twFork(int kcode_XFORK);
bool twEdit  (wnd *vp, QString filename, txt *referer = 0, bool isNew = false);
void twEditNew        (QString filename, txt *referer = 0); //  ^
void twDirPush(void);                                       // new window, use
void twDirPop (void);                                       // vipActivate(vp)
void twDirCopy(wnd *from, wnd *to), twExit();
//
// Find an existing text descriptor for given file, or create new one (with or
// without undo capability); referer text is used to resolve relative pathnames
//
txt *tmDesc(QString filename, bool needUndo, txt *referer = NULL);
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
bool tmLoad (txt *t);   /* сделать файл перманентным (i.e. load into memory) */
void tmSaveAll(void);   /* сохранить ВСЕ изменения (всех файлов) на диске    */
void tmFentr  (void),
     tmDoFentr(void), tmFnewByTtxt(void);
void tmLoadIn(txt *t, QString  filename); // replace (empty) text with the file
bool tmSaveAs(txt *t, QString  filename); // rename to given name && force save
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
bool twSave     (txt *t, wnd *vp); /* either Save or SaveAs */
bool twSafeClose(txt *t, wnd *vp); /* close if not changed  */
void twShowFile(QString filename);
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
extern "C" {
#endif
bool tmsave(txt *t);               /* сохранить файл на диске (used in dq.c) */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#ifdef __cplusplus
}
#endif
/*---------------------------------------------------------------------------*/
#endif                                                     /* TWM_H_INCLUDED */
