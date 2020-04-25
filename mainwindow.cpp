#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QString>
#include <QFileDialog>
#include <QFile>
#include <QVector>
#include <QTableWidgetItem>
#include <QImage>
#include <QGraphicsScene>
#include <QTreeWidgetItem>
#include <QMap>
#include <QAudioDeviceInfo>
#include <QBuffer>
#include <QTime>
#include <QToolTip>
#include <QCursor>
#include <QPair>
#include <QIcon>
#include "enums.h"
#include "tablenumberitem.h"

#define ICON_PATH ":/icon/icon/nre-48x48.png"

MainWindow::MainWindow(QString gameDir, QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  m_aboutBox(nullptr),
  m_gametext(m_lib),
  m_invObject(m_lib, m_palette, m_gametext),
  m_places(m_lib, m_palette, m_pclasses),
  m_palette(m_lib),
  m_pclasses(m_lib),
  m_aliens(m_lib, m_palette),
  m_ships(m_lib),
  m_shipClasses(m_lib),
  m_inventory(m_lib),
  m_facts(m_lib),
  m_audio(m_lib),
  m_fullscreenImages(m_lib, m_palette),
  m_stamps(m_lib, m_palette),
  m_convText(m_lib, m_aliens, m_gametext),
  m_currentNNVSoundCount(0),
  m_currentNNVSoundId(-1),
  m_currentNNVFilename(""),
  m_currentSoundDat(DatFileType_INVALID),
  m_audioOutput(nullptr),
  m_currentConvTopic(ConvTopicCategory_GreetingInitial)
{
  setWindowIcon(QIcon(ICON_PATH));
  ui->setupUi(this);
  putResourceLabelsInArray();

  ui->m_objectImageView->scale(3, 3);
  ui->m_planetView->scale(2, 2);
  ui->m_alienView->scale(3, 3);
  ui->m_fullscreenView->scale(2, 2);
  ui->m_stampView->scale(2, 2);

  setupAudio();
  clearAllResourceLabels();

  if (!gameDir.isEmpty())
  {
    openNewData(gameDir);
  }
}

MainWindow::~MainWindow()
{
  delete m_audioOutput;
  delete m_aboutBox;
  delete ui;
}

void MainWindow::on_actionAbout_triggered()
{
  if (!m_aboutBox)
  {
    m_aboutBox = new AboutBox(this);
  }
  m_aboutBox->show();
}

/**
 * Create a convenience array of the QLabels used to display the natural resources of a planet.
 */
void MainWindow::putResourceLabelsInArray()
{
  m_resourceLabels[PlanetResourceType_Animal] = { { 0, ui->m_pAnimals0 },
                                                  { 1, ui->m_pAnimals1 },
                                                  { 2, ui->m_pAnimals2 } };
  m_resourceLabels[PlanetResourceType_ArchaeologicalArtifact] = { { 0, ui->m_pArchArtifacts0 },
                                                                  { 1, ui->m_pArchArtifacts1 },
                                                                  { 2, ui->m_pArchArtifacts2 } };
  m_resourceLabels[PlanetResourceType_EspionageItem] = { { 0, ui->m_pSpyArtifacts0 },
                                                         { 1, ui->m_pSpyArtifacts1 },
                                                         { 2, ui->m_pSpyArtifacts2 } };
  m_resourceLabels[PlanetResourceType_Food] = { { 0, ui->m_pFoods0 },
                                                { 1, ui->m_pFoods1 },
                                                { 2, ui->m_pFoods2 } };
  m_resourceLabels[PlanetResourceType_Gas] = { { 0, ui->m_pGasses0 },
                                               { 1, ui->m_pGasses1 },
                                               { 2, ui->m_pGasses2 } };
  m_resourceLabels[PlanetResourceType_Mineral] = { { 0, ui->m_pMinerals0 },
                                                   { 1, ui->m_pMinerals1 },
                                                   { 2, ui->m_pMinerals2 } };
}

/**
 * Set all of the planet resource labels to a string that contains a bullet but no further text.
 */
void MainWindow::clearAllResourceLabels()
{
  for (int rtypeIdx = 0; rtypeIdx < PlanetResourceType_NumTypes; rtypeIdx++)
  {
    const PlanetResourceType prType = static_cast<PlanetResourceType>(rtypeIdx);
    foreach (QLabel* l, m_resourceLabels[prType])
    {
      l->setText(QString::fromUtf8(" \u2022"));
    }
  }
}

void MainWindow::clearPlaceLabels()
{
  ui->m_placeTypeData->clear();
  ui->m_placeClassData->clear();
  ui->m_placeRaceData->clear();
  ui->m_placeTemperatureData->clear();
  ui->m_placeRepData->clear();
}

/**
 * Clears the cached data in each of the data handling class instances.
 */
void MainWindow::clearData()
{
  m_lib.closeData();
  m_invObject.clear();
  m_places.clear();
  m_palette.clear();
  m_pclasses.clear();
  m_aliens.clear();
  m_ships.clear();
  m_shipClasses.clear();
  m_inventory.clear();
  m_facts.clear();

  m_alienFrames.clear();
  m_stampImages.clear();

  m_fullscreenScene.clear();
  m_objScene.clear();
  m_stampScene.clear();
  m_alienScene.clear();
  m_planetSurfaceScene.clear();
}

/**
 * Opens a new game data directory and sets up the form widgets the display the new data.
 */
void MainWindow::openNewData(const QString gameDir)
{
  clearData();
  ui->statusBar->showMessage(QString("Using directory: %1").arg(gameDir));

  m_lib.openData(gameDir);
  populatePlaceWidgets();
  populateObjectWidgets();
  populateAlienWidgets();
  populateShipWidgets();
  populateAudioWidgets();
  populateFactWidgets();
  populateFullscreenLbmWidgets();
  populateStampWidgets();
  populateConversationWidgets();
}

/**
 * Sets up audio output to match the PCM sound format used by the game.
 */
void MainWindow::setupAudio()
{
  m_audioFormat.setSampleRate(7042);
  m_audioFormat.setChannelCount(1);
  m_audioFormat.setSampleSize(8);
  m_audioFormat.setCodec("audio/pcm");
  m_audioFormat.setByteOrder(QAudioFormat::LittleEndian);
  m_audioFormat.setSampleType(QAudioFormat::UnSignedInt);

  QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());

  if (info.isFormatSupported(m_audioFormat))
  {
    m_audioOutput = new QAudioOutput(m_audioFormat, this);
    connect(m_audioOutput, SIGNAL(stateChanged(QAudio::State)), this, SLOT(onAudioStateChanged(QAudio::State)));
    setAudioStateLabel(m_audioOutput->state());
  }
  else
  {
    ui->m_soundWarningLabel->setText("Warning: device audio output device does not support this audio format."
                                     " Sound output is disabled.");
  }
}

/**
 * Sets the audio playback state QLabel to display the current playback state.
 * This routine also calls stop() on the QAudioOutput when playback of the data
 * has finished and the output device is idle.
 */
void MainWindow::setAudioStateLabel(QAudio::State state)
{
  switch (state)
  {
  case QAudio::ActiveState:
    ui->m_soundStateLabel->setText("Playing");
    break;
  case QAudio::SuspendedState:
    ui->m_soundStateLabel->setText("Suspended");
    break;
  case QAudio::StoppedState:
    ui->m_soundStateLabel->setText("Stopped");
    break;
  case QAudio::IdleState:
    ui->m_soundStateLabel->setText("Idle");
    if (m_audioOutput)
    {
      m_audioOutput->stop();
    }
    break;
  case QAudio::InterruptedState:
    ui->m_soundStateLabel->setText("Interrupted");
    break;
  }

  setSoundButtonStates();
}

/**
 * Responds to the audio playback state changing by calling a routine
 * that updates labels and button states.
 */
void MainWindow::onAudioStateChanged(QAudio::State state)
{
  setAudioStateLabel(state);
}

/**
 * Responds to the "open game directory" menu action being triggered by
 * calling a routine that will causes the form to be updated with new data.
 */
void MainWindow::on_actionOpen_game_data_dir_triggered()
{
  m_gamedir = QFileDialog::getExistingDirectory(this, "Select directory containing Nomad .DAT files", "/home", QFileDialog::ShowDirsOnly);
  openNewData(m_gamedir);
}

/**
 * Closes files and exits the application.
 */
void MainWindow::onExit()
{
  m_lib.closeData();
  this->close();
}

/**
 * Closes files and clears the forms.
 */
void MainWindow::onCloseDataFiles()
{
  clearData();
}

/**
 * Populates the table of places.
 */
void MainWindow::populatePlaceWidgets()
{
  QMap<int,Place> places = m_places.getPlaceList();
  ui->m_placeTable->setRowCount(0);

  foreach (Place p, places.values())
  {
    const int rowcount = ui->m_placeTable->rowCount();
    ui->m_placeTable->insertRow(rowcount);
    ui->m_placeTable->setItem(rowcount, 0, new TableNumberItem(QString("%1").arg(p.id)));
    ui->m_placeTable->setItem(rowcount, 1, new QTableWidgetItem(p.name));
  }
  ui->m_placeTable->resizeColumnsToContents();
  ui->m_placeTable->resizeRowsToContents();
}

/*
 * Populates the table of inventory objects.
 */
void MainWindow::populateObjectWidgets()
{
  QMap<int,InventoryObj> objs = m_invObject.getList();
  ui->m_objTable->setRowCount(0);

  foreach (InventoryObj obj, objs.values())
  {
    const int rowcount = ui->m_objTable->rowCount();

    ui->m_objTable->insertRow(rowcount);
    ui->m_objTable->setItem(rowcount, 0, new TableNumberItem(QString("%1").arg(obj.id)));
    ui->m_objTable->setItem(rowcount, 1, new QTableWidgetItem(obj.name));
    for (int r = 0; r < AlienRace_NumRaces; r++)
    {
      ui->m_objTable->setItem(rowcount, 2 + r, new TableNumberItem(QString("%1").arg(obj.valueByRace[r])));
    }
  }
  ui->m_objTable->resizeColumnsToContents();
  ui->m_objTable->resizeRowsToContents();
}

/*
 * Populates the table of aliens.
 */
void MainWindow::populateAlienWidgets()
{
  QMap<int,Alien> aliens = m_aliens.getList();
  ui->m_alienTable->setRowCount(0);

  foreach (Alien a, aliens.values())
  {
    const int rowcount = ui->m_alienTable->rowCount();
    ui->m_alienTable->insertRow(rowcount);
    ui->m_alienTable->setItem(rowcount, 0, new TableNumberItem(QString("%1").arg(a.id)));
    ui->m_alienTable->setItem(rowcount, 1, new QTableWidgetItem(a.name));

    const QString racename = s_raceNames.contains(a.race) ? s_raceNames[a.race] : "(invalid/unknown)";
    ui->m_alienTable->setItem(rowcount, 2, new QTableWidgetItem(racename));
  }
  ui->m_alienTable->resizeColumnsToContents();
  ui->m_alienTable->resizeRowsToContents();
}

/*
 * Populates the table of ships.
 */
void MainWindow::populateShipWidgets()
{
  QMap<int,Ship> ships = m_ships.getList();
  ui->m_shipTable->setRowCount(0);

  foreach (Ship s, ships.values())
  {
    const int rowcount = ui->m_shipTable->rowCount();
    ui->m_shipTable->insertRow(rowcount);
    ui->m_shipTable->setItem(rowcount, 0, new TableNumberItem(QString("%1").arg(s.id)));
    ui->m_shipTable->setItem(rowcount, 1, new QTableWidgetItem(s.name));
    ui->m_shipTable->setItem(rowcount, 2, new QTableWidgetItem(m_shipClasses.getName(s.shipclass)));
    ui->m_shipTable->setItem(rowcount, 3, new QTableWidgetItem(m_aliens.getName(s.pilot)));
    ui->m_shipTable->setItem(rowcount, 4, new QTableWidgetItem(m_places.getName(s.location)));
  }
  ui->m_shipTable->resizeColumnsToContents();
  ui->m_shipTable->resizeRowsToContents();
}

/*
 * Populates the table of facts.
 */
void MainWindow::populateFactWidgets()
{
  QMap<int,Fact> facts = m_facts.getList();
  ui->m_factTable->setRowCount(0);

  foreach (Fact f, facts.values())
  {
    const int rowcount = ui->m_factTable->rowCount();
    ui->m_factTable->insertRow(rowcount);
    ui->m_factTable->setItem(rowcount, 0, new TableNumberItem(QString("%1").arg(f.id)));
    for (int raceId = 0; raceId < AlienRace_NumRaces; raceId++)
    {
      AlienRace race = static_cast<AlienRace>(raceId);
      ui->m_factTable->setItem(rowcount, 1 + raceId, new TableNumberItem(QString("%1").arg(f.receptivity[race])));
    }
  }
  ui->m_factTable->resizeColumnsToContents();
  ui->m_factTable->resizeRowsToContents();
}

/*
 * Populates the tree of audio containers and audio file names.
 */
void MainWindow::populateAudioWidgets()
{
  ui->m_soundTree->clear();

  const QMap<DatFileType,QStringList> nnvList = m_audio.getAllSoundList();
  foreach (DatFileType dat, nnvList.keys())
  {
    if (nnvList[dat].size() > 0)
    {
      const QString datFilename = m_lib.s_datFileNames[dat];
      QTreeWidgetItem* datTreeParent = new QTreeWidgetItem(ui->m_soundTree);
      datTreeParent->setText(0, datFilename);

      foreach (QString nnvFilename, nnvList[dat])
      {
        QTreeWidgetItem* nnvChild = new QTreeWidgetItem();
        const int numSounds = m_audio.getNumberOfSoundsInNNV(dat, nnvFilename);
        nnvChild->setText(0, nnvFilename);
        nnvChild->setText(1, QString("%1").arg(numSounds));
        datTreeParent->addChild(nnvChild);
      }
    }
  }

  ui->m_soundTree->expandAll();
  ui->m_soundTree->resizeColumnToContents(0);
}

/**
 * Populates the tree of fullscreen image files.
 */
void MainWindow::populateFullscreenLbmWidgets()
{
  ui->m_fullscreenTree->clear();
  const QMap<DatFileType,QStringList> lbmList = m_fullscreenImages.getAllLbmList();
  foreach (DatFileType dat, lbmList.keys())
  {
    if (lbmList[dat].size() > 0)
    {
      const QString datFilename = m_lib.s_datFileNames[dat];
      QTreeWidgetItem* datTreeParent = new QTreeWidgetItem(ui->m_fullscreenTree);
      datTreeParent->setText(0, datFilename);

      foreach (QString lbmFilename, lbmList[dat])
      {
        QTreeWidgetItem* lbmChild = new QTreeWidgetItem();
        lbmChild->setText(0, lbmFilename);
        datTreeParent->addChild(lbmChild);
      }
    }
  }

  ui->m_fullscreenTree->expandAll();
  ui->m_fullscreenTree->resizeColumnToContents(0);
}

/**
 * Populates the tree of stamp (STP and ROL) image files.
 */
void MainWindow::populateStampWidgets()
{
  ui->m_stampTree->clear();
  const QMap<DatFileType,QStringList> stampList = m_stamps.getAllStampsList();

  foreach (DatFileType dat, stampList.keys())
  {
    if (stampList[dat].size() > 0)
    {
      const QString datFilename = m_lib.s_datFileNames[dat];
      QTreeWidgetItem* datTreeParent = new QTreeWidgetItem(ui->m_stampTree);
      datTreeParent->setText(0, datFilename);

      foreach (QString stampFilename, stampList[dat])
      {
        QTreeWidgetItem* stampChild = new QTreeWidgetItem();
        stampChild->setText(0, stampFilename);
        datTreeParent->addChild(stampChild);
      }
    }
  }

  ui->m_stampTree->expandAll();
  ui->m_stampTree->resizeColumnToContents(0);
}

/**
 * Populates the only conversation text widget that must always contains some data,
 * which is the primary list of aliens.
 */
void MainWindow::populateConversationWidgets()
{
  QMap<int,Alien> aliens = m_aliens.getList();
  ui->m_convAlienTable->setRowCount(0);

  foreach (Alien a, aliens.values())
  {
    const int rowcount = ui->m_convAlienTable->rowCount();
    ui->m_convAlienTable->insertRow(rowcount);
    ui->m_convAlienTable->setItem(rowcount, 0, new TableNumberItem(QString("%1").arg(a.id)));
    ui->m_convAlienTable->setItem(rowcount, 1, new QTableWidgetItem(a.name));

    getConversationLinesForCurrentTopic();
  }

  ui->m_convAlienTable->resizeColumnsToContents();
  ui->m_convAlienTable->resizeRowsToContents();
}

/**
 * Responds to a ship being selected in the ship table by populating
 * the neighboring table to show that ship's inventory.
 */
void MainWindow::on_m_shipTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
  Q_UNUSED(currentColumn)
  Q_UNUSED(previousRow)
  Q_UNUSED(previousColumn)

  ui->m_shipInventoryTable->setRowCount(0);
  const QTableWidgetItem* const selectedItem = ui->m_shipTable->item(currentRow, 0);

  if (selectedItem)
  {
    const int shipid = selectedItem->text().toInt();
    QMap<int,int> inventory = m_inventory.getInventory(shipid);

    foreach(int obj, inventory.keys())
    {
      const int count = inventory[obj];
      const int rowcount = ui->m_shipInventoryTable->rowCount();
      ui->m_shipInventoryTable->insertRow(rowcount);
      ui->m_shipInventoryTable->setItem(rowcount, 0, new QTableWidgetItem(QString("%1").arg(obj)));
      ui->m_shipInventoryTable->setItem(rowcount, 1, new QTableWidgetItem(m_invObject.getName(obj)));
      ui->m_shipInventoryTable->setItem(rowcount, 2, new QTableWidgetItem(QString("%1").arg(count)));
    }

    // resize the columns individually, but skip the last column so that
    // the stretchLastSection property is not ignored (QTBUG-52307)
    ui->m_shipInventoryTable->resizeColumnToContents(0);
    ui->m_shipInventoryTable->resizeColumnToContents(1);
    ui->m_shipInventoryTable->resizeRowsToContents();
  }
}

/*
 * Responds to an object being selected in the object table by loading and
 * display its .STP image, and associated object text, and other parameters.
 */
void MainWindow::on_m_objTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
  Q_UNUSED(currentColumn)
  Q_UNUSED(previousRow)
  Q_UNUSED(previousColumn)

  m_objScene.clear();
  ui->m_objectText->setPlainText("");

  const QTableWidgetItem* const selectedItem = ui->m_objTable->item(currentRow, 0);

  if (selectedItem)
  {
    const int id = selectedItem->text().toInt();
    QImage img;

    if (m_invObject.getImage(id, img))
    {
      m_objScene.addPixmap(QPixmap::fromImage(img));
      ui->m_objectImageView->setScene(&m_objScene);
    }

    ui->m_objectTypeLabel->setText("Type: " + getInventoryObjTypeText(m_invObject.getObjectType(id)));
    if (m_invObject.isUnique(id))
    {
      ui->m_objectUniqueLabel->setText("Unique: Yes");
    }
    else
    {
      ui->m_objectUniqueLabel->setText("Unique: No");
    }
    ui->m_objectText->setHtml(m_invObject.getObjectText(id));
  }
}

/**
 * Responds to a row being selected in the fact table by loading and displaying the text for that fact.
 */
void MainWindow::on_m_factTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
  Q_UNUSED(currentColumn)
  Q_UNUSED(previousRow)
  Q_UNUSED(previousColumn)

  ui->m_factText->clear();

  const QTableWidgetItem* const selectedItem = ui->m_factTable->item(currentRow, 0);
  if (selectedItem)
  {
    const int id = selectedItem->text().toInt();
    const Fact f = m_facts.getFact(id);
    ui->m_factText->setPlainText(f.text);
  }
}

/**
 * Responds to a row being selected in the place table by loading and displaying all of its parameters,
 * including class name, temperature, and resources.
 */
void MainWindow::on_m_placeTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
  Q_UNUSED(currentColumn)
  Q_UNUSED(previousRow)
  Q_UNUSED(previousColumn)

  clearAllResourceLabels();
  clearPlaceLabels();
  m_planetSurfaceScene.clear();

  const QTableWidgetItem* const selectedItem = ui->m_placeTable->item(currentRow, 0);

  if (selectedItem)
  {
    bool status = true;
    const int id = selectedItem->text().toInt();

    Place p;
    if (m_places.getPlace(id, p))
    {
      if (p.isPlanet)
      {
        ui->m_placeTypeData->setText("Planet");

        if (id != 0x132) // special check for Second Harmony space station, which uses a 3D model
        {
          QImage surfaceImg = m_places.getPlaceSurfaceImage(id, status);
          if (status)
          {
            m_planetSurfaceScene.addPixmap(QPixmap::fromImage(surfaceImg));
            ui->m_planetView->setScene(&m_planetSurfaceScene);
          }
        }

        PlanetClass pclassData;
        if (m_pclasses.pclassData(p.classId, pclassData))
        {
          ui->m_placeClassData->setText(pclassData.name);
          const QString tempString = QString("%1 (%2)").arg(pclassData.temperature).arg(pclassData.temperatureRange);
          ui->m_placeTemperatureData->setText(tempString);
          ui->m_placeRaceData->setText(s_raceNames.contains(p.race) ? s_raceNames[p.race] : "(none)");
          ui->m_placeRepData->setText(m_aliens.getName(p.representativeId));

          foreach (PlanetResourceType resource, pclassData.resources.keys())
          {
            int resourceSlot = 0;
            const QMap<int,int> resourcesOfType = pclassData.resources[resource];
            while ((resourceSlot < 3) &&
                   (resourceSlot < resourcesOfType.count()) &&
                   (resourceSlot < m_resourceLabels[resource].count()))
            {
              const int resourceItemId = resourcesOfType.keys().at(resourceSlot);
              const QString resourceName = m_invObject.getName(resourceItemId);
              m_resourceLabels[resource][resourceSlot]->setText(QString::fromUtf8(" \u2022  ") + resourceName);
              resourceSlot++;
            }
          }
        }
      }
      else // place is a star
      {
        ui->m_placeTypeData->setText("Star");
        ui->m_placeClassData->setText(m_pclasses.getStarClassName(p.classId));
        ui->m_placeRaceData->setText("");
        ui->m_placeTemperatureData->setText("");
        ui->m_placeRepData->setText("");
      }
    }
  }
}

/**
 * Responds to a row being selected in the alien table by loading its animation frames.
 */
void MainWindow::on_m_alienTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
  Q_UNUSED(currentColumn)
  Q_UNUSED(previousRow)
  Q_UNUSED(previousColumn)

  m_alienFrames.clear();
  m_aliens.clear();
  const QTableWidgetItem* const selectedItem = ui->m_alienTable->item(currentRow, 0);

  if (selectedItem)
  {
    const int id = selectedItem->text().toInt();

    Alien a;
    if (m_aliens.getAlien(id, a))
    {
      if (m_aliens.getAnimationFrames(id, m_alienFrames) && (m_alienFrames.count() > 0))
      {
        ui->m_alienFrameSlider->setEnabled(true);
        ui->m_alienFrameSlider->setMaximum(m_alienFrames.count() - 1);
        ui->m_alienFrameSlider->setSliderPosition(0);
        loadAlienFrame(0);
      }
      else
      {
        ui->m_alienFrameSlider->setMaximum(63);
        ui->m_alienFrameSlider->setEnabled(false);
        m_alienScene.clear();
        ui->m_alienView->setScene(&m_alienScene);
      }
    }
  }
}

/**
 * Responds to a change in the alien animation frame slider position by displaying the appropriate frame.
 */
void MainWindow::on_m_alienFrameSlider_valueChanged(int value)
{
  loadAlienFrame(value);
}

/**
 * Loads and displays a single alien animation frame.
 */
void MainWindow::loadAlienFrame(int frameId)
{
  m_alienScene.clear();
  if (m_alienFrames.values().count() > frameId)
  {
    m_alienScene.addPixmap(QPixmap::fromImage(m_alienFrames.values().at(frameId)));
    ui->m_alienView->setScene(&m_alienScene);
  }
}

/**
 * Responds to the selection in the sound .NNV tree widget being changed.
 */
void MainWindow::on_m_soundTree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
  Q_UNUSED(previous)

  if (current)
  {
    m_currentNNVFilename = current->text(0);
    if (current->parent())
    {
      const QString datFilename = current->parent()->text(0);
      m_currentSoundDat = DatLibrary::s_datFileNames.key(datFilename);
      m_currentNNVSoundCount = m_audio.getNumberOfSoundsInNNV(m_currentSoundDat, m_currentNNVFilename);

      if (m_currentNNVSoundCount > 0)
      {
        m_currentNNVSoundId = 0;
        setSoundIDLabel(m_currentNNVFilename, m_currentNNVSoundId);
      }
    }
    else
    {
      m_currentNNVSoundId = -1;
      setSoundIDLabel(m_currentNNVFilename, m_currentNNVSoundId);
    }
  }
  else
  {
    m_currentNNVSoundCount = 0;
    m_currentNNVSoundId = -1;
    m_currentNNVFilename = "";
    m_currentSoundDat = DatFileType_INVALID;
  }

  setSoundButtonStates();
}

/**
 * Decrements the currently selected sound ID.
 */
void MainWindow::on_m_soundPrevButton_clicked()
{
  if (m_currentNNVSoundId > 0)
  {
    m_currentNNVSoundId--;
    setSoundIDLabel(m_currentNNVFilename, m_currentNNVSoundId);
    setSoundButtonStates();
  }
}

/**
 * Loads the currently selected sound into a buffer, and start playing it.
 */
void MainWindow::on_m_soundPlayButton_clicked()
{
  if (m_currentNNVSoundId >= 0)
  {
    m_audioPcmData.clear();
    if (m_audio.readSound(m_currentSoundDat, m_currentNNVFilename, m_currentNNVSoundId, m_audioPcmData))
    {
      if (m_audioBuffer.isOpen())
      {
        m_audioBuffer.close();
      }
      m_audioBuffer.setData(m_audioPcmData);
      m_audioBuffer.open(QIODevice::ReadOnly);
      m_audioOutput->start(&m_audioBuffer);
    }
  }
}

/**
 * Stops the audio output.
 * The enable/disable state of the sound related widgets will be updated by the stateChanged()
 * handler for the QAudioOutput object, which will be triggered by stopping the playback.
 */
void MainWindow::on_m_soundStopButton_clicked()
{
  m_audioOutput->stop();
}

/**
 * Increments the currently selected sound ID.
 */
void MainWindow::on_m_soundNextButton_clicked()
{
  if (m_currentNNVSoundId < (m_currentNNVSoundCount - 1))
  {
    m_currentNNVSoundId++;
    setSoundIDLabel(m_currentNNVFilename, m_currentNNVSoundId);
    setSoundButtonStates();
  }
}

/**
 * Set the enable state of each of the sound-playing buttons, depending
 * on the currently selected sound ID and the audio output state.
 */
void MainWindow::setSoundButtonStates()
{
  if (m_audioOutput)
  {
    if ((m_audioOutput->state() == QAudio::StoppedState) && (m_currentNNVSoundId >= 0))
    {
      ui->m_soundTree->setEnabled(true);
      ui->m_soundStopButton->setEnabled(false);

      if (m_currentNNVSoundId >= 0)
      {
        ui->m_soundPlayButton->setEnabled(true);
      }

      if (m_currentNNVSoundCount > 0)
      {
        ui->m_soundNextButton->setEnabled(m_currentNNVSoundId < (m_currentNNVSoundCount - 1));
        ui->m_soundPrevButton->setEnabled(m_currentNNVSoundId > 0);
      }
      else
      {
        ui->m_soundNextButton->setEnabled(false);
        ui->m_soundPrevButton->setEnabled(false);
      }
    }
    else if (m_audioOutput->state() == QAudio::ActiveState)
    {
      ui->m_soundTree->setEnabled(false);
      ui->m_soundPlayButton->setEnabled(false);
      ui->m_soundNextButton->setEnabled(false);
      ui->m_soundPrevButton->setEnabled(false);
      ui->m_soundStopButton->setEnabled(true);
    }
  }
  else
  {
    ui->m_soundPlayButton->setEnabled(false);
    ui->m_soundNextButton->setEnabled(false);
    ui->m_soundPrevButton->setEnabled(false);
    ui->m_soundStopButton->setEnabled(false);
  }
}

/**
 * Sets the text of the label that displays the currently selected sound ID.
 */
void MainWindow::setSoundIDLabel(QString nnvName, int soundId)
{
  if (soundId >= 0)
  {
    const int dotPosition = nnvName.indexOf('.');
    const QString baseFilename = (dotPosition > 0) ? nnvName.mid(0, dotPosition) : nnvName;

    ui->m_soundIDLabel->setText(QString("Sound ID: %1/%2").arg(baseFilename).arg(soundId));
  }
  else
  {
    ui->m_soundIDLabel->setText("Sound ID: (none selected)");
    ui->m_soundPlayButton->setEnabled(false);
  }
}

/**
 * Loads and displays one of the fullscreen LBM images.
 */
void MainWindow::on_m_fullscreenTree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
  Q_UNUSED(previous)
  m_fullscreenScene.clear();

  if (current)
  {
    const QString lbmFilename = current->text(0);
    if (current->parent())
    {
      const QString datFilename = current->parent()->text(0);
      const DatFileType dat = DatLibrary::s_datFileNames.key(datFilename);

      QVector<QRgb> tempPal;
      const QString paletteFilename = m_fullscreenImages.getPalette(dat, lbmFilename, tempPal);
      ui->m_fullscreenPaletteName->setText(QString("Displaying with palette: ") + paletteFilename);

      QImage fsLbm;
      if (m_fullscreenImages.getImage(dat, lbmFilename, fsLbm))
      {
        m_fullscreenScene.addPixmap(QPixmap::fromImage(fsLbm));
      }
    }
    ui->m_fullscreenView->setScene(&m_fullscreenScene);
  }
}

/**
 * Responds to an item being selected in the tree of stamp/roll images.
 */
void MainWindow::on_m_stampTree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
  Q_UNUSED(previous)
  m_stampScene.clear();
  m_stampImages.clear();

  if (current)
  {
    const QString stampFilename = current->text(0);
    if (current->parent())
    {
      const QString datFilename = current->parent()->text(0);
      const DatFileType dat = DatLibrary::s_datFileNames.key(datFilename);

      if (m_stamps.getStamp(dat, stampFilename, m_stampImages))
      {
        ui->m_stampRollSlider->setEnabled(m_stampImages.count() > 0);
        ui->m_stampRollSlider->setValue(0);
        if (m_stampImages.count() > 0)
        {
          ui->m_stampRollSlider->setMaximum(m_stampImages.count() - 1);
          displayStamp(0);
        }
        else
        {
          ui->m_stampRollSlider->setMaximum(0);
        }
      }
    }
  }
}

/**
 * Responds to a row being selected in the alien table of the conversation text tab.
 */
void MainWindow::on_m_convAlienTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
  Q_UNUSED(currentRow)
  Q_UNUSED(currentColumn)
  Q_UNUSED(previousRow)
  Q_UNUSED(previousColumn)

  clearDialogLineAndCommandList();

  // We're going to repopulate the list of conversation topics for the currently selected alien and topic
  // category. Before we do, save the ID of the last topic selected by the user so that we can auto-select
  // it in the newly repopulated table *if* that topic is one of interest for the newly selected alien.
  const int currentTopicRow = ui->m_convTopicTable->currentRow();
  const QTableWidgetItem* const selectedTopicItem = ui->m_convTopicTable->item(currentTopicRow, 0);
  const int lastSelectedTopicId = (selectedTopicItem) ? (selectedTopicItem->text().toInt()) : -1;

  populateConversationTopicTable(lastSelectedTopicId);
}

/**
 * Switches the current conversation topic to "initial greeting".
 */
void MainWindow::on_m_convTopicButtonGreeting0_clicked()
{
  m_currentConvTopic = ConvTopicCategory_GreetingInitial;
  populateConversationTopicTable();
}

/**
 * Switches the current conversation topic to "subsequent greeting".
 */
void MainWindow::on_m_convTopicButtonGreeting1_clicked()
{
  m_currentConvTopic = ConvTopicCategory_GreetingSubsequent;
  populateConversationTopicTable();
}

/**
 * Switches the current conversation topic to "ask about person".
 */
void MainWindow::on_m_convTopicButtonPerson_clicked()
{
  m_currentConvTopic = ConvTopicCategory_AskAboutPerson;
  populateConversationTopicTable();
}

/**
 * Switches the current conversation topic to "ask about location".
 */
void MainWindow::on_m_convTopicButtonPlace_clicked()
{
  m_currentConvTopic = ConvTopicCategory_AskAboutLocation;
  populateConversationTopicTable();
}

/**
 * Switches the current conversation topic to "ask about object".
 */
void MainWindow::on_m_convTopicButtonObject_clicked()
{
  m_currentConvTopic = ConvTopicCategory_AskAboutObject;
  populateConversationTopicTable();
}

/**
 * Switches the current conversation topic to "ask about race".
 */
void MainWindow::on_m_convTopicButtonRace_clicked()
{
  m_currentConvTopic = ConvTopicCategory_AskAboutRace;
  populateConversationTopicTable();
}

/**
 * Switches the current conversation topic to "display object".
 */
void MainWindow::on_m_convTopicButtonDispObj_clicked()
{
  m_currentConvTopic = ConvTopicCategory_DisplayObject;
  populateConversationTopicTable();
}

/**
 * Switches the current conversation topic to "give object".
 */
void MainWindow::on_m_convTopicButtonGiveObj_clicked()
{
  m_currentConvTopic = ConvTopicCategory_GiveObject;
  populateConversationTopicTable();
}

/**
 * Switches the current conversation topic to "give fact".
 */
void MainWindow::on_m_convTopicButtonGiveFact_clicked()
{
  m_currentConvTopic = ConvTopicCategory_GiveFact;
  populateConversationTopicTable();
}

/**
 * Switches the current conversation topic to "sees item".
 */
void MainWindow::on_m_convTopicButtonSeesItem_clicked()
{
  m_currentConvTopic = ConvTopicCategory_SeesObject;
  populateConversationTopicTable();
}

/**
 * Responds to the "show only interesting topics" checkbox being checked or unchecked
 * by applying or removing a filter on the conversation topic list box.
 */
void MainWindow::on_m_convFilterTopicsCheckbox_stateChanged(int arg1)
{
  Q_UNUSED(arg1)
  populateConversationTopicTable();
}

/**
 * Populates the list widget with the topics for the provided category. The topicList parameter provides
 * a comprehensive list of all possible topics in the category, but only a subset may actually be added
 * to the list, depending on whether an alien is selected or the show-only-interesting-dialogue checkbox
 * is checked.
 */
void MainWindow::populateTopicTableForCategory(ConvTopicCategory category, QMap<int, QString> topicList, int lastSelectedTopicId)
{
  const int currentAlienRow = ui->m_convAlienTable->currentRow();
  const QTableWidgetItem* const selectedAlienItem = ui->m_convAlienTable->item(currentAlienRow, 0);
  const int alienId = (selectedAlienItem) ? (selectedAlienItem->text().toInt()) : 0;

  // we will show all topics for the selected topic category if either:
  // (a) no alien is selected in the first list box, or
  // (b) the user has deselected the checkbox that filters the list down to the interesting topics
  const bool alwaysAddAllTopics = ((alienId == 0) || (ui->m_convFilterTopicsCheckbox->checkState() == Qt::Unchecked));

  foreach (int topicId, topicList.keys())
  {
    if (alwaysAddAllTopics || m_convText.doesInterestingDialogExist(alienId, category, topicId))
    {
      const int rowcount = ui->m_convTopicTable->rowCount();

      ui->m_convTopicTable->insertRow(rowcount);
      ui->m_convTopicTable->setItem(rowcount, 0, new TableNumberItem(QString("%1").arg(topicId)));
      ui->m_convTopicTable->setItem(rowcount, 1, new QTableWidgetItem(topicList[topicId]));
    }
  }
  ui->m_convTopicTable->resizeColumnToContents(0);
  ui->m_convTopicTable->resizeRowsToContents();

  clearDialogLineAndCommandList();

  // search through the rows to find one with the topic ID matching the previously selected topic ID;
  // if such a row is found, select it
  if (lastSelectedTopicId >= 0)
  {
    bool foundMatchingRow = false;
    int rowId = 0;
    const int rowCount = ui->m_convTopicTable->rowCount();

    while (!foundMatchingRow && (rowId < rowCount))
    {
      const QTableWidgetItem* const currentIdItem = ui->m_convTopicTable->item(rowId, 0);
      if (currentIdItem && (currentIdItem->text().toInt() == lastSelectedTopicId))
      {
        foundMatchingRow = true;
        ui->m_convTopicTable->selectRow(rowId);
      }
      rowId++;
    }
  }
}

/**
 * Populates the table of specific conversation topics, given the currently selected topic category.
 * Note that two categories -- Greeting/Initial and Greeting/Subsequent -- do not require the selection
 * of a specific topic, and they will not trigger the population of the table.
 *
 * If the parameter (the previously selected topic ID) is not negative, then automatically
 * select the row in the repopulated table with the same ID, if one exists.
 */
void MainWindow::populateConversationTopicTable(int lastSelectedTopicId)
{
  ui->m_convTopicTable->setRowCount(0);

  if (m_currentConvTopic == ConvTopicCategory_AskAboutPerson)
  {
    const QMap<int,Alien> aliens = m_aliens.getList();
    QMap<int,QString> alienNames;
    foreach (Alien a, aliens.values())
    {
      alienNames.insert(a.id, a.name);
    }
    populateTopicTableForCategory(m_currentConvTopic, alienNames, lastSelectedTopicId);
  }
  else if (m_currentConvTopic == ConvTopicCategory_AskAboutLocation)
  {
    const QMap<int,Place> places = m_places.getPlaceList();
    QMap<int,QString> placeNames;
    foreach (Place p, places.values())
    {
      placeNames.insert(p.id, p.name);
    }
    populateTopicTableForCategory(m_currentConvTopic, placeNames, lastSelectedTopicId);
  }
  else if ((m_currentConvTopic == ConvTopicCategory_AskAboutObject) ||
           (m_currentConvTopic == ConvTopicCategory_GiveObject)     ||
           (m_currentConvTopic == ConvTopicCategory_DisplayObject)  ||
           (m_currentConvTopic == ConvTopicCategory_SeesObject))
  {
    const QMap<int,InventoryObj> objs = m_invObject.getList();
    QMap<int,QString> objectNames;
    foreach (InventoryObj obj, objs.values())
    {
      objectNames.insert(obj.id, obj.name);
    }
    populateTopicTableForCategory(m_currentConvTopic, objectNames, lastSelectedTopicId);
  }
  else if (m_currentConvTopic == ConvTopicCategory_AskAboutRace)
  {
    QMap<int,QString> raceNames;
    for (int raceId = 0; raceId < AlienRace_NumRaces; raceId++)
    {
      const AlienRace race = static_cast<AlienRace>(raceId);
      raceNames.insert(raceId, s_raceNames[race]);
    }
    populateTopicTableForCategory(m_currentConvTopic, raceNames, lastSelectedTopicId);
  }
  else if (m_currentConvTopic == ConvTopicCategory_GiveFact)
  {
    const QMap<int,Fact> facts = m_facts.getList();
    QMap<int,QString> factText;
    foreach (Fact f, facts.values())
    {
      factText.insert(f.id, f.text);
    }
    populateTopicTableForCategory(m_currentConvTopic, factText, lastSelectedTopicId);
  }
  else if ((m_currentConvTopic == ConvTopicCategory_GreetingInitial) ||
           (m_currentConvTopic == ConvTopicCategory_GreetingSubsequent))
  {
    getConversationLinesForCurrentTopic();
  }
}

/**
 * Responds to the selection of a row in the conversation topic table.
 */
void MainWindow::on_m_convTopicTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
  Q_UNUSED(currentColumn)
  Q_UNUSED(previousRow)
  Q_UNUSED(previousColumn)

  if (currentRow >= 0)
  {
    getConversationLinesForCurrentTopic();
  }
}

/**
 * Loads any conversation text that is applicable to the currently selected alien and conversation topic.
 */
void MainWindow::getConversationLinesForCurrentTopic()
{
  const int currentAlienRow = ui->m_convAlienTable->currentRow();
  const int currentTopicRow = ui->m_convTopicTable->currentRow();

  const QTableWidgetItem* const selectedAlienItem = ui->m_convAlienTable->item(currentAlienRow, 0);
  const QTableWidgetItem* const selectedTopicItem = ui->m_convTopicTable->item(currentTopicRow, 0);

  const int alienId = (selectedAlienItem) ? (selectedAlienItem->text().toInt()) : 0;
  const int thingId = (selectedTopicItem) ? (selectedTopicItem->text().toInt()) : 0;

  // certain conversation topic categories do not require a topic ID greater than zero
  const bool thingIdOfZeroAllowed = (m_currentConvTopic == ConvTopicCategory_AskAboutRace) ||
                                    (m_currentConvTopic == ConvTopicCategory_GreetingInitial) ||
                                    (m_currentConvTopic == ConvTopicCategory_GreetingSubsequent);

  if ((alienId > 0) && ((thingId > 0) || thingIdOfZeroAllowed))
  {
    QVector<QPair<GTxtCmd,int> > embeddedCommands;
    QString currentConvLine = m_convText.getConversationText(alienId, m_currentConvTopic, thingId, embeddedCommands);

    clearDialogLineAndCommandList();
    ui->m_convDialogueLine->setHtml(currentConvLine);

    // populate the table to display the details of the commands embedded in this dialogue line
    QPair<GTxtCmd,int> cmdPair;
    foreach (cmdPair, embeddedCommands)
    {
      const GTxtCmd command = cmdPair.first;
      const int param = cmdPair.second;

      const int rowcount = ui->m_convCommandList->rowCount();

      ui->m_convCommandList->insertRow(rowcount);
      ui->m_convCommandList->setItem(rowcount, 0, new QTableWidgetItem(g_gameTextCommandName[command]));
      ui->m_convCommandList->setItem(rowcount, 1, new QTableWidgetItem(QString("%1").arg(param)));

      const QString paramName = getNameForGameTextCommandParameter(command, param);
      if (!paramName.isEmpty())
      {
        ui->m_convCommandList->setItem(rowcount, 2, new QTableWidgetItem(paramName));
      }
    }

    ui->m_convCommandList->resizeColumnToContents(0);
    ui->m_convCommandList->resizeColumnToContents(1);
    ui->m_convCommandList->resizeRowsToContents();
  }
}

/**
 * Get the human-readable name for the person/place/thing/fact described by the combination of
 * command byte and parameter ID.
 */
QString MainWindow::getNameForGameTextCommandParameter(GTxtCmd cmd, int param)
{
  QString name;

  if ((cmd == GTxtCmd_AddItem) || (cmd == GTxtCmd_GrantKnowledgeObject))
  {
    name = m_invObject.getName(param);
  }
  else if ((cmd == GTxtCmd_GrantKnowledgeAlien) || (cmd == GTxtCmd_AStateTableModifyB))
  {
    name = m_aliens.getName(param);
  }
  else if (cmd == GTxtCmd_GrantKnowledgeRace)
  {
    const AlienRace race = static_cast<AlienRace>(param);
    name = s_raceNames.contains(race) ? s_raceNames[race] : "(invalid)";
  }
  else if (cmd == GTxtCmd_GrantKnowledgePlace)
  {
    name = m_places.getName(param);
  }
  else if (cmd == GTxtCmd_StoreShipIDInSetupTab)
  {
    name = m_ships.getName(param);
  }
  else if (cmd == GTxtCmd_GrantKnowledgeFact)
  {
    name = m_facts.getFact(param).text;
  }

  return name;
}

/**
 * Clears the box showing the current line of dialogue, and clears the table that
 * shows all the embedded commands in that dialogue.
 */
void MainWindow::clearDialogLineAndCommandList()
{
  ui->m_convDialogueLine->clear();
  ui->m_convDialogueLine->clearHistory();
  ui->m_convCommandList->setRowCount(0);
}

/**
 * Responds to one of the conversation text synonyms being clicked by displaying a tooltip with
 * the alternatives from which the game randomly selects. The list of synonyms is sent from the
 * QTextBrowser to this slot by encoding them in a URL.
 */
void MainWindow::on_m_convDialogueLine_anchorClicked(const QUrl& arg1)
{
  const QString url = arg1.toString(QUrl::DecodeReserved).replace("|", "\n").trimmed();
  QToolTip::showText(QCursor::pos(), url);
}

/**
 * Responds to the stamp roll slider being moved by displaying the stamp at the selected index.
 */
void MainWindow::on_m_stampRollSlider_sliderMoved(int position)
{
  displayStamp(position);
}

/**
 * Displays a single static image on the stamp/roll tab, based on the currently selected roll index.
 */
void MainWindow::displayStamp(int rollIndex)
{
  m_stampScene.clear();
  if (m_stampImages.count() > rollIndex)
  {
    m_stampScene.addPixmap(QPixmap::fromImage(m_stampImages[rollIndex]));
    ui->m_stampView->setScene(&m_stampScene);
  }
}
