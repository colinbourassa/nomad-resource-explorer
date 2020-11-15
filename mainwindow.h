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
#include <QTableWidget>
#include <QTimer>
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
#include "missions.h"

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
  void onTimer();
  void on_m_objTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
  void on_m_placeTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
  void on_m_alienTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
  void on_m_alienFrameSlider_valueChanged(int value);
  void on_m_soundTree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
  void on_m_soundPrevButton_clicked();
  void on_m_soundPlayButton_clicked();
  void on_m_soundNextButton_clicked();
  void on_m_soundStopButton_clicked();
  void on_m_soundMakeWav_clicked();
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
  void on_m_convTopicButtonSeesItem_clicked();
  void on_m_stampTree_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
  void on_m_stampRollSlider_sliderMoved(int position);
  void on_actionAbout_triggered();
  void on_m_convFilterTopicsCheckbox_stateChanged(int arg1);
  void on_m_convDialogueLine_anchorClicked(const QUrl &arg1);
  void on_m_missionIdSpinBox_valueChanged(int arg1);
  void on_m_missionStartText_anchorClicked(const QUrl &arg1);
  void on_m_missionEndText_anchorClicked(const QUrl &arg1);
  void on_m_3dModelTree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
  void on_m_3dSpinXButton_toggled(bool checked);
  void on_m_3dSpinYButton_toggled(bool checked);
  void on_m_3dSpinZButton_toggled(bool checked);
  void timerControl();
  void reset3DView();
  void on_m_3dResetButton_clicked();
  void on_m_paletteTree_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

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
  Missions m_missions;

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

  ConvTopicCategory m_currentConvTopic;
  QString m_currentConvLine;

  QMap<PlanetResourceType,QMap<int,QLabel*> > m_resourceLabels;
  QTimer m_timer;

  void clearData();
  void openNewData(const QString gameDir);
  void connectGLViewerSliders();
  void setupAudio();
  void setupTimer();
  void populatePlaceWidgets();
  void populateObjectWidgets();
  void populateAlienWidgets();
  void populateShipWidgets();
  void populateAudioWidgets();
  void populateFullscreenLbmWidgets();
  void populateStampWidgets();
  void populateFactWidgets();
  void populateConversationWidgets();
  void populateMissionWidgets();
  void populate3dModelWidgets();
  void populatePaletteWidgets();
  void loadAlienFrame(int frameId);
  void populateConversationTopicTable(int lastSelectedTopicId = -1);
  void populateTopicTableForCategory(ConvTopicCategory category, QMap<int,QString> topicList, int lastSelectedTopicId);
  void getConversationLinesForCurrentTopic();
  QString getNameForGameTextCommandParameter(GTxtCmd cmd, int param);
  void clearDialogLineAndCommandList();
  void putResourceLabelsInArray();
  void clearAllResourceLabels();
  void clearPlaceLabels();
  void setSoundButtonStates();
  void setSoundIDLabel(QString nnvName, int soundId);
  void setAudioStateLabel(QAudio::State state);
  void displayStamp(int rollIndex);
  void showInfoForMission(int id);
  void showAnchorTooltip(const QUrl& url);
  void populateGameTextCommandList(QTableWidget* table, QVector<QPair<GTxtCmd,int> >& commands);
};

#endif // MAINWINDOW_H
