#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QString>
#include <QMessageBox>
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
#include "enums.h"

MainWindow::MainWindow(QString gameDir, QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  m_invObject(m_lib, m_palette),
  m_places(m_lib, m_palette, m_pclasses),
  m_palette(m_lib),
  m_pclasses(m_lib),
  m_aliens(m_lib, m_palette),
  m_ships(m_lib),
  m_inventory(m_lib),
  m_audio(m_lib),
  m_currentNNVSoundCount(0),
  m_currentNNVSoundId(-1),
  m_currentNNVFilename(""),
  m_currentSoundDat(DatFileType_INVALID),
  m_audioOutput(nullptr)
{
  ui->setupUi(this);
  ui->m_objectImageView->scale(3, 3);
  ui->m_planetView->scale(2, 2);
  ui->m_alienView->scale(3, 3);

  setupAudio();

  m_lib.openData(gameDir);
  populatePlaceWidgets();
  populateObjectWidgets();
  populateAlienWidgets();
  populateShipWidgets();
  populateAudioWidgets();
}

MainWindow::~MainWindow()
{
  delete m_audioOutput;
  delete ui;
}

void MainWindow::setupAudio()
{
  m_audioFormat.setSampleRate(8000);
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
}

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
    m_audioOutput->stop();
    break;
  case QAudio::InterruptedState:
    ui->m_soundStateLabel->setText("Interrupted");
    break;
  }

  setSoundButtonStates();
}

void MainWindow::onAudioStateChanged(QAudio::State state)
{
  setAudioStateLabel(state);
}

void MainWindow::on_actionOpen_game_data_dir_triggered()
{
  m_gamedir = QFileDialog::getExistingDirectory(this, "Select directory containing Nomad .DAT files", "/home", QFileDialog::ShowDirsOnly);
  m_lib.openData(m_gamedir);
}

void MainWindow::onExit()
{
  this->close();
}

void MainWindow::onCloseDataFiles()
{
  m_lib.closeData();
  m_aliens.clear();
  m_places.clear();
  m_pclasses.clear();
  m_invObject.clear();
}

void MainWindow::populatePlaceWidgets()
{
  QMap<int,Place> places = m_places.getPlaceList();
  ui->m_placeTable->clearContents();

  foreach (Place p, places.values())
  {
    const int rowcount = ui->m_placeTable->rowCount();
    ui->m_placeTable->insertRow(rowcount);
    ui->m_placeTable->setItem(rowcount, 0, new QTableWidgetItem(QString("%1").arg(p.id)));
    ui->m_placeTable->setItem(rowcount, 1, new QTableWidgetItem(p.name));
  }
  ui->m_placeTable->resizeColumnsToContents();
}

void MainWindow::populateObjectWidgets()
{
  QMap<int,InventoryObj> objs = m_invObject.getObjectList();
  ui->m_objTable->clearContents();

  foreach (InventoryObj obj, objs.values())
  {
    const int rowcount = ui->m_objTable->rowCount();

    ui->m_objTable->insertRow(rowcount);
    ui->m_objTable->setItem(rowcount, 0, new QTableWidgetItem(QString("%1").arg(obj.id)));
    ui->m_objTable->setItem(rowcount, 1, new QTableWidgetItem(obj.name));
  }
  ui->m_objTable->resizeColumnsToContents();
}

void MainWindow::populateAlienWidgets()
{
  QMap<int,Alien> aliens = m_aliens.getAlienList();
  ui->m_alienTable->clearContents();

  foreach (Alien a, aliens.values())
  {
    const int rowcount = ui->m_alienTable->rowCount();
    ui->m_alienTable->insertRow(rowcount);
    ui->m_alienTable->setItem(rowcount, 0, new QTableWidgetItem(QString("%1").arg(a.id)));
    ui->m_alienTable->setItem(rowcount, 1, new QTableWidgetItem(a.name));

    const QString racename = s_raceNames.contains(a.race) ? s_raceNames[a.race] : "(invalid/unknown)";
    ui->m_alienTable->setItem(rowcount, 2, new QTableWidgetItem(racename));
  }
  ui->m_alienTable->resizeColumnsToContents();
}

void MainWindow::populateShipWidgets()
{
  QMap<int,Ship> ships = m_ships.getShipList();
  ui->m_shipTable->clearContents();

  foreach (Ship s, ships.values())
  {
    const int rowcount = ui->m_shipTable->rowCount();
    ui->m_shipTable->insertRow(rowcount);
    ui->m_shipTable->setItem(rowcount, 0, new QTableWidgetItem(QString("%1").arg(s.id)));
    ui->m_shipTable->setItem(rowcount, 1, new QTableWidgetItem(s.name));
    ui->m_shipTable->setItem(rowcount, 2, new QTableWidgetItem(m_aliens.getName(s.pilot)));
    ui->m_shipTable->setItem(rowcount, 3, new QTableWidgetItem(m_places.getName(s.location)));
  }
  ui->m_shipTable->resizeColumnsToContents();
}

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
        nnvChild->setText(0, nnvFilename);
        datTreeParent->addChild(nnvChild);
      }
    }
  }

  ui->m_soundTree->expandAll();
}

void MainWindow::on_m_shipTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
  Q_UNUSED(currentColumn)
  Q_UNUSED(previousRow)
  Q_UNUSED(previousColumn)

  ui->m_shipInventoryTable->setRowCount(0);
  const int shipid = ui->m_shipTable->item(currentRow, 0)->text().toInt();
  QMap<int,int> inventory = m_inventory.getInventory(shipid);

  foreach(int obj, inventory.keys())
  {
    const int count = inventory[obj];
    const int rowcount = ui->m_shipInventoryTable->rowCount();
    ui->m_shipInventoryTable->insertRow(rowcount);
    ui->m_shipInventoryTable->setItem(rowcount, 0, new QTableWidgetItem(QString("%1").arg(obj)));
    ui->m_shipInventoryTable->setItem(rowcount, 1, new QTableWidgetItem(m_invObject.getObjectName(obj)));
    ui->m_shipInventoryTable->setItem(rowcount, 2, new QTableWidgetItem(QString("%1").arg(count)));
  }
  ui->m_shipInventoryTable->resizeColumnsToContents();
}

void MainWindow::on_m_objTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
  Q_UNUSED(currentColumn)
  Q_UNUSED(previousRow)
  Q_UNUSED(previousColumn)

  const int id = ui->m_objTable->item(currentRow, 0)->text().toInt();
  QPixmap pm = m_invObject.getObjectImage(id);
  m_objScene.addPixmap(pm);
  ui->m_objectImageView->setScene(&m_objScene);
  ui->m_objectTypeLabel->setText("Type: " + getInventoryObjTypeText(m_invObject.getObjectType(id)));
}

void MainWindow::on_m_placeTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
  Q_UNUSED(currentColumn)
  Q_UNUSED(previousRow)
  Q_UNUSED(previousColumn)

  bool status = true;
  const int id = ui->m_placeTable->item(currentRow, 0)->text().toInt();
  QPixmap pm = m_places.getPlaceSurfaceImage(id, status);
  if (status)
  {
    m_planetSurfaceScene.addPixmap(pm);
    ui->m_planetView->setScene(&m_planetSurfaceScene);
  }

  Place p;
  if (m_places.getPlace(id, p))
  {
    PlaceClass pclassData;
    if (m_pclasses.pclassData(p.pclass, pclassData))
    {
      ui->m_planetClassData->setText(pclassData.name);
      const QString tempString = QString("%1 (%2)").arg(pclassData.temperature).arg(pclassData.temperatureRange);
      ui->m_planetTemperatureData->setText(tempString);
    }
  }
}

void MainWindow::on_m_alienTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
  Q_UNUSED(currentColumn)
  Q_UNUSED(previousRow)
  Q_UNUSED(previousColumn)

  const int id = ui->m_alienTable->item(currentRow, 0)->text().toInt();

  Alien a;
  if (m_aliens.getAlien(id, a))
  {
    m_alienFrames.clear();
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

void MainWindow::on_m_alienFrameSlider_valueChanged(int value)
{
  loadAlienFrame(value);
}

void MainWindow::loadAlienFrame(int frameId)
{
  m_alienScene.clear();
  if (m_alienFrames.values().count() > frameId)
  {
    m_alienScene.addPixmap(QPixmap::fromImage(m_alienFrames.values().at(frameId)));
    ui->m_alienView->setScene(&m_alienScene);
  }
}

void MainWindow::on_m_soundTree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
  Q_UNUSED(previous)

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
  setSoundButtonStates();
}

void MainWindow::on_m_soundPrevButton_clicked()
{
  if (m_currentNNVSoundId > 0)
  {
    m_currentNNVSoundId--;
    ui->m_soundSlider->setValue(m_currentNNVSoundId);
    setSoundIDLabel(m_currentNNVFilename, m_currentNNVSoundId);
    setSoundButtonStates();
  }
}

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

void MainWindow::on_m_soundStopButton_clicked()
{
  m_audioOutput->stop();
}

void MainWindow::on_m_soundNextButton_clicked()
{
  if (m_currentNNVSoundId < (m_currentNNVSoundCount - 1))
  {
    m_currentNNVSoundId++;
    ui->m_soundSlider->setValue(m_currentNNVSoundId);
    setSoundIDLabel(m_currentNNVFilename, m_currentNNVSoundId);
    setSoundButtonStates();
  }
}

void MainWindow::setSoundButtonStates()
{
  if ((m_audioOutput->state() == QAudio::StoppedState) && (m_currentNNVSoundId >= 0))
  {
    ui->m_soundTree->setEnabled(true);
    ui->m_soundStopButton->setEnabled(false);

    if (m_currentNNVSoundId >= 0)
    {
      ui->m_soundPlayButton->setEnabled(true);
      ui->m_soundSlider->setMaximum(m_currentNNVSoundCount - 1);
      ui->m_soundSlider->setEnabled(true);
    }
    else
    {
      ui->m_soundSlider->setMaximum(0);
      ui->m_soundSlider->setEnabled(false);
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
    ui->m_soundSlider->setEnabled(false);
  }

  ui->m_soundSlider->setValue(0);
}

void MainWindow::setSoundIDLabel(QString nnvName, int soundId)
{
  if (soundId >= 0)
  {
    int dotPosition = nnvName.indexOf('.');
    const QString baseFilename = (dotPosition > 0) ? nnvName.mid(0, dotPosition) : nnvName;

    ui->m_soundIDLabel->setText(QString("Sound ID: %1/%2").arg(baseFilename).arg(soundId));
  }
  else
  {
    ui->m_soundIDLabel->setText("Sound ID: (none selected)");
  }
}
