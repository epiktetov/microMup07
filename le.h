/*------------------------------------------------------+----------------------
// МикроМир07    le = Line Editor -- Редактор строки    | (c) Epi MG, 2006-2011
//------------------------------------------------------+--------------------*/
#ifndef LE_H_INCLUDED      /* Old "le.h" (c) Attic 1989, (c) EpiMG 1996-2003 */
#define LE_H_INCLUDED
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void tleload(void), /* load LE line from text, see also EnterLEmode in vip.h */
   tleunload(void); /* unload LE line to text, see also ExitLEmode (ibid)    */
BOOL tleread(void); /* read Lebuf from text (read-only), true if non-empty   */
void LeStart();
void  blktspac(tchar *p, small len);
small lstrlen (small lmax, tchar *string);
void  llmove  (small xl, small xr, small dx, tchar *ns);
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
extern tchar  Lebuf[];    /* Буфер строки                                    */
extern tchar  lfbuf[];    /* альтернативный буфер                            */
extern small  Lleng;      /* Длина строки без хвостовых пробелов             */
extern small  Lxlm, Lxrm; /* левая/правая граница для перемещения            */
extern small  Lxle, Lxre; /* левая/правая граница для редактирования         */
extern large  Ly;         /* Y строки в тексте (для окна)                    */
extern small  Lx;         /* X курсора в строке (а не в окне!)               */
extern BOOL   Lredit;     /* Можно менять строку                             */
extern BOOL   Lchange;    /* Строка изменялась                               */
extern txt   *Ltxt;       /* Текст, в который сливается откатка              */
void tlesniff(txt *tx);   /* sets Lxlm, Lxrm, Lxle, Lxre from text and Lebuf */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
BOOL leNword(small *cwbeg,  /* Найти (unless ptr=0): начало текущего слова   */
             small *cwend,  /*                       конец текущего слова    */
             small *nwbeg); /* (return "на слове?")  начало следующего слова */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void leIC(), ledword(),       /* некоторые команды (используются в clip.cpp) */
     leDC(), lepword();
void leLLCE (tchar lchar), ledeol();            /* low-level character entry */
/*---------------------------------------------------------------------------*/
void LenterARG(tchar *buf, int *bufLen, /* buffer for argument & its length  */
                           int promLen, /* len of prompt (in buffer already) */
          tchar **history, int *kbcode, /* place for history and return code */
          int do_on_CR,  int do_on_RCR, /* commands to execute on CR and RCR */
                         int opt_flag); /* allowed options (st/wc/re//ic/cs) */
                                        /*                                   */
#define LE_HISTORY_SIZE  8              /* assuming SDOWN == CR, SUP == RCR  */
#define LeARG_STANDARD   1 /* Ctrl+6 ^                                       */
#define LeARG_REGEXP     2 /* Ctrl+7 & regular expressions, marked AT_SUBSCR */
#define LeARG_WILDCARD   4 /* Ctrl+8 * wildcard search, marked with AT_LIGHT */
#define LeARG_IGNORECASE 8 /* Ctrl+I i ignore case toggle (default is 'yes') */
extern int leOptMode; /* options corresponding to final result is found here */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#define tcharIsBlank(tc) ((short)(tc & AT_CHAR) == ' ')

extern int  char2tcharsX(const  char *orig, tchar *buffer, tchar attr);
extern int  char2tchars (const  char *orig, tchar *buffer);
extern void tchar2chars (const tchar *orig, int len, char *buffer);
/*---------------------------------------------------------------------------*/
#endif                                                      /* LE_H_INCLUDED */
