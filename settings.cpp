#include "settings.h"
#include "ui_settings.h"

const ArduinoIO::grblSettings Settings::defaultGrblSettings = {1, 755.906,755.906,755.906,480,480,0.1,4.8,50,0b00000000,30};
const Settings::plotSettings Settings::defaultPlotSettings = {0, 24, 16, 0, QColor("#cfcfcf"), QColor("#00ff00"), QColor("#0000ff"), 0.05};
const Settings::arduinoSettings Settings::defaultArduinoSettings = {"/dev/ttyACM0", "9600", 1};
QString Settings::currentMachineName = "Machine0.dat";
QDir Settings::applicationDirectory = QDir("");
Settings::Settings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);
    ui->stepMask_lineEdit->setInputMask("BBBBBBBB");
    applicationDirectory = QDir(QCoreApplication::applicationDirPath());
    Qt::WindowFlags flags = ((this->windowFlags()) | Qt::WindowStaysOnTopHint);
    this->setWindowFlags(flags);
    writingToArduino = false;
    GetGrblSettings(&userGrblSettings);
    GetPlotSettings();
    on_grblVerson_combo_currentIndexChanged(ui->grblVerson_combo->currentIndex());
}

Settings::~Settings()
{
    delete ui;
}

void Settings::SetErrorHandler(ErrorHandler *handler)
{
    err = handler;
}

/** ********************(Events)******************** **/

void Settings::closeEvent(QCloseEvent *event)
{
    this->hide();
    event->ignore();
}

void Settings::showEvent(QShowEvent *event)
{
    GetGrblSettings(&userGrblSettings);
    GetPlotSettings();
    DisplayPlotSettings();
    DisplayGrblSettings();
    emit settingsShown();
    RefreshPortList();
    arduino->SetPortName(ui->arduinoPort_combo->currentText());
    arduino->OpenPort();
    ui->arduinoPort_combo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    event->accept();
}

void Settings::hideEvent(QHideEvent *event)
{
    emit settingsHidden();
    event->accept();
}

/** ********************(File i/o)******************** **/

inline int Settings::OpenFileStream(QDataStream* fStream)
{
    qDebug() << "openfilestream";
    bool first = 0;
    if(!QDir("MachineSettings").exists())
        QDir().mkdir("MachineSettings");
    QDir::setCurrent("MachineSettings");
//    if(!QDir(currentMachineName).exists()) {
//        first = 1;}
    currentMachineFile = new QFile(currentMachineName);
    if(!currentMachineFile->open(QIODevice::ReadWrite)) {
//        qDebug() << "file not open";
        return -1; }
    fStream->setDevice(currentMachineFile);
    QDir::setCurrent(applicationDirectory.absolutePath());
    if(first) {
////        //make a default file
//        GrblDefaults();
//        PlotDefaults();
////        SaveGrblSettings();
////        SavePlotSettings();
//        *fStream << userGrblSettings;
//        *fStream << userPlotSettings;
    }
//    qDebug() << "file opened";
    return 0;
}

inline int Settings::CloseFileStream()
{
    qDebug() << "CloseFileStream";
    currentMachineFile->close();
    return 0;
}
/** ********************(Function Overloads)******************** **/



QDataStream &operator <<(QDataStream &output, const ArduinoIO::grblSettings &settings)
{
    output << settings.grblVersion;
    output << settings.stepsX;
    output << settings.stepsY;
    output << settings.stepsZ;
    output << settings.feedRate;
    output << settings.seekRate;
    output << settings.arcSegment;
    output << settings.acceleration;
    output << settings.cornering;
    output << settings.stepPulse;
    output << settings.stepInvert;
    return output;
}

QDataStream &operator >>(QDataStream &input, ArduinoIO::grblSettings &settings)
{
    input >> settings.grblVersion;
    input >> settings.stepsX;
    input >> settings.stepsY;
    input >> settings.stepsZ;
    input >> settings.feedRate;
    input >> settings.seekRate;
    input >> settings.arcSegment;
    input >> settings.acceleration;
    input >> settings.cornering;
    input >> settings.stepPulse;
    input >> settings.stepInvert;
    return input;
}

QDataStream &operator <<(QDataStream &output, const Settings::plotSettings &settings)
{
    output << settings.machineSizeUnits;
    output << settings.sizeX;
    output << settings.sizeY;
    output << settings.gridUnits;
    output << settings.gridColor;
    output << settings.moveColor;
    output << settings.cutColor;
    output << settings.arcPrecision;
    return output;
}

QDataStream &operator >>(QDataStream &input, Settings::plotSettings &settings)
{
    input >> settings.machineSizeUnits;
    input >> settings.sizeX;
    input >> settings.sizeY;
    input >> settings.gridUnits;
    input >> settings.gridColor;
    input >> settings.moveColor;
    input >> settings.cutColor;
    input >> settings.arcPrecision;
    return input;
}

/** ******************** Grbl Setttings ******************** **/
ArduinoIO::grblSettings DeviceGrblSettings()
{

}

void Settings::GetGrblSettings(ArduinoIO::grblSettings *settings)
{
    QDataStream fileStream;
    OpenFileStream(&fileStream);
    if(fileStream.atEnd())
    {
        qDebug() << "file empty";
        GrblDefaults();
        SaveGrblSettings();
    }
    else
        fileStream >> *settings;
     CloseFileStream();
}

void Settings::PutGrblSettings()
{
    QDataStream fileStream;
    OpenFileStream(&fileStream);
    fileStream << userGrblSettings;
    CloseFileStream();
}

void Settings::DisplayGrblSettings()
{
    ui->grblVerson_combo->setCurrentIndex(userGrblSettings.grblVersion);
    ui->stepX_dspinbox->setValue(userGrblSettings.stepsX);
    ui->stepY_dspinbox->setValue(userGrblSettings.stepsY);
    ui->stepZ_dspinbox->setValue(userGrblSettings.stepsZ);
    ui->feedRate_dspinbox->setValue(userGrblSettings.feedRate);
    ui->seekRate_dspinbox->setValue(userGrblSettings.seekRate);
    ui->arcSeg_dspinbox->setValue(userGrblSettings.arcSegment);
    ui->acceleration_dspinbox->setValue(userGrblSettings.acceleration);
    ui->maxJerk_dspinbox->setValue(userGrblSettings.cornering);
    ui->stepPulse_dspinbox->setValue(userGrblSettings.stepPulse);
    ui->stepMask_lineEdit->setText(QString("").setNum(userGrblSettings.stepInvert, 2));
}

void Settings::SaveGrblSettings()
{
    userGrblSettings.grblVersion = ui->grblVerson_combo->currentIndex();
    userGrblSettings.stepsX = ui->stepX_dspinbox->value();
    userGrblSettings.stepsY = ui->stepY_dspinbox->value();
    userGrblSettings.stepsZ = ui->stepZ_dspinbox->value();
    userGrblSettings.feedRate = ui->feedRate_dspinbox->value();
    userGrblSettings.seekRate = ui->seekRate_dspinbox->value();
    userGrblSettings.arcSegment = ui->arcSeg_dspinbox->value();
    userGrblSettings.acceleration = ui->acceleration_dspinbox->value();
    userGrblSettings.cornering = ui->maxJerk_dspinbox->value();
    userGrblSettings.stepPulse = ui->stepPulse_dspinbox->value();
    userGrblSettings.stepInvert = (ui->stepMask_lineEdit->text()).toInt(0,2);
    PutGrblSettings();
}

void Settings::AdjustForGrblVersion()
{

}

void Settings::GrblDefaults()
{
    userGrblSettings = defaultGrblSettings;
    DisplayGrblSettings();
}

void Settings::on_grblSave_pButton_clicked()
{
    SaveGrblSettings();
//    PutGrblSettings();

}

void Settings::on_grblDefaults_pButton_clicked()
{
    GrblDefaults();
    DisplayGrblSettings();
}

/** ******************** General Settings ******************** **/

Settings::plotSettings Settings::PlotSettings()
{
    return userPlotSettings;
}

void Settings::GetPlotSettings()
{
    QDataStream fileStream;
    OpenFileStream(&fileStream);
    ArduinoIO::grblSettings dummy;
    fileStream >> dummy;        //advance to the end of grblSettings
    if(fileStream.atEnd())
    {
        qDebug() << "file still empty";
        PlotDefaults();
        SavePlotSettings();
    }
    else
        fileStream >> userPlotSettings;
    CloseFileStream();
}

void Settings::PutPlotSettings()
{
    QDataStream fileStream;
    OpenFileStream(&fileStream);
    ArduinoIO::grblSettings dummy;
    fileStream >> dummy;        //advance to the end of grblSettings
    fileStream << userPlotSettings;
    CloseFileStream();
}

void Settings::DisplayPlotSettings()
{
    ui->machineUnits_combo->setCurrentIndex(userPlotSettings.machineSizeUnits);
    ui->sizeX_dBox->setValue(userPlotSettings.sizeX);
    ui->sizeY_dBox->setValue(userPlotSettings.sizeY);
    ui->gridUnits_combo->setCurrentIndex(userPlotSettings.gridUnits);
    ui->gridColor_lEdit->setText(userPlotSettings.gridColor.name());
    ui->moveColor_lEdit->setText(userPlotSettings.moveColor.name());
    ui->cutColor_lEdit->setText(userPlotSettings.cutColor.name());
    ui->arcPrecision_dSpin->setValue(userPlotSettings.arcPrecision);
    SetColorSample(ui->gridColor_tButton, userPlotSettings.gridColor, 0);
    SetColorSample(ui->moveColor_tButton, userPlotSettings.moveColor, 0);
    SetColorSample(ui->cutColor_tButton, userPlotSettings.cutColor, 0);
}

void Settings::SavePlotSettings()
{
    userPlotSettings.machineSizeUnits = ui->machineUnits_combo->currentIndex();
    userPlotSettings.sizeX = ui->sizeX_dBox->value();
    userPlotSettings.sizeY = ui->sizeY_dBox->value();
    userPlotSettings.gridUnits = ui->gridUnits_combo->currentIndex();
    userPlotSettings.gridColor = QColor(ui->gridColor_lEdit->text());
    userPlotSettings.moveColor = QColor(ui->moveColor_lEdit->text());
    userPlotSettings.cutColor = QColor(ui->cutColor_lEdit->text());
    userPlotSettings.arcPrecision = ui->arcPrecision_dSpin->value();
    PutPlotSettings();
    emit plotSettingsChanged();
}

void Settings::PlotDefaults()
{
    userPlotSettings = defaultPlotSettings;
    DisplayPlotSettings();
}

void Settings::on_plotSave_pButton_clicked()
{
    SavePlotSettings();
    userArduinoSettings.portName = ui->arduinoPort_combo->currentText();
    userArduinoSettings.baudRate = ui->baudRate_combo->currentText();
}

QColor Settings::SetColorSample(QToolButton *button, QColor currentColor, bool dialog)
{
    QColor color;
    if(dialog)
        color = QColorDialog::getColor(currentColor, this, "Select Color", QColorDialog::DontUseNativeDialog);
    else
        color = currentColor;
    if(color.isValid())
    {
        const QString COLOR_STYLE("QToolButton { background-color : %1; border-style: outset; border-width: 1px; border-radius: 7px; border-color: grey; }");
        //    QColor IdealTextColor = getIdealTextColor(ChosenColor);
        button->setStyleSheet(COLOR_STYLE.arg(color.name())/*.arg(IdealTextColor.name())*/);
    }
    return color;
}

void Settings::on_gridColor_tButton_clicked()
{
    QColor color = SetColorSample(ui->gridColor_tButton, userPlotSettings.gridColor);
    if(color.isValid())
        ui->gridColor_lEdit->setText(color.name());
}

void Settings::on_cutColor_tButton_clicked()
{
    QColor color = SetColorSample(ui->cutColor_tButton, userPlotSettings.cutColor);
    if(color.isValid())
        ui->cutColor_lEdit->setText(color.name());}

void Settings::on_moveColor_tButton_clicked()
{
    QColor color = SetColorSample(ui->moveColor_tButton, userPlotSettings.moveColor);
    if(color.isValid())
        ui->moveColor_lEdit->setText(color.name());}

/** ********************(Serial Communication)******************** **/

void Settings::SetArduino(ArduinoIO *a)
{
    arduino = a;
}

void Settings::RefreshPortList()
{
    ui->arduinoPort_combo->clear();
    QList<QextPortInfo> ports = arduino->GetPorts();
    for(int i = 0; i < ports.size(); i++)
    {
        ui->arduinoPort_combo->addItem(ports.at(i).physName);
    }
}

void Settings::PutDeviceGrblSettings()      //maybe write this functionality into the arduino class
{
    arduino->Flush();
    writingToArduino = true;
    QObject::connect(arduino, SIGNAL(newData()), this, SLOT(PutDeviceGrblSettings2()));
    arduino << QString("$0 = ").append(QString("").setNum(userGrblSettings.stepsX)).append(QString("\n"));
//    qDebug() << "PUTBEGIN";
}

void Settings::PutDeviceGrblSettings2()
{
    qDebug() << "PutDeviceGrblSettings2";
    if(arduino->getLine() != "ok")
        return;
    static int count = 1;
    switch(count)
    {
    case 1:
        arduino << QString("$1 = ").append(QString("").setNum(userGrblSettings.stepsY)).append(QString("\n"));
        count ++;
        break;
    case 2:
        arduino << QString("$2 = ").append(QString("").setNum(userGrblSettings.stepsZ)).append(QString("\n"));
        count ++;
        break;
    case 3:
        arduino << QString("$3 = ").append(QString("").setNum(userGrblSettings.stepPulse)).append(QString("\n"));
        count ++;
        break;
    case 4:
        arduino << QString("$4 = ").append(QString("").setNum(userGrblSettings.feedRate)).append(QString("\n"));
        count ++;
        break;
    case 5:
        arduino << QString("$5 = ").append(QString("").setNum(userGrblSettings.seekRate)).append(QString("\n"));
        count ++;
        break;
    case 6:
        arduino << QString("$6 = ").append(QString("").setNum(userGrblSettings.arcSegment)).append(QString("\n"));
        count ++;
        break;
    case 7:
        arduino << QString("$7 = ").append(QString("").setNum(userGrblSettings.stepInvert)).append(QString("\n"));
        count ++;
        if(arduino->currentGrblSettings.grblVersion == 0) {
            disconnect(arduino, SIGNAL(newData()), this, SLOT(PutDeviceGrblSettings2()));
        }
        break;
    case 8:
        arduino << QString("$8 = ").append(QString("").setNum(userGrblSettings.acceleration)).append(QString("\n"));
        count ++;
        break;
    case 9:
        arduino << QString("$9 = ").append(QString("").setNum(userGrblSettings.cornering)).append(QString("\n"));
        disconnect(arduino, SIGNAL(newData()), this, SLOT(PutDeviceGrblSettings2()));
        count = 1;
        writingToArduino = false;
        break;
    }
//    qDebug() << count;
}


void Settings::FindMachine()
{
    qDebug() << "FINDMACHINESTART";
    arduino->GetPorts();
    if(arduino->ports.isEmpty())
        return;
    arduino->SetPortName(arduino->ports.at(0).physName);
    arduino->OpenPort();
    connect(arduino, SIGNAL(deviceStateChanged(int)), this, SLOT(CompareGrblSettings(int)));
    connect(this, SIGNAL(CompareGrblSettingsResult(bool)), this, SLOT(FindMachine2(bool)));
    qDebug() << "FINDMACHINEEND";
}

void Settings::CompareGrblSettings(int state)
{
    qDebug() << "COMPAREGRBLSETTINGS";

    bool result;

    if(state != ArduinoIO::READY)
        return;
    disconnect(arduino, SIGNAL(deviceStateChanged(int)), this, SLOT(CompareGrblSettings(int)));
    qDebug() << "local ==" << "device" << endl
             << userGrblSettings.grblVersion << "=="<< arduino->currentGrblSettings.grblVersion << endl
             << userGrblSettings.stepsX << "==" << arduino->currentGrblSettings.stepsX << endl
             << userGrblSettings.stepsY << "==" << arduino->currentGrblSettings.stepsY << endl
             << userGrblSettings.stepsZ  << "==" << arduino->currentGrblSettings.stepsZ << endl
             << userGrblSettings.stepPulse << "==" << arduino->currentGrblSettings.stepPulse << endl
             << userGrblSettings.feedRate << "==" << arduino->currentGrblSettings.feedRate << endl
             << userGrblSettings.seekRate << "==" << arduino->currentGrblSettings.seekRate << endl
             << userGrblSettings.arcSegment << "==" << arduino->currentGrblSettings.arcSegment << endl
             << userGrblSettings.stepInvert  << "==" << arduino->currentGrblSettings.stepInvert << endl
             << userGrblSettings.acceleration  << "==" << arduino->currentGrblSettings.acceleration << endl
             << userGrblSettings.cornering << "==" << arduino->currentGrblSettings.cornering << endl;

    if(!(userGrblSettings == arduino->currentGrblSettings))
        result = 0;
    else
        result = 1;
    emit CompareGrblSettingsResult(result);
}

void Settings::FindMachine2(bool result)
{
    qDebug() << "FINDMACHINE2";
    static int index = 0;       //use new instead of static
    if(result) {
        disconnect(this, SIGNAL(CompareGrblSettingsResult(bool)), this, SLOT(FindMachine2(bool)));
        qDebug() << "the machine is connected on " << arduino->portName();
        return;
    }
    else if(index != (arduino->ports.size() - 1)) {
        index ++;
        qDebug() << "the machine connected on " << arduino->portName() << "does not match the current settings";
        arduino->ClosePort();
        arduino->SetPortName(arduino->ports.at(index).physName);
        arduino->OpenPort();
        connect(arduino, SIGNAL(deviceStateChanged(int)), this, SLOT(CompareGrblSettings(int)));
    }
    else {
        qDebug() << "the machine connected on " << arduino->portName() << "does not match the current settings";
        disconnect(this, SIGNAL(CompareGrblSettingsResult(bool)), this, SLOT(FindMachine2(bool)));
        qDebug() << "could not find a connected machine that matches the current settings";
        index = 0;
    }
}

void Settings::on_refreshArduinoPortList_tButton_clicked()
{
    RefreshPortList();
}

void Settings::on_grblVerson_combo_currentIndexChanged(int index)   //disable settings that are not applicable to the current grbl version
{
    if(index == 0) {
        ui->acceleration_dspinbox->setEnabled(false);
        ui->acceleration_label->setEnabled(false);
        ui->maxJerk_dspinbox->setEnabled(false);
        ui->maxJerk_label->setEnabled(false);
        ui->maxJerk_label->setText("Max Jerk (delta mm/min)");
    }
    else if(index == 1) {
        ui->acceleration_dspinbox->setEnabled(true);
        ui->acceleration_label->setEnabled(true);
        ui->maxJerk_dspinbox->setEnabled(true);
        ui->maxJerk_label->setEnabled(true);
        ui->maxJerk_label->setText("Max Jerk (delta mm/min)");
    }
    else if(index == 2) {
        ui->acceleration_dspinbox->setEnabled(true);
        ui->acceleration_label->setEnabled(true);
        ui->maxJerk_dspinbox->setEnabled(true);
        ui->maxJerk_label->setEnabled(true);
        ui->maxJerk_label->setText("Cornering Junction Deviance (mm)");
    }
}

void Settings::on_plotDefaults_pButton_clicked()
{
    PlotDefaults();
}

void Settings::on_upload_pButton_clicked()
{
    PutDeviceGrblSettings();
}
