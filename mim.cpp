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
extern "C" { extern const char microVERSION[]; }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
BOOL dosEOL = false;                              /* Настраиваемые параметры */
int TABsize = 4;
static bool MiApp_debugKB    = false;
static bool MiApp_timeDELAYS = false;
       bool MiApp_useDIAGRAD = true;
quint64 last_MiCmd_time;
QString MiApp_defaultFont, MiApp_defFontAdjH;
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
# define mimFONTFACENAME "Menlo Regular"
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
inline QString myPackAdj2string (int X, int Y)
{
  return QString("%1/%2").arg(QChar((X > 0) ? '+' : (X < 0) ? '-' : '0'))
                         .arg(QChar((Y > 0) ? '+' : (Y < 0) ? '-' : '0'));
}
inline int myUnpackAdj2int (QString adj, int ix)
{
  const char c = adj[ix].toAscii();
  return (c == '+') ? +1 : (c == '-') ? -1 : 0;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QCoreApplication::setOrganizationDomain("epiktetov.com");
  QCoreApplication::setOrganizationName("EpiMG");
  QCoreApplication::setApplicationName("micro7"); tmInitialize();
  QSettings Qs;
  TABsize = Qs.value("TABsize", 4).toInt();
  MiApp_defWidth    = Qs.value("frameWidth", defWinWIDTH).toInt();
  MiApp_defHeight   = Qs.value("frameHeight", defWinHEIGHT).toInt();
  MiApp_defaultFont = Qs.value("font", QString(mimFONTFACENAME)).toString();
  MiApp_defFontSize = Qs.value("fontSize", mimFONTSIZE).toInt();
  MiApp_defFontAdjH = Qs.value("fontAdjH", 0).toString();
  MiApp_fontAdjOver = myUnpackAdj2int(MiApp_defFontAdjH, 0);
  MiApp_fontAdjUnder = myUnpackAdj2int(MiApp_defFontAdjH, 2);
#ifdef Q_OS_MAC
  app.installEventFilter(new MacEvents);
#else
  menuBarHeight = Qs.value("private/menuBarHeight", -1).toInt();
#endif
  for (int i = 1; i < argc; i++) {
    QString param = Utf8(argv[i]);
         if (param.compare("-dos",IGNORE_CASE) == 0) dosEOL           =  TRUE;
    else if (param.compare("-kb", IGNORE_CASE) == 0) MiApp_debugKB    =  true;
    else if (param.compare("-ti", IGNORE_CASE) == 0) MiApp_timeDELAYS =  true;
    else if (param.compare("-dg", IGNORE_CASE) == 0) MiApp_useDIAGRAD =  true;
    else if (param.compare("-lg", IGNORE_CASE) == 0) MiApp_useDIAGRAD = false;
    else {
#ifdef Q_OS_WIN                // Qt operates with correct forward slashes, but
      param.replace('\\','/'); // argv here comes from Windows, have to replace
#endif
      if (param.startsWith("'")) param.replace(0,1,QfsXEQ_PREFIX);
      if (tmStart(param)) return app.exec();
      else                return 1;
  } }
  if (tmStart(QfsEMPTY_NAME)) return app.exec();
  else                        return 2;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void mimExit()
{
  cpsave(); QCoreApplication::quit();
}
//-----------------------------------------------------------------------------
MiFrame::MiFrame (MiFrame *base) : tSize(0,0), oldSize(0,0), wrapped(false),
                                   mbox(0),         main(0), scwin(0)
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
  MiApp_defFontAdjH = myPackAdj2string(MiApp_fontAdjOver, MiApp_fontAdjUnder);
  Qs.setValue("font",     MiApp_defaultFont);
  Qs.setValue("fontSize", MiApp_defFontSize);  if (main) delete main;
  Qs.setValue("fontAdjH", MiApp_defFontAdjH);  if (scwin) delete scwin;
}                          
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiFrame::makeFonts() // makes textFont and boldFont from MiApp_defaultFont
{
  textFont = QFont(MiApp_defaultFont, MiApp_defFontSize);
#if QT_VERSION >= 0x040700
  textFont.setStyleHint(QFont::AnyStyle, QFont::ForceIntegerMetrics);
#endif
//+
//  QFontMetrics fm = textFont;
//  int wi1 = fm.width("X"),
//    wi100 = fm.width("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
//                     "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
//  fprintf(stderr, "width=%d,x100=%d\n", wi1, wi100);
//-
  boldFont = QFont(textFont);  boldFont.setBold(true);
}
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
void MiFrame::Preferences()
{
  MiConfigDlg dialog;                              // font parameters are saved
  if (dialog.Ask()) {                              // in frame destructor along
    QSettings Qs; Qs.setValue("TABsize", TABsize); //< with current window size
    makeFonts();
    if (main)  {  main->UpdateMetrics();  main->vpResize(); }
    if (scwin) { scwin->UpdateMetrics(); scwin->vpResize(); } shrinkwrap();
} }
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
  MiScTwin *sctw = new MiScTwin(this, &colorWinGrad, &colorWinBgnd, vp);
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
  QString title = Utf8("µMup07 version %1 (" QtPLATF ")").arg(microVERSION);
  if (main && main->vp && main->vp->wtext && !isNoRealText(main->vp->wtext)
                                          &&         main->vp->wtext->file)
                                title = QfsShortName(main->vp->wtext->file);
  if (scwin &&
      scwin->vp && scwin->vp->wtext && scwin->vp->wtext->file)
                      title.push_back(QString::fromUtf8(" • ") +
                                      QfsShortName(scwin->vp->wtext->file));
  setWindowTitle(title);
}
//-----------------------------------------------------------------------------
inline QString myQSize2string (QSize& size)
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
    tSize.rwidth() = main->vp->wsw; size_menu->setTitle(myQSize2string(tSize));
    if (oldSize.isNull()) {
      if (tSize == defWinSize) size_act->setText(myQSize2string(altWinSize));
      else                     size_act->setText(myQSize2string(defWinSize));
    } else                     size_act->setText(myQSize2string(oldSize));
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
  QSize tmp = tSize;
  if (oldSize.isNull()) {
    if (tSize == defWinSize) tSize = altWinSize;
    else                     tSize = defWinSize;
  } else                     tSize = oldSize;
  oldSize = tmp;
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
  if (wrapped) wrapped = false;
  else {
    if (oldSize.isNull()) oldSize = tSize;
    QTimer::singleShot(0, this, SLOT(shrinkwrap()));
} }
//=============================================================================
MiInfoWin::MiInfoWin (MiScTwin *parent, const QColor *bgnd) : QWidget(parent), 
                                        infoType(MitLINE_BLOCK), sctw(parent)
{
  QPalette palette(*bgnd);   setFocusPolicy(Qt::NoFocus);
  palette.setColor(QPalette::WindowText, colorDarkWheat);
  setPalette(palette);       setAutoFillBackground(true);
  setFont(sctw->mf->getTextFont());
}
void MiInfoWin::vpResize() { resize(2 * mimBORDER + sctw->Tw2qtW(9),
                                    2 * mimBORDER + sctw->Th2qtH(1)); }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiInfoWin::paintEvent (QPaintEvent *)
{
  QPainter dc(this); int Y = sctw->Ty2qtY(0)+sctw->fontBaseline;
  QString info;                                      int dx, dy;
  if (infoType == MitCHARK && sctw->vp->cx >= 0) {
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
    if (sctw->vp->wtext && sctw->vp->wtext->lastSynts) {
      int *Synts = sctw->vp->wtext->lastSynts,
        numSynts = (Synts[0] & (~AT_COMMENT));
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
MiConfigDlg::MiConfigDlg(QWidget *parent) : QDialog(parent)
{
  fontButton = new QPushButton(tr("Font"));
  fontLabel  = new QLabel();
  fontLabel->setMinimumWidth(150); setFontLabelText();
  fontAdjust = new QLineEdit();
  fontAdjust->setMaximumWidth(32);
  fontAdjust->setText(myPackAdj2string(MiApp_fontAdjOver, MiApp_fontAdjUnder));

  QHBoxLayout *topLayout = new QHBoxLayout;
  topLayout->addWidget(fontButton);
  topLayout->addWidget(fontLabel);
  topLayout->addWidget(fontAdjust);
  connect(fontButton, SIGNAL(clicked()), this, SLOT(selectFont()));

  tabSizeBox = new QSpinBox();      QLabel *tabl = new QLabel(tr("TAB size:"));
  tabSizeBox->setRange(2, 8);       tabl->setBuddy(tabSizeBox);
  tabSizeBox->setMaximumWidth(40);  QHBoxLayout *botLayout = new QHBoxLayout;
  tabSizeBox->setValue(TABsize);    botLayout->addWidget(tabl);
                                    botLayout->addWidget(tabSizeBox);
                                    botLayout->addStretch(1);

  QDialogButtonBox *okButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
  connect(okButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout(topLayout);
  mainLayout->addLayout(botLayout);
  mainLayout->addWidget(okButtonBox); setLayout(mainLayout);
}
void MiConfigDlg::setFontLabelText() // - - - - - - - - - - - - - - - - - - - -
{
  fontLabel->setText(QString("%1, %2pt").arg(MiApp_defaultFont)
                                        .arg(MiApp_defFontSize));
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool MiConfigDlg::Ask()
{
  if (exec() != QDialog::Accepted) return false;
  TABsize = tabSizeBox->value();
  MiApp_defFontAdjH = fontAdjust->text();
  MiApp_fontAdjOver = myUnpackAdj2int(MiApp_defFontAdjH, 0);
  MiApp_fontAdjUnder = myUnpackAdj2int(MiApp_defFontAdjH, 2); return true;
}
void MiConfigDlg::selectFont() // - - - - - - - - - - - - - - - - - - - - - - -
{
  QFont refont = QFont(MiApp_defaultFont, MiApp_defFontSize);
  bool ok;   
//
// Bug in Qt 4.5+ (or in Carbon version?): native (i.e. Cocoa) font selection
// dialog does not really works. In addition to some funky issues with focus,
// it does not allow selection of some fonts... like Menlo (which is the best)
//
#ifdef notdef_Q_OS_MAC_does_not_work
  setModal(false); refont = QFontDialog::getFont(&ok, refont, parentWidget());
  setModal(true);
#elif defined(Q_OS_MAC) && (QT_VERSION >= 0x040500)
  QString title = "Font";
  refont = QFontDialog::getFont(&ok, refont, parentWidget(),
                              title, QFontDialog::DontUseNativeDialog);
#else
  refont = QFontDialog::getFont(&ok, refont, parentWidget());
#endif
  if (ok) { MiApp_defaultFont = refont.family();
            MiApp_defFontSize = refont.pointSize(); setFontLabelText(); }
}
//=============================================================================
MiScTwin::MiScTwin(MiFrame *frame, const QColor *prim,
                                   const QColor *bg_color, wnd *win)
 : gradColor(*prim),
 bgColor(*bg_color), gotFocus(0),  vp(win), mf(frame),
 info  (this, prim), gradPixSize(0), gradPixHeight(0),
                     gradPixmap(NULL),  cmd2repeat(0), timerID(0)
{
  vp->sctw = this; setAttribute(Qt::WA_NoSystemBackground);
                   setAttribute(Qt::WA_OpaquePaintEvent);   UpdateMetrics();
  info.show();     setFocusPolicy(Qt::ClickFocus);          UpdateGradient();
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
            height() - info.height() - fontWidth); UpdateGradient();
}                    
//-----------------------------------------------------------------------------
void MiScTwin::UpdateMetrics()          // NOTE: assuming fixed-width font with
{                                       // adjustments: MiApp_fontAdjOver/Under
  QFontMetrics fm =  mf->getTextFont();
  fontBaseline = fm.ascent();
  fontHeight = fm.height()-1; // Qt height always equal to ascent()+descent()+1
  fontWidth  = fm.width("X");
  fontHeight += MiApp_fontAdjOver; fontBaseline += MiApp_fontAdjOver;
  fontHeight += MiApp_fontAdjUnder;                  info.vpResize();
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiScTwin::UpdateGradient()   // fancy diagonal gradient -- because we can!
{
  int gradPixWidth, winSize = width() + (MiApp_useDIAGRAD ? height() : 0);
  if (winSize == gradPixSize &&
                  gradPixmap && fontHeight <= gradPixHeight) return;
  else {
    gradPixSize  = winSize; /* gradient pixmap depends only */
    gradPixWidth = winSize; /* on win size, not proportions */
  }
  if (gradPixmap) delete gradPixmap;
  gradPixmap = new QPixmap(gradPixWidth, gradPixHeight = fontHeight);
  QRect gradRect = gradPixmap->rect();       QPainter dc(gradPixmap);
  QPen noPen(Qt::NoPen);
  QLinearGradient grad(gradRect.topLeft(), gradRect.bottomRight());
  grad.setColorAt(0, gradColor);
  grad.setColorAt(1,   bgColor);  dc.setPen(noPen);
  dc.setBrush(QBrush(grad)); dc.drawRect(gradRect);
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
  QBrush brush(colorDarkWheat, Qt::SolidPattern);
  dc.setBrush(brush);       QPen pen(brush, 1.0);
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
    dc.drawPixmap(rect.x(),                        y, *gradPixmap,
                  rect.x()+(MiApp_useDIAGRAD?y:0), 0, rect.width(), H);
} }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiScTwin::Erase (QPainter& dc, int tx, int ty, int len)
{
  int X = Tx2qtX(tx), Y = Ty2qtY(ty);
  dc.drawPixmap(X,                        Y, *gradPixmap,
                X+(MiApp_useDIAGRAD?Y:0), 0, Tw2qtW(len), Th2qtH(1));
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
// AT_TAB      converts space into DarkWheat » (should not be used otherwise)
// AT_QOPEN    - opening quote ‘“
// AT_QCLOSE   - closing quote ’”
// AT_KEYWORD  dark brown text, used to highlight known keywords
// AT_COMMENT  dark grey text, indicates comments (depend on the language)
// AT_BADCHAR  solid red text for bad UTF-8 characters
//
void MiScTwin::Text (QPainter& dc, int x, int y, int attr, QString text)
{
  int len = text.length();    QPen noPen(Qt::NoPen);
  if (attr & AT_BOLD) dc.setFont(mf->getBoldFont());
  else                dc.setFont(mf->getTextFont());

  if (attr & (AT_INVERT|AT_MARKFLG|AT_BG_CLR)) {
    const QColor *bg_color = &colorWhite, *fg_color = &colorBlack;
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
  else { if (attr & AT_TAB)     dc.setPen(colorDarkWheat);
    else if (attr & AT_KEYWORD) dc.setPen(colorDarkBrown);
    else if (attr & AT_REGEX)   dc.setPen(colorDarkRed);
    else if (attr & AT_PROMPT)  dc.setPen(colorDarkBlue);
    else if (attr & AT_SUPER)   dc.setPen(colorLightBlue);
    else if (attr & AT_BADCHAR) dc.setPen(colorSolidRed);
    else if (attr & AT_COMMENT) dc.setPen(colorDarkGrey); //- testing that last
    else                        dc.setPen(colorBlack);    // to preserve colors
    Erase(dc, x, y, len);                                 // in comments (jic)
  }
  dc.drawText(Tx2qtX(x), Ty2qtY(y)+fontBaseline, text);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline QChar vChar (tchar tc) // convert chars below space into printable ones
{ 
  if (tc & AT_QUOTE) {
    switch (tc & (AT_CHAR|AT_QOPEN|AT_QCLOSE)) {
    case AT_QOPEN |'\'': return QChar(0x2018); // ‘
    case AT_QCLOSE|'\'': return QChar(0x2019); // ’
    case AT_QOPEN | '"': return QChar(0x201C); // “
    case AT_QCLOSE| '"': return QChar(0x201D); // ”
    default:             return int(tc & AT_CHAR);
  } }
  else {
    tchar tcc = (tc & AT_CHAR);  if (tcc  >  ' ') return int(tcc);
                            else if (tcc  <  ' ') return int(tcc)+'@';
                            else if (tc & AT_TAB) return QChar(0x0BB); // »
                            else                  return ' ';
} }
inline tchar vAttr (tchar tc) 
{ 
  tchar attr = tc & AT_ALL; return ((tc & AT_CHAR) < ' ') ? AT_TAB|attr : attr;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiScTwin::Text (QPainter& dc, int x, int y, tchar *tp, int len)
{
  tchar *tpbeg, *tpend = tp + len, attr;
  QChar str[MAXLPAC], *p;
  for (tpbeg = tp; tp < tpend; tpbeg = tp) {  attr = vAttr(*tp);
    p = str;
    do { *p++ = vChar(*tp++); } while (tp < tpend && vAttr(*tp) == attr);
    len = tp - tpbeg;
    Text(dc, x, y, attr, QString(str, len)); x += len;
} }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiScTwin::Repaint (int x, int y, int width, int height, bool NOW)
{
  if (NOW) repaint(Tx2qtX(x), Ty2qtY(y), Tw2qtW(width), Th2qtH(height));
  else     update (Tx2qtX(x), Ty2qtY(y), Tw2qtW(width), Th2qtH(height));
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiScTwin::Scroll (int srcx, int ty, int width, int height, int dy)
{
// Unfortunately, when "info" window intersects with scrolling rectangle (which
// will be "always" -- as otherwise this function is not used), Qt 4.x does not
// behave very good. It either forces redraw of everything or leaves ugly marks
// (depending on scrolling direction).  To work around, adjust scrolling region
// to exclude last two lines (and invalidate exclusion to be repainted over).
//
  QRect rect(Tx2qtX(srcx), Ty2qtY(ty+dy), Tw2qtW(width), Th2qtH(height-2));
  scroll (0, Th2qtH(dy), rect);
  Repaint(srcx, ty+height+dy-2, width, 2, false);
}
//-----------------------------------------------------------------------------
void MiScTwin::keyPressEvent (QKeyEvent *event)
{
  QString text = event->text();
  int key      = event->key() | int(event->modifiers() &  Qt::KeypadModifier);
  int modMask  = ((event->modifiers() & Qt::ShiftModifier) ? mod_SHIFT : 0)|
#ifdef Q_OS_MAC
                 ((event->modifiers() & Qt::MetaModifier)    ? mod_CTRL : 0)|
                 ((event->modifiers() & Qt::ControlModifier) ? mod_META : 0)|
#else
                 ((event->modifiers() & Qt::ControlModifier) ? mod_CTRL : 0)|
                 ((event->modifiers() & Qt::MetaModifier)    ? mod_META : 0)|
#endif
                 ((event->modifiers() & Qt::AltModifier) ? mod_ALT : 0);
  if (MiApp_debugKB)
    fprintf(stderr, "OnKey(%x:%s)mod=%x,native=%x:%x:%x", key,
       text.cStr(), modMask >> 24, event->nativeScanCode(),
                                   event->nativeModifiers(),
                                   event->nativeVirtualKey());
  if (vipOSmode) {
    switch (key) {
    case Qt::Key_Escape:    setkbhin   (4); return; // ^D = end-of-file
    case Qt::Key_Return:    setkbhin('\n'); return;
    case Qt::Key_Backspace: setkbhin('\b'); return;
    }
    if ((modMask & mod_CTRL) && 0x40 <= key && key < 0x60) setkbhin(key-0x40);
    else
      for (int i=0; i<text.length(); i++) setkbhin(text.at(i).toAscii());
  }
  else {    
    micom *mk = Mk_IsSHIFT(key) ? NULL : key2mimCmd(key | modMask);
    if (MiApp_debugKB) fprintf(stderr, ",micom=%x\n", mk ? mk->ev : 0);
    if ((mk != NULL && mk->ev == TK_NONE) ||
        (mk == NULL && text.isEmpty() )) event->ignore();
    else {                stopTimer();   event->accept();  setkbhin(0);
      //
      // Here we have either valid microMir command (mk != NULL), or non-empty
      // text (otherwise) that should be fed char-by-char into same function:
      //
      if (mk) vipOnKeyCode(vp, mk->ev, mk->attr);
      else {
        for (int i=0; i<text.length(); i++) {
          int k = text.at(i).unicode();
          if (Mk_IsCHAR(k)) vipOnKeyCode(vp, k, KxSEL);
      } }
      vipReady();
} } }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MiScTwin::timerEvent(QTimerEvent *)
{
  if (vipOnMimCmd(vp, cmd2repeat, 0)) { stopTimer(); vipReady(); }
  else {
    info.updateInfo(MitLINE_BLOCK);
    QApplication::syncX();   // only for X11 (does nothing on other platforms)
} }
void MiScTwin::repeatCmd (int kcode) { cmd2repeat = kcode;
                                       timerID = startTimer(10); }
void MiScTwin::stopTimer()
{
  if (cmd2repeat) { cmd2repeat = 0; killTimer(timerID); }
}
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
