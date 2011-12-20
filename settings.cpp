#include "settings.h"
#include "ui_settings.h"

QString Settings::currentMachineName = "Machine0.txt";
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
    GetAllSettings();
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
    GetAllSettings();
    DisplayGeneralSettings();
    DisplayGrblSettings();
    DisplayPreprocessSettings();
    emit settingsShown();
    RefreshPortList();
//    arduino->SetPortName(ui->arduinoPort_combo->currentText());
//    arduino->OpenPort();
    ui->arduinoPort_combo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    event->accept();
}

void Settings::hideEvent(QHideEvent *event)
{
    emit settingsHidden();
    event->accept();
}

/** ********************(File i/o)******************** **/

inline int Settings::OpenFile()
{
    qDebug() << "Open File";
    bool first = 0;
    if(!QDir("MachineSettings").exists())
        QDir().mkdir("MachineSettings");
    QDir::setCurrent("MachineSettings");
//    if(!QDir(currentMachineName).exists()) {
//        first = 1;}
    machineFile = new QFile(currentMachineName);
    if(!machineFile->open(QIODevice::ReadWrite | QIODevice::Text)) {
//        qDebug() << "file not open";
        return -1; }
    QDir::setCurrent(applicationDirectory.absolutePath());
    if(first) {

    }
//    qDebug() << "file opened";
    return 0;
}

inline int Settings::CloseFile()
{
    qDebug() << "Close File";
    machineFile->close();
    return 0;
}

/** ******************** Grbl Setttings ******************** **/

void Settings::GetGrblSettings()
{
    OpenFile();
    settings.grbl.version = GetSetting<int>(grblVersion);
    settings.grbl.stepsX = GetSetting<float>(grblStepsX);
    settings.grbl.stepsY = GetSetting<float>(grblStepsY);
    settings.grbl.stepsZ = GetSetting<float>(grblStepsZ);
    settings.grbl.feedRate = GetSetting<float>(grblFeedRate);
    settings.grbl.seekRate = GetSetting<float>(grblSeekRate);
    settings.grbl.arcSegment = GetSetting<float>(grblArcSegment);
    settings.grbl.acceleration = GetSetting<float>(grblAcceleration);
    settings.grbl.cornering = GetSetting<float>(grblCornering);
    settings.grbl.stepInvert = GetSetting<uint8_t>(grblStepInvert);
    settings.grbl.stepPulse = GetSetting<uint8_t>(grblStepPulse);
    CloseFile();
}

void Settings::PutGrblSettings()
{
    OpenFile();
    PutSetting("Grbl Version", grblVersion, settings.grbl.version);
    PutSetting("Steps per mm X", grblStepsX, settings.grbl.stepsX);
    PutSetting("Steps per mm Y", grblStepsY, settings.grbl.stepsY);
    PutSetting("Steps per mm Z", grblStepsZ, settings.grbl.stepsZ);
    PutSetting("Default Feed Rate", grblFeedRate, settings.grbl.feedRate);
    PutSetting("Default Seek Rate", grblSeekRate, settings.grbl.seekRate);
    PutSetting("MM per Arc Segment", grblArcSegment, settings.grbl.arcSegment);
    PutSetting("Max Acceleration (mm/s^2)", grblAcceleration, settings.grbl.acceleration);
    PutSetting("Cornering Tolerance?", grblCornering, settings.grbl.cornering);
    PutSetting("Step Pulse Width (us)", grblStepPulse, settings.grbl.stepPulse);
    PutSetting("Step Invert Mask", grblStepInvert, settings.grbl.stepInvert);
    CloseFile();
}

void Settings::DisplayGrblSettings()
{
    ui->grblVerson_combo->setCurrentIndex(settings.grbl.version);
    ui->stepX_dspinbox->setValue(settings.grbl.stepsX);
    ui->stepY_dspinbox->setValue(settings.grbl.stepsY);
    ui->stepZ_dspinbox->setValue(settings.grbl.stepsZ);
    ui->feedRate_dspinbox->setValue(settings.grbl.feedRate);
    ui->seekRate_dspinbox->setValue(settings.grbl.seekRate);
    ui->arcSeg_dspinbox->setValue(settings.grbl.arcSegment);
    ui->acceleration_dspinbox->setValue(settings.grbl.acceleration);
    ui->maxJerk_dspinbox->setValue(settings.grbl.cornering);
    ui->stepPulse_dspinbox->setValue(settings.grbl.stepPulse);
    ui->stepMask_lineEdit->setText(QString("").setNum(settings.grbl.stepInvert, 2));
}

void Settings::SaveGrblSettings()
{
    settings.grbl.version = ui->grblVerson_combo->currentIndex();
    settings.grbl.stepsX = ui->stepX_dspinbox->value();
    settings.grbl.stepsY = ui->stepY_dspinbox->value();
    settings.grbl.stepsZ = ui->stepZ_dspinbox->value();
    settings.grbl.feedRate = ui->feedRate_dspinbox->value();
    settings.grbl.seekRate = ui->seekRate_dspinbox->value();
    settings.grbl.arcSegment = ui->arcSeg_dspinbox->value();
    settings.grbl.acceleration = ui->acceleration_dspinbox->value();
    settings.grbl.cornering = ui->maxJerk_dspinbox->value();
    settings.grbl.stepPulse = ui->stepPulse_dspinbox->value();
    settings.grbl.stepInvert = (ui->stepMask_lineEdit->text()).toInt(0,2);
    PutGrblSettings();
}

void Settings::AdjustForGrblVersion()
{

}

void Settings::on_grblSave_pButton_clicked()
{
    SaveGrblSettings();
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

/** ******************** General Settings ******************** **/

void Settings::GetGeneralSettings()
{
    OpenFile();
    settings.machine.units = GetSetting<bool>(machSizeUnits);
    settings.machine.sizeX = GetSetting<float>(machSizeX);
    settings.machine.sizeY = GetSetting<float>(machSizeY);
    settings.plot.gridUnits = GetSetting<bool>(plotGridUnits);
    settings.plot.gridColor = GetSettingStr(plotGridColor);
    settings.plot.moveColor = GetSettingStr(plotMoveColor);
    settings.plot.cutColor = GetSettingStr(plotCutColor);
    settings.plot.arcPrecision = GetSetting<float>(plotArcPrecision);
    settings.arduino.portName = GetSettingStr(ardPortName);
    settings.arduino.baudRate = GetSetting<int>(ardBaudRate);
    CloseFile();
}

void Settings::PutGeneralSettings()
{
    OpenFile();
    PutSetting("Machine Size Units", machSizeUnits, settings.machine.units);
    PutSetting("Machine X Size", machSizeX, settings.machine.sizeX);
    PutSetting("Machine Y Size", machSizeY, settings.machine.sizeY);
    PutSetting("Grid Units", plotGridUnits, settings.plot.gridUnits);
    PutSettingStr("Grid Color", plotGridColor, settings.plot.gridColor);
    PutSettingStr("Seek Color", plotMoveColor, settings.plot.moveColor);
    PutSettingStr("Cut Color", plotCutColor, settings.plot.cutColor);
    PutSetting("Arc Precision", plotArcPrecision, settings.plot.arcPrecision);
    PutSettingStr("Default Port Name", ardPortName, settings.arduino.portName);
    PutSetting("Default Baud Rate", ardBaudRate, settings.arduino.baudRate);
    CloseFile();
}

void Settings::DisplayGeneralSettings()
{
    ui->machineUnits_combo->setCurrentIndex(settings.machine.units);
    ui->sizeX_dBox->setValue(settings.machine.sizeX);
    ui->sizeY_dBox->setValue(settings.machine.sizeY);
    ui->gridUnits_combo->setCurrentIndex(settings.plot.gridUnits);
    ui->gridColor_lEdit->setText(settings.plot.gridColor);
    ui->moveColor_lEdit->setText(settings.plot.moveColor);
    ui->cutColor_lEdit->setText(settings.plot.cutColor);
    ui->arcPrecision_dSpin->setValue(settings.plot.arcPrecision);
    SetColorSample(ui->gridColor_tButton, settings.plot.gridColor, 0);
    SetColorSample(ui->moveColor_tButton, settings.plot.moveColor, 0);
    SetColorSample(ui->cutColor_tButton, settings.plot.cutColor, 0);
}

void Settings::SaveGeneralSettings()
{
    settings.machine.units = ui->machineUnits_combo->currentIndex();
    settings.machine.sizeX = ui->sizeX_dBox->value();
    settings.machine.sizeY = ui->sizeY_dBox->value();
    settings.plot.gridUnits = ui->gridUnits_combo->currentIndex();
    settings.plot.gridColor = ui->gridColor_lEdit->text();
    settings.plot.moveColor = ui->moveColor_lEdit->text();
    settings.plot.cutColor = ui->cutColor_lEdit->text();
    settings.plot.arcPrecision = ui->arcPrecision_dSpin->value();
    PutGeneralSettings();
    emit plotSettingsChanged();
}

void Settings::on_plotSave_pButton_clicked()
{
    SaveGeneralSettings();
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
	button->setStyleSheet(COLOR_STYLE.arg(color.name()));
    }
    return color;
}

void Settings::on_gridColor_tButton_clicked()
{
    QColor color = SetColorSample(ui->gridColor_tButton, settings.plot.gridColor);
    if(color.isValid())
        ui->gridColor_lEdit->setText(color.name());
}

void Settings::on_cutColor_tButton_clicked()
{
    QColor color = SetColorSample(ui->cutColor_tButton, settings.plot.cutColor);
    if(color.isValid())
        ui->cutColor_lEdit->setText(color.name());}

void Settings::on_moveColor_tButton_clicked()
{
    QColor color = SetColorSample(ui->moveColor_tButton, settings.plot.moveColor);
    if(color.isValid())
        ui->moveColor_lEdit->setText(color.name());}

/** ********************(Preprocess Settings)******************** **/

void Settings::GetPreprocessSettings()
{
    OpenFile();
    settings.preprocess.enabled = GetSetting<bool>(preEnabled);
    settings.preprocess.optimize = GetSetting<bool>(preOptimize);
    settings.preprocess.checkCompatibility = GetSetting<bool>(preCheckCompatibility);
    settings.preprocess.alwaysDefaultFeed = GetSetting<bool>(preAlwaysDefaultFeed);
    settings.preprocess.removeUnnecessaryCommands = GetSetting<bool>(preRemoveUnnecessary);
    settings.preprocess.removeUnsupportedCommands = GetSetting<bool>(preRemoveUnsupported);
    settings.preprocess.removeArcs = GetSetting<bool>(preRemoveArcs);
    settings.preprocess.arcSegments = GetSetting<int>(preArcSegments);
    settings.preprocess.uniformZ = GetSetting<bool>(preUniformZ);
    settings.preprocess.zMagnitude = GetSetting<float>(preZMagnitude);
    settings.preprocess.saveFile = GetSetting<bool>(preSaveFile);
    settings.preprocess.fileAppend = GetSettingStr(preFileAppend);
    CloseFile();
}

void Settings::PutPreprocessSettings()
{
    OpenFile();
    PutSetting("Preprocess Enabled", preEnabled, settings.preprocess.enabled);
    PutSetting("Optimize Enabled", preOptimize, settings.preprocess.optimize);
    PutSetting("Check Compatibility", preCheckCompatibility, settings.preprocess.checkCompatibility);
    PutSetting("Always Use Default Feed Rate", preAlwaysDefaultFeed, settings.preprocess.alwaysDefaultFeed);
    PutSetting("Remove Line Numbers", preRemoveLineNum, settings.preprocess.removeLineNumbers);
    PutSetting("Remove Unnecessary Commands", preRemoveUnnecessary, settings.preprocess.removeUnnecessaryCommands);
    PutSetting("Remove Unsupported Commands", preRemoveUnsupported, settings.preprocess.removeUnsupportedCommands);
    PutSetting("Replace Arcs With Linear Interpolation", preRemoveArcs, settings.preprocess.removeArcs);
    PutSetting("Arc Segments Per Degree?", preArcSegments, settings.preprocess.arcSegments);
    PutSetting("Make Z Changes Uniform", preUniformZ, settings.preprocess.uniformZ);
    PutSetting("Magnitude of Uniform Z Changes", preZMagnitude, settings.preprocess.zMagnitude);
    PutSetting("Save Modified File", preSaveFile, settings.preprocess.saveFile);
    PutSettingStr("Append to Modified File",preFileAppend, settings.preprocess.fileAppend);
    CloseFile();
}

void Settings::DisplayPreprocessSettings()
{
    GetPreprocessSettings();
    ui->preEnabled_cBox->setChecked(settings.preprocess.enabled);
    ui->preOptimize_cBox->setChecked(settings.preprocess.optimize);
    ui->preCheckCompatibility_cBox->setChecked(settings.preprocess.checkCompatibility);
    ui->preAlwaysDefaultFeed_cBox->setChecked(settings.preprocess.alwaysDefaultFeed);
    ui->preRemoveLineNum_cBox->setChecked(settings.preprocess.removeLineNumbers);
    ui->preRemoveUnnecessary_cBox->setChecked(settings.preprocess.removeUnnecessaryCommands);
    ui->preRemoveUnsupported_cBox->setChecked(settings.preprocess.removeUnsupportedCommands);
    ui->preRemoveArcs_cBox->setChecked(settings.preprocess.removeArcs);
    ui->preArcPrecision_dspin->setValue(settings.preprocess.arcSegments);
    ui->preUniformZ_cBox->setChecked(settings.preprocess.uniformZ);
    ui->preZMagnitude_dspin->setValue(settings.preprocess.zMagnitude);
    ui->preSaveFile_cBox->setChecked(settings.preprocess.saveFile);
    ui->preFileAppend_lEdit->setText(settings.preprocess.fileAppend);
}
void Settings::SavePreprocessSettings()
{
    settings.preprocess.enabled = ui->preEnabled_cBox->isChecked();
    settings.preprocess.optimize = ui->preOptimize_cBox->isChecked();
    settings.preprocess.checkCompatibility = ui->preCheckCompatibility_cBox->isChecked();
    settings.preprocess.alwaysDefaultFeed = ui->preAlwaysDefaultFeed_cBox->isChecked();
    settings.preprocess.removeLineNumbers = ui->preRemoveLineNum_cBox->isChecked();
    settings.preprocess.removeUnnecessaryCommands = ui->preRemoveUnnecessary_cBox->isChecked();
    settings.preprocess.removeUnsupportedCommands = ui->preRemoveUnsupported_cBox->isChecked();
    settings.preprocess.removeArcs = ui->preRemoveArcs_cBox->isChecked();
    settings.preprocess.arcSegments = ui->preArcPrecision_dspin->value();
    settings.preprocess.uniformZ = ui->preUniformZ_cBox->isChecked();
    settings.preprocess.zMagnitude = ui->preZMagnitude_dspin->value();
    settings.preprocess.saveFile = ui->preSaveFile_cBox->isChecked();
    settings.preprocess.fileAppend = ui->preFileAppend_lEdit->text();
    PutPreprocessSettings();
}

void Settings::on_preProcSave_pButton_clicked()
{
    SavePreprocessSettings();
}

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
    arduino->ChangeSetting(0, Get(grblStepsX));
//    qDebug() << "PUTBEGIN";
}

void Settings::PutDeviceGrblSettings2()
{
    qDebug() << "PutDeviceGrblSettings2";
    if(arduino->getLine() != "ok")
        return;
    static int count = 1;
    arduino->ChangeSetting(count, Get(count + grblStepsX));
    if(count == 9 || (count == 7 && arduino->currentGrblSettings.version == 0)) {
	disconnect(arduino, SIGNAL(newData()), this, SLOT(PutDeviceGrblSettings2()));
	count = 1;
	writingToArduino = false;
    }
    count ++;
}


void Settings::FindMachine1()
{
    qDebug() << "FINDMACHINE1START";
    arduino->GetPorts();
    if(arduino->ports.isEmpty())
        return;
    else if(arduino->ports.size() == 1) {
        //maybe do something here
    }
    arduino->SetPortName(arduino->ports.at(0).physName);
    arduino->OpenPort();
    connect(arduino, SIGNAL(deviceStateChanged(int)), this, SLOT(CompareGrblSettings(int)));
    connect(this, SIGNAL(CompareGrblSettingsResult(bool)), this, SLOT(FindMachine2(bool)));
    //arduino << QString("$\n");       //this might help when the arduino does not reset upon starting the serial connection
    qDebug() << "FINDMACHINE1END";
}

void Settings::CompareGrblSettings(int state)
{
    qDebug() << "COMPAREGRBLSETTINGS";

    bool result;

    if(state != ArduinoIO::READY)
        return;
    disconnect(arduino, SIGNAL(deviceStateChanged(int)), this, SLOT(CompareGrblSettings(int)));
    qDebug() << "local ==" << "device" << endl
	     << settings.grbl.version << "=="<< arduino->currentGrblSettings.version << endl
	     << settings.grbl.stepsX << "==" << arduino->currentGrblSettings.stepsX << endl
	     << settings.grbl.stepsY << "==" << arduino->currentGrblSettings.stepsY << endl
	     << settings.grbl.stepsZ  << "==" << arduino->currentGrblSettings.stepsZ << endl
	     << settings.grbl.stepPulse << "==" << arduino->currentGrblSettings.stepPulse << endl
	     << settings.grbl.feedRate << "==" << arduino->currentGrblSettings.feedRate << endl
	     << settings.grbl.seekRate << "==" << arduino->currentGrblSettings.seekRate << endl
	     << settings.grbl.arcSegment << "==" << arduino->currentGrblSettings.arcSegment << endl
	     << settings.grbl.stepInvert  << "==" << arduino->currentGrblSettings.stepInvert << endl
	     << settings.grbl.acceleration  << "==" << arduino->currentGrblSettings.acceleration << endl
	     << settings.grbl.cornering << "==" << arduino->currentGrblSettings.cornering << endl;

    if(!(settings.grbl == arduino->currentGrblSettings))
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

void Settings::on_upload_pButton_clicked()
{
    PutDeviceGrblSettings();
}

QString Settings::GetStr(int sett)
{
    switch(sett)
    {
    case ardPortName:
	return settings.arduino.portName;
	break;
    case preFileAppend:
	return settings.preprocess.fileAppend;
	break;
    case plotGridColor:
	return settings.plot.gridColor;
	break;
    case plotMoveColor:
	return settings.plot.moveColor;
	break;
    case plotCutColor:
	return settings.plot.cutColor;
	break;
    }
}
float Settings::Get(int sett)
{
    switch(sett)
    {
    case machSizeUnits:
	return settings.machine.units;
	break;
    case machSizeX:
	return settings.machine.sizeX;
	break;
    case machSizeY:
	return settings.machine.sizeY;
	break;
    case plotGridUnits:
	return settings.plot.gridUnits;
	break;
    case plotArcPrecision:
	return settings.plot.arcPrecision;
	break;
    case ardBaudRate:
	return settings.arduino.baudRate;
	break;
    case preEnabled:
	return settings.preprocess.enabled;
	break;
    case preOptimize:
	return settings.preprocess.optimize;
	break;
    case preCheckCompatibility:
	return settings.preprocess.checkCompatibility;
	break;
    case preAlwaysDefaultFeed:
	return settings.preprocess.alwaysDefaultFeed;
	break;
    case preRemoveUnnecessary:
	return settings.preprocess.removeUnnecessaryCommands;
	break;
    case preRemoveUnsupported:
	return settings.preprocess.removeUnsupportedCommands;
	break;
    case preRemoveArcs:
	return settings.preprocess.removeArcs;
	break;
    case preArcSegments:
	return settings.preprocess.arcSegments;
	break;
    case preUniformZ:
	return settings.preprocess.uniformZ;
	break;
    case preZMagnitude:
	return settings.preprocess.zMagnitude;
	break;
    case preSaveFile:
	return settings.preprocess.saveFile;
	break;
    case grblVersion:
	return settings.grbl.version;
	break;
    case grblStepsX:
	return settings.grbl.stepsX;
	break;
    case grblStepsY:
	return settings.grbl.stepsY;
	break;
    case grblStepsZ:
	return settings.grbl.stepsZ;
	break;
    case grblStepPulse:
	return settings.grbl.stepPulse;
	break;
    case grblFeedRate:
	return settings.grbl.feedRate;
	break;
    case grblSeekRate:
	return settings.grbl.seekRate;
	break;
    case grblArcSegment:
	return settings.grbl.arcSegment;
	break;
    case grblStepInvert:
	return settings.grbl.stepInvert;
	break;
    case grblAcceleration:
	return settings.grbl.acceleration;
	break;
    case grblCornering:
	return settings.grbl.cornering;
	break;
    default:
	return 0;
    }
}

int Settings::GetAllSettings()
{
    GetGeneralSettings();
    GetGrblSettings();
    GetPreprocessSettings();
}

QString Settings::GetSettingStr(int index)
{
    QString buffer;
    int pos = 0;
    machineFile->seek(index*50);
    buffer = machineFile->readLine(50);
    while(buffer[pos] != '=')
    {
	pos++;
	if(pos > 50)
	    return QString("Error");
    }
    pos++;
    while(buffer[pos] == ' ')
	pos++;
    buffer.remove(0, (pos-1));
    return buffer.trimmed();
}

template <typename T>
T Settings::GetSetting(int index)
{
    QString buffer;
    T value;
    int pos = 0;
    machineFile->seek(index*50);
    buffer = machineFile->readLine(50);
    while(buffer[pos] != '=')
    {
	pos++;
	if(pos > 50)
	    return T(1);
    }
    pos++;
    while(buffer[pos] == ' ')
	pos++;
    pos--;//because of a quirk in getnumstring -- will fix
    buffer = GetNumString(buffer, &pos);
    value = buffer.toFloat();
    return value;
}

void Settings::PutSettingStr(QString description, int index, QString setting)
{
    char s[] = {' ', '\0'};
    description += " = ";
    machineFile->seek(index*50);
    machineFile->write(QByteArray(description.toAscii()) + setting.toAscii());
    while(machineFile->pos() < index*50 + 49)
	machineFile->write(s);
    machineFile->write(QByteArray("\n"));
}

template<typename S>
void Settings::PutSetting(QString description, int index, S setting, bool convertToString)
{
    char s[] = {' ', '\0'};
    QString settingString = "";
    if(convertToString)
	settingString = QString::number(setting);
    else
	settingString = setting;
    description += " = ";
    machineFile->seek(index*50);
    machineFile->write(QByteArray(description.toAscii()) + settingString.toAscii());
    while(machineFile->pos() < index*50 + 49)
	machineFile->write(s);
    machineFile->write(QByteArray("\n"));
}


int Settings::SaveAllSettings()
{
    SaveGeneralSettings();
    SaveGrblSettings();
}


