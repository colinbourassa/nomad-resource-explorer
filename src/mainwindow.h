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
#include <QMultiHash>
#include <QSet>
#include <QPair>
#include <QVector>
#include <QAction>
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

//! Tabbed entity kinds that navigateToEntity can jump to.
enum class EntityType { Alien, Place, Object, Ship, Fact, Race };

//! One entry in the back/forward navigation history: either an entity-tab
//! location or a conversation-tab location (link-jump destinations and the
//! locations jumps were made from). See navigateToEntity/navigateToConversation.
struct NavLocation
{
  enum class Kind { Entity, Conversation };
  Kind kind;
  EntityType entityType; // valid when kind == Entity
  int entityId;          // valid when kind == Entity
  ConversationRef conv;  // valid when kind == Conversation

  bool operator==(const NavLocation& other) const
  {
    if (kind != other.kind)
    {
      return false;
    }
    return (kind == Kind::Entity) ? ((entityType == other.entityType) && (entityId == other.entityId))
                                   : (conv == other.conv);
  }
};

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
  void on_m_alienTable_cellClicked(int row, int column);
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
  void on_m_stampRollSlider_valueChanged(int value);
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
  void on_m_factSources_itemDoubleClicked(QListWidgetItem* item);
  void on_m_factSources_itemClicked(QListWidgetItem* item);
  void on_m_shipTable_cellClicked(int row, int column);
  void on_m_shipInventoryTable_cellClicked(int row, int column);
  void on_m_convCommandList_cellClicked(int row, int column);
  void on_m_missionStartCommandList_cellClicked(int row, int column);
  void on_m_missionEndCommandList_cellClicked(int row, int column);
  void on_m_missionReqText_anchorClicked(const QUrl& arg1);
  void on_m_convTopicTable_customContextMenuRequested(const QPoint& pos);
  void on_m_objUsages_itemClicked(QListWidgetItem* item);
  void onEntityLinkActivated(const QString& link);
  void on_m_raceTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
  void on_m_raceMembers_itemClicked(QListWidgetItem* item);
  void on_m_racePlaces_itemClicked(QListWidgetItem* item);
  void on_m_raceFacts_itemClicked(QListWidgetItem* item);
  void on_m_raceObjValues_itemClicked(QListWidgetItem* item);
  void on_m_raceDialogue_itemClicked(QListWidgetItem* item);
  void onNavBack();
  void onNavForward();

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

  static const int NAV_HISTORY_MAX = 50;
  QVector<NavLocation> m_navHistory;
  int m_navIndex = -1;             // index of the current location in m_navHistory
  bool m_navigatingHistory = false; // suppresses recording during back/forward traversal
  QAction* m_navBackAction = nullptr;
  QAction* m_navForwardAction = nullptr;

  //! Reverse index of embedded GTxtCmd commands, keyed by (command, parameter), to the
  //! conversation lines that contain them. Built lazily on first use.
  QMultiHash<QPair<int,int>, ConversationRef> m_convIndex;
  bool m_convIndexBuilt;

  //! Reverse index of conversation topics (not just embedded commands), keyed by (topic,
  //! thingId), so e.g. "an alien has a line about this object" is findable even when the line
  //! embeds no command. Populated alongside m_convIndex.
  QMultiHash<QPair<int,int>, ConversationRef> m_convTopicIndex;

  //! Refs (isRace/alienOrRaceId/topic/thingId) already processed while building m_convIndex --
  //! many alienIds fall back to the same race-level dialogue line, so this avoids reprocessing
  //! (and rescanning m_convIndex for) an identical ref once per alien that falls back to it.
  QSet<QPair<QPair<int,int>, QPair<int,int> > > m_convIndexSeenRefs;

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
  void populateRaceWidgets();
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
  void buildConversationIndexIfNeeded();
  void indexConversationEntry(int alienId, ConvTopicCategory topic, int thingId);
  QList<ConversationRef> conversationRefsFor(GTxtCmd cmd, int param);
  QList<ConversationRef> conversationTopicRefsFor(ConvTopicCategory topic, int thingId);
  QString describeConversationTopic(const ConversationRef& ref);
  void navigateToConversation(const ConversationRef& ref);

  void navigateToEntity(EntityType type, int id);
  bool currentNavLocation(NavLocation& out) const;
  void recordNavigation(const NavLocation& dest);
  void goToNavLocation(const NavLocation& loc);
  void updateNavActions();
  QString entityHref(EntityType type, int id) const;
  bool parseEntityHref(const QString& href, EntityType& outType, int& outId) const;
  bool getEntityLinkForGameTextCommand(GTxtCmd cmd, int param, EntityType& outType) const;
  void styleAsLinkItem(QListWidgetItem* item);
  void styleAsLinkItem(QTableWidgetItem* item);
  void handleLinkCellClicked(QTableWidget* table, int row, int column);
  void populateObjectUsages(int id);
  void handleRaceDetailItemClicked(QListWidgetItem* item);
  QList<ConversationRef> raceOwnedConversationRefs(int raceId);
};

#endif // MAINWINDOW_H
