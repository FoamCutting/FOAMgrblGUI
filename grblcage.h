#ifndef GRBLCAGE_H
#define GRBLCAGE_H

#include <QMainWindow>
#include <QTextDocument>
#include <QTextDocumentWriter>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include "settings.h"
#include "gcodeplot.h"
#include "gcodetext.h"
#include "offsetdialog.h"
#include "scaledialog.h"
#include "eventfilterizer.h"

#include <QDebug>


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
    QTextDocumentWriter* GCodeSaver;
    bool gcodeFileOpen;
    bool panning;
    QPoint lastPanPoint;
    int saveGCode(QString);
    void SetCenter(const QPointF&);
    QPointF GetCenter() { return currentCenterPoint; }
    QPointF currentCenterPoint;
    Ui::GrblCage *ui;
    Settings *settings;
    ArduinoIO *arduino;
    int gridScale;

public:
    explicit GrblCage(QWidget *parent = 0);
    ~GrblCage();

private slots:
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
};

#endif // GRBLCAGE_H
