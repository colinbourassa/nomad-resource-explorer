#ifndef DATLIBRARY_H
#define DATLIBRARY_H

#include <stdint.h>
#include <QByteArray>
#include <QString>
#include <QMap>
#include <QImage>
#include <QVector>
#include <QRgb>
#include <QPixmap>
#include <QStringList>

#define LZ_RINGBUF_SIZE 0x1000

#define DAT_FILENAME_ANIM     "ANIM.DAT"
#define DAT_FILENAME_CONVERSE "CONVERSE.DAT"
#define DAT_FILENAME_INVENT   "INVENT.DAT"
#define DAT_FILENAME_SAMPLES  "SAMPLES.DAT"
#define DAT_FILENAME_TEST     "TEST.DAT"

#define INDEX_FILENAME_LEN    14

enum DatFileType
{
  DatFileType_ANIM,
  DatFileType_CONVERSE,
  DatFileType_INVENT,
  DatFileType_SAMPLES,
  DatFileType_TEST,
  DatFileType_NUM_DAT_FILES,
  DatFileType_INVALID
};

// TODO: support struct packing in other compilers
typedef struct __attribute__((packed)) DatFileIndex
{
  uint8_t flags_a;
  uint8_t flags_b;
  int32_t uncompressed_size;
  int32_t compressed_size;
  char    filename[INDEX_FILENAME_LEN]; // includes null terminator
  uint32_t offset;
} DatFileIndex;

class DatLibrary
{
public:
  DatLibrary();
  virtual ~DatLibrary();
  bool openData(QString pathToGameDir);
  void closeData();

  static const QMap<DatFileType,QString> s_datFileNames;

  bool getFileByName(DatFileType dat, QString filename, QByteArray& filedata) const;
  QString getGameText(int offset);
  QStringList getFilenamesByExtension(DatFileType dat, QString extension);

private:
  QByteArray m_datContents[DatFileType_NUM_DAT_FILES];
  QByteArray m_gameText; // keep a copy of GAMETEXT.TXT since it is referenced frequently

  bool lzDecompress(QByteArray compressedfile, QByteArray& decompressedFile, int skipUncompressedBytes) const;
  bool getFileAtIndex(DatFileType dat, unsigned int index, QByteArray& decompressedFile) const;
};

#endif // DATLIBRARY_H

