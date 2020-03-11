#include "fullscreenimages.h"
#include "imageconverter.h"

const QMap<QString,QString> FullscreenImages::s_lbmToPal =
{
  { "backg.lbm", "backg.pal" },
  { "cock1.lbm", "backg.pal" },
  { "cock2.lbm", "backg.pal" },
  { "cock3.lbm", "backg.pal" },
  { "cock4.lbm", "backg.pal" },
  { "cock5.lbm", "backg.pal" },
  { "crashed.lbm", "backg.pal" },
  { "end1a.lbm", "backg.pal" },
  { "end1b.lbm", "backg.pal" },
  { "fixed.lbm", "backg.pal" },
  { "getname.lbm",  "getname.pal"  },
  { "cred0001.lbm", "cred0001.pal" },
  { "cred0002.lbm", "cred0001.pal" },
  { "cred0003.lbm", "cred0001.pal" },
  { "cred0004.lbm", "cred0001.pal" },
  { "cred0005.lbm", "cred0001.pal" },
  { "cred0006.lbm", "cred0001.pal" },
  { "korok01.lbm",  "korok01.pal"  },
  { "korok02.lbm",  "korok02.pal"  },
  { "open08.lbm",  "open08.pal"  },
  { "oesi.lbm", "backg.pal" },
  { "snow.lbm", "backg.pal" },
  { "win01.lbm",  "win01.pal"  },
  { "win03.lbm",  "win03.pal"  },
  { "win04.lbm",  "win04.pal"  },
};

FullscreenImages::FullscreenImages(DatLibrary& lib, Palette& pal) :
  m_lib(&lib),
  m_pal(&pal)
{

}

QMap<DatFileType,QStringList> FullscreenImages::getAllLbmList()
{
  QMap<DatFileType,QStringList> lbmContainerList;

  for (int datIdx = 0; datIdx < DatFileType_NUM_DAT_FILES; datIdx++)
  {
    const DatFileType datType = static_cast<DatFileType>(datIdx);
    const QStringList lbmsInDat = m_lib->getFilenamesByExtension(datType, ".LBM");
    lbmContainerList[datType] = lbmsInDat;
  }

  return lbmContainerList;
}

bool FullscreenImages::getImage(DatFileType dat, QString lbmFilename, QImage& img)
{
  bool status = false;

  QByteArray lbmData;

  if (m_lib->getFileByName(dat, lbmFilename, lbmData))
  {
    QVector<QRgb> pal;
    if (s_lbmToPal.contains(lbmFilename) && m_pal->paletteByName(dat, s_lbmToPal[lbmFilename], pal))
    {
      status = ImageConverter::lbmToImage(lbmData, pal, img);
    }
  }

  return status;
}
