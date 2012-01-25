#ifndef GRBLCAGE_H
#define GRBLCAGE_H

#include <QMainWindow>
#include <QTextDocument>
#include <QTextDocumentWriter>
#include <QFileDialog>
//#include <QDir>
#include <QGraphicsScene>
#include <QGraphicsView>
#include "settings.h"
#include "gcodeplot.h"
#include "gcodetext.h"
#include "offsetdialog.h"
#include "scaledialog.h"
#include "eventfilterizer.h"
#include "arduinoio.h"
#include "machineselect.h"
#include "newmachinedialog.h"
#include <QScrollBar>
#include <QDebug>
#include <QLabel>
#include <math.h>
//#include "serialdeviceenumerator.h"


namespace Ui {
    class GrblCage;
}

class GrblCage : public QMainWindow
{
    Q_OBJECT

private:
    GCodePlot* plotter;
    GCodeText* editor;
    QTextDocument* GCodeDocument;
    ErrorHandler* err;
    EventFilterizer* interceptor;
    bool gcodeFileOpen;
    bool panning;
    QPoint lastPanPoint;
    int saveGCode(QString);
    int openGCode(QString);
    void SetCenter(const QPointF&);
    QPointF GetCenter() { return currentCenterPoint; }
    QPointF currentCenterPoint;
    Ui::GrblCage *ui;
    Settings *settings;
    ArduinoIO *arduino;
    int gridScale;
    QFile *GCodeFile;
    void StreamFile();
    bool CheckForUnsavedChanges();
    QString PreProcess();
    QList<QString> StreamList;
    int lineCount;
    bool streamInProgress;
    void PutDeviceGrblSettings();
    QLabel *deviceStatus;

public:
    explicit GrblCage(QWidget *parent = 0);
    ~GrblCage();

private slots:
    void PutDeviceGrblSettings2();
    void UpdateTitleBar();
    void UpdateDeviceStatusBar();
    void ScrollBarsMoved();

    void on_actionScale_G_Code_triggered();
    void on_actionOpen_GCode_File_triggered();
    void on_actionSave_GCode_File_triggered();
    void on_actionConvert_to_Incremental_GCode_triggered();
    void on_actionConvert_to_Absolute_GCode_triggered();
    void on_actionOffset_GCode_triggered();
    void on_actionAdd_Line_Numbers_triggered();
    void on_actionRemove_Line_Numbers_triggered();
    void on_editMode_toggled(bool checked);
    void on_refresh_clicked();
    void on_actionSettings_triggered();
    void on_undo_clicked();
    void on_redo_clicked();
    void on_actionNew_GCode_File_triggered();
    void on_jogYpositive_clicked();
    void on_jogXYpositive_clicked();
    void on_jogXnegativeYpositive_clicked();
    void on_jogXnegative_clicked();
    void on_jogXpositive_clicked();
    void on_jogXYnegative_clicked();
    void on_jogYnegative_clicked();
    void on_jogXpositiveYnegative_clicked();
    void on_needleStartStop_toggled(bool checked);
    void on_autoStart_pButton_clicked();
    void on_actionSave_as_triggered();
    void on_autoStop_pButton_clicked();
    void on_zero_pButton_clicked();
    void on_actionUpload_Settings_triggered();
    void on_actionCheck_Settings_triggered();
    void on_actionConnect_triggered();
    void on_actionDisconnect_triggered();
    void on_actionLoad_Machine_triggered();
    void on_actionNew_Machine_triggered();

    void on_pushButton_clicked();   //probably will not keep this

signals:
    void GCodeDocumentAltered();

public slots:
    void EnableMainWindow();
    void DisableMainWindow();
    void scaleView(float,QPoint);
    void mouseMovement(QPoint);
    void panStart(QPoint);
    void panEnd();
    void AdjustForResize();
    void GetGridScale();
    void StreamFileLoop();
    void StreamFileTerminate();
    void refreshPlot();
    void CheckMachine();
};

#endif // GRBLCAGE_H
