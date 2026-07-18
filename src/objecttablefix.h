#pragma once
#include <QByteArray>
#include <QString>
#include "datlibrary.h"

/**
 * Detects and repairs the known Korok/Kenelm value-swap bug in OBJECT.TAB.
 *
 * The original game data ships with the per-race relative-value bytes for the
 * Kenelm and the Korok swapped in every OBJECT.TAB record (see
 * https://colinbourassa.github.io/software/nomad,
 * "OBJECT.TAB - Inventory Object Table" section): based on the
 * storyline, the Korok should place value primarily on raw materials and
 * tactical equipment, but the data has exactly the opposite.
 *
 * Detection is exact: the decompressed OBJECT.TAB is hashed (SHA-256) and
 * compared against the fingerprint of the pristine retail file, so modded or
 * already-repaired data never triggers a false positive.
 *
 * The repair rewrites CONVERSE.DAT (after backing it up to CONVERSE.DAT.orig):
 * the corrected table is appended to the end of the container stored
 * uncompressed — a storage mode the game engine supports (and which
 * standalone_utils/dat_builder.c already relies on exclusively) — and the
 * OBJECT.TAB index entry is repointed at it. All other members and index
 * entries are untouched; the stale compressed copy simply becomes dead bytes.
 */
class ObjectTableFix
{
public:
  /**
   * Returns true if the OBJECT.TAB in the currently opened game data is
   * byte-identical to the known-buggy pristine retail file.
   */
  static bool isKnownBuggy(DatLibrary& lib);

  /**
   * Repairs the CONVERSE.DAT in the given game directory. The container must
   * hold the pristine OBJECT.TAB (call isKnownBuggy() first). Creates
   * CONVERSE.DAT.orig alongside it and refuses to run if that backup already
   * exists. On failure, returns false and sets errorMsg.
   */
  static bool apply(const QString& gameDir, QString& errorMsg);

private:
  static QByteArray fixedTable(const QByteArray& pristine);
  static QString findConverseDat(const QString& gameDir);
};
