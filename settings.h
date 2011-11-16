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
#include "arduinoio.h"
#include "errorhandler.h"

#define DEFAULT_STEPS_PER_MM_X 755.906
#define DEFAULT_STEPS_PER_MM_Y 755.906
#define DEFAULT_STEPS_PER_MM_Z 755.906
#define DEFAULT_FEED_RATE 480.000
#define DEFAULT_SEEK_RATE 480.000
#define DEFAULT_MM_PER_ARC_SEGMENT 0.100
#define DEFAULT_ACCELERATION 4.800
#define DEFAULT_MAX_CORNERING_SPEED 50.0
#define DEFAULT_STEP_PORT_INVERT_MASK 0


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
    struct plotSettings {
        bool machineSizeUnits;
        float sizeX;
        float sizeY;
        bool gridUnits;
        QColor gridColor;
        QColor moveColor;
        QColor cutColor;
        float arcPrecision;
    };
    struct arduinoSettings {
        QString portName;
        QString baudRate;
        int grblVersion;
    };

    static QString filePath;
    static QString currentMachineName;
    static QDir applicationDirectory;
    static const plotSettings defaultPlotSettings;
    static const ArduinoIO::grblSettings defaultGrblSettings;
    static const arduinoSettings defaultArduinoSettings;
    plotSettings PlotSettings();
    ArduinoIO::grblSettings DeviceGrblSettings();
    ArduinoIO::grblSettings LocalGrblSettings();
    arduinoSettings ArduinoSettings();
    void GetPlotSettings();
    void GetGrblSettings(ArduinoIO::grblSettings*);
    void SetArduino(ArduinoIO*);
    void FindMachine();
    void SetErrorHandler(ErrorHandler*);
//    bool CompareFloats(float, float);

private slots:
    void on_grblSave_pButton_clicked();
    void on_grblDefaults_pButton_clicked();
    void on_gridColor_tButton_clicked();
    void on_cutColor_tButton_clicked();
    void on_moveColor_tButton_clicked();
    void on_plotSave_pButton_clicked();
    void PutDeviceGrblSettings2();
    void on_refreshArduinoPortList_tButton_clicked();
    void on_grblVerson_combo_currentIndexChanged(int index);
    void CompareGrblSettings(int);
    void FindMachine2(bool);
    void on_plotDefaults_pButton_clicked();

    void on_upload_pButton_clicked();

private:
    Ui::Settings *ui;
    QFile* currentMachineFile;
    inline int OpenFileStream(QDataStream*);
    inline int CloseFileStream();

    void PutDeviceGrblSettings();
    void PutGrblSettings();
    void GrblDefaults();
    void DisplayGrblSettings();
    void SaveGrblSettings();
    int GetGrblVersion();
    void AdjustForGrblVersion();

    void PutPlotSettings();
    void DisplayPlotSettings();
    void PlotDefaults();
    void SavePlotSettings();

    ArduinoIO::grblSettings userGrblSettings;
    ArduinoIO::grblSettings deviceGrblSettings;
    plotSettings userPlotSettings;
    arduinoSettings userArduinoSettings;
    ErrorHandler *err;

    QColor SetColorSample(QToolButton*,QColor, bool dialog = 1);

//    QextSerialEnumerator enumerator;
    ArduinoIO *arduino;
    bool writingToArduino;
    void RefreshPortList();


protected:
    void hideEvent(QHideEvent *);
    void showEvent(QShowEvent *);
    void closeEvent(QCloseEvent*);

signals:
    void settingsHidden();
    void settingsShown();
    void plotSettingsChanged();
    void CompareGrblSettingsResult(bool);
};

#endif // SETTINGS_H
