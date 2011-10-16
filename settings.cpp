#include "settings.h"
#include "ui_settings.h"

const Settings::grblSettings Settings::defaultGrblSettings = {1, 755.906,755.906,755.906,480,480,0.1,4.8,50,0b00000000,30};
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
    GetPlotSettings(&userPlotSettings);
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
    GetPlotSettings(&userPlotSettings);
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
//    if(!QDir("MachineSettings").exists())
//        QDir().mkdir("MachineSettings");
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

bool Settings::grblSettings::operator== (Settings::grblSettings other)
{
//    if( grblVersion != other.grblVersion) qDebug() << 1;
//    qDebug() << stepsX << " = " << other.stepsX << endl;
//    if(    stepsX != other.stepsX) qDebug() << 2;
//       if( stepsY != other.stepsY ) qDebug() << 3;
//       if( stepsZ != other.stepsZ ) qDebug() << 4;
//         if(feedRate != other.feedRate ) qDebug() << 5;
//         if(seekRate != other.seekRate) qDebug() << 6;
//       if( arcSegment != other.arcSegment) qDebug() << 7;
//        if(acceleration != other.acceleration ) qDebug() << 8;
//        if(cornering != other.cornering ) qDebug() << 9;
//        if(stepPulse != other.stepPulse ) qDebug() << 10;
//        if (stepInvert != other.stepInvert ) qDebug() << 11;

    if( grblVersion != other.grblVersion ||
        !CompareFloats(stepsX, other.stepsX) ||
        !CompareFloats(stepsY, other.stepsY) ||
        !CompareFloats(stepsZ, other.stepsZ) ||
        !CompareFloats(feedRate, other.feedRate) ||
        !CompareFloats(seekRate, other.seekRate) ||
        !CompareFloats(arcSegment, other.arcSegment) ||
        !CompareFloats(acceleration, other.acceleration) ||
        !CompareFloats(cornering, other.cornering) ||
        stepPulse != other.stepPulse ||
        stepInvert != other.stepInvert )

    {
        qDebug() << "false";
        return false;
    }

    else
        qDebug() << "true";
    return true;
}

bool Settings::grblSettings::CompareFloats(float x, float y)
{
    if (fabs(x - y) < 0.0001)
        return true;
    else
        return false;
}

QDataStream &operator <<(QDataStream &output, const Settings::grblSettings &settings)
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

QDataStream &operator >>(QDataStream &input, Settings::grblSettings &settings)
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

void Settings::GetGrblSettings(grblSettings *settings)
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

void Settings::GetPlotSettings(plotSettings *settings)
{
    QDataStream fileStream;
    OpenFileStream(&fileStream);
    fileStream.skipRawData(sizeof(grblSettings));
    if(fileStream.atEnd())
    {
        qDebug() << "file still empty";
        PlotDefaults();
        SavePlotSettings();
    }
    else
        fileStream >> *settings;
    CloseFileStream();
}

void Settings::PutPlotSettings()
{
    QDataStream fileStream;
    OpenFileStream(&fileStream);
    fileStream.skipRawData(sizeof(grblSettings));
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

void Settings::GetDeviceGrblSettings()
{
    qDebug() << "GETDEVICEGRBLSETTINGS";
    arduino->flush();
    arduino << QString("\r\n\r\n");
    arduino << QString("$\n");
    QTimer::singleShot(2000, this, SLOT(GetDeviceGrblSettings2()));
}

void Settings::GetDeviceGrblSettings2()
{
    qDebug() << "GetDeviceGrblSettings2";
    QString line = arduino->getLine();
    int escape = 0;
    while(line != "ok")
    {
        qDebug() << line;
        if(line.startsWith('$'))
        {
            switch(line.at(1).digitValue())
            {
            case 0:
//                qDebug() << line;
                deviceGrblSettings.stepsX = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
                break;
            case 1:
//                qDebug() << line;
                deviceGrblSettings.stepsY = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
                break;
            case 2:
//                qDebug() << line;
                deviceGrblSettings.stepsZ = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
                break;
            case 3:
//                qDebug() << line;
                deviceGrblSettings.stepPulse = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
                break;
            case 4:
//                qDebug() << line;
                deviceGrblSettings.feedRate = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
                break;
            case 5:
//                qDebug() << line;
                deviceGrblSettings.seekRate = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
                break;
            case 6:
//                qDebug() << line;
                deviceGrblSettings.arcSegment = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
                break;
            case 7:
//                qDebug() << line;
                deviceGrblSettings.stepInvert = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
                deviceGrblSettings.grblVersion = 0;
                arduino->SetGrblVersion(0);     // 0.51
                break;
            case 8:
//                qDebug() << line;
                deviceGrblSettings.acceleration = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
                deviceGrblSettings.grblVersion = 1;
                arduino->SetGrblVersion(1);     //0.6
                break;
            case 9:
//                qDebug() << line;
                deviceGrblSettings.cornering = line.remove(line.indexOf('('), 75).remove(0,5).toFloat();
                break;
            }
        }
        line = arduino->getLine();
        if(escape > 100)
            break;
        escape++;
//        qDebug() << escape;
    }
    qDebug() << "GetDeviceGrblSettings2End";
    emit getDeviceGrblSettingsFinished();
//    qDebug() << deviceGrblSettings.stepsX;
//    qDebug() << deviceGrblSettings.stepsY;
//    qDebug() << deviceGrblSettings.stepsZ;
//    qDebug() << deviceGrblSettings.stepPulse;
//    qDebug() << deviceGrblSettings.feedRate;
//    qDebug() << deviceGrblSettings.seekRate;
//    qDebug() << deviceGrblSettings.arcSegment;
//    qDebug() << deviceGrblSettings.stepInvert;
//    qDebug() << deviceGrblSettings.acceleration;
//    qDebug() << deviceGrblSettings.cornering;
}

void Settings::PutDeviceGrblSettings()
{
    arduino->flush();
    writingToArduino = true;
    QObject::connect(arduino, SIGNAL(newData()), this, SLOT(PutDeviceGrblSettings2()));
    arduino << QString("$0 = ").append(QString("").setNum(userGrblSettings.stepsX)).append(QString("\n"));
//    qDebug() << "PUTBEGIN";
}

void Settings::PutDeviceGrblSettings2()
{
//    qDebug() << "PUTNEXT";
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


void Settings::CompareGrblSettings()
{
    qDebug() << "COMPAREGRBLSETTINGS";
    connect(this, SIGNAL(getDeviceGrblSettingsFinished()), this, SLOT(CompareGrblSettings2()));
    GetDeviceGrblSettings(); 
}

void Settings::CompareGrblSettings2()
{
    qDebug() << "COMPAREGRBLSETTINGS2";
    bool result;
    grblSettings local;
    GetGrblSettings(&local);

    qDebug() << "local" << endl << local.grblVersion << endl << local.stepsX << endl << local.stepsY << endl << local.stepsZ << endl << local.stepPulse << endl << local.feedRate << endl << local.seekRate << endl << local.arcSegment << endl << local.stepInvert << endl << local.acceleration << endl << local.cornering << endl;
    qDebug() << "device" << endl << deviceGrblSettings.grblVersion << endl << deviceGrblSettings.stepsX << endl << deviceGrblSettings.stepsY << endl << deviceGrblSettings.stepsZ << endl << deviceGrblSettings.stepPulse << endl << deviceGrblSettings.feedRate << endl << deviceGrblSettings.seekRate << endl << deviceGrblSettings.arcSegment << endl << deviceGrblSettings.stepInvert << endl << deviceGrblSettings.acceleration << endl << deviceGrblSettings.cornering << endl;

    if(!(local == deviceGrblSettings))
        result = 0;
    else
        result = 1;
    emit CompareGrblSettingsResult(result);
}

void Settings::FindMachine()
{
    qDebug() << "FINDMACHINESTART";
    arduino->GetPorts();
    if(arduino->ports.isEmpty())
        return;
    arduino->SetPortName(arduino->ports.at(0).physName);
    arduino->OpenPort();
    connect(this, SIGNAL(CompareGrblSettingsResult(bool)), this, SLOT(FindMachine2(bool)));
    CompareGrblSettings();
    qDebug() << "FINDMACHINEEND";
}

void Settings::FindMachine2(bool result)
{
    qDebug() << "FINDMACHINE2";
    static int index = 0;
    if(result) {
        disconnect(this, SIGNAL(CompareGrblSettingsResult(bool)), this, SLOT(FindMachine2(bool)));
        qDebug() << "the machine is connected on " << arduino->portName();
        return;
    }
    else if(index != (arduino->ports.size() - 1)) {
        index ++;
        qDebug() << "the machine connected on " << arduino->portName() << "does not match the current settings";
        arduino->SetPortName(arduino->ports.at(index).physName);
        arduino->OpenPort();
        CompareGrblSettings();
    }
    else {
        disconnect(this, SIGNAL(CompareGrblSettingsResult(bool)), this, SLOT(FindMachine2(bool)));
        qDebug() << "could not find a connected machine that matches the current settings";
        index = 0;
    }
}

void Settings::on_settings_clicked()
{
//    arduino = new ArduinoIO;
//    arduino->SetPortName(ui->arduinoPort_combo->currentText());
//    arduino->OpenPort();
//    QString data;
//    arduino << QString("\r\n\r\n");
//    arduino << QString("$\n");
    PutDeviceGrblSettings();
}

void Settings::on_report_clicked()
{
//    for(int i = 15; i>0; i--)
//    {
//        qDebug() << arduino->getLine();
//    }
    GetDeviceGrblSettings();
//    PlotDefaults();
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
    }
    else if(index == 1) {
        ui->acceleration_dspinbox->setEnabled(true);
        ui->acceleration_label->setEnabled(true);
        ui->maxJerk_dspinbox->setEnabled(true);
        ui->maxJerk_label->setEnabled(true);
    }
}

void Settings::on_plotDefaults_pButton_clicked()
{
    PlotDefaults();
}
