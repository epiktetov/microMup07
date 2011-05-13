//------------------------------------------------------+----------------------
// МикроМир07           Text & Window Manager           | (c) Epi MG, 2004-2011
//------------------------------------------------------+----------------------
#ifndef TWM_H_INCLUDED  /* Old "nm.h" (c) Attic 1989-90, (c) EpiMG 1998-2001 */
#define TWM_H_INCLUDED  /* old "wd.h" (c) Attic 1989, then (c) EpiMG 1996,98 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#define LDS_MCD '|'
#define MCD_LEFT     63     /* первый (основной) разделитель в micros.dir    */
#define MCD_RIGHT    68     /* второй разделитель в micros.dir (optional)    */
#define DIRLST_FNPOS 40     /* position of the filename in the TS_DIRLST txt */
#define TXT_MARKS     5     /* макс.количество запомненных маркеров в тексте */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
struct txt_tag 
{
  qfile *file;              /* Файл (or dir/pseudo-name) связанный с текстом */
  deq *txustk, *txdstk;     /* Деки - верхний и нижний стеки                 */
  large txy;                /* Число строк над указателем                    */
  deq  *txudeq;             /* Дек - буфер откатки                           */
  large txudcptr;           /* - указатель в буфере откатки                  */
  large txudlptr;           /* - граница строчной откатки                    */
  large txudfile;           /* - положение откатки равное "текст не менялся" */
  small txredit;            /* Можно ли менять текст:                        */
#define TXED_YES  0         /* - можно                                       */
#define TXED_NO   1         /* - нельзя                                      */
#define TXED_NONO 2         /* - совсем нельзя                               */
  small clang;              /* Язык для раскрашивания текста ('C','S'hell..) */
  deq *clustk, *cldstk;     /* - верхний и нижний стеки для раскрашивания    */
  small txlm,     txrm;     /* Left margin / правая граница текста           */
  small txmarkx[TXT_MARKS]; /* Маркеры (установлены в ноль если не занято),  */
  large txmarky[TXT_MARKS]; /* см. te(c|s)mark в файле te.c и TxMarks0(tx.c) */
  wnd  *txwndptr;           /* Список окон, наложенных на текст              */
  small txlructr;           /* Счетчик для алгоритма выталкивания            */
  small txstat;             /* Биты слова-состояния текста:                  */
#define TS_BUSY     001     /* - слот занят                                  */
#define TS_FILE     002     /* - файл в памяти                               */
#define TS_UNDO     004     /* - откатка файла в памяти                      */
#define TS_DIRLST   010     /* - список файлов - временный индекс            */
#define TS_CHANGED  020     /* - файл изменялся                              */
#define TS_WND      040     /* - есть хотя бы одно окно                      */
#define TS_DqSWAP  0777     /* - биты используются для чистки в tmswap |     */
#define TS_PERM    0100     /* - файл постоянно резидентен в памяти    |dq.c */
#define TS_SAVERR  0200     /* - в процессе сохранения возникла ошибка |     */
#define TS_PSEUDO  0400     /* - corresponding file is not real file         */
#define TS_RDONLY 01000     /* - force read-only (for ':help' viewing)       */
#define TS_NEW    02000     /* - новый файл - записывать по FO_NEW           */
#define TS_MCD    04000     /* - this file == micros.dir                     */
#define TS_GITPL 010000     /* - this file == git pretty log or blame result */
  small cx, tcx;            /* Последняя позиция курсора в тексте            */
  large cy, tcy;            /*                    и окна на тексте           */
  struct txt_tag *txnext;
  int thisSynts[MAXSYNTBUF];  /* Syntax info for after-line-on-top-of-txdstk */
  int prevSynts[MAXSYNTBUF];  /* Previos syntax info = between txustk/txdstk */
  large maxTy;                /* max Ty (total number of lines in the text)  */
  int lastSynts[MAXSYNTBUF];  /* Last syntax info (from after the last line) */
};
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
struct dirstk_tag
{
  qfile *file; small dswtx, dsx;    /* Имя файла и сохраненные wnd->wtx, Tx */
               large dswty, dsy;    /*                         wnd->wty, Ty */
};
typedef struct dirstk_tag dirstk;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
struct wnd_tag 
{
  small  wtx;   large wty;  /* координаты левого верхнего угла окна в тексте */
  small  wcx;   large wcy;  /* курсор в связанном с окном тексте (inactive)  */
  int cx, cy;   tchar ctc;  /* - оконные координаты курсора и аттрибуты      */
  small wsh,          wsw;  /* высота и ширина окна                          */
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
bool tmStart (QString param);                   /* <-- начать редактирование */
bool twStart (QString filename, long ipos);
void wattach (txt *text, wnd *vp);
void wdetach (txt *text, wnd *vp);
bool twEdit  (wnd *vp, QString filename, txt *referer = 0, bool isNew = false);
void twDirPush        (QString filename, txt *referer = 0); //  ^
void twDirPop (void);                                       // new window, use
void twDirCopy(wnd *from, wnd *to), twExit();               // vipActivate(vp)
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
bool twSave     (txt *t, wnd *vp, bool saveBackup); /* either Save or SaveAs */
bool twSafeClose(txt *t, wnd *vp);                  /* close if not changed  */
#ifdef notdef
void tmReconfig(void);
void tmReconf2 (void);
void tmConfParse(char *p);   /* parses NULL-terminated string into mimParams */
#endif
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
extern "C" {
#endif
BOOL tmsave (txt *t, BOOL qback); /* сохранить файл на диске  (used in dq.c) */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#ifdef __cplusplus
}
#endif
/*---------------------------------------------------------------------------*/
#endif                                                     /* TWM_H_INCLUDED */
