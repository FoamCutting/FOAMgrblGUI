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
#include <typeinfo>

#define DEFAULT_STEPS_PER_MM_X 755.906
#define DEFAULT_STEPS_PER_MM_Y 755.906
#define DEFAULT_STEPS_PER_MM_Z 755.906
#define DEFAULT_FEED_RATE 480.000
#define DEFAULT_SEEK_RATE 480.000
#define DEFAULT_MM_PER_ARC_SEGMENT 0.100
#define DEFAULT_ACCELERATION 4.800
#define DEFAULT_MAX_CORNERING_SPEED 50.0
#define DEFAULT_STEP_PORT_INVERT_MASK 0

#define G_DEFMACH "Default_Machine_File"
#define P_GRIDUNITS "Grid_Units"
#define P_GRIDCOLOR "Grid_Color"
#define P_SEEKCOLOR "Seek_Color"
#define P_CUTCOLOR "Cut_Color"
#define P_ARCPREC "Arc_Precision"

#define M_UNITS "Machine_Size_Units"
#define M_SIZEX "Machine_X_Size"
#define M_SIZEY "Machine_Y_Size"
#define M_JOGINC "Jog_Increment"
#define A_PORTNAME "Default_Port_Name"
#define A_DEFBAUD "Default_Baud_Rate"
#define PR_ENABLED "Preprocess_Enabled"
#define PR_OPTENAB "Optimize_Enabled"
#define PR_CHECKCOMP "Check_Compatibility"
#define PR_ALWAYSDEFFEED "Always_Use_Default_Feed_Rate"
#define PR_RMLINENUM "Remove_Line_Numbers"
#define PR_RMUNNEC "Remove_Unnecessary_Commands"
#define PR_RMUNSUPP "Remove_Unsupported_Commands"
#define PR_RMARCS "Replace_Arcs_With_Linear_Interpolation"
#define PR_ARCSEG "Arc_Segment_Per_DegreeSegments"
#define PR_UNIFZ "Make_Z_Changes_Uniform"
#define PR_ZMAG "Magnitude_of_Uniform_Z_Changes"
#define PR_SAVE "Save_Modified_File"
#define PR_APP "Append_to_Modified_File"
#define G_VER "Grbl_Version"
#define G_STEPSX "Steps_per_mm_X"
#define G_STEPSY "Steps_per_mm_Y"
#define G_STEPSZ "Steps_per_mm_Z"
#define G_FEED "Default_Feed_Rate"
#define G_SEEK "Default_Seek_Rate"
#define G_MMARC "MM_per_Arc_Segment"
#define G_ACC "Max_Acceleration_(mm/s^2)"
#define G_CORN "Cornering_Tolerance?"
#define G_PULSE "Step_Pulse_Width_(us)"
#define G_MASK "Step_Invert_Mask"



namespace Ui {
	class Settings;
}

class Settings : public QWidget
{
	Q_OBJECT

public:
	~Settings();
	explicit Settings(QWidget *parent = 0);
//	  struct grblSettings {
//		  int grblVersion;
//		  float stepsX;
//		  float stepsY;
//		  float stepsZ;
//		  float feedRate;
//		  float seekRate;
//		  float arcSegment;
//		  float acceleration;
//		  float cornering;
//		  uint8_t stepInvert;
//		  uint8_t stepPulse;
//		  //uint8_t microsteps;

//		  bool operator==(grblSettings);
//		  bool CompareFloats(float, float);
//	  };
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

	plotSettings PlotSettings(){return settings.plot;}
	grblSettings GrblSettings(){return settings.grbl;}
	preprocessSettings PreprocessSettings(){return settings.preprocess;}
	machineSettings MachineSettings(){return settings.machine;}
	arduinoSettings ArduinoSettings(){return settings.arduino;}
	globalSettings GlobalSettings(){return settings.global;}

	QString MachineName() {
	return settings.global.currentMachine;
	}
	void setMachineName(QString name) {
	settings.global.currentMachine = name;
	}
	QDir ApplicationDirectory() {
	return applicationDirectory;
	}

	//	  bool CompareFloats(float, float);


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

	int Init();

	int ReadGlobalFile(QString fileName);
	int ReadMachineFile(QString fileName);
	template<typename S>
	void PutSetting(QString description, QFile *file, S setting);
	int SaveAllSettings();

	int GetGlobalSettings();
	int PutGlobalSettings(QString fileName);
	void SaveGlobalSettings();
	void DisplayGlobalSettings();


	void GetGrblSettings();
	void PutGrblSettings();
	void DisplayGrblSettings();
	void SaveGrblSettings();
	void AdjustForGrblVersion();
	void PutDeviceGrblSettings();

	int GetMachineSettings(QString fileName);
	int PutMachineSettings(QString fileName);
	void SaveMachineSettings();
	void DisplayMachineSettings();

	void GetPreprocessSettings();
	void PutPreprocessSettings();
	void SavePreprocessSettings();
	void DisplayPreprocessSettings();

	grblSettings deviceGrblSettings;
	ErrorHandler *err;

	QColor SetColorSample(QToolButton*,QColor, bool dialog = 1);

//	  QextSerialEnumerator enumerator;
	bool writingToArduino;
	void RefreshPortList();

protected:
	void hideEvent(QHideEvent *);
	void showEvent(QShowEvent *);
	void closeEvent(QCloseEvent*);

public slots:
	void refresh();

private slots:
	void on_gridColor_tButton_clicked();
	void on_cutColor_tButton_clicked();
	void on_moveColor_tButton_clicked();
	void on_refreshArduinoPortList_tButton_clicked();
	void on_grblVerson_combo_currentIndexChanged(int index);
	void on_setMachDefault_pButton_clicked();
	void on_save_pButton_clicked();

	void on_close_pButton_clicked();

signals:
	void settingsHidden();
	void settingsShown();
	void plotSettingsChanged();
};

#endif // SETTINGS_H
