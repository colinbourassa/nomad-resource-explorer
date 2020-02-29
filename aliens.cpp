#include <QByteArray>
#include "aliens.h"
#include "imageconverter.h"

const QVector<QString> Aliens::s_animationMap =
{
  "00000",
  "ALT01", "ALT01", "ALT01", "ALT01", "ALT01", "ALT01", "ALT01", "ALT01",
  "ALT01", "ALT01",
  "ARD20", "ARD15", "ARD11", "ARD26", "ARD25", "ARD30", "ARD33", "ARD36",
  "ARD35", "ARD43", "ARD40", "ARD23", "ARD24", "ARD18", "ARD10", "ARD16",
  "ARD13", "ARD39", "ARD22", "ARD31", "ARD37", "ARD17", "ARD12", "ARD14",
  "ARD45", "ARD19", "ARD27", "ARD42", "ARD29", "ARD34", "ARD38", "ARD44",
  "BEL03", "BEL02", "BEL16", "BEL17", "BEL18", "BEL17", "BEL05", "BEL19",
  "BEL20", "BEL09", "BEL10", "BEL12", "BEL14", "BEL21", "BEL01", "BEL07",
  "BEL08", "BEL13", "BEL08", "BEL08", "BEL08", "BEL08", "BEL06", "BEL06",
  "BEL06",
  "CHA15", "CHA07", "CHA08", "CHA29", "CHA23", "CHA28", "CHA10", "CHA11",
  "CHA16", "CHA20", "CHA24", "CHA25", "CHA21", "CHA22", "CHA01", "CHA03",
  "CHA00", "CHA17", "CHA06", "CHA05", "CHA09", "CHA27", "CHA30",
  "KEN00",
  "KOR06", "KOR02", "KOR04", "KOR03", "KOR04", "KOR00",
  "MUS30", "MUS08", "MUS07", "MUS09", "MUS30", "MUS25", "MUS11", "MUS01",
  "MUS18", "MUS21", "MUS22", "MUS20", "MUS00", "MUS10", "MUS16", "MUS27",
  "MUS30", "MUS03", "MUS06", "MUS15", "MUS29", "MUS12",
  "PAH00", "PAH04", "PAH06", "PAH08", "PAH02", "PAH07", "PAH09", "PAH01",
  "PAH08", "PAH04", "PAH07", "PAH02", "PAH02", "PAH05", "PAH00", "PAH03",
  "PAH08", "PAH06", "PAH10",
  "PHE10", "PHE04", "PHE08", "PHE31", "PHE27", "PHE13", "PHE33", "PHE21",
  "PHE28", "PHE19", "PHE06", "PHE15", "PHE29", "PHE05", "PHE11", "PHE00",
  "PHE22", "PHE12", "PHE01", "PHE09", "PHE20", "PHE26", "PHE07", "PHE34",
  "PHE35", "PHE02",
  "SHA04", "SHA09", "SHA04", "SHA04", "SHA04", "SHA04", "SHA02", "SHA06",
  "SHA07", "SHA10", "SHA00", "SHA01", "SHA05", "SHA03", "SHA08",
  "URS00", "URS17", "URS03", "URS02", "URS23", "URS28", "URS12", "URS01",
  "URS11", "URS09", "URS27", "URS24", "URS11", "URS05", "URS06", "URS07",
  "URS15", "URS08", "URS18", "URS25", "URS26", "URS22", "URS21", "URS10",
  "URS13", "URS16"
};

Aliens::Aliens(DatLibrary& lib, Palette& pal) :
  m_lib(&lib),
  m_pal(&pal)
{

}

void Aliens::clear()
{
  m_alienList.clear();
}

QMap<int,Alien> Aliens::getAlienList()
{
  if (m_alienList.isEmpty())
  {
    populateAlienList();
  }

  return m_alienList;
}

bool Aliens::getAlien(int id, Alien& alien)
{
  bool status = false;

  if (m_alienList.isEmpty())
  {
    populateAlienList();
  }

  if (m_alienList.contains(id))
  {
    status = true;
    alien = m_alienList[id];
  }

  return status;
}

QString Aliens::getName(int id)
{
  QString name("");

  if (m_alienList.isEmpty())
  {
    populateAlienList();
  }

  if (m_alienList.contains(id))
  {
    name = m_alienList[id].name;
  }

  return name;
}

bool Aliens::populateAlienList()
{
  QByteArray aliendata;
  bool status = false;

  if (m_lib->getFileByName(DatFileType_CONVERSE, "ALIEN.TAB", aliendata))
  {
    status = true;
    const uint8_t* rawdata = reinterpret_cast<const uint8_t*>(aliendata.data());
    unsigned int offset = 0;
    int id = 0;

    while (offset <= (aliendata.size() - sizeof(AlienTableEntry)))
    {
      const AlienTableEntry* currentEntry = reinterpret_cast<const AlienTableEntry*>(rawdata + offset);

      // the object is only valid if the name offset into GAMETEXT.TXT is not 0xFFFF
      if (currentEntry->nameOffset != 0xFFFF)
      {
        Alien a;
        a.id = id;
        a.name = m_lib->getGameText(currentEntry->nameOffset);
        a.race = static_cast<AlienRace>(currentEntry->race);

        if (!a.name.isEmpty())
        {
          m_alienList.insert(id, a);
        }
      }
      offset += sizeof(AlienTableEntry);
      id++;
    }
  }

  return status;
}

bool Aliens::getAnimationFrames(int alienId, QMap<int, QImage>& frames)
{
  bool status = true;

  if ((alienId > 0) && (alienId < s_animationMap.size()))
  {
    // first, determine the filename of the ANM and attempt to open it
    const QString anmFilename = QString("%1.ANM").arg(s_animationMap[alienId]);
    QByteArray anmFileData;

    if (m_lib->getFileByName(DatFileType_ANIM, anmFilename, anmFileData))
    {
      // the ASCII string for the palette filename begins at offset 00 in the ANM file
      const QString palFilename = QString::fromLocal8Bit(anmFileData.data());
      QVector<QRgb> pal;

      if (m_pal->paletteByName(DatFileType_ANIM, palFilename, pal))
      {
        // get a list of frame numbers, and the list of composite sections (.del files) used to build each frame
        const QMap<int, QVector<int>> frameList = getListOfFrames(anmFileData);

        // the prefix used on the .del files is the first two letters of the ANM filename, but in lowercase
        const QString delFilenamePrefix = anmFilename.mid(0, 2).toLower();

        foreach (int frameNum, frameList.keys())
        {
          if (status)
          {
            QImage frame;
            if (buildFrame(frameList[frameNum], delFilenamePrefix, pal, frame))
            {
              frames.insert(frameNum, frame);
            }
            else
            {
              status = false;
            }
          }
        }
      }
    }
  }

  return status;
}

bool Aliens::buildFrame(QVector<int> delIdList, QString delFilenamePrefix, const QVector<QRgb> pal, QImage& frame) const
{
  bool status = true;
  QByteArray delFileData;

  foreach (int delNumber, delIdList)
  {
    const QString delFilename = delFilenamePrefix + QString("%1.del").arg(delNumber, 4, 10, QChar('0'));

    if (status && m_lib->getFileByName(DatFileType_ANIM, delFilename, delFileData))
    {
      if (!ImageConverter::delToPixmap(delFileData, pal, frame))
      {
        status = false;
      }
    }
  }

  return status;
}

QMap<int, QVector<int> > Aliens::getListOfFrames(const QByteArray& anmData) const
{
  QMap<int, QVector<int> > frames;

  int anmFrameRecordIndex = 0;
  int anmFrameRecordOffset = (anmFrameRecordIndex * ANM_RECORD_SIZE_BYTES) + ANM_FIRST_RECORD_OFFSET;
  const uint8_t* anmDataPtr = reinterpret_cast<const uint8_t*>(anmData.data());

  // keep going until we encounter an unused record (which will start with 0xFF)
  // or we reach the end of the section (after 64 records)
  while ((anmDataPtr[anmFrameRecordOffset] != 0xFF) && (anmFrameRecordIndex < 64))
  {
    QVector<int> overlayNums;

    int offsetWithinRecord = 0;
    uint8_t byte = anmDataPtr[anmFrameRecordOffset + offsetWithinRecord];
    while ((byte != 0xFF) && (byte != 0x00))
    {
      overlayNums.append(byte);
      offsetWithinRecord++;
      byte = (offsetWithinRecord < ANM_RECORD_SIZE_BYTES) ?
             anmDataPtr[anmFrameRecordOffset + offsetWithinRecord] : 0xFF;
    }

    if (!overlayNums.isEmpty())
    {
      // the list of composite sections for this single frame is complete, so
      // add it to the list of frames (indexed by the same index as the record
      // in the ANM file)
      frames.insert(anmFrameRecordIndex, overlayNums);
    }

    // advance offset to the start of the next record
    anmFrameRecordIndex++;
    anmFrameRecordOffset = (anmFrameRecordIndex * ANM_RECORD_SIZE_BYTES) + ANM_FIRST_RECORD_OFFSET;
  }

  return frames;
}
