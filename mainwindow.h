#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QGraphicsScene>
#include <QTreeWidgetItem>
#include <QByteArray>
#include <QBuffer>
#include <QAudio>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QListWidgetItem>
#include <QProgressBar>
#include "datlibrary.h"
#include "invobject.h"
#include "palette.h"
#include "places.h"
#include "placeclasses.h"
#include "aliens.h"
#include "ships.h"
#include "shipclasses.h"
#include "shipinventory.h"
#include "facts.h"
#include "audio.h"
#include "fullscreenimages.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QString gameDir, QWidget* parent = nullptr);
    ~MainWindow();

public slots:
  void onAudioStateChanged(QAudio::State);

private slots:
  void on_actionOpen_game_data_dir_triggered();
  void onExit();
  void onCloseDataFiles();
  void on_m_objTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
  void on_m_placeTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
  void on_m_alienTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
  void on_m_alienFrameSlider_valueChanged(int value);
  void on_m_soundTree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
  void on_m_soundPrevButton_clicked();
  void on_m_soundPlayButton_clicked();
  void on_m_soundNextButton_clicked();
  void on_m_soundStopButton_clicked();
  void on_m_shipTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
  void on_m_factTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
  void on_m_fullscreenTree_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

private:
  Ui::MainWindow *ui;

  QString m_gamedir;

  DatLibrary m_lib;
  InvObject m_invObject;
  Places m_places;
  Palette m_palette;
  PlaceClasses m_pclasses;
  Aliens m_aliens;
  Ships m_ships;
  ShipClasses m_shipClasses;
  ShipInventory m_inventory;
  Facts m_facts;
  Audio m_audio;
  FullscreenImages m_fullscreenImages;

  QMap<int,QImage> m_alienFrames;

  QGraphicsScene m_objScene;
  QGraphicsScene m_planetSurfaceScene;
  QGraphicsScene m_alienScene;
  QGraphicsScene m_fullscreenScene;

  int m_currentNNVSoundCount;
  int m_currentNNVSoundId;
  QString m_currentNNVFilename;
  DatFileType m_currentSoundDat;
  QAudioFormat m_audioFormat;
  QByteArray m_audioPcmData;
  QAudioOutput* m_audioOutput;
  QBuffer m_audioBuffer;

  QMap<AlienRace,QProgressBar*> m_factProgressBars;

  void clearData();
  void openNewData(const QString gameDir);
  void setupAudio();
  void populatePlaceWidgets();
  void populateObjectWidgets();
  void populateAlienWidgets();
  void populateShipWidgets();
  void populateAudioWidgets();
  void populateFullscreenLbmWidgets();
  void populateFactWidgets();
  void loadAlienFrame(int frameId);

  void setSoundButtonStates();
  void setSoundIDLabel(QString nnvName, int soundId);
  void setAudioStateLabel(QAudio::State state);
};

#endif // MAINWINDOW_H
