#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>
#include <QFile>
#include <stdio.h>
#include <QTextStream>
#include <inttypes.h>
#include <QColorDialog>
#include <QGraphicsView>
#include <QToolButton>
#include <QDebug>
#include <QTimer>
#include <QDir>
#include <QCloseEvent>
#include <math.h>
#include "errorhandler.h"
#include "Toolbox.h"
#include <inttypes.h>

#define DEFAULT_STEPS_PER_MM_X 755.906
#define DEFAULT_STEPS_PER_MM_Y 755.906
#define DEFAULT_STEPS_PER_MM_Z 755.906
#define DEFAULT_FEED_RATE 480.000
#define DEFAULT_SEEK_RATE 480.000
#define DEFAULT_MM_PER_ARC_SEGMENT 0.100
#define DEFAULT_ACCELERATION 4.800
#define DEFAULT_MAX_CORNERING_SPEED 50.0
#define DEFAULT_STEP_PORT_INVERT_MASK 0

#define ENTRY_SIZE 100


namespace Ui {
    class Settings;
}

class Settings : public QWidget
{
    Q_OBJECT

public:
    ~Settings();
    explicit Settings(QWidget *parent = 0);
//    struct grblSettings {
//        int grblVersion;
//        float stepsX;
//        float stepsY;
//        float stepsZ;
//        float feedRate;
//        float seekRate;
//        float arcSegment;
//        float acceleration;
//        float cornering;
//        uint8_t stepInvert;
//        uint8_t stepPulse;
//        //uint8_t microsteps;

//        bool operator==(grblSettings);
//        bool CompareFloats(float, float);
//    };
    enum{machSizeUnits, machSizeX, machSizeY, machJogIncrement,
	 plotGridUnits, plotGridColor, plotMoveColor, plotCutColor, plotArcPrecision,
	 ardPortName, ardBaudRate,
	 preEnabled, preOptimize, preCheckCompatibility, preAlwaysDefaultFeed, preRemoveLineNum, preRemoveUnnecessary, preRemoveUnsupported,
	 preRemoveArcs, preArcSegments, preUniformZ, preZMagnitude, preSaveFile, preFileAppend,
	 grblVersion, grblStepsX, grblStepsY, grblStepsZ, grblStepPulse, grblFeedRate, grblSeekRate, grblArcSegment, grblStepInvert, grblAcceleration, grblCornering,
	 SEPARATOR,
	 globDefaultMach, globCurrentMach
    };
    struct machineSettings{
	bool units;
	float sizeX;
	float sizeY;
	float jogIncrement;
    };
    struct plotSettings {
	bool gridUnits;
	QString gridColor;
	QString moveColor;
	QString cutColor;
	float arcPrecision;
    };
    struct arduinoSettings {
	QString portName;
	int baudRate;
    };
    struct grblSettings {
	int version;
	float stepsX;
	float stepsY;
	float stepsZ;
	float feedRate;
	float seekRate;
	float arcSegment;
	float acceleration;
	float cornering;
	uint8_t stepInvert;
	uint8_t stepPulse;
	//uint8_t microsteps;

	bool operator==(grblSettings);
    };
    struct preprocessSettings
    {
	bool enabled;
	bool optimize;
	bool checkCompatibility;
	bool alwaysDefaultFeed;
	bool removeLineNumbers;
	bool removeUnnecessaryCommands;
	bool removeUnsupportedCommands;
	bool removeArcs;
	int arcSegments;
	bool uniformZ;
	float zMagnitude;
	bool saveFile;
	QString fileAppend;
    };
    struct globalSettings
    {
	QString defaultMachine;
	QString currentMachine;
    };

    void SetErrorHandler(ErrorHandler*);
    QString GetStr(int);
    float Get(int);
    grblSettings GrblSettings();
    QString MachineName() {
	return settings.global.currentMachine;
    }
    void setMachineName(QString name) {
	settings.global.currentMachine = name;
    }
    QDir ApplicationDirectory() {
	return applicationDirectory;
    }

    //    bool CompareFloats(float, float);


private:
    QWidget *par;
    static QString filePath;
    static QDir applicationDirectory;

    class allSettings{
    public:
	machineSettings machine;
	plotSettings plot;
	arduinoSettings arduino;
	preprocessSettings preprocess;
	grblSettings grbl;
	globalSettings global;
    }settings,defaultSettings;
    Ui::Settings *ui;
    QFile* machineFile;
    QFile* globalFile;

    inline int OpenMachineFile();
    inline int CloseMachineFile();
    inline int OpenGlobalFile();
    inline int CloseGlobalFile();

    QString GetSettingStr(int);
    template<typename T>
    T GetSetting(int);
    template<typename S>
    void PutSetting(QString description, int index, S setting, bool convertToString = 1);
    void PutSettingStr(QString description, int index, QString setting);
    int GetMachineSettings();
    int SaveAllSettings();

    int GetGlobalSettings();
    int PutGlobalSettings();
    void SaveGlobalSettings();
    void DisplayGlobalSettings();


    void GetGrblSettings();
    void PutGrblSettings();
    void DisplayGrblSettings();
    void SaveGrblSettings();
    void AdjustForGrblVersion();
    void PutDeviceGrblSettings();

    void GetGeneralSettings();
    void PutGeneralSettings();
    void SaveGeneralSettings();
    void DisplayGeneralSettings();

    void GetPreprocessSettings();
    void PutPreprocessSettings();
    void SavePreprocessSettings();
    void DisplayPreprocessSettings();

    grblSettings deviceGrblSettings;
    ErrorHandler *err;

    QColor SetColorSample(QToolButton*,QColor, bool dialog = 1);

//    QextSerialEnumerator enumerator;
    bool writingToArduino;
    void RefreshPortList();

protected:
    void hideEvent(QHideEvent *);
    void showEvent(QShowEvent *);
    void closeEvent(QCloseEvent*);

public slots:
    void refresh();

private slots:
    void on_grblSave_pButton_clicked();
    void on_gridColor_tButton_clicked();
    void on_cutColor_tButton_clicked();
    void on_moveColor_tButton_clicked();
    void on_plotSave_pButton_clicked();
    void on_refreshArduinoPortList_tButton_clicked();
    void on_grblVerson_combo_currentIndexChanged(int index);
    void on_preProcSave_pButton_clicked();

    void on_setMachDefault_pButton_clicked();

    void on_pushButton_clicked();

signals:
    void settingsHidden();
    void settingsShown();
    void plotSettingsChanged();
};

#endif // SETTINGS_H
