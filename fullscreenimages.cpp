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
    QVector<QRgb> palData;

    if (!getPalette(dat, lbmFilename, palData).isEmpty())
    {
      status = ImageConverter::lbmToImage(lbmData, palData, img);
    }
  }

  return status;
}

/**
 * Gets the palette data for the specified LBM image. This function assumes that the palette
 * required for the image is stored in the same DAT archive. It first attempts to use the
 * same base filename as the LBM and appending the ".pal" extension; if this fails, it will
 * then attempt to look up the palette filename for the LBM in this class's static dictionary.
 * @return Palette filename is a valid palette is found; otherwise an empty string
 */
QString FullscreenImages::getPalette(DatFileType dat, QString lbmFilename, QVector<QRgb>& palData) const
{
  QString palFilename;

  // position in string of the filename's extension (including the dot)
  const int lbmExtPos = lbmFilename.length() - 4;

  // if the provided filename ends with ".LBM" (or ".lbm") as expected,
  // then first try to use a palette with the same filename base but with a .pal extension
  if ((lbmFilename.length() >= 5) && // minimum length of an 8.3 filename
      (lbmFilename.mid(lbmExtPos, 4).toLower() == s_lbmExtension))
  {
    QString testFilename = lbmFilename;
    testFilename.replace(lbmExtPos, 4, s_palExtension);
    if (m_pal->paletteByName(dat, testFilename, palData))
    {
      palFilename = testFilename;
    }
  }

  // otherwise, fall back to the the dictionary of LBM/PAL pairings
  if (palFilename.isEmpty() && s_lbmToPal.contains(lbmFilename))
  {
    if (m_pal->paletteByName(dat, s_lbmToPal[lbmFilename], palData))
    {
      palFilename = s_lbmToPal[lbmFilename];
    }
  }

  return palFilename;
}
