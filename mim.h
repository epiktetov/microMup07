//------------------------------------------------------+----------------------
// МикроМир07 Main Header + Scrollable/Gradient Window  | (c) Epi MG, 2004-2011
//------------------------------------------------------+----------------------
#ifndef MIM_H_INCLUDED
#define MIM_H_INCLUDED
#include "mic.h"
//-----------------------------------------------------------------------------
#include <QMainWindow>
#include <QDialog>
#include <QWidget>
class MiScTwin;
class QLabel;
class QMessageBox; class QSpinBox;
class QPushButton; class QSplitter;
class QLineEdit;   class QTextEdit;
extern int  MiApp_defWidth, MiApp_defHeight;
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#define defWinWIDTH  80 // default window width (in characters)
#define defWinHEIGHT 50 // - - height (lines)
#define altWinHEIGHT 42 // alternate default height
#define mimBORDER     2 // border around text area (in pixels)
#define mimVpPOSBAR   2 // width of "position" bar (in pixels)
#define mimTxtTOP      mimBORDER
#define mimTxtBOTTOM   mimBORDER
#define mimTxtLEFT     mimBORDER
#define mimTxtRIGHT (2*mimBORDER+mimVpPOSBAR)
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class MiFrame : public QMainWindow
{
  Q_OBJECT QAction *size_act; QSize tSize; bool wrapped;
           QMessageBox *mbox; QSplitter *sash;
           QMenu  *size_menu; int  sashHeight; QFont textFont, boldFont;
  MiScTwin *main, *scwin;
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
public:    MiFrame(MiFrame *base); MiScTwin *NewScTwin(wnd *vp);
  virtual ~MiFrame();              void DeleteScTwin(MiScTwin*);
  void updateWinTitle(void);
  const QFont& getTextFont() const { return textFont; }
  const QFont& getBoldFont() const { return boldFont; }
protected:
  void makeFonts(); // makes textFont and boldFont from MiApp_defaultFont etc
public slots:
  void OpenFile();  void SaveAs (); void Preferences();
  void OpenDir ();  void SaveAll(); void ShowLicense();
  void finishClose(int qtStandBtn); void ShowHelp();
  void shrinkwrap();    
  void fallbackSize(); 
  void splitterMoved(int pos);
public:
  void closeEvent (QCloseEvent  *ev); bool safeClose(MiScTwin *pane);
  void resizeEvent(QResizeEvent *ev);   QString saveAsDialog(txt *t);
};
void mimExit();
void mimSetNamedColor(QColor& color, const QString descr);
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
public:    MiInfoWin(MiScTwin *parent);
  virtual ~MiInfoWin() { }
  void SetPalette(QColor bgnd, QColor text); void vpResize();
  void paintEvent(QPaintEvent *ev);
  void updateInfo(MiInfoType mit = MitUSE_CURRENT);
};
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class MiConfigDlg : public QDialog
{
  Q_OBJECT QPushButton *fontButton; QTextEdit *keymapEdit;
           QLabel      *fontLabel,            *keymapLabl;
           QLineEdit   *fontAdjust; QSpinBox  *tabSizeBox;
           QLineEdit   *gradDescr;
protected:
  void setFontLabelText();
public slots:
  void selectFont();
public:
  MiConfigDlg(QWidget *parent = 0); bool Ask();
};
//--------------------------------------//-------------------------------------
class MiScTwin : public QWidget         //   MicroMir Scrollable Text Window
{
public: MiScTwin(MiFrame *frame, const QString bgndGrad, wnd *vp);
  virtual ~MiScTwin();
  void vpResize();                      // resize based on vp->wsh/wsw info
  void vpResize(int width, int height); // resize to given dimensions
private:
  QColor gradColor, bgColor, tabColor, keyColor;
  qreal gradStart, gradStop;
  int              gradTilt; quint64 gotFocus;
public:
  wnd     *vp; // ViewPort (interface between Qt/C++ and legacy C code)
  MiFrame *mf;
  MiInfoWin info;   int fontBaseline, fontHeight, fontWidth;
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  void SetGradient(const QString grad);
  void UpdateGradientPixmap();
  void UpdateMetrics();
  void resizeEvent(QResizeEvent *ev);
protected:
  int gradPixSize, gradPixHeight;
  QPixmap *gradPixmap;
public:
  int Tw2qtW(int tw) const { return fontWidth  * tw; }
  int Th2qtH(int th) const { return fontHeight * th; }
  int Qw2txW(int  W) const { return W / fontWidth;   }
  int Qh2txH(int  H) const { return H / fontHeight;  }

  int Tx2qtX(int tx) const { return mimTxtLEFT + Tw2qtW(tx); }
  int Ty2qtY(int ty) const { return mimTxtTOP  + Th2qtH(ty); }
  int Qx2txX(int  X) const { return Qw2txW (X - mimTxtLEFT); }
  int Qy2txY(int  Y) const { return Qh2txH (Y - mimTxtTOP ); }

  int Tsw2qtWb(int tx) const { return mimTxtLEFT + mimTxtRIGHT + Tw2qtW(tx); }
  int Tsh2qtHb(int ty) const { return mimTxtTOP + mimTxtBOTTOM + Th2qtH(ty); }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  void paintEvent(QPaintEvent *ev);
  void repaintPosBar(QPainter& dc);
  void Erase(QPainter& dc, QRect& rect);             // Qt coords, whole widget
  void Erase(QPainter& dc, int tx, int ty, int len); // text coords / area only
  void RepaintVpPB();
  void Text (QPainter& dc, int tx, int ty, int tattr, QString text);
  void Text (QPainter& dc, int tx, int ty, tchar *tp, int len);
  void Repaint(int     tx, int ty, int width, int height, bool NOW = false);
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
extern quint64 last_MiCmd_time;
#define pgt_1SEC 1000
quint64 pgtime(void);   /* returns current time with "pretty good" precision */
/*---------------------------------------------------------------------------*/
#endif                                                     /* MIM_H_INCLUDED */
