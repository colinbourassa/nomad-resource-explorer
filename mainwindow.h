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
#include <QLabel>
#include "aboutbox.h"
#include "datlibrary.h"
#include "gametext.h"
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
#include "stampimages.h"
#include "conversationtext.h"

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
  void on_m_convAlienTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
  void on_m_convTopicButtonPerson_clicked();
  void on_m_convTopicButtonPlace_clicked();
  void on_m_convTopicButtonObject_clicked();
  void on_m_convTopicButtonRace_clicked();
  void on_m_convTopicTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
  void on_m_convTopicButtonGreeting0_clicked();
  void on_m_convTopicButtonGreeting1_clicked();
  void on_m_convTopicButtonDispObj_clicked();
  void on_m_convTopicButtonGiveObj_clicked();
  void on_m_convTopicButtonGiveFact_clicked();
  void on_m_convNextButton_clicked();
  void on_m_convPrevButton_clicked();
  void on_m_convTopicButtonSeesItem_clicked();
  void on_m_stampTree_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
  void on_m_stampRollSlider_sliderMoved(int position);

  void on_actionAbout_triggered();

private:
  Ui::MainWindow *ui;
  AboutBox* m_aboutBox;

  QString m_gamedir;

  DatLibrary m_lib;
  GameText m_gametext;
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
  StampImages m_stamps;
  ConversationText m_convText;

  QMap<int,QImage> m_alienFrames;
  QList<QImage> m_stampImages;

  QGraphicsScene m_objScene;
  QGraphicsScene m_planetSurfaceScene;
  QGraphicsScene m_alienScene;
  QGraphicsScene m_fullscreenScene;
  QGraphicsScene m_stampScene;

  int m_currentNNVSoundCount;
  int m_currentNNVSoundId;
  QString m_currentNNVFilename;
  DatFileType m_currentSoundDat;
  QAudioFormat m_audioFormat;
  QByteArray m_audioPcmData;
  QAudioOutput* m_audioOutput;
  QBuffer m_audioBuffer;

  ConvTopic m_currentConvTopic;
  QStringList m_currentConvLines;
  int m_currentConvLinesPos;

  QMap<PlanetResourceType,QMap<int,QLabel*> > m_resourceLabels;

  void clearData();
  void openNewData(const QString gameDir);
  void setupAudio();
  void populatePlaceWidgets();
  void populateObjectWidgets();
  void populateAlienWidgets();
  void populateShipWidgets();
  void populateAudioWidgets();
  void populateFullscreenLbmWidgets();
  void populateStampWidgets();
  void populateFactWidgets();
  void populateConversationWidgets();
  void loadAlienFrame(int frameId);

  void populateConversationTopicTable();
  void getConversationLinesForCurrentTopic();
  void displayCurrentConversationLine();
  void setConvNextPrevButtonState();

  void putResourceLabelsInArray();
  void clearAllResourceLabels();
  void clearPlaceLabels();

  void setSoundButtonStates();
  void setSoundIDLabel(QString nnvName, int soundId);
  void setAudioStateLabel(QAudio::State state);

  void displayStamp(int rollIndex);
};

#endif // MAINWINDOW_H
