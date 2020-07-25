#include "palette.h"

const QVector<QRgb> Palette::s_defaultVgaPalette =
{
  0xff000000, 0xff0000aa, 0xff00aa00, 0xff00aaaa, 0xffaa0000, 0xffaa00aa, 0xffaa5500, 0xffaaaaaa,
  0xff555555, 0xff5555ff, 0xff55ff55, 0xff55ffff, 0xffff5555, 0xffff55ff, 0xffffff55, 0xffffffff,
  0xff000000, 0xff141414, 0xff202020, 0xff2c2c2c, 0xff383838, 0xff454545, 0xff515151, 0xff616161,
  0xff717171, 0xff828282, 0xff929292, 0xffa2a2a2, 0xffb6b6b6, 0xffcbcbcb, 0xffe3e3e3, 0xffffffff,
  0xff0000ff, 0xff4100ff, 0xff7d00ff, 0xffbe00ff, 0xffff00ff, 0xffff00be, 0xffff007d, 0xffff0041,
  0xffff0000, 0xffff4100, 0xffff7d00, 0xffffbe00, 0xffffff00, 0xffbeff00, 0xff7dff00, 0xff41ff00,
  0xff00ff00, 0xff00ff41, 0xff00ff7d, 0xff00ffbe, 0xff00ffff, 0xff00beff, 0xff007dff, 0xff0041ff,
  0xff7d7dff, 0xff9e7dff, 0xffbe7dff, 0xffdf7dff, 0xffff7dff, 0xffff7ddf, 0xffff7dbe, 0xffff7d9e,
  0xffff7d7d, 0xffff9e7d, 0xffffbe7d, 0xffffdf7d, 0xffffff7d, 0xffdfff7d, 0xffbeff7d, 0xff9eff7d,
  0xff7dff7d, 0xff7dff9e, 0xff7dffbe, 0xff7dffdf, 0xff7dffff, 0xff7ddfff, 0xff7dbeff, 0xff7d9eff,
  0xffb6b6ff, 0xffc7b6ff, 0xffdbb6ff, 0xffebb6ff, 0xffffb6ff, 0xffffb6eb, 0xffffb6db, 0xffffb6c7,
  0xffffb6b6, 0xffffc7b6, 0xffffdbb6, 0xffffebb6, 0xffffffb6, 0xffebffb6, 0xffdbffb6, 0xffc7ffb6,
  0xffb6ffb6, 0xffb6ffc7, 0xffb6ffdb, 0xffb6ffeb, 0xffb6ffff, 0xffb6ebff, 0xffb6dbff, 0xffb6c7ff,
  0xff000071, 0xff1c0071, 0xff380071, 0xff550071, 0xff710071, 0xff710055, 0xff710038, 0xff71001c,
  0xff710000, 0xff711c00, 0xff713800, 0xff715500, 0xff717100, 0xff557100, 0xff387100, 0xff1c7100,
  0xff007100, 0xff00711c, 0xff007138, 0xff007155, 0xff007171, 0xff005571, 0xff003871, 0xff001c71,
  0xff383871, 0xff453871, 0xff553871, 0xff613871, 0xff713871, 0xff713861, 0xff713855, 0xff713845,
  0xff713838, 0xff714538, 0xff715538, 0xff716138, 0xff717138, 0xff617138, 0xff557138, 0xff457138,
  0xff387138, 0xff387145, 0xff387155, 0xff387161, 0xff387171, 0xff386171, 0xff385571, 0xff384571,
  0xff515171, 0xff595171, 0xff615171, 0xff695171, 0xff715171, 0xff715169, 0xff715161, 0xff715159,
  0xff715151, 0xff715951, 0xff716151, 0xff716951, 0xff717151, 0xff697151, 0xff617151, 0xff597151,
  0xff517151, 0xff517159, 0xff517161, 0xff517169, 0xff517171, 0xff516971, 0xff516171, 0xff515971,
  0xff000041, 0xff100041, 0xff200041, 0xff300041, 0xff410041, 0xff410030, 0xff410020, 0xff410010,
  0xff410000, 0xff411000, 0xff412000, 0xff413000, 0xff414100, 0xff304100, 0xff204100, 0xff104100,
  0xff004100, 0xff004110, 0xff004120, 0xff004130, 0xff004141, 0xff003041, 0xff002041, 0xff001041,
  0xff202041, 0xff282041, 0xff302041, 0xff382041, 0xff412041, 0xff412038, 0xff412030, 0xff412028,
  0xff412020, 0xff412820, 0xff413020, 0xff413820, 0xff414120, 0xff384120, 0xff304120, 0xff284120,
  0xff204120, 0xff204128, 0xff204130, 0xff204138, 0xff204141, 0xff203841, 0xff203041, 0xff202841,
  0xff2c2c41, 0xff302c41, 0xff342c41, 0xff3c2c41, 0xff412c41, 0xff412c3c, 0xff412c34, 0xff412c30,
  0xff412c2c, 0xff41302c, 0xff41342c, 0xff413c2c, 0xff41412c, 0xff3c412c, 0xff34412c, 0xff30412c,
  0xff2c412c, 0xff2c4130, 0xff2c4134, 0xff2c413c, 0xff2c4141, 0xff2c3c41, 0xff2c3441, 0xff2c3041,
  0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000
};

const QString Palette::s_gamePalFilename = "GAME.PAL";

/**
 * Loads and interprets Nomad's color palette files, filling in any missing entries
 * with default VGA palette data.
 */
Palette::Palette(DatLibrary& lib) :
  m_lib(&lib)
{

}

/**
 * Clears the cached GAME.PAL data so that it will be loaded again from the data file
 * when it is next requested.
 */
void Palette::clear()
{
  m_gamePal.clear();
}

/**
 * Loads a palette file with the specified name (and from the specified DAT container),
 * overlays it on the default VGA palette, and returns it in the provided vector.
 * @return True when loading the palette data was successful, false otherwise
 */
bool Palette::loadPalData(DatFileType datContainer, QString palFileName, QVector<QRgb>& palette, bool prefillWithDefaultVga) const
{
  bool status = false;
  QByteArray paldata;

  if (m_lib->getFileByName(datContainer, palFileName, paldata))
  {
    if (prefillWithDefaultVga)
    {
      // start with the default VGA palette, so that we can overlay the palette subset from the file
      palette = s_defaultVgaPalette;
    }
    else
    {
      palette.fill(QColor(0, 0, 0).rgb(), 256);
    }

    // Some palettes (such as GAME.PAL) have fewer than 256 colors,
    // but we'll want to fill in the remaining colors because some images
    // will index beyond this limited palette.
    //
    // The palette colors used by hardware of the period may have been
    // determined by the VGA BIOS, but since we don't have a VGA register
    // map that we can probe for the actual colors, we need to make a best
    // guess. The palette used by DOSBox should be fairly authentic.

    if (paldata.size() >= 3)
    {
      uint8_t startIndex = static_cast<uint8_t>(paldata[1]);
      int colorCount = paldata[2];

      if (colorCount == 0)
      {
        colorCount = 256;
      }

      if (paldata.size() >= (3 + 3 * (colorCount)))
      {
        for (int sourcePalIdx = 0; sourcePalIdx < colorCount; sourcePalIdx++)
        {
          const int sourcePalOffset = (sourcePalIdx + 1) * 3;

          // upconvert the 6-bit palette data to 8-bit by leftshifting and
          // replicating the two high bits in the low positions
          uint8_t rawbyteA = static_cast<uint8_t>(paldata[sourcePalOffset]);
          uint8_t rawbyteB = static_cast<uint8_t>(paldata[sourcePalOffset+1]);
          uint8_t rawbyteC = static_cast<uint8_t>(paldata[sourcePalOffset+2]);
          int r = (rawbyteA << 2) | (rawbyteA >> 4);
          int g = (rawbyteB << 2) | (rawbyteB >> 4);
          int b = (rawbyteC << 2) | (rawbyteC >> 4);

          palette[startIndex + sourcePalIdx] = qRgb(r,g,b);
        }

        status = true;
      }
    }
  }

  return status;
}

/**
 * Returns the default VGA palette in the provided vector.
 */
void Palette::defaultVgaPalette(QVector<QRgb>& palette)
{
  palette = s_defaultVgaPalette;
}

/**
 * Loads the file "GAME.PAL" from the game's data files, overlays it on the default VGA palette,
 * and returns it in the provided vector.
 * @return True when loading the palette data was successful, false otherwise
 */
bool Palette::gamePalette(QVector<QRgb>& palette)
{
  bool status = true;
  if (m_gamePal.size() == 0)
  {
    status = loadPalData(DatFileType_TEST, s_gamePalFilename, m_gamePal);
  }
  palette = m_gamePal;
  return status;
}

/**
 * Loads a palette file with the specified name (and from the specified DAT container),
 * overlays it on the default VGA palette, and returns it in the provided vector.
 * @return True when loading the palette data was successful, false otherwise
 */
bool Palette::paletteByName(DatFileType datContainer, QString palFileName, QVector<QRgb>& palette, bool prefillWithDefaultVga) const
{
  return loadPalData(datContainer, palFileName, palette, prefillWithDefaultVga);
}


/**
 * Gets a list of all *.PAL files across all of the DAT containers used by the game.
 */
QMap<DatFileType,QStringList> Palette::getAllPaletteList()
{
  QMap<DatFileType,QStringList> palContainerList;

  for (int datIdx = 0; datIdx < DatFileType_NUM_DAT_FILES; datIdx++)
  {
    const DatFileType datType = static_cast<DatFileType>(datIdx);
    QStringList palsInDat = m_lib->getFilenamesByExtension(datType, ".PAL");
    palContainerList[datType] = palsInDat;
  }

  return palContainerList;
}
