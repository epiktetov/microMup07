/*------------------------------------------------------+----------------------
// МикроМир07           Qt-based File System            | (c) Epi MG, 2007
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
#ifdef Q_OS_LINUX
#  define QfsIGNORE_CASE Qt::CaseSensitive
#else
#  define QfsIGNORE_CASE Qt::CaseInsensitive
#endif
enum QfsFtype { QftPSEUDO = 0, QftTEXT = 1, QftDIR = 2 };
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
struct qfile_tag        /* mic.h defines 'typedef struct qfile_tag == qfile' */
{
  enum QfsFtype ft;
#ifndef __cplusplus
};
#else
  QString  full_name;         /* Каноническое имя файла (with absolute path) */
  QString path, name;         /* Отдельно path и собственно имя файла        */
  qint64        size;         /* Размер файла в байтах                       */
  QDateTime  updated;         /* Время последней модификации                 */
  bool      writable;
};
struct qtxtfile : public qfile_tag { QFile Qf; };
struct qdirfile : public qfile_tag { QDir  QD; };

qfile *QfsDup (qfile *file);
QDir  *QfsDir (qfile *file);
QFile *QfsFile(qfile *file);
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
large QfsRead (qfile *file, large size, char *buffer);
large QfsWrite(qfile *file, large size, char *buffer);

void QfsClear     (qfile *file);
int QfsFullName2tc(qfile *file, tchar *buffer);
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#ifdef __cplusplus
}
#endif
/*---------------------------------------------------------------------------*/
#endif                                                     /* QFS_H_INCLUDED */
