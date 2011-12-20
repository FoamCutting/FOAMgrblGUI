#ifndef GCODEPLOT_H
#define GCODEPLOT_H


#include <QDebug>
#include <QElapsedTimer>

#include <QObject>
#include <QGraphicsScene>
#include <QTextStream>
#include <math.h>
#include "settings.h"
#include "Toolbox.h"

#define UNITS_PER_INCH 254
#define UNITS_PER_MM 10
#define PI 3.14

class GCodePlot : public QObject
{
    Q_OBJECT

public:
    explicit GCodePlot(QObject *parent = 0);
    ~GCodePlot();
    void processGCodes(QString);
    QGraphicsScene* scene();
    void setSettings(Settings*);

public slots:

private:
    QElapsedTimer timer;

    void setDefaults();
    void applySettings();
    Settings *settings;

    QPen drawPen;
    QGraphicsScene* GCodeScene;

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
//    inline QString GetNumString(QString gcodeString, int *pos);
};

#endif // GCODEPLOT_H
