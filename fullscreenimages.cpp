#include "fullscreenimages.h"
#include "imageconverter.h"

///
/// Dictionary of .lbm images and their associated palette files.
/// Note that this dictionary only contains files whose palette
/// filename does not match the .lbm.
///
const QMap<QString,QString> FullscreenImages::s_lbmToPal =
{
  { "cock1.lbm",    "backg.pal"    },
  { "cock2.lbm",    "backg.pal"    },
  { "cock3.lbm",    "backg.pal"    },
  { "cock4.lbm",    "backg.pal"    },
  { "cock5.lbm",    "backg.pal"    },
  { "crashed.lbm",  "backg.pal"    },
  { "end1a.lbm",    "backg.pal"    },
  { "end1b.lbm",    "backg.pal"    },
  { "fixed.lbm",    "backg.pal"    },
  { "cred0002.lbm", "cred0001.pal" },
  { "cred0003.lbm", "cred0001.pal" },
  { "cred0004.lbm", "cred0001.pal" },
  { "cred0005.lbm", "cred0001.pal" },
  { "cred0006.lbm", "cred0001.pal" },
  { "oesi.lbm",     "backg.pal"    },
  { "snow.lbm",     "backg.pal"    },
  { "credit5.lbm",  "credit4.pal"  },
  { "credit6.lbm",  "credit4.pal"  },
  { "credit7.lbm",  "credit4.pal"  }
};

const QString FullscreenImages::s_lbmExtension(".lbm");
const QString FullscreenImages::s_palExtension(".pal");

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
    const QStringList lbmsInDat = m_lib->getFilenamesByExtension(datType, s_lbmExtension);
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
    bool foundPal = false;

    // position in string of the filename's extension (including the dot)
    const int lbmExtPos = lbmFilename.length() - 4;

    // if the provided filename ends with ".LBM" (or ".lbm") as expected,
    // then first try to use a palette with the same filename base but with a .pal extension
    if ((lbmFilename.length() >= 5) && // minimum length of an 8.3 filename
        (lbmFilename.mid(lbmExtPos, 4).toLower() == s_lbmExtension))
    {
      QString palFilename = lbmFilename;
      palFilename.replace(lbmExtPos, 4, s_palExtension);
      foundPal = m_pal->paletteByName(dat, palFilename, pal);
    }

    // otherwise, fall back to the the dictionary of LBM/PAL pairings
    if (!foundPal && s_lbmToPal.contains(lbmFilename))
    {
      foundPal = m_pal->paletteByName(dat, s_lbmToPal[lbmFilename], pal);
    }

    if (foundPal)
    {
      status = ImageConverter::lbmToImage(lbmData, pal, img);
    }
  }

  return status;
}
