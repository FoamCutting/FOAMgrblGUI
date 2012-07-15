#include "settings.h"
#include "ui_settings.h"

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
	on_grblVerson_combo_currentIndexChanged(ui->grblVerson_combo->currentIndex());
	Init();
}

Settings::~Settings()
{
	delete ui;
}

int Settings::Init()
{
	if(!QFile("GlobalSettings").exists()) {
		settings.global.defaultMachine = "Default";
		settings.plot.gridUnits = 0;
		settings.plot.gridColor = "#dedede";
		settings.plot.moveColor = "#00ff00";
		settings.plot.cutColor = "#0000ff";
		settings.plot.arcPrecision = 0.1;
		PutGlobalSettings("GlobalSettings");
	}

	QDir::setCurrent(applicationDirectory.absolutePath());
	if(!QDir("MachineSettings").exists())
		QDir().mkdir("MachineSettings");
	qDebug() << "MachineSettings.count()" << QDir("MachineSettings").count();
	QDir::setCurrent("MachineSettings");

/// TODO:check detection of empty directory in Windows
	if(QDir().current().count() < 3)	  {
		qDebug() << "Machine Settings directory is empty: creating a Machine Settings file called Default \n Please configure this machine before continuing";
		settings.global.defaultMachine = "Default";
	}

	ReadGlobalFile("GlobalSettings");
	settings.global.currentMachine = settings.global.defaultMachine;
	ReadMachineFile(settings.global.currentMachine);
	return 0;
}

template<typename S>
void Settings::PutSetting(QString description, QFile *file, S setting)
{
	 QString settingString;
	 settingString = QString::number(setting);
	 file->write(QByteArray(description.toAscii()) + QByteArray(" = ") + settingString.toAscii() + QByteArray("\n"));
}

template<>
void Settings::PutSetting<QString>(QString description, QFile *file, QString setting)
{
	 QString settingString;
	  settingString = setting;
	 file->write(QByteArray(description.toAscii()) + QByteArray(" = ") + settingString.toAscii() + QByteArray("\n"));
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
	ReadMachineFile(settings.global.currentMachine);
	DisplayGlobalSettings();
	DisplayMachineSettings();
	emit settingsShown();
	RefreshPortList();
//	arduino->SetPortName(ui->arduinoPort_combo->currentText());
//	arduino->OpenPort();
	ui->arduinoPort_combo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	event->accept();
}

void Settings::hideEvent(QHideEvent *event)
{
	emit settingsHidden();
	event->accept();
}

/** ********************(File i/o)******************** **/

/** ********************( Global Settings )******************** **/

int Settings::ReadGlobalFile(QString fileName)
{
	qDebug() << "Open Global File";
	QDir::setCurrent(applicationDirectory.absolutePath());

	QFile *globalFile = new QFile(fileName);
	if(!globalFile->open(QIODevice::ReadWrite | QIODevice::Text)) {
			qDebug() << "Failed to open Global Settings File";
		return -1;
	}
	while(!globalFile->atEnd()) {
		QStringList list = QString(globalFile->readLine()).split("=",QString::SkipEmptyParts);
		if(list.at(0).trimmed() == G_DEFMACH) settings.global.defaultMachine = list.at(1).trimmed();
		else if(list.at(0).trimmed() == P_GRIDUNITS) settings.plot.gridUnits = list.at(1).trimmed().toInt();
		else if(list.at(0).trimmed() == P_GRIDCOLOR) settings.plot.gridColor = list.at(1).trimmed();
		else if(list.at(0).trimmed() == P_SEEKCOLOR) settings.plot.moveColor = list.at(1).trimmed();
		else if(list.at(0).trimmed() == P_CUTCOLOR) settings.plot.cutColor = list.at(1).trimmed();
		else if(list.at(0).trimmed() == P_ARCPREC) settings.plot.arcPrecision = list.at(1).trimmed().toFloat();
		else qDebug() << "Invalid setting" << list.at(0) << "in" << globalFile->fileName();
	}
	qDebug() << "Close Global File";
	globalFile->close();
	return 0;
}

int Settings::PutGlobalSettings(QString fileName)
{
	QFile *globalFile = new QFile(fileName);
	if(!globalFile->open(QIODevice::ReadWrite | QIODevice::Text)) {
			qDebug() << "Failed to open Global Settings File";
		return -1;
	}
	PutSetting(G_DEFMACH, globalFile, settings.global.defaultMachine);
	PutSetting(P_GRIDUNITS, globalFile, settings.plot.gridUnits);
	PutSetting(P_GRIDCOLOR, globalFile, settings.plot.gridColor);
	PutSetting(P_SEEKCOLOR , globalFile, settings.plot.moveColor);
	PutSetting(P_CUTCOLOR, globalFile, settings.plot.cutColor);
	PutSetting(P_ARCPREC, globalFile, settings.plot.arcPrecision);
	globalFile->close();
	return 0;
}

void Settings::DisplayGlobalSettings()
{
	ui->defaultMachName_label->setText(settings.global.defaultMachine);
	ui->currentMachName_label->setText(settings.global.currentMachine);
	ui->gridUnits_combo->setCurrentIndex(settings.plot.gridUnits);
	ui->gridColor_lEdit->setText(settings.plot.gridColor);
	ui->moveColor_lEdit->setText(settings.plot.moveColor);
	ui->cutColor_lEdit->setText(settings.plot.cutColor);
	ui->arcPrecision_dSpin->setValue(settings.plot.arcPrecision);
}

void Settings::SaveGlobalSettings()
{
	settings.global.defaultMachine = ui->defaultMachName_label->text();
	settings.plot.gridUnits = ui->gridUnits_combo->currentIndex();
	settings.plot.gridColor = ui->gridColor_lEdit->text();
	settings.plot.moveColor = ui->moveColor_lEdit->text();
	settings.plot.cutColor = ui->cutColor_lEdit->text();
	settings.plot.arcPrecision = ui->arcPrecision_dSpin->value();
	PutGlobalSettings("GlobalSettings");
}

/** ******************** Grbl Settings ******************** **/

void Settings::on_grblVerson_combo_currentIndexChanged(int index)	//disable settings that are not applicable to the current grbl version
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
	else if(index == 3) {
	ui->acceleration_dspinbox->setEnabled(true);
	ui->acceleration_label->setEnabled(true);
	ui->maxJerk_dspinbox->setEnabled(true);
	ui->maxJerk_label->setEnabled(true);
	ui->maxJerk_label->setText("Cornering Junction Deviance (mm)");
	}
}

/** ******************** Machine Specific Settings ******************** **/

int Settings::ReadMachineFile(QString fileName)
{
	qDebug() << "Opening Machine Settings File " << fileName;
	QFile *machineFile = new QFile(fileName);
	QDir::setCurrent("MachineSettings");
	if(!machineFile->open(QIODevice::ReadWrite | QIODevice::Text)) {
		qDebug() << "Failed to open Machine Settings File " << machineFile->fileName();
		QDir::setCurrent(applicationDirectory.absolutePath());
		return -1; }

	while(!machineFile->atEnd()) {
		QStringList list = QString(machineFile->readLine()).split("=",QString::SkipEmptyParts);
		if(list.at(0).trimmed() == G_DEFMACH) settings.global.defaultMachine = list.at(1).trimmed();
		else if(list.at(0).trimmed() == M_UNITS) settings.machine.units = list.at(1).trimmed().toInt();
		else if(list.at(0).trimmed() == M_SIZEX) settings.machine.sizeX = list.at(1).trimmed().toFloat();
		else if(list.at(0).trimmed() == M_SIZEY) settings.machine.sizeY = list.at(1).trimmed().toFloat();
		else if(list.at(0).trimmed() == M_JOGINC) settings.machine.jogIncrement = list.at(1).trimmed().toFloat();
		else if(list.at(0).trimmed() == A_PORTNAME) settings.arduino.portName = list.at(1).trimmed().toInt();
		else if(list.at(0).trimmed() == A_DEFBAUD) settings.arduino.baudRate= list.at(1).trimmed().toInt();
		else if(list.at(0).trimmed() == PR_ENABLED) settings.preprocess.enabled = list.at(1).trimmed().toInt();
		else if(list.at(0).trimmed() == PR_OPTENAB) settings.preprocess.optimize = list.at(1).trimmed().toInt();
		else if(list.at(0).trimmed() == PR_CHECKCOMP) settings.preprocess.checkCompatibility = list.at(1).trimmed().toInt();
		else if(list.at(0).trimmed() == PR_ALWAYSDEFFEED) settings.preprocess.alwaysDefaultFeed = list.at(1).trimmed().toInt();
		else if(list.at(0).trimmed() == PR_RMLINENUM) settings.preprocess.removeLineNumbers = list.at(1).trimmed().toInt();
		else if(list.at(0).trimmed() == PR_RMUNNEC) settings.preprocess.removeUnnecessaryCommands = list.at(1).trimmed().toInt();
		else if(list.at(0).trimmed() == PR_RMUNSUPP) settings.preprocess.removeUnsupportedCommands = list.at(1).trimmed().toInt();
		else if(list.at(0).trimmed() == PR_RMARCS) settings.preprocess.removeArcs = list.at(1).trimmed().toInt();
		else if(list.at(0).trimmed() == PR_ARCSEG) settings.preprocess.arcSegments = list.at(1).trimmed().toInt();
		else if(list.at(0).trimmed() == PR_UNIFZ) settings.preprocess.uniformZ = list.at(1).trimmed().toInt();
		else if(list.at(0).trimmed() == PR_ZMAG) settings.preprocess.zMagnitude = list.at(1).trimmed().toFloat();
		else if(list.at(0).trimmed() == PR_SAVE ) settings.preprocess.saveFile = list.at(1).trimmed().toInt();
		else if(list.at(0).trimmed() == PR_APP) settings.preprocess.fileAppend = list.at(1).trimmed();
		else if(list.at(0).trimmed() == G_VER) settings.grbl.version = list.at(1).trimmed().toInt();
		else if(list.at(0).trimmed() == G_STEPSX) settings.grbl.stepsX = list.at(1).trimmed().toFloat();
		else if(list.at(0).trimmed() == G_STEPSY) settings.grbl.stepsY = list.at(1).trimmed().toFloat();
		else if(list.at(0).trimmed() == G_STEPSZ) settings.grbl.stepsZ = list.at(1).trimmed().toFloat();
		else if(list.at(0).trimmed() == G_FEED) settings.grbl.feedRate = list.at(1).trimmed().toFloat();
		else if(list.at(0).trimmed() == G_SEEK) settings.grbl.seekRate = list.at(1).trimmed().toFloat();
		else if(list.at(0).trimmed() == G_MMARC) settings.grbl.arcSegment = list.at(1).trimmed().toFloat();
		else if(list.at(0).trimmed() == G_ACC) settings.grbl.acceleration = list.at(1).trimmed().toFloat();
		else if(list.at(0).trimmed() == G_CORN) settings.grbl.cornering = list.at(1).trimmed().toFloat();
		else if(list.at(0).trimmed() == G_PULSE) settings.grbl.stepPulse = list.at(1).trimmed().toUInt(0,8);
		else if(list.at(0).trimmed() == G_MASK) settings.grbl.stepInvert = list.at(1).trimmed().toUInt(0,8);
		else qDebug() << "Invalid setting" << list.at(0) << "in" << machineFile->fileName();
	}

	qDebug() << "Closing Machine Settings File " << machineFile->fileName();
	machineFile->close();
	QDir::setCurrent(applicationDirectory.absolutePath());
	return 0;
}

int Settings::PutMachineSettings(QString fileName)
{
	qDebug() << "Opening Machine Settings File " << fileName;
	QFile *machineFile = new QFile(fileName);
	QDir::setCurrent("MachineSettings");
	if(!machineFile->open(QIODevice::ReadWrite | QIODevice::Text)) {
		qDebug() << "Failed to open Machine Settings File " << machineFile->fileName();
		QDir::setCurrent(applicationDirectory.absolutePath());
		return -1; }

	PutSetting(M_UNITS, machineFile, settings.machine.units);
	PutSetting(M_SIZEX, machineFile, settings.machine.sizeX);
	PutSetting(M_SIZEY, machineFile, settings.machine.sizeY);
	PutSetting(M_JOGINC, machineFile, settings.machine.jogIncrement);
	PutSetting(A_PORTNAME, machineFile, settings.arduino.portName);
	PutSetting(A_DEFBAUD, machineFile, settings.arduino.baudRate);
	PutSetting(PR_ENABLED, machineFile, settings.preprocess.enabled);
	PutSetting(PR_OPTENAB, machineFile, settings.preprocess.optimize);
	PutSetting(PR_CHECKCOMP, machineFile, settings.preprocess.checkCompatibility);
	PutSetting(PR_ALWAYSDEFFEED, machineFile, settings.preprocess.alwaysDefaultFeed);
	PutSetting(PR_RMLINENUM, machineFile, settings.preprocess.removeLineNumbers);
	PutSetting(PR_RMUNNEC, machineFile, settings.preprocess.removeUnnecessaryCommands);
	PutSetting(PR_RMUNSUPP, machineFile, settings.preprocess.removeUnsupportedCommands);
	PutSetting(PR_RMARCS, machineFile, settings.preprocess.removeArcs);
	PutSetting(PR_ARCSEG, machineFile, settings.preprocess.arcSegments);
	PutSetting(PR_UNIFZ, machineFile, settings.preprocess.uniformZ);
	PutSetting(PR_ZMAG, machineFile, settings.preprocess.zMagnitude);
	PutSetting(PR_SAVE, machineFile, settings.preprocess.saveFile);
	PutSetting(PR_APP,machineFile, settings.preprocess.fileAppend);
	PutSetting(G_VER, machineFile, settings.grbl.version);
	PutSetting(G_STEPSX, machineFile, settings.grbl.stepsX);
	PutSetting(G_STEPSY, machineFile, settings.grbl.stepsY);
	PutSetting(G_STEPSZ, machineFile, settings.grbl.stepsZ);
	PutSetting(G_FEED, machineFile, settings.grbl.feedRate);
	PutSetting(G_SEEK, machineFile, settings.grbl.seekRate);
	PutSetting(G_MMARC, machineFile, settings.grbl.arcSegment);
	PutSetting(G_ACC, machineFile, settings.grbl.acceleration);
	PutSetting(G_CORN, machineFile, settings.grbl.cornering);
	PutSetting(G_PULSE, machineFile, settings.grbl.stepPulse);
	PutSetting(G_MASK, machineFile, settings.grbl.stepInvert);

	qDebug() << "Closing Machine Settings File " << machineFile->fileName();
	machineFile->close();
	QDir::setCurrent(applicationDirectory.absolutePath());
	return 0;
}

void Settings::DisplayMachineSettings()
{
	ui->machineUnits_combo->setCurrentIndex(settings.machine.units);
	ui->sizeX_dBox->setValue(settings.machine.sizeX);
	ui->sizeY_dBox->setValue(settings.machine.sizeY);
	ui->jogIncrement_dBox->setValue(settings.machine.jogIncrement);
	SetColorSample(ui->gridColor_tButton, settings.plot.gridColor, 0);
	SetColorSample(ui->moveColor_tButton, settings.plot.moveColor, 0);
	SetColorSample(ui->cutColor_tButton, settings.plot.cutColor, 0);
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

void Settings::SaveMachineSettings()
{
	settings.machine.units = ui->machineUnits_combo->currentIndex();
	settings.machine.sizeX = ui->sizeX_dBox->value();
	settings.machine.sizeY = ui->sizeY_dBox->value();
	settings.machine.jogIncrement = ui->jogIncrement_dBox->value();
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
	PutMachineSettings(settings.global.currentMachine);
	emit plotSettingsChanged();
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

/** ********************(Serial Communication)******************** **/

void Settings::RefreshPortList()
{
	ui->arduinoPort_combo->clear();
}

void Settings::on_refreshArduinoPortList_tButton_clicked()
{
	RefreshPortList();
}

int Settings::SaveAllSettings()
{
	SaveMachineSettings();
	return 0;
}

bool Settings::grblSettings::operator== (grblSettings other)
{
	if( version != other.version ||
	!CompareFloats(stepsX, other.stepsX) ||
	!CompareFloats(stepsY, other.stepsY) ||
	!CompareFloats(stepsZ, other.stepsZ) ||
	!CompareFloats(feedRate, other.feedRate) ||
	!CompareFloats(seekRate, other.seekRate) ||
	!CompareFloats(arcSegment, other.arcSegment) ||
	stepPulse != other.stepPulse ||
	stepInvert != other.stepInvert )
	{
		qDebug() << "False -- grblSettings operator==()";
		return false;
	}
	else if(version != 0)
	{
		if(!CompareFloats(acceleration, other.acceleration) ||
		   !CompareFloats(cornering, other.cornering))
		{
			qDebug() << "False -- grblSettings operator==()";
			return false;
		}
	}
	qDebug() << "True -- grblSettings operator==()";
	return true;
}

void Settings::on_setMachDefault_pButton_clicked()
{
	settings.global.defaultMachine = settings.global.currentMachine;
	ui->defaultMachName_label->setText(settings.global.defaultMachine);
}

void Settings::refresh()
{
	ReadGlobalFile("GlobalSettings");
	ReadMachineFile(settings.global.currentMachine);
}

void Settings::on_save_pButton_clicked()
{
	if(ui->settings_tabWidget->currentIndex() ==0)
		SaveGlobalSettings();
	else
		SaveMachineSettings();
}

void Settings::on_close_pButton_clicked()
{
	this->close();
}
