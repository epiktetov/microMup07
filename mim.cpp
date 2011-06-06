//------------------------------------------------------+----------------------
// МикроМир07   Main Frame + Application Data + ScTwin  | (c) Epi MG, 2004-2011
//------------------------------------------------------+----------------------
#include <QApplication>
#include <QtGui>
#include "mim.h"
#include "ccd.h"
#include "qfs.h"
#include "twm.h"
#include "vip.h"
#include "clip.h"
#include "synt.h"
extern "C" { extern const char microVERSION[]; }
static QString MimVersion()
{
  return Utf8("µMup07 version %1 (" QtPLATF ")").arg(microVERSION);
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
bool dosEOL = false;                              /* Настраиваемые параметры */
int TABsize = 4;
QSize MiFrameSize;
static bool MiApp_debugKB    = false;
static bool MiApp_timeDELAYS = false;
int      last_MiCmd_code;
quint64  last_MiCmd_time;
QMap<int,int> MiApp_keyMap;
QString MiApp_defaultFont, MiApp_defFontAdjH, MiApp_keyMaps, MiApp_gradDescr;
int     MiApp_defFontSize;
int     MiApp_fontAdjOver, MiApp_fontAdjUnder, MiApp_defWidth, MiApp_defHeight;
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline bool isNoRealText (txt *t) // == text with no name, which is not changed
{                                 // (as if µMup is started without parameters)
  return (t->file &&              //
          t->file->name == QfsEMPTY_NAME && !(t->txstat & TS_CHANGED));
}
#ifdef Q_OS_MAC
bool MacEvents::eventFilter (QObject*, QEvent *ev)
{
  if (ev->type() == QEvent::FileOpen) {
    QString filename = static_cast<QFileOpenEvent*>(ev)->file();
    if (Ttxt && isNoRealText(Ttxt)) tmLoadIn(Ttxt, filename);
    else                            twStart (filename,    1); return true;
  } else                                                      return false;
}
# define mimFONTFACENAME "Menlo"
# define mimFONTSIZE  12
# define menuBarHeight 0
#else
  int menuBarHeight;
#endif
#ifdef Q_OS_LINUX
# define mimFONTFACENAME "DejaVu Sans Mono"
# define mimFONTSIZE  10
#endif
#ifdef Q_OS_WIN
# define mimFONTFACENAME "Lucida Console"
# define mimFONTSIZE  10
#endif
#define defWinGRADIENT Utf8("40°/white,0.0-0.25;4")
#define IGNORE_CASE Qt::CaseInsensitive
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
QColor colorBlack     (  0,  0,  0); // black (used for regular text)
QColor colorWhite     (255,255,255); // white (inverted text under cursor)
QColor colorWinBgnd   (255,255,255); // white
QColor colorWinGrad   (255,240,213); // wheat == #fff0D5 (H=39° S=16% B=100%)
QColor colorDarkWheat (213,150, 36); // dark wheat (#d59624, S=83% and B=84%)
QColor colorDarkBrown (108, 70,  0); // darker brown  (#6c4600, S=100% B=42%)
QColor colorDarkGrey  ( 85, 85, 85); // dark grey (#555555, B=33%) for comments
QColor colorDarkRed   (120,  0,  0); // dark red cursor (cannot edit), regex
QColor colorDarkGreen (  0,100,  0); // dark green cursor
QColor colorDarkBlue  (  0,  0,150); // dark blue cursor / prompt text
QColor colorLightBlue (  0,100,255); // special characters
QColor colorLightPink (200,200,172); // "temporary" block, not very pink really
QColor colorLightCyan (172,200,255); // "permanent" block
QColor colorLightGreen(204,255,155); // "good" mark (used for matched brackets)
QColor colorLightRed  (255,155,155); // "bad" mark, indicates some error
QColor colorSolidRed  (255,  0,  0); // solid red
QColor colorSolidGreen(  0,166,  0); // - green | for numbered marks (used for
QColor colorSolidBlue (  0, 85,215); // - blue  |          gradient background)
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int main(int argc, char *argv[])
{
#if defined(Q_OS_MAC) && QT_VERSION >= 0x040500
  QApplication::setGraphicsSystem("raster");
#endif  //                         ^
// when using "native" graphics, scroll() posts update requeston on all widgets
// (comments says "This is a hack around Apple's missing functionality, pending
// the toolbox team fix. --Sam"), resulting in full window update and rendering
// our efforts to optimize repaint useless; setting "raster" system explicitly
// seems to fix that (must be called before QApplication constructor)
//
  QApplication app(argc, argv);
  QCoreApplication::setOrganizationDomain("epiktetov.com"); tmInitialize();
  QCoreApplication::setOrganizationName("EpiMG");
  QCoreApplication::setApplicationName("micro7");     mimReadPreferences();
#ifdef Q_OS_MAC
  app.installEventFilter(new MacEvents);
#elif defined(UNIX)
  app.setWindowIcon(QIcon(":/qtmim.png"));
#endif
  for (int i = 1; i < argc; i++) {
    QString param = Utf8(argv[i]);
         if (param.compare("-dos",IGNORE_CASE) == 0) dosEOL           =  TRUE;
    else if (param.compare("-kb", IGNORE_CASE) == 0) MiApp_debugKB    =  true;
    else if (param.compare("-ti", IGNORE_CASE) == 0) MiApp_timeDELAYS =  true;
    else if (param == "-") { if (tmStart(QfsEMPTY_NAME)) return app.exec();
                             else                        return 2;        }
    else {
#ifdef Q_OS_WIN                // Qt operates with correct forward slashes, but
      param.replace('\\','/'); // argv here comes from Windows, have to replace
#endif
      if (param.startsWith("'")) param.replace(0,1,QfsXEQ_PREFIX);
      if (tmStart(param)) return app.exec();
      else                return 1;
  } }
#ifdef Q_OS_MAC                 // When file/directory is drag-n-dropped on our
#define mimNoNAME QfsEMPTY_NAME // application on Mac, the name of the file/dir
#else                           // is not specified in command line; open empty
#define mimNoNAME "."           // file to avoid showing unwanted contents
#endif
  if (tmStart(mimNoNAME)) return app.exec();
  else                    return 2;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void mimExit()
{
  cpsave(); QCoreApplication::quit();
}
//-----------------------------------------------------------------------------
MiFrame::MiFrame (MiFrame *base) : tSize(0,0), wrapped(false),
                                   mbox(0), main(0), scwin(0)
{
#ifndef Q_OS_MAC
  QMenu *dummy_menu = menuBar()->addMenu(Utf8("µMup07"));
         dummy_menu->setDisabled(true);
#endif
  QMenu *file_menu = menuBar()->addMenu("File");
  QAction *act = file_menu->addAction("&Open file...");
  connect(act, SIGNAL(triggered()), this, SLOT(OpenFile()));
  act = file_menu->addAction("Open &dir...");
  connect(act, SIGNAL(triggered()), this, SLOT(OpenDir()));
  act = file_menu->addAction("&Save as...");
  connect(act, SIGNAL(triggered()), this, SLOT(SaveAs()));
  act = file_menu->addAction("Save &ALL changes\tCtrl+S");
  connect(act, SIGNAL(triggered()), this, SLOT(SaveAll()));
  file_menu->addSeparator();
  act = file_menu->addAction("&Preferences...");
  connect(act, SIGNAL(triggered()), this, SLOT(Preferences()));
  size_menu = menuBar()->addMenu("size");
  size_act = size_menu->addAction("fallback-size");
  connect(size_act, SIGNAL(triggered()), this, SLOT(fallbackSize()));
  QString short_version = QString("ver.%1").arg(microVERSION);
  QMenu *dummy_version_menu = menuBar()->addMenu(short_version);
         dummy_version_menu->setDisabled(true);

  QMenu *help_menu = menuBar()->addMenu("Help");
  act =  help_menu->addAction("License");
  connect(act, SIGNAL(triggered()), this, SLOT(ShowLicense()));
  act =  help_menu->addAction(Utf8("µMup07 help"));
  connect(act, SIGNAL(triggered()), this, SLOT(ShowHelp()));

  sash = new QSplitter(Qt::Vertical, this);
  sashHeight = sash->handleWidth();
  connect(sash, SIGNAL(splitterMoved(int,int)),
          this,   SLOT(splitterMoved(int)   ));

  setCentralWidget(sash); // Qt says: QMainWindow takes ownership of the widget
  if (base) {             //     pointer and deletes it at the appropriate time
    QPoint basePos = base->pos();
    basePos.rx() += 120;
    basePos.ry() +=  20;    move(basePos);
    textFont = QFont(base->getTextFont());
    boldFont = QFont(base->getBoldFont());
  }
  else makeFonts();
}
MiFrame::~MiFrame() //- - - - - - - - - - - - - - - - - - - - - - - - - - - - -
{                   // Unfortunately, we need to know menuBar height before Qt
  QSettings Qs;     // knows it (i.e. before window is shown), saving in prefs
#ifndef Q_OS_MAC
  Qs.setValue("private/menuBarHeight", menuBar()->height());
#endif
  Qs.setValue("frameWidth",  MiApp_defWidth  = tSize.width());
  Qs.setValue("frameHeight", MiApp_defHeight = tSize.height());
  if (main) delete main;         MiFrameSize = tSize;
  if (scwin) delete scwin;
}                          
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiFrame::makeFonts() // makes textFont and boldFont from MiApp_defaultFont
{
  textFont = QFont(MiApp_defaultFont, MiApp_defFontSize);
#if QT_VERSION >= 0x040700
  textFont.setStyleHint(QFont::AnyStyle, QFont::ForceIntegerMetrics);
#endif                        //                ^
  boldFont = QFont(textFont); // Older Qt versions ALWAYS used integer metrics;
  boldFont.setBold(true);     // sub-pixel glyphs may look better, but MicroMir
}                             // cannot handle them correctly: cursor position?
//-----------------------------------------------------------------------------
#define MiFrameOPEN_XXX(XXX,XXXtext,getMETHOD)                        \
void MiFrame::Open##XXX()                                             \
{                                                                     \
  QString init = (Ttxt && Ttxt->file) ? Ttxt->file->path : QString(); \
  QString name = QFileDialog::getMETHOD(this, "Open " XXXtext, init); \
  if (name.isEmpty()) return;                                         \
  if (Ttxt) { vipFocusOff(Twnd); twDirPush(name, Ttxt); vipReady(); } \
  else                           twStart  (name,    1);               \
}
MiFrameOPEN_XXX(File,    "file", getOpenFileName)
MiFrameOPEN_XXX(Dir,"directory", getExistingDirectory)
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
QString MiFrame::saveAsDialog (txt *t)
{
  if (t && t->file) {
    QString suggest_name;
    switch (t->file->ft) {
    case QftPSEUDO: suggest_name = t->file->path+"/?"; break;
    case QftNOFILE: suggest_name = t->file->full_name; break; //++ TODO: check
    default:        suggest_name = t->file->full_name;        // all platforms
    }
    return QFileDialog::getSaveFileName(this, "Save as", suggest_name);
  } return QString();
}
void MiFrame::SaveAs() // - - - - - - - - - - - - - - - - - - - - - - - - - - -
{                        
  QString file = saveAsDialog(Ttxt);
  if (! file.isEmpty()) { vipFocusOff(Twnd); tmSaveAs(Ttxt,file); vipReady(); }
}
void MiFrame::SaveAll() { vipFocusOff(Twnd); tmSaveAll();         vipReady(); }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiFrame::ShowLicense(void) { twShowFile(":/LICENSE"); }
void MiFrame::ShowHelp   (void) { twShowFile(":/help");    }
//-----------------------------------------------------------------------------
void MiFrame::closeEvent (QCloseEvent *ev)
{
  if (safeClose(scwin) && safeClose(main)) ev->accept();
  else                                     ev->ignore();
}                                    
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool MiFrame::safeClose (MiScTwin *sctw)  // NOTE: sctw pointer in the MiFrame
{                                         // is supposed to be cleaned up from
  if (sctw == NULL) return true;          // twSafeClose/twExit->vipFreeWindow
  txt *t = sctw->vp->wtext;
  if (twSafeClose(t, sctw->vp)) return true;
  if (mbox) delete mbox;
  mbox = new QMessageBox(Utf8("µMup07 warning"),
          t->file->name+" has been modified.\n"
           "Do you want to save your changes?", QMessageBox::Warning,
        QMessageBox::Save|QMessageBox::Default, QMessageBox::Discard,
       QMessageBox::Cancel|QMessageBox::Escape, this,     Qt::Sheet);
  connect(mbox, SIGNAL(finished(int)), this, SLOT(finishClose(int)));
  mbox->show();
  return false; // not safe to close yet (user notified, waiting for reaction)
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiFrame::finishClose (int qtStandBtn) // finish closing either scwin (when
{                                          // still present) or main & then try
  switch (qtStandBtn) {                    // to close the window again
  case QMessageBox::Cancel: return;
  case QMessageBox::Save:
         if (scwin) twSave(scwin->vp->wtext, scwin->vp, false);
    else if (main)  twSave( main->vp->wtext,  main->vp, false); break;
  case QMessageBox::Discard:
         if (scwin) scwin->vp->wtext->txstat &= ~TS_CHANGED;
    else if (main)   main->vp->wtext->txstat &= ~TS_CHANGED;
  }
  close();
}
//-----------------------------------------------------------------------------
MiScTwin *MiFrame::NewScTwin (wnd *vp) // creates new pane (unless two already
{                                      // exist - then do nothing, returns 0)
  if (scwin) return NULL;
  MiScTwin *sctw = new MiScTwin(this, MiApp_gradDescr, vp);
  sash->addWidget(sctw);
  if (main)
     { scwin = sctw; main->vpResize(); sashHeight = sash->handleWidth(); }
  else main  = sctw; sctw->vpResize();
                     sctw->setFocus(Qt::OtherFocusReason); shrinkwrap();
  return sctw;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiFrame::DeleteScTwin (MiScTwin *sctw)
{
  if (scwin) {
    if (sctw == scwin) main->setFocus();
    else {
      wnd * main_vp =  main->vp; // swap ViewPort data between QWindow (easier
      wnd *scwin_vp = scwin->vp; //                          than re-parenting)
      main->vp = scwin_vp;  main->vp->sctw =  main;
      scwin->vp = main_vp; scwin->vp->sctw = scwin;
    }
    delete scwin;     scwin = NULL;
    main->vpResize(); shrinkwrap();
  }
  else if (sctw == main) { delete main; main = NULL; }
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiFrame::updateWinTitle(void)
{
  QString title;
  if (main && main->vp && main->vp->wtext && !isNoRealText(main->vp->wtext)
                                          &&         main->vp->wtext->file)
                                title = QfsShortName(main->vp->wtext->file);
  else title = MimVersion();
  if (scwin && scwin->vp && scwin->vp->wtext && scwin->vp->wtext->file)
          title.append(Utf8(" • ")+QfsShortName(scwin->vp->wtext->file));
  setWindowTitle(title);
}
//-----------------------------------------------------------------------------
inline QString myQSize2txt (QSize& size)
{
  return QString("%1x%2").arg(size.width()).arg(size.height());
}
static QSize defWinSize(defWinWIDTH, defWinHEIGHT),
             altWinSize(defWinWIDTH, altWinHEIGHT);
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiFrame::shrinkwrap()
{
  if (main) {    QSize mxSz =  main->size(); tSize.rheight()  =  main->vp->wsh;
    if (scwin) { QSize sxSz = scwin->size(); tSize.rheight() += scwin->vp->wsh;
      QList<int> sizes;
      sizes << mxSz.height() << sxSz.height();
      sash->setSizes(sizes);
      mxSz.rheight() += sashHeight + sxSz.height();
    } 
    tSize.rwidth() = main->vp->wsw;    size_menu->setTitle(myQSize2txt(tSize));
         if (tSize != MiFrameSize) size_act->setText(myQSize2txt(MiFrameSize));
    else if (tSize != defWinSize)  size_act->setText(myQSize2txt(defWinSize));
    else                           size_act->setText(myQSize2txt(altWinSize));
#ifndef Q_OS_MAC
    if (menuBarHeight < 0) menuBarHeight = menuBar()->height();
    mxSz.rheight() += menuBarHeight;
#endif
    wrapped = true; resize(mxSz); vipReady();
} }
#ifdef Q_OS_LINUX_does_not_work
  int dX = main->fontHeight, dY = main->fontWidth; setSizeIncrement(dX, dY);
  if (scwin)
    { mxs.rheight()-= dX*scwin->vp->wsw; mxs.rwidth()-= dY*scwin->vp->wsw; }
      mxs.rheight()-= dX* main->vp->wsw; mxs.rwidth()-= dY* main->vp->wsw;
  setBaseSize(mxs);
#endif
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiFrame::fallbackSize()
{
       if (tSize != MiFrameSize) tSize = MiFrameSize;
  else if (tSize != defWinSize)  tSize = defWinSize;
  else                           tSize = altWinSize;
  if (main) {  int maH = tSize.height();
    if (scwin) { scwin->vp->wsh = maH/2;         maH = maH - maH/2;
                 scwin->vp->wsw = tSize.width(); scwin->vpResize(); 
    }
    main->vp->wsh = maH;
    main->vp->wsw = tSize.width(); main->vpResize();  shrinkwrap();
} }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiFrame::splitterMoved (int pos)
{
  if (main && scwin) { int maH = main->Qy2txY(pos), scH,
                          totH = main->vp->wsh + scwin->vp->wsh;
    // Mimimum sizes:
    //
    if (maH < 3)   maH = 3; scH = totH - maH;
    if (scH < 3) { scH = 3; maH = totH - scH; }
    //
    // Set text height, resize both panes from text dimensions and shrinkwrap:
    //
     main->vp->wsh = maH;  main->vpResize();
    scwin->vp->wsh = scH; scwin->vpResize(); shrinkwrap();
} }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiFrame::resizeEvent (QResizeEvent*)
{
  if (main) {
    int fcw = width(), fch = height() - menuBarHeight;
    if (scwin) {
      int mh =  main->vp->wsh; // change sub-window sizes proportionally
      int sh = scwin->vp->wsh;
       main->vpResize(fcw, mh * (fch - sashHeight) / (mh+sh));
      scwin->vpResize(fcw, sh * (fch - sashHeight) / (mh+sh));
    }
    else main->vpResize(fcw, fch);
  }
  if (wrapped)                         wrapped = false;
  else QTimer::singleShot(0, this, SLOT(shrinkwrap()));
}
//=============================================================================
MiScTwin::MiScTwin (MiFrame *frame, const QString bgndGrad, wnd *win)
  : gotFocus(0), vp(win), mf(frame),
    info (this),
    gradPixSize(0), gradPixHeight(0),
    gradPixmap(NULL),  cmd2repeat(0), timerID(0)
{
  vp->sctw = this;       setFocusPolicy(Qt::ClickFocus);
  UpdateMetrics();       setAttribute(Qt::WA_OpaquePaintEvent);
  SetGradient(bgndGrad); setAttribute(Qt::WA_NoSystemBackground); info.show();
}
MiScTwin::~MiScTwin() { vipCleanupWindow(vp);
                        if (gradPixmap) delete gradPixmap; }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiScTwin::vpResize (int width, int height)
{
  int dW = width  - Tsw2qtWb(vp->wsw),
      dH = height - Tsh2qtHb(vp->wsh);
//
// Nothing to do if the window size is correct already, otherwise change text 
// size first, then re-calculate new pixel size based on that:
//
  if (dW || dH ) { vp->wsw += dW/fontWidth;
                   vp->wsh += dH/fontHeight; vpResize(); }
}
void MiScTwin::vpResize() { resize(Tsw2qtWb(vp->wsw), Tsh2qtHb(vp->wsh)); }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiScTwin::resizeEvent(QResizeEvent*) // using fontWidth as vertical offset
{                                         //                       for symmetry
  info.move(width()  - info.width()  - fontWidth,
            height() - info.height() - fontWidth); UpdateGradientPixmap();
}                    
//-----------------------------------------------------------------------------
void mimSetNamedColor (QColor& color, const QString descr)
{
  QRegExp Hue(Utf8("([0-9]+)(°|\\*)"));
  if (Hue.exactMatch(descr)) { qreal H = Hue.cap(1).toFloat()/360.0;
                               color.setHsvF(H, 0.16, 1.0);       }
  else                         color.setNamedColor (descr);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiScTwin::SetGradient(const QString grad)
{
  mimSetNamedColor(gradPool[0], Utf8( "40°")); // ≈ SVG papaywhip (Hue=40°)
  mimSetNamedColor(gradPool[1], Utf8( "80°")); // very light green
  mimSetNamedColor(gradPool[2], Utf8("200°")); // very light sky blue
  mimSetNamedColor(gradPool[3], "mistyrose" );
  QRegExp gf("([^/;,]+)(?:/([^/;,]+))?(?:,([0-9.]+)-([0-9.]+))?(?:;([0-4]))?"
             "(?:,2nd:([^/;,]+))?(?:,3rd:([^/;,]+))?(?:,4th:([^/;,]+))?");
//
  if (gf.exactMatch(grad)) { mimSetNamedColor(gradColor, gf.cap(1));
    if (gf.cap(2).isEmpty()) mimSetNamedColor(  bgColor,   "white");
    else                     mimSetNamedColor(  bgColor, gf.cap(2));
//
    gradTilt = gf.cap(5).toInt();
    if (gf.cap(3).isEmpty() && gradTilt > 0) { gradStart = 0.0;
                                               gradStop  = 1.0; }
    else {
      gradStart = gf.cap(3).toFloat();
      gradStop  = gf.cap(4).toFloat();
    }                                          gradPool[0] = gradColor;
    if (!gf.cap(6).isEmpty()) mimSetNamedColor(gradPool[1], gf.cap(6));
    if (!gf.cap(7).isEmpty()) mimSetNamedColor(gradPool[2], gf.cap(7));
    if (!gf.cap(8).isEmpty()) mimSetNamedColor(gradPool[3], gf.cap(8));
  }
  else { gradTilt = 1; gradStart = 0.0; gradColor = colorWinGrad;
                       gradStop  = 1.0;   bgColor = colorWinBgnd; }
  gradPixSize = 0;
  UpdateGradientPixmap(); UpdatePrimeColors(); update();
}
void MiScTwin::SetGradFromPool(int N)
{
  gradColor = gradPool[N];
  gradPixSize = 0;
  UpdateGradientPixmap(); UpdatePrimeColors(); update();
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiScTwin::UpdateGradientPixmap()
{
  int gradPixWidth,  winSize = width() + height() * gradTilt;
  if (winSize == gradPixSize &&
                  gradPixmap && fontHeight <= gradPixHeight) return;
  else {
    gradPixSize  = winSize; /* gradient pixmap depends only */
    gradPixWidth = winSize; /* on win size, not proportions */
  }
  upperNoScroll = 0; // first, assume it's safe to scroll (except for infoWin)
  lowerNoScroll = 2; //
  if (gradPixmap) delete gradPixmap;
  gradPixmap = new QPixmap(gradPixWidth, gradPixHeight  =  fontHeight);
  QPainter    dc(gradPixmap);      QRect gradRect = gradPixmap->rect();
  dc.setPen(QPen(Qt::NoPen));
  if (my_abs(gradStart-gradStop) > 0.01) {
    qreal X = winSize / qreal(gradTilt * gradTilt + 1),   Y = X * gradTilt;
    QLinearGradient grad(gradStart*X, gradStart*Y, gradStop*X, gradStop*Y);
    grad = QLinearGradient(gradStart*X, gradStart*Y, gradStop*X, gradStop*Y);
    grad.setColorAt(0, gradColor);
    grad.setColorAt(1,   bgColor); dc.setBrush(QBrush(grad));
                                   dc.drawRect(gradRect);
    if (gradTilt) {
      if (gradStart < 0.5 && gradStop < 0.5) { // gradient in upper-left corner
        X *= my_max(gradStart, gradStop);      // some lines on top unscrolable
        Y *= my_max(gradStart, gradStop);
        upperNoScroll = int((X/gradTilt + Y)/fontHeight + 0.99);
      }
      else if (gradStart > 0.5 && gradStop > 0.5) { // gradient in lower-right
        X *= my_min(gradStart, gradStop);           // corner...
        Y *= my_min(gradStart, gradStop);
        qreal Yref = (X + gradTilt*Y - width())/gradTilt;
        lowerNoScroll = int((height() - Yref)/fontHeight + 0.99);
        if (lowerNoScroll < 2) lowerNoScroll = 2;
      }
      else upperNoScroll = MiSc_NO_SCROLL_AT_ALL;
  } }
  else { if (gradStart > 0.01) dc.setBrush(QBrush(gradColor));
         else                  dc.setBrush(QBrush(  bgColor));
                               dc.drawRect(gradRect); // solid background
} }                                                   // (safe to scroll)
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiScTwin::UpdatePrimeColors()          // Calculate "tab" color  (used for
{                                           // TAB mark and non-printable chars
  qreal H,S,V; gradColor.getHsvF(&H,&S,&V); // before space, and for info text)
  if (H < 0) { tabColor = colorDarkWheat;   // && "key" color (that is used for
               keyColor = colorDarkBrown; } // keywords and bold text)
  else {
    tabColor.setHsvF(H, 0.83, 0.84*V); // preserving Hue, fixed Saturation and
    keyColor.setHsvF(H, 1.00, 0.42*V); // value in percentage of the original
  }
  if (gradStart < gradStop) info.SetPalette(gradColor, tabColor);
  else                      info.SetPalette(  bgColor, tabColor);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiScTwin::UpdateMetrics()          // NOTE: assuming fixed-width font with
{                                       // adjustments: MiApp_fontAdjOver/Under
  QFontMetrics fm =  mf->getTextFont();
  fontBaseline = fm.ascent();
  fontHeight = fm.height()-1; // Qt height always equal to ascent()+descent()+1
  fontWidth  = fm.width("X");
  fontHeight += MiApp_fontAdjOver; fontBaseline += MiApp_fontAdjOver;
  fontHeight += MiApp_fontAdjUnder;                  info.vpResize();
}
//-----------------------------------------------------------------------------
void MiScTwin::Repaint (int x, int y, int width, int height, bool NOW)
{
//  fprintf(stderr, "Repaint(%d,%d,%dx%d%s)\n", x, y, width,height,
//                                              NOW ? ",NOW" : "");
//+
  if (NOW) repaint(Tx2qtX(x), Ty2qtY(y), Tw2qtW(width), Th2qtH(height));
  else     update (Tx2qtX(x), Ty2qtY(y), Tw2qtW(width), Th2qtH(height));
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiScTwin::Scroll (int srcx, int ty, int width, int height, int dy)
{
  int scTy = ty, scH = height + my_abs(dy), scTop = my_min(ty,ty+dy);
  int over = (upperNoScroll - scTop);
  if (over > 0) { scTop = upperNoScroll; scTy -= over;
                                         scH  -= over; }
//
// NOTE: using the fact that Scroll is always called for a region at the bottom
// of the window (which holds for both "true" scrolling and line insert/delete)
//
  scTy -= lowerNoScroll; // ..and, because of MiInfoWin, there are always some
  scH  -= lowerNoScroll; // unscrollable area at the bottom (even without grad)
  if (scH > 0) {
    QRect scrollArea = QRect(Tx2qtX(srcx), Ty2qtY(scTop), Tw2qtW(width),
                                                          Th2qtH(scH) );
    scroll(0, Th2qtH(dy), scrollArea);
    if (over > 0) Repaint(srcx, my_min(ty,ty+dy), width,          over);
    Repaint(srcx, ty+height + dy - lowerNoScroll, width, lowerNoScroll);
  }
  else Repaint(srcx, ty+dy, width, height);
}
//-----------------------------------------------------------------------------
void MiScTwin::paintEvent (QPaintEvent *ev) // main PaintEvent (called from Qt)
{
  QPainter dc(this);                            QVector<QRect> rlist;
  QRegion paintArea = ev->region();             QVector<QRect>::iterator it;
  QRegion textArea(mimTxtLEFT, mimTxtTOP, Tw2qtW(vp->wsw), Th2qtH(vp->wsh));
  QRegion border   = paintArea.subtracted (textArea);
  for (rlist = border.rects(), it  = rlist.begin(); it != rlist.end(); ++it)
    Erase(dc, *it);

  QRegion text2upd = paintArea.intersected(textArea);
  for (rlist = text2upd.rects(), it  = rlist.begin(); it != rlist.end(); ++it)
    vipRepaint(vp, dc, this, Qx2txX(it->left()), Qx2txX(it->right()),
                             Qy2txY(it->top()),  Qy2txY(it->bottom()));
  if (!border.isEmpty())
       repaintPosBar(dc);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiScTwin::repaintPosBar (QPainter& dc)
{
  txt *tx = vp->wtext; if (!tx) return;
  double pre = vp->wty,
         win = vp->wsh,  post = tx->maxTy - win - pre; if (post < 0) post = 0;
  double total = pre+win+post;
  if (win < total / 60) { win = total/60; total = pre+win+post; }
  int wH = Th2qtH(vp->wsh);
  int vppb0 = int(wH * pre / total + 0.5);
  int vppbh = int(wH * win / total + 0.5);
  QBrush brush(tabColor, Qt::SolidPattern);
  dc.setBrush(brush); QPen pen(brush, 1.0);
  dc.setPen(pen);
  QRect vppBar(mimTxtLEFT + Tw2qtW(vp->wsw) + mimBORDER,
                                      vppb0 + mimBORDER, mimVpPOSBAR-1, vppbh);
  dc.drawRect(vppBar);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  static QColor *colors[] = { &colorDarkWheat,  &colorSolidRed,
                              &colorSolidGreen, &colorSolidBlue };
  int N;
  for (int i = 0; i < TXT_TempMARK; i++) {
    if (tx->txmarky[i] < 0) continue;
    if (tx->txmarky[i] < vp->wty) N = int(wH*double(tx->txmarky[i])/total+0.5);
    else if (tx->txmarky[i] >= vp->wty + vp->wsh)
        N = Th2qtH(vp->wsh)-int(wH*double(tx->maxTy-tx->txmarky[i])/total+0.5);
    else continue;
    brush.setColor(*colors[i]); pen.setBrush(brush);
    dc.setBrush(brush);
    dc.setPen(pen);
    dc.drawRect(vppBar.left()-1, N + mimBORDER, mimVpPOSBAR+1, mimVpPOSBAR+1);
} }
void MiScTwin::RepaintVpPB (void) //- - - - - - - - - - - - - - - - - - - - - -
{
  update(Tx2qtX(vp->wsw)+1, mimTxtTOP, mimVpPOSBAR+2, Th2qtH(vp->wsh));
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiScTwin::Erase (QPainter& dc, QRect& rect)
{
  int ymax = rect.y() + rect.height();
  for (int y = rect.top(); y < ymax; y += fontHeight) {
    int H = my_min(ymax - y, fontHeight);
    dc.drawPixmap(rect.x(),                y,  *gradPixmap,
                  rect.x() + gradTilt * y, 0, rect.width(), H);
} }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiScTwin::Erase (QPainter& dc, int tx, int ty, int len)
{
  int X = Tx2qtX(tx), Y = Ty2qtY(ty);
  dc.drawPixmap(X,                Y, *gradPixmap,
                X + gradTilt * Y, 0, Tw2qtW(len), Th2qtH(1));
}
//-----------------------------------------------------------------------------
// Colors/fonts used (from attribute bits of tchar, see AT_xxx in mic.h header)
//
// AT_BOLD (indep) set ʁbold fontʀ - permanent (saved in file) no special meaning
// AT_PROMPT       ʂdark blueʀ text - used for prompts, indicates wildcard search
// AT_REGEX        ʄdark redʀ chars - marks regex search
// AT_SUPER        ʈsky blueʀ chars - special characters -- forces Insert mode
// AT_UNDERL       reserved for underline
//
// AT_MARKFLG (alone) marker flag 2 (DarkWheat gradient)
// AT_MARKFLG+BG_CLR: flags 1,3,4 (solid red/blue/green gradient)
// AT_BG_CLR:         background color (by itself, used for selection blocks):
//   AT_BG_RED        - red, temporary block / "can't edit" cursor / error mark
//   AT_BG_GRN        - green, "can edit, text unchanged" cursor / matched mark
//   AT_BG_BLU        - blue, regular block mark / "text changed" cursor
// AT_BRIGHT          bright background (for errors / non-closed bracket marks)
// AT_INVERT          when added to AT_BG_CLR => indicates cursor, replace mode
// AT_INVERT+AT_SUPER with AT_BG_CLR indicates gradient cursor, insert mode
//
// AT_TAB      converts space into tabColor » and marks non-printable chars
// AT_QOPEN    - opening quote ‘“
// AT_QCLOSE   - closing quote ’”
// AT_KEYWORD  dark brown text, used to highlight known keywords
// AT_COMMENT  dark grey text, indicates comments (depend on the language)
// AT_BADCHAR  solid red text for bad UTF-8 characters
//
void MiScTwin::Text (QPainter& dc, int x, int y, int attr, QString text)
{
  const QColor *bg_color = &colorWhite, *fg_color;
  int len = text.length();  QPen noPen(Qt::NoPen);
  if (attr & AT_BOLD)
       { dc.setFont(mf->getBoldFont()); fg_color = &keyColor;   }
  else { dc.setFont(mf->getTextFont()); fg_color = &colorBlack; }

  if (attr & (AT_INVERT|AT_MARKFLG|AT_BG_CLR)) {
    if (attr & AT_INVERT) {
           if ((attr & AT_BG_CLR) == AT_BG_RED) bg_color = &colorDarkRed;
      else if ((attr & AT_BG_CLR) == AT_BG_GRN) bg_color = &colorDarkGreen;
      else if ((attr & AT_BG_CLR) == AT_BG_BLU) bg_color = &colorDarkBlue;
      else                                      bg_color = &colorBlack;
      fg_color = &colorWhite;
    }
    else if (attr & AT_MARKFLG) {
           if ((attr & AT_BG_CLR) == AT_BG_RED) bg_color = &colorSolidRed;
      else if ((attr & AT_BG_CLR) == AT_BG_GRN) bg_color = &colorSolidGreen;
      else if ((attr & AT_BG_CLR) == AT_BG_BLU) bg_color = &colorSolidBlue;
      else                                      bg_color = &colorDarkWheat;
      fg_color = &colorWhite; attr |= AT_SUPER;
    }
    else if (attr & AT_BRIGHT) {
           if (attr & AT_BG_RED) bg_color = &colorLightRed;
      else if (attr & AT_BG_GRN) bg_color = &colorLightGreen;
    }
    else if ((attr & AT_BG_CLR) == AT_BG_RED) bg_color = &colorLightPink;
    else if ((attr & AT_BG_CLR) == AT_BG_GRN) bg_color = &colorDarkWheat;
    else if ((attr & AT_BG_CLR) == AT_BG_BLU) bg_color = &colorLightCyan;

    if (attr & AT_SUPER) { // special insert-mode gradient cursor (or marker)
      QLinearGradient grad(Tx2qtX(x), 0, Tx2qtX(x)+1.5*Tw2qtW(len), 0);
      grad.setColorAt(0, *bg_color);
      grad.setColorAt(1, gradColor); dc.setBrush(QBrush(grad));
    }
    else dc.setBrush(*bg_color); // regular (solid color) brush for rectangle
    dc.setPen(noPen);
    dc.drawRect(Tx2qtX(x), Ty2qtY(y), Tw2qtW(len), Th2qtH(1));
    dc.setPen(*fg_color);
  }
  else { if (attr & AT_TAB)     dc.setPen(tabColor);
    else if (attr & AT_KEYWORD) dc.setPen(keyColor);
    else if (attr & AT_REGEX)   dc.setPen(colorDarkRed);
    else if (attr & AT_PROMPT)  dc.setPen(colorDarkBlue);
    else if (attr & AT_SUPER)   dc.setPen(colorLightBlue);
    else if (attr & AT_BADCHAR) dc.setPen(colorSolidRed);
    else if (attr & AT_COMMENT) dc.setPen(colorDarkGrey); //- testing that last
    else                        dc.setPen(*fg_color);     // to preserve colors
  }                                                       // in comments (jic)
#if defined(Q_OS_MAC) && QT_VERSION >= 0x040500
# define MiTEXT_AT(x) Tx2qtX(x)-1 //
#else                             // With "raster" graphics system text painted
# define MiTEXT_AT(x) Tx2qtX(x)   // 1 pixel to the right of requested position
#endif                            //                           (moving it back)
  dc.drawText(MiTEXT_AT(x), Ty2qtY(y)+fontBaseline, text);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiScTwin::Text (QPainter& dc, int x, int y, tchar *tp, int len)
{
  static QChar str [MAXLPAC]; tchar *tpend = tp + len, tc, tcc;
         QChar *sf = str, qc; tchar tA = 1, tC;  Erase(dc, x,y, len);
  while (tp < tpend) {                           // ^
    tc = *tp++;                                  // erase the entire text area
    tC = tATTR(tc);                              // first to reduce dc switches
    tcc = (tc & AT_CHAR);
    if (tc & AT_QUOTE) { tC &= ~(AT_QOPEN|AT_QCLOSE);
           switch (tc & (AT_CHAR|AT_QOPEN|AT_QCLOSE)) {
           case AT_QOPEN |'\'': qc.unicode() = 0x2018; break; // ‘
           case AT_QCLOSE|'\'': qc.unicode() = 0x2019; break; // ’
           case AT_QOPEN | '"': qc.unicode() = 0x201C; break; // “
           case AT_QCLOSE| '"': qc.unicode() = 0x201D; break; // ”
           default:             qc.unicode() = ushort(tc & AT_CHAR);
    }      }
    else if (tcc  >  ' ') qc.unicode() = ushort(tcc);
    else if (tcc  <  ' ') qc.unicode() = ushort(tcc)+'@';
    else if (tc & AT_TAB) qc.unicode() = 0x0BB; // »
    else {                qc.unicode() = ' ';
      if (((tA^tC) & (AT_INVERT|AT_MARKFLG|AT_BG_CLR)) == 0) tC = tA;
    }
    if (tC == tA) *sf++ = qc;
    else {
      if ((len = sf-str) > 0) {
        Text(dc, x, y, tA, QString(str,len)); x += len;
      }
      sf = str; *sf++ = qc; tA = tC;
  } }
  if ((len = sf-str) > 0) Text(dc, x, y, tA, QString(str,len));
}
//-----------------------------------------------------------------------------
void MiScTwin::keyPressEvent (QKeyEvent *event)
{
  int     key  = event->key() | int(event->modifiers() &  Qt::KeypadModifier);
  QString text = event->text();
  int  modMask = ((event->modifiers() & Qt::ShiftModifier) ? mod_SHIFT : 0)|
                 ((event->modifiers() & Qt::AltModifier)   ? mod_ALT   : 0)|
#ifdef Q_OS_MAC
                 ((event->modifiers() & Qt::MetaModifier)    ? mod_CTRL : 0)|
                 ((event->modifiers() & Qt::ControlModifier) ? mod_META : 0);
#else
                 ((event->modifiers() & Qt::ControlModifier) ? mod_CTRL : 0)|
                 ((event->modifiers() & Qt::MetaModifier)    ? mod_META : 0);
#endif
  if (MiApp_debugKB) {            fprintf(stderr, "OnKey(%x",    key);
    if (text[0].unicode() > 0x1f) fprintf(stderr, ":%s", text.cStr());
    else for (int i=0; i<text.length(); i++)
       fprintf(stderr, ".%04x", text[i].unicode());
  //
    fprintf(stderr, ")mods=%c%c%c%c,native=%x:%x:%x",
       (modMask & mod_META)  ? 'M' : '.', (modMask & mod_ALT)   ? 'a' : '.',
       (modMask & mod_CTRL)  ? 'c' : '.', (modMask & mod_SHIFT) ? 's' : '.',
       event->nativeScanCode(),
       event->nativeModifiers(), event->nativeVirtualKey());
  }
  if (vipOSmode) {
    switch (key) {
    case Qt::Key_Escape:    setkbhin   (4); return; // ^D = end-of-file
    case Qt::Key_Return:    setkbhin('\n'); return;
    case Qt::Key_Backspace: setkbhin('\b'); return;
    }
    if ((modMask & mod_CTRL) && 0x40 <= key && key < 0x60) setkbhin(key-0x40);
    else   for (int i=0; i<text.length(); i++) setkbhin(text.at(i).toAscii());
    return;
  }
#ifdef Q_OS_MAC                            // hack for KeyRemap4MacBook 5.1.0,
  if (key == Mk_McENTER) key = Mk_INSERT;  // which can only remap Fn to Enter
# if (QT_VERSION < 0x040500)
    if (key == Qt::Key_unknown)            //- Legacy Qt version 4.3.3 does not
      switch (event->nativeVirtualKey()) { // convert keys F13…F15 correctly on
      case 0x69: key = Qt::Key_F13; break; // Apple new solid aluminum keyboard
      case 0x6b: key = Qt::Key_F14; break; // (model A1243), fixing that, using
      case 0x71: key = Qt::Key_F15; break; // native virtual key (which is Ok)
      }
# endif // Make MicroMir numpad commands work the same with any NumLock state
#else   //                        (not applicble to Mac, which has no NumLock)
  if (key & Qt::KeypadModifier) {
# ifdef Q_OS_WIN                    // completely ignore Alt+Numpad keys,
    if (modMask == mod_ALT) return; // let Windows handle them by itself
# endif
    int keyName = (key & ~Qt::KeypadModifier);
    if (keyName == Qt::Key_Enter) keyName = Mk_PAD_ENTER; // merge numpad Enter
    switch (modMask + keyName) {                          // to Return/Enter(↵)
    case mod_CTRL+'8':
    case mod_CTRL+          Qt::Key_Up:
    case mod_CTRL+mod_SHIFT+Qt::Key_Up:    key = Mk_PAD_UP;    break;
    case mod_CTRL+'2':
    case mod_CTRL+          Qt::Key_Down:
    case mod_CTRL+mod_SHIFT+Qt::Key_Down:  key = Mk_PAD_DOWN;  break;
    case mod_CTRL+'4':
    case mod_CTRL+          Qt::Key_Left:
    case mod_CTRL+mod_SHIFT+Qt::Key_Left:  key = Mk_PAD_LEFT;  break;
    case mod_CTRL+'6':
    case mod_CTRL+          Qt::Key_Right:
    case mod_CTRL+mod_SHIFT+Qt::Key_Right: key = Mk_PAD_RIGHT; break;
    case mod_CTRL+'-': //
    case mod_CTRL+'+': //
    case mod_CTRL+'/': // these keys are already Ok, leave them alone
    case mod_CTRL+'*': //
    case mod_CTRL+Qt::Key_Return: break;
    default:
      if (modMask) {       // Force corresponding command with any modifier..
        switch (keyName) { //
        case '.': key = Mk_DELETE;   break;
        case '0': key = Mk_INSERT;   break;   case '5': key = Mk_CLEAR;  break;
        case '1': key = Mk_END;      break;   case '6': key = Mk_RIGHT;  break;
        case '2': key = Mk_DOWN;     break;   case '7': key = Mk_HOME;   break;
        case '3': key = Mk_PAGEDOWN; break;   case '8': key = Mk_UP;     break;
        case '4': key = Mk_LEFT;     break;   case '9': key = Mk_PAGEUP; break;
        default:  key = keyName;
      } }
      else key = keyName; // ..and ingnore Qt::KeypadModifier otherwise
  } }
#endif // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  last_MiCmd_code = key|modMask|key2mimHomeEscMask();
  int mk_ev = MiApp_keyMap.value(last_MiCmd_code);
  micom *mk = NULL, userMk;
  if (mk_ev) {      userMk.ev  = (micom_enum)mk_ev;
    mk = &userMk;   userMk.kcode = last_MiCmd_code;
    switch (mk_ev & 0xf0000) {                      // NOTE: text/line editors
    case 0xc0000: userMk.attr = 0;          break;  // do not care about KxTS,
    case 0xd0000: userMk.attr =      KxBLK; break;  // thus we must convert it
    case 0xf0000: userMk.attr =      KxSEL; break;  // to KxBLK (same for all)
    case 0xe0000: userMk.attr = KxTS|KxSEL;         //
      userMk.ev = (micom_enum)((userMk.ev & ~KxTMP) | KxBLK);
  } }
  else if (key && !Mk_IsSHIFT(key)) mk = key2mimCmd(key|modMask);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  if (MiApp_debugKB) {
            fprintf(stderr, ",micom=%x",            last_MiCmd_code);
    if (mk) fprintf(stderr, ":0x%x[%02x]\n", mk->ev, mk->attr >> 12);
    else    fprintf(stderr, "\n");                 info.updateInfo();
  }
  if ((mk != NULL && mk->ev == TK_NONE) ||
      (mk == NULL && text.isEmpty() )) event->ignore();
  else {                stopTimer();   event->accept();  setkbhin(0);
    //
    // Here we have either valid microMir command (mk != NULL) or non-empty
    // text (otherwise) that should be fed char-by-char into same function:
    //
    if (mk) vipOnKeyCode(vp, mk->ev, mk->attr);
    else {
      for (int i=0; i<text.length(); i++) {
        int k = text.at(i).unicode();
        if (Mk_IsCHAR(k)) vipOnKeyCode(vp, k, KxSEL);
    } }
    vipReady();
} }
//-----------------------------------------------------------------------------
#define MiRPT_TIME 10
//
void MiScTwin::repeatCmd (int kcode, int count)
{
  if (vipOnMimCmd(vp, kcode, 0)) return; // <- sets initial last_MiCmd_time
  if (count > 1) {
    repeatCount = count - 1;
    cmd2repeat = kcode;
    timerID = startTimer(10); pastDue = 0;
} }
void MiScTwin::stopTimer()
{
  if (cmd2repeat) { cmd2repeat = 0; killTimer(timerID); }
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiScTwin::timerEvent(QTimerEvent *)
{
  quint64 now = pgtime(), elapsed = (now - last_MiCmd_time) + pastDue;
  if (elapsed < MiRPT_TIME) pastDue = 0;
  else {
    KbCount = elapsed / MiRPT_TIME;
    pastDue = elapsed - MiRPT_TIME*KbCount;
  }
  if (vipOnMimCmd(vp, cmd2repeat, 0) ||
      (repeatCount -= KbCount) <= 0) { stopTimer(); vipReady(); }
  else {
    info.updateInfo(MitLINE_BLOCK);
    QApplication::syncX();   // only for X11 (does nothing on other platforms)
} }
//-----------------------------------------------------------------------------
void MiScTwin::focusInEvent(QFocusEvent *) 
{ 
  gotFocus = pgtime(); vipOnFocus(vp); info.show();
}                                                                           
void MiScTwin::focusOutEvent(QFocusEvent *) { vipFocusOff(vp); info.hide();
                                              clipFocusOff (); stopTimer(); }
// Not sure how to check if we're losing
// focus for good, or will regain it very soon (useful for saving clipboard)...
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiScTwin::mousePressEvent (QMouseEvent *ev)  // silently ignore 1st click,
{                                                 // if event comes fast enough
  if (vp == Twnd) {                               // after the window got focus
    quint64 now = pgtime();                       // (but make sure next click
    if (now - gotFocus < 100) gotFocus = now-100; //        will get processed)
    else
      vipGotoXY(Qx2txX(ev->pos().x()), Qy2txY(ev->pos().y())); ev->accept();
} }
void MiScTwin::wheelEvent (QWheelEvent *ev)
{
  if (ev->orientation() == Qt::Vertical) { stopTimer();
    int delta = ev->delta();
    int steps = my_abs(delta)/120; KbCount = steps ? steps : 1;
    static quint64 prev_event = 0;
           quint64 now = pgtime();
    bool fast = (prev_event && (now - prev_event) < 250); setkbhin(0);
    prev_event = now;
    vipOnKeyCode(vp, (delta > 0) ? (fast ? TW_SCROLUPN : TW_SCROLUP) :
                                   (fast ? TW_SCROLDNN : TW_SCROLDN), KxBLK);
    vipReady(); ev->accept();
} }
//=============================================================================
MiInfoWin::MiInfoWin (MiScTwin *parent)  :  QWidget(parent),
                      infoType(MitLINE_BLOCK), sctw(parent)
{
  setFocusPolicy(Qt::NoFocus);
  setAutoFillBackground(true); setFont(sctw->mf->getTextFont());
}
void MiInfoWin::SetPalette (QColor bgnd, QColor text)
{
  QPalette palette(bgnd);
  palette.setColor(QPalette::WindowText, text); setPalette(palette);
}
void MiInfoWin::vpResize() { resize(2 * mimBORDER + sctw->Tw2qtW(9),
                                    2 * mimBORDER + sctw->Th2qtH(1)); }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiInfoWin::paintEvent (QPaintEvent *)
{
  QPainter dc(this); int Y = sctw->Ty2qtY(0)+sctw->fontBaseline;
  QString info;                                      int dx, dy;
  if (MiApp_debugKB) {
    info.sprintf("%8x",  last_MiCmd_code);
    dc.drawText(sctw->Tx2qtX(0), Y, info);
  }
  else if (infoType == MitCHARK && sctw->vp->cx >= 0) {
    tchar tc = 0, *textln;
    if (TxSetY(sctw->vp->wtext, sctw->vp->wty+sctw->vp->cy)) {
      textln = TxInfo(sctw->vp, sctw->vp->wty+sctw->vp->cy, &dx);
      tc = textln[sctw->vp->wtx+sctw->vp->cx]; // ^
    }                                          // assuming end of textln string
    info.sprintf("U+%X",(uint)(tc & AT_CHAR)); // cleared with spaces after EOL
    dc.drawText(sctw->Tx2qtX(0), Y, info);
  }
  else if (BlockXYsize(&dx ,&dy)) { info.sprintf ("%dx%d",      dx,   dy);
                                    dc.drawText(sctw->Tx2qtX(0), Y, info); }
  else if (MiApp_timeDELAYS &&
     (dx = (int)(pgtime()-last_MiCmd_time)) > 1) { info.sprintf("%4dms", dx);
                                       dc.drawText(sctw->Tx2qtX(0), Y, info); }
  else {
    info.sprintf("%7d", int((sctw->vp == Twnd) ? Ty : sctw->vp->wcy)+1);
    if (sctw->vp->wtext && 0 < sctw->vp->wtext->clang &&
                               sctw->vp->wtext->clang < CLangMAX) {
      int *Synts = sctw->vp->wtext->lastSynts,
        numSynts = Synts[0] & AT_CHAR;
      for (int i = 0; i < numSynts && i < 3; i++) {
        if (info[i] > ' ') info[i+1] = QChar(0x2026);
                           info[i]   = QChar(Synts[i+1] & 0xFF);
    } }
    dc.drawText(sctw->Tx2qtX(0),                     Y, info.mid(0, 4));
    dc.drawText(sctw->Tx2qtX(4) + sctw->fontWidth/2, Y, info.mid(4, 3));
  }
  dc.drawText(sctw->Tx2qtX(8), Y, clipStatus());
}
void MiInfoWin::updateInfo (MiInfoType mit) { if (mit) infoType = mit;
                                                            repaint(); }
//-----------------------------------------------------------------------------
static void myParseKeyMap() // parse QString MiApp_keyMaps -> QMap MiApp_keyMap
{
  QStringList lines = MiApp_keyMaps.split(QChar('\n'));
  MiApp_keyMap.clear();
  for (QStringList::const_iterator it  = lines.constBegin();
                                   it != lines.constEnd(); it++)
  { int key, mk;
    if (sscanf(it->cStr(), "%x:0x%x", &key, &mk) == 2)
      MiApp_keyMap.insert(key, mk);
} }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline QString myPackAdj2string (int X, int Y)
{
  return QString("%1/%2").arg(QChar((X > 0) ? '+' : (X < 0) ? '-' : '0'))
                         .arg(QChar((Y > 0) ? '+' : (Y < 0) ? '-' : '0'));
}
inline int myUnpackAdj2int (QString adj, int ix) // - - - - - - - - - - - - - -
{
  const char c = adj[ix].toAscii();
  return (c == '+') ? +1 : (c == '-') ? -1 : 0;
}
//-----------------------------------------------------------------------------
MiConfigDlg::MiConfigDlg(QWidget *parent) : QDialog(parent)
{
  QVBoxLayout *leftLayout = new QVBoxLayout; setWindowTitle(MimVersion());
  QLabel *microLabel = new QLabel();
  QPixmap *microIcon = new QPixmap(":/microMir.png");
  microLabel->setPixmap(*microIcon);
  leftLayout->addWidget(microLabel); leftLayout->addStretch(1);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  gradDescr = new QLineEdit(MiApp_gradDescr);
  QLabel *grad = new QLabel(tr("Gradient:")); grad->setBuddy(gradDescr);
  QHBoxLayout *layoutH1 = new QHBoxLayout;
  layoutH1->addWidget(grad);
  layoutH1->addWidget(gradDescr);  leftLayout->addLayout(layoutH1);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  fontButton = new QPushButton(tr("Font"));
  fontLabel  = new QLabel();
  fontLabel->setMinimumWidth(150);              setFontLabelText();
  connect(fontButton, SIGNAL(clicked()), this, SLOT(selectFont()));
  fontAdjust = new QLineEdit();
  fontAdjust->setMaximumWidth(32);
  fontAdjust->setText(myPackAdj2string(MiApp_fontAdjOver, MiApp_fontAdjUnder));
  QHBoxLayout *layoutH2 = new QHBoxLayout;
  layoutH2->addWidget(fontButton);
  layoutH2->addWidget(fontLabel);
  layoutH2->addWidget(fontAdjust); leftLayout->addLayout(layoutH2);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  static QString fontHelpString("<small>Font height adjustment parameter on "
    "right alllows adding (+) or removing (-) one pixel on top (first symbol) "
    "or bottom (second one) of character glyph; use 0/0 to leave the height "
    "unchanged</small><hr>");
  QLabel *fontHelpLabel = new QLabel(fontHelpString);
  fontHelpLabel->setWordWrap(true);  leftLayout->addWidget(fontHelpLabel);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  tabSizeBox = new QSpinBox(); QLabel *tabl = new QLabel(tr("TAB size:"));
  tabSizeBox->setRange(2, 8);          tabl->setBuddy(tabSizeBox);
  tabSizeBox->setMaximumWidth(40);
  tabSizeBox->setValue(TABsize);
  QHBoxLayout *layoutH3 = new QHBoxLayout;
  layoutH3->addWidget(tabl);
  layoutH3->addWidget(tabSizeBox);
  layoutH3->addStretch(1);    leftLayout->addLayout(layoutH3);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  QVBoxLayout *rightLayout = new QVBoxLayout;
  keymapLabl = new QLabel(tr("Keycodes mapping (see help):"));
  keymapEdit = new QTextEdit();
  keymapEdit->setAcceptRichText(false);
  keymapEdit->setLineWrapMode(QTextEdit::NoWrap);
  keymapEdit->setPlainText(MiApp_keyMaps);
  keymapEdit->setMaximumWidth  (250); keymapEdit->setMinimumWidth (250);
  rightLayout->addWidget(keymapLabl); keymapEdit->setMinimumHeight(320);
  rightLayout->addWidget(keymapEdit);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  QHBoxLayout *outerLayout = new QHBoxLayout;
  outerLayout->addLayout(leftLayout);
  outerLayout->addLayout(rightLayout);
  QDialogButtonBox *okButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
  connect(okButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout(outerLayout);
  mainLayout->addWidget(okButtonBox); setLayout(mainLayout);
}
void MiConfigDlg::setFontLabelText() // - - - - - - - - - - - - - - - - - - - -
{
  fontLabel->setText(QString("%1, %2pt").arg(MiApp_defaultFont)
                                        .arg(MiApp_defFontSize));
}
//-----------------------------------------------------------------------------
bool MiConfigDlg::Ask()
{
  if (exec() != QDialog::Accepted) return false;
  TABsize = tabSizeBox->value();
  MiApp_gradDescr = gradDescr->text();
  MiApp_defFontAdjH = fontAdjust->text();
  MiApp_fontAdjOver = myUnpackAdj2int(MiApp_defFontAdjH, 0);
  MiApp_fontAdjUnder = myUnpackAdj2int(MiApp_defFontAdjH, 2);
  MiApp_keyMaps = keymapEdit->toPlainText(); myParseKeyMap(); return true;
}
void MiConfigDlg::selectFont() // - - - - - - - - - - - - - - - - - - - - - - -
{
  QFont refont = QFont(MiApp_defaultFont, MiApp_defFontSize);
  bool ok;   
#ifdef notdef_Q_OS_MAC_hack_not_required
//
// Because of bug in Carbon-based Qt 4.5+, native (i.e. Cocoa) font selection
// dialog did not really work -- in addition to some funky issues with focus,
// it did not allow selection of some fonts... like Menlo (which is the best)
//
// Since the only reason to use Carbon build was inability to get Qt::Key_Help
// (or Qt::Key_Insert in any form) and that may be fixed by KeyRemap4MacBook,
//                          switched permanently to Qt-recommended Cocoa build
  QString title = "Font";
  refont = QFontDialog::getFont(&ok, refont, parentWidget(),
                              title, QFontDialog::DontUseNativeDialog);
#else
  refont = QFontDialog::getFont(&ok, refont, parentWidget());
#endif
  if (ok) { MiApp_defaultFont = refont.family();
            MiApp_defFontSize = refont.pointSize(); setFontLabelText(); }
}
//-----------------------------------------------------------------------------
void mimReadPreferences()
{
  QSettings Qs;
  TABsize = Qs.value("TABsize", 4).toInt();
  MiApp_gradDescr = Qs.value("bgndGrad", defWinGRADIENT).toString();
  MiApp_defHeight = Qs.value("frameHeight", defWinHEIGHT).toInt();
  MiApp_defWidth   =  Qs.value("frameWidth", defWinWIDTH).toInt();
  MiApp_defaultFont = Qs.value("font", QString(mimFONTFACENAME)).toString();
  MiApp_defFontSize = Qs.value("fontSize", mimFONTSIZE).toInt();
  MiApp_defFontAdjH = Qs.value("fontAdjH", "0/0").toString();
  MiApp_fontAdjOver = myUnpackAdj2int (MiApp_defFontAdjH, 0);
  MiApp_fontAdjUnder = myUnpackAdj2int(MiApp_defFontAdjH, 2);
  MiApp_keyMaps  =  Qs.value("keyMaps", "").toString();
  key2mimStart();                      myParseKeyMap();
  MiFrameSize = QSize(MiApp_defWidth, MiApp_defHeight);
#ifndef Q_OS_MAC
  menuBarHeight = Qs.value("private/menuBarHeight", -1).toInt();
#endif
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiFrame::Preferences()
{
  MiConfigDlg dialog;
  if (dialog.Ask()) {
    QSettings Qs;                              Qs.setValue("TABsize", TABsize);
    MiApp_defFontAdjH = myPackAdj2string(MiApp_fontAdjOver,MiApp_fontAdjUnder);
    Qs.setValue("font",     MiApp_defaultFont);
    Qs.setValue("fontSize", MiApp_defFontSize);
    Qs.setValue("fontAdjH", MiApp_defFontAdjH); makeFonts();
    Qs.setValue("keyMaps",  MiApp_keyMaps);
    Qs.setValue("bgndGrad", MiApp_gradDescr);
    if (main) main->SetGradient(MiApp_gradDescr);
    if (main)  {  main->UpdateMetrics();  main->vpResize(); }
    if (scwin) { scwin->UpdateMetrics(); scwin->vpResize(); } shrinkwrap();
} }
//=============================================================================
#include <time.h>

#ifdef Q_OS_WIN
#  include <windows.h>
#  define EPOCH_BIAS 116444736000000000LL
#  define TEN_MILLION          10000000LL
#  define TEN_THOUSAND            10000LL
typedef union {
  FILETIME         ft_struct;
  unsigned __int64 ft_scalar;
}
FILETIME_as_int64;

void usectime_p (struct timeval *tv)
{
  FILETIME_as_int64 ft;
  GetSystemTimeAsFileTime(&(ft.ft_struct));
  tv->tv_sec = (time_t)((ft.ft_scalar -   EPOCH_BIAS) /  TEN_MILLION);
  tv->tv_usec = (long)(((ft.ft_scalar / TEN_THOUSAND) % 1000) * 1000);
}
#else 
# include <sys/time.h>
# define usectime_p(_stv) gettimeofday((_stv),0)
#endif
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
quint64 pgtime(void)    /* returns current time with "pretty good" precision */
{
  struct timeval tv;                   usectime_p(&tv);
  return (quint64)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
//-----------------------------------------------------------------------------
