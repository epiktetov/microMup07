/*------------------------------------------------------+----------------------
// МикроМир07  Clipboard (using wxClipboard) & CS/LS op | (c) Epi MG, 2007-2014
//------------------------------------------------------+--------------------*/
#ifndef CLIP_H_INCLUDED
#define CLIP_H_INCLUDED
#include "mic.h"
#include "ccd.h"

void clipStart();
void clipFocusOff();  /* loosing focus - save unpasted data to Clipboard...  */
void clipRefocus ();  /* ...but ignore focus change if switching our windows */
/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
void cpsave();                       /* save CP-buffer to file (on exit)     */
void cpclose(), cpreopen();          /* close/re-open CP-buffer for savings  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void lecchar(), lecdchar();          /* le "copy character", "copy & delete" */
void lecword(), lecdword();          /* le "copy word", "copy & delete word" */
void cpaste();                       /* paste anything (works in LE/TE mode) */
void tallblockop(comdesc *cp);       /* "tall cursor" block ops (exits LE)   */
void clipToCB(), clipFromCB();       /* forced to/from clipboard operation   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void teclin(), tecdlin();            /* te "copy line" and "copy & delete"   */
void tecsblock(),                    /*          "character save" for blocks */
     tesdblock(),                    /*  save & delete / collapse for blocks */
     teclrblock(), tedelblock();     /* clear / delete (collapse) the block  */
extern txt *LCtxt;                   /* Текст-хранилище строк (just in case) */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
int  Block1size (int *x0, int *x1);  /* <- Return 1-line block size (+x0/x1) */
bool BlockXYsize(int *dx, int *dy);  /*   0 if no block, exc() if multi-line */
int  Block1move(tchar *b, int max);
/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#ifdef QSTRING_H
  QString clipStatus();
#endif
#include <QObject>
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
class dummyClip : public QObject /* here to avoid GCC "vtable not found" msg */
{
  Q_OBJECT        public slots: void changed1();
                                void changed2();
};
#endif                                                        /* __cplusplus */
/*---------------------------------------------------------------------------*/
#endif                                                    /* CLIP_H_INCLUDED */
