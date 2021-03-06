/*------------------------------------------------------+----------------------
// МикроМир07           Qt-based File System            | (c) Epi MG, 2007-2018
//------------------------------------------------------+--------------------*/
#ifndef QFS_H_INCLUDED
#define QFS_H_INCLUDED
#ifdef __cplusplus
# include <QFileInfo>
# include <QDir>
# include <QDateTime>
#endif
#define QfsROOTFILE  "micros.dir"
#define QfsELLIPSIS   QString::fromUtf8("…")
#define QfsEMPTY_NAME QString::fromUtf8("÷")
#define QfsXEQ_PREFIX QString::fromUtf8("«")
#ifdef Q_OS_LINUX
#  define QfsIGNORE_CASE Qt::CaseSensitive
#else
#  define QfsIGNORE_CASE Qt::CaseInsensitive
#endif
enum QfsFtype { QftNIL=0, QftNOFILE=2,
                          QftPSEUDO=3, QftREAL=4, QftTEXT=4, QftDIR=5 };
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
struct qfile_tag        /* mic.h defines 'typedef struct qfile_tag == qfile' */
{
  enum QfsFtype ft;
#ifdef __cplusplus          /* Видно только в C++ коде:                      */
  QString  full_name;       /*   Каноническое имя файла (with absolute path) */
  QString path, name;       /*   Отдельно path и собственно имя файла        */
  qint64        size;       /*   Размер файла в байтах                       */
  QDateTime  updated;       /*   Время последней модификации                 */
  bool       utf16le;       /* (internal) file in UTF-16LE converted in read */
  bool      writable;
  QString         lf; /* Load Filter, index in MkLF[] = Lua method to filter */
#endif                /* the content of the file on load; empty => no filter */
};                    /* (set by tmDesc / used by tmDoLoad, not used in Qfs) */
#ifdef __cplusplus
qfile *QfsDup (qfile *file);    // qfile* pointers refer to either qtxtfile or
QDir  *QfsDir (qfile *file);    // qdirfile objects (inheritance for hidden C)
QFile *QfsFile(qfile *file);    //
struct qtxtfile : public qfile_tag { QFile Qf; };
struct qdirfile : public qfile_tag { QDir  QD; };
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
qfile *QfsNew (QString filename, qfile *referer); /* create file descriptor  */
int  QfsOpen  (qfile *file,       int open_mode); /* Open returns: 0 no file */
void QfsClose (qfile *file);                      /*               1 success */
void QfsDelete(qfile *file);                      /*        negative = error */
bool QfsRename(qfile *file, QString new_name);

/* modes for QfsOpen:                    */
#define FO_READ  0 /* Read  only         */
#define FO_WRITE 1 /* Write only         */
#define FO_NEW   2 /* New file for write */

bool QfsSameAs(QString fullName, QString loadFilt, qfile *file);
bool QfsExists(QString fullName);
bool QfsIsUpToDate (qfile *file);   
void QfsUpdateInfo (qfile *file);
void QfsChDir      (qfile *file);     /* change current dir to file location */
/*---------------------------------------------------------------------------*/
#define QfsSHORT_NAME_MAX 20    /* максимальная длина "короткого" имени окна */
QString QfsShortName   (qfile *file);
QString QfsShortDirName(qfile *file);
QString QfsModDateText(QFileInfo Qi);
bool    QfsIsWritable (QFileInfo Qi); // no file => check if dir is writable
extern "C" {
#endif                                                        /* __cplusplus */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
long QfsRead (qfile *file, long size, char *buffer);
long QfsWrite(qfile *file, long size, char *buffer);

void QfsClear     (qfile *file);
int QfsFullName2tc(qfile *file, tchar *buffer);
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#ifdef __cplusplus
}
#endif
/*---------------------------------------------------------------------------*/
#endif                                                     /* QFS_H_INCLUDED */
