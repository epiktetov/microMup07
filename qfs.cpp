//------------------------------------------------------+----------------------
// МикроМир07           Qt-based File System            | (c) Epi MG, 2007,2014
//------------------------------------------------------+----------------------
#include <QString>
#include "mic.h"
#include "qfs.h"
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
qfile *QfsNew (QString filename, qfile *referer)
{
  QfsFtype new_ft = QftNIL; qfile *file;
  QString wildcards;
  if (filename == QfsELLIPSIS || filename == QfsEMPTY_NAME) new_ft = QftNOFILE;
  else if (filename.startsWith(QfsXEQ_PREFIX)) {            new_ft = QftPSEUDO;
    if (!filename.endsWith(QString::fromUtf8("»")))
         filename.append  (QString::fromUtf8("»"));
  }
  if (new_ft) {
    QString pseudo_path = referer ? referer->path : QDir::currentPath();
    file = new qfile_tag;
    file->size = 0;        file->ft = new_ft;
    file->name = filename; file->path = pseudo_path;
    file->writable = true;
    file->full_name = pseudo_path+"/"+file->name; return file;
  }
  if (filename.startsWith('~')) filename.replace(0,1, QDir::homePath());
  if (filename.contains("*") ||
      filename.contains("?")) {
    int Ndir = filename.lastIndexOf("/"); wildcards = filename.mid(Ndir+1);
    if (Ndir < 0)
         filename =                       QString::fromUtf8("./…");
    else filename = filename.left(Ndir) + QString::fromUtf8( "/…");
  }
  QFileInfo Qi(filename);
//
// Convert path to absolute, using referer directory if available (otherwise
// working directory will be used, which is, most likely, the last one where
// some shell command was executed):
//
  if (Qi.isRelative()) {
    if (referer && QDir::currentPath() != referer->path) {
      QDir::setCurrent(referer->path);
      Qi.setFile(filename); // reparse the name relative to referer's directory
    }
    Qi.makeAbsolute(); filename = Qi.filePath();
  }
// Some intelligence here: if filename refer to directory with micros.dir file,
// use that file, otherwise assume user want to see dirlist:
//
  if (Qi.isDir() && wildcards.isEmpty()) {
    QFileInfo Qmicros(filename+"/" QfsROOTFILE);
    if (Qmicros.exists()) filename = Qmicros.canonicalFilePath();
    else {
      wildcards = QString::fromAscii("*");
      filename.push_back(QString::fromUtf8("/…"));
  } }
  Qi.setFile(filename); // - - - - - - - - - filename processed, creating qfile
  if (wildcards.isEmpty() && !Qi.isDir()) {
    qtxtfile *tfile = new qtxtfile;
    tfile->ft = QftTEXT;
    tfile->name = Qi.fileName(); tfile->Qf.setFileName(filename); file = tfile;
  }
  else {
    qdirfile *dfile = new qdirfile;
    dfile->ft = QftDIR;
    dfile->name = wildcards; dfile->QD.setPath(Qi.path()); file = dfile;
  }
  file->updated  = Qi.lastModified(); file->size =  Qi.size();
  file->writable = QfsIsWritable(Qi); file->path =  Qi.path();
  file->full_name = Qi.path() + "/" + file->name; return file;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
qfile *QfsDup (qfile *base)
{
  qdirfile *dfile; qfile *qf;
  qtxtfile *tfile;
  switch (base->ft) {
  case QftNOFILE:
  case QftPSEUDO:
  default:  qf = new qfile_tag; break;
  case QftTEXT:
    qf = tfile = new qtxtfile; tfile->Qf.setFileName(base->full_name);
    break;
  case QftDIR:
    qf = dfile = new qdirfile; dfile->QD.setPath(base->path);
    break;
  }
  qf->name = base->name; qf->full_name = base->full_name; qf->ft = base->ft;
  qf->path = base->path; qf->updated   = base->updated;   qf->lf = base->lf;
  qf->size = base->size; qf->writable  = base->writable;          return qf;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
QDir *QfsDir (qfile *file)
{                            
  return (file->ft == QftDIR) ? &((qdirfile*)(file))->QD : NULL;
}
QFile *QfsFile (qfile *file)
{
  return (file->ft == QftTEXT) ? &((qtxtfile*)(file))->Qf : NULL;
}
/*---------------------------------------------------------------------------*/
const QIODevice::OpenMode QfsMode[4] = 
{ 
  QIODevice::ReadOnly,                     // FO_READ   0  /* Read  only     */
  QIODevice::WriteOnly,                    // FO_WRITE  1  /* Write only     */
  QIODevice::WriteOnly|QIODevice::Truncate // FO_NEW    2  /* New file/write */
};
int QfsOpen (qfile *file, int open_mode)
{
  if (file->ft == QftTEXT) {
    qtxtfile *tfile = (qtxtfile *)file;
    QfsUpdateInfo(file);
    if (open_mode == FO_READ && !tfile->Qf.exists()) return 0;
    else   return tfile->Qf.open(QfsMode[open_mode]) ? 1 : -1;
  } else   return                                          -1;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void QfsClose (qfile *file)
{
  if (file->ft == QftTEXT) ((qtxtfile*)(file))->Qf.close();
}
void QfsDelete (qfile *file)
{
  if (file->ft == QftTEXT) ((qtxtfile*)(file))->Qf.remove();
}
bool QfsRename (qfile *file, QString to_name)
{
  if (file->ft == QftTEXT) return ((qtxtfile*)file)->Qf.rename(to_name);
  else                     return false;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
long QfsRead (qfile *file, long size, char *buffer)
{
  if (file->ft == QftTEXT) return ((qtxtfile*)(file))->Qf.read(buffer, size);
  else                     return -1;
}
long QfsWrite (qfile *file, long size, char *buffer)
{
  if (file->ft == QftTEXT) return ((qtxtfile*)(file))->Qf.write(buffer, size);
  else                     return -1;
}
/*---------------------------------------------------------------------------*/
bool QfsIsWritable (QFileInfo Qi)     /* no file => check if dir is writable */
{
       if ( Qi      .exists()) return Qi.isWritable();
  else if (!Qi.dir().exists()) return false;
  else { 
    QFileInfo Qd(Qi.path());   return Qd.isWritable();
//+
//  fprintf(stderr, "Dir(%s)%s[%x]\n",                Qd.fileName().cStr(),
//-     Qd.isWritable() ? "writable" : "read-only", (int)Qd.permissions());
} }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
bool QfsIsUpToDate (qfile *file)  /* check if file is up-to-date and refresh */
{                                 /* file "writable" property regardless     */
  QFileInfo Qi(file->full_name);
  file->writable = QfsIsWritable(Qi);
  if (file->updated >= Qi.lastModified()) return true;
  file->size = Qi.size();
  file->updated = Qi.lastModified();     return false;
}
void QfsUpdateInfo(qfile *file) /*- - - - - - - - - - - - - - - - - - - - - -*/
{                                 
  QFileInfo Qi(file->full_name); file->size = Qi.size();
                                 file->updated = Qi.lastModified();
                                 file->writable = QfsIsWritable(Qi);
}
bool QfsSameAs(QString name, QString filt, qfile *qf)
{
  return (name.compare(qf->full_name, QfsIGNORE_CASE) == 0 && filt == qf->lf);
}
void QfsChDir (qfile  *file) { QDir::setCurrent(file->path);           }
bool QfsExists(QString name) { QFileInfo Qi(name); return Qi.exists(); }
/*---------------------------------------------------------------------------*/
QString QfsShorten (QString name_to_shorten)
{
  QStringList dirs = name_to_shorten.split('/'); QString name, part;
  QStringListIterator it(dirs);     it.toBack();
  while (it.hasPrevious()) {
    part = it.previous();
         if (name.isEmpty()) name = part;
    else if (name.length() + part.length() > QfsSHORT_NAME_MAX) break;
    else     name.push_front(part + QChar('/'));
  }
  return name;
}
QString QfsShortName    (qfile *file) { return QfsShorten(file->full_name); }
QString QfsShortDirName (qfile *file) { return QfsShorten(file->path);      }
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
QString QfsModDateText (QFileInfo Qi)
{
  QDateTime dt = Qi.lastModified();
  QString text;
  if (dt.daysTo(QDateTime::currentDateTime()) > 300)
       text = dt.toString("MMM dd, yyyy");
  else text = dt.toString("MMM dd hh:mm");
  text.push_front(" ");
  text.push_front(QDate::shortDayName(dt.date().dayOfWeek())); return text;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void QfsClear     (qfile *pfile)       { if (pfile) delete pfile;          }
int QfsFullName2tc(qfile *f, tchar *p) { return qstr2tcs(f->full_name, p); }
/*---------------------------------------------------------------------------*/
