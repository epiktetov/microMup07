//------------------------------------------------------+----------------------
// МикроМир07 Main Header + Scrollable/Gradient Window  | (c) Epi MG, 2004-2016
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
extern bool MiApp_debugKB;
extern int  MiApp_defWidth, MiApp_defHeight;  extern QString MiApp_autoLoadLua;
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
public:    MiFrame(MiFrame *base); MiScTwin *NewScTwin(wnd *vp,MiScTwin *base);
  virtual ~MiFrame();              void   DeleteScTwin        (MiScTwin *that);
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
void mimReadPreferences();
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
  MiInfoType infoType; QString displayText;
  MiScTwin      *sctw;
public:    MiInfoWin(MiScTwin *parent);
  virtual ~MiInfoWin() { }
  void SetPalette(QColor bgnd, QColor text); void vpResize();
  void paintEvent(QPaintEvent *ev);
  void updateInfo(MiInfoType mit = MitUSE_CURRENT);
  void display (QString text) { displayText = text; }
};
class MiDiagWin : public QWidget
{
  QString displayText;
  MiScTwin      *sctw;
public:    MiDiagWin(MiScTwin *parent);
  virtual ~MiDiagWin() { }
  void vpResize();
  void update(QString text, int t);
  void paintEvent(QPaintEvent *ev);
};
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class MiConfigDlg : public QDialog
{
  Q_OBJECT QPushButton *fontButton, *shellButton;
           QLabel      *fontLabel,  *shellLabel;  QTextEdit *gradDescr;
           QLineEdit   *fontAdjust;               QTextEdit *keymapEdit;
           QSpinBox    *tabSizeBox;               QTextEdit    *luaEdit;
protected:
  void setFontLabelText();
  void setShellLabelText();
public slots: void selectFont();        void selectShell();
public:       MiConfigDlg(QWidget *parent = 0); bool Ask();
};
//--------------------------------------//-------------------------------------
class MiScTwin : public QWidget         //   MicroMir Scrollable Text Window
{
public: MiScTwin(MiFrame *frame, const QString bgndGrad, int usePool, wnd *vp);
  virtual ~MiScTwin();
  void vpResize();                      // resize based on vp->wsh/wsw info
  void vpResize(int width, int height); // resize to given dimensions
  void resizeEvent(QResizeEvent *reEv); // event generated by Qt on resize
private:
  QColor gradColor, bgColor, tabColor, keyColor, gradPool[4];
  qreal gradStart, gradStop;
  int              gradTilt; quint64 gotFocus;
public:
  wnd     *vp; // ViewPort (interface between Qt/C++ and legacy C code)
  MiFrame *mf;
  MiInfoWin info;
  MiDiagWin diag; int gradInPool, fontBaseline, fontHeight, fontWidth;
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  void SetGradient(const QString grad);     void SetGradFromPool(int N);
  void UpdateGradientPixmap();
  void UpdatePrimeColors();
  void UpdateMetrics();
  void Repaint(int     tx, int ty, int width, int height, bool NOW = false);
  void Scroll (int src_tx, int ty, int width, int height, int dy);
protected:
  int gradPixSize,gradPixHeight; // painting pre-calculated bitmap works faster
  QPixmap *gradPixmap;           //      even on Mac (and much faster on Linux)
  int upperNoScroll, lowerNoScroll;
#define MiSc_NO_SCROLL_AT_ALL 65539
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
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  void keyPressEvent(QKeyEvent *ev);
  void repeatCmd(int kcode, int count = 2147483647);
  void stopTimer();
  void timerEvent   (QTimerEvent *ev);
  void focusInEvent (QFocusEvent *ev);  void mousePressEvent(QMouseEvent *ev);
  void focusOutEvent(QFocusEvent *ev);  void wheelEvent     (QWheelEvent *ev);
protected:
  int cmd2repeat, repeatCount, timerID, pastDue;
public:
  void DisplayInfo(QString text) { info.display(text); }
};
//-----------------------------------------------------------------------------
extern QString last_MiCmd_key;
extern quint64 last_MiCmd_time;
#define pgt_1SEC 1000
quint64 pgtime(void);   /* returns current time with "pretty good" precision */
/*---------------------------------------------------------------------------*/
#endif                                                     /* MIM_H_INCLUDED */
