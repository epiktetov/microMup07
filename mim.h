//------------------------------------------------------+----------------------
// МикроМир07 Main Header + Scrollable/Gradient Window  | (c) Epi MG, 2004-2008
//------------------------------------------------------+----------------------
#ifndef MIM_H_INCLUDED
#define MIM_H_INCLUDED
#include "mic.h"
//-----------------------------------------------------------------------------
#include <QMainWindow>
#include <QWidget>
class MiScTwin;
class QSplitter;
class QMessageBox;
extern bool MiApp_useDIAGRAD; 
extern int  MiApp_defWidth, MiApp_defHeight;
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#define defWinWIDTH  80 // default window width (in characters)
#define defWinHEIGHT 50 // - - height (lines)
#define altWinHEIGHT 42 // alternate default height
#define mimBORDER     2 // border around text area (in pixels)
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class MiFrame : public QMainWindow
{
  Q_OBJECT

  MiScTwin *main, *scwin; int sashHeight;
  QSplitter *sash;          bool wrapped;
  QFont ref_Font, textFont, boldFont;
  QMessageBox *mbox;
  QMenu  *size_menu; 
  QAction *size_act; QSize tSize, oldSize;

public:    MiFrame(MiFrame *base); MiScTwin *NewScTwin(wnd *vp);
  virtual ~MiFrame();              void DeleteScTwin(MiScTwin*);
  void updateWinTitle(void);
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  const QFont& getRef_Font() const { return ref_Font; }
  const QFont& getTextFont() const { return textFont; }
  const QFont& getBoldFont() const { return boldFont; }
public slots:
  void SelectFont(); void OpenFile(); void SaveAs ();
  void SetTABsize(); void OpenDir (); void SaveAll();
  void shrinkwrap();                  void finishClose(int qtStdBtn);
  void fallbackSize(); 
  void splitterMoved(int pos);
public:
  void closeEvent (QCloseEvent  *ev); bool safeClose(MiScTwin *pane);
  void resizeEvent(QResizeEvent *ev);   QString saveAsDialog(txt *t);
};
void mimExit();
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#ifdef Q_OS_MAC
class MacEvents : public QObject { Q_OBJECT 
  protected:  bool eventFilter(QObject *obj, QEvent *event); };
#endif
//-----------------------------------------------------------------------------
enum MiInfoType { MitUSE_CURRENT = 0, MitLINE_BLOCK = 1, MitCHARK = 2 };
class MiInfoWin : public QWidget
{
  MiInfoType infoType;
  MiScTwin *sctw;
public:    MiInfoWin(MiScTwin *parent, const QColor *bgnd);
  virtual ~MiInfoWin() { }
  void vpResize();
  void paintEvent(QPaintEvent *ev);
  void updateInfo(MiInfoType mit = MitUSE_CURRENT);
};
//--------------------------------------//-------------------------------------
class MiScTwin : public QWidget         //   MicroMir Scrollable Text Window
{
public:
  MiScTwin(MiFrame *frame, const QColor *prim,
                           const QColor *bgnd, wnd *vp);
  ~MiScTwin();
  void vpResize();                      // resize based on vp->wsh/wsw info
  void vpResize(int width, int height); // resize to given dimensions
private:
  QColor gradColor, bgColor;
  quint64 gotFocus;
  int scrollbarWidth; // calculated dynamically on window creation
public:
  wnd     *vp; // ViewPort (interface between Qt/C++ and legacy C code)
  MiFrame *mf;
  MiInfoWin info;
  int fontBaseline, fontHeight, fontWidth;
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  void UpdateMetrics();
  void UpdateGradient(); 
  void resizeEvent(QResizeEvent *ev);
protected:
  int gradPixSize, gradPixHeight;
  QPixmap *gradPixmap;
public:
  int Tw2qtW(int tw) const { return fontWidth  * tw; }
  int Th2qtH(int th) const { return fontHeight * th; }
  int Qw2txW(int  W) const { return W / fontWidth;   }
  int Qh2txH(int  H) const { return H / fontHeight;  }

  int Tx2qtX(int tx) const { return mimBORDER + Tw2qtW(tx); }
  int Ty2qtY(int ty) const { return mimBORDER + Th2qtH(ty); }
  int Qx2txX(int  X) const { return Qw2txW(X - mimBORDER);  }
  int Qy2txY(int  Y) const { return Qh2txH(Y - mimBORDER);  }

  int Tsw2qtWb(int tx) const { return 2*mimBORDER + Tw2qtW(tx); }
  int Tsh2qtHb(int ty) const { return 2*mimBORDER + Th2qtH(ty); }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  void paintEvent(QPaintEvent *ev);
  void Erase(QPainter& dc, QRect& rect);             // Qt coords, whole widget
  void Erase(QPainter& dc, int tx, int ty, int len); // text coords / area only

  void Text (QPainter& dc, int tx, int ty, int tattr, QString text);
  void Text (QPainter& dc, int tx, int ty, tchar *tp, int len);
  void Repaint    (int tx, int ty, int width, int height, bool NOW = false);
  void Scroll (int src_tx, int ty, int width, int height, int dy);
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  void keyPressEvent(QKeyEvent *ev);    void repeatCmd(int kcode);
  void timerEvent (QTimerEvent *ev);
  void stopTimer();
  void focusInEvent (QFocusEvent *ev);  void mousePressEvent(QMouseEvent *ev);
  void focusOutEvent(QFocusEvent *ev);  void wheelEvent     (QWheelEvent *ev);
protected:
  int cmd2repeat, timerID;
};
//-----------------------------------------------------------------------------
#define pgt_1SEC 1000
quint64 pgtime(void);   /* returns current time with "pretty good" precision */
/*---------------------------------------------------------------------------*/
#endif                                                     /* MIM_H_INCLUDED */
