#ifndef GCODEPLOT_H
#define GCODEPLOT_H


#include <QDebug>


#include <QObject>
#include <QTextDocument>
#include <QGraphicsScene>
#include <QTextStream>
#include <math.h>
#include "settings.h"

#define UNITS_PER_INCH 254
#define UNITS_PER_MM 10
#define PI 3.14

class GCodePlot : public QObject
{
    Q_OBJECT

public:
    explicit GCodePlot(QObject *parent = 0);
    ~GCodePlot();
    void processGCodes();
    QTextDocument* document();
    void setDocument(QTextDocument*);
    QGraphicsScene* scene();
    void setSettings(Settings*);

public slots:
    void refreshView();

private:
    void setDefaults();
    void applySettings();
    Settings::plotSettings plot;
    Settings *settings;

    QPen drawPen;
    QGraphicsScene* GCodeScene;
    QTextDocument* GCodeDocument;
    bool GCodeDocumentSet;

    enum{INCHES, MM};
    bool arcMode;
    enum{Radius, Center};
    bool arcCenterMode;
    bool coordinateMode;
    enum{Absolute, Incremental};

    int drawMode;
    enum{None, Linear, CWArc, CCWArc};

    int scaleFactor;

    void drawGrid();
    void drawLine(float,float,float,float);
    void calculateArcElements(float,float,float,float,float);
    void calculateArcElements(float,float,float,float,float,float);
    void drawArc(float,float,float,float,float);
    float calculateTheta(float,float);

};

#endif // GCODEPLOT_H
