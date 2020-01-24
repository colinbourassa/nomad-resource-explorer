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

#define LZ_RINGBUF_SIZE 0x1000

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
  char    filename[14]; // includes null terminator
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

  bool getFileByName(DatFileType dat, QString filename, QByteArray& filedata);
  QString getGameText(int offset);
  QPixmap convertStpToPixmap(QByteArray& stpData, QVector<QRgb> palette, bool& status);
  QVector<QRgb> getGamePalette();

private:
  QByteArray m_datContents[DatFileType_NUM_DAT_FILES];
  uint8_t m_lzRingBuffer[LZ_RINGBUF_SIZE];

  QVector<QRgb> m_gamePalette;
  QByteArray m_gameText; // keep a copy of GAMETEXT.TXT since it is referenced frequently

  bool loadGamePalette();
  bool lzDecompress(QByteArray compressedfile, QByteArray& decompressedFile);
  bool getFileAtIndex(DatFileType dat, unsigned int index, QByteArray& decompressedFile);
  QPoint getPixelLocation(int imgWidth, int pixelnum) const;
};

#endif // DATLIBRARY_H

