#include "gcodeplot.h"
#include "settings.h"

GCodePlot::GCodePlot(QObject *parent) :
    QObject(parent)
{
    GCodeDocumentSet = false;
    GCodeScene = new QGraphicsScene;
    coordinateMode = Absolute;
    arcCenterMode = Incremental;
    scaleFactor = UNITS_PER_INCH;
}

GCodePlot::~GCodePlot()
{
    delete GCodeScene;
}

void GCodePlot::setSettings(Settings *s)
{
    settings = s;
}

void GCodePlot::applySettings()
{
//    qDebug() << "applySettings";
//    settings->GetPlotSettings(&plot);
    plot = settings->PlotSettings();
    scaleFactor = UNITS_PER_INCH;
    arcCenterMode = Incremental;
}

void GCodePlot::refreshView()
{
    if(GCodeDocumentSet)
    {
        GCodeScene->clear();
        processGCodes();
    }
}

void GCodePlot::setDocument(QTextDocument *document)
{
    GCodeDocument = document;
    GCodeDocumentSet = true;
}

QTextDocument* GCodePlot::document()
{
    return GCodeDocument;
}

QGraphicsScene* GCodePlot::scene()
{
    return GCodeScene;
}

void GCodePlot::drawGrid()
{
    float gridSizeX, gridSizeY;
    int gridIncrement;
    if(plot.gridUnits == INCHES)
        gridIncrement = UNITS_PER_INCH;
    else
        gridIncrement = UNITS_PER_MM * 10;
    if(plot.machineSizeUnits == INCHES) {
        gridSizeX = plot.sizeX * UNITS_PER_INCH;
        gridSizeY = plot.sizeY * UNITS_PER_INCH;
    }
    else {
        gridSizeX = (plot.sizeX * UNITS_PER_MM * 10);
        gridSizeY = (plot.sizeY * UNITS_PER_MM * 10);
    }
    drawPen.setColor(plot.gridColor);
    for(int i = 0; i <= gridSizeY; i+=gridIncrement)
        GCodeScene->addLine(0, i*(-1), gridSizeX, i*(-1), drawPen);
    for(int i = 0; i <= gridSizeX; i+=gridIncrement)
        GCodeScene->addLine(i, 0, i, gridSizeY*(-1), drawPen);
    GCodeScene->addLine(0, gridSizeY*(-1), gridSizeX, gridSizeY*(-1), drawPen);
    GCodeScene->addLine(gridSizeX, 0, gridSizeX, gridSizeY*(-1), drawPen);
}

void GCodePlot::processGCodes()
{
    applySettings();
    drawGrid();
//    qDebug() << plot.sizeX;
//    qDebug() << plot.sizeY;
//    qDebug() << plot.gridUnits;

    float startCoord[2] = {0};
    float endCoord[2] = {0};
    float arcCenter[2] = {0};
    float radius = 0;
    QString docString = GCodeDocument->toPlainText();
    QTextStream docStream(&docString);
    docStream.seek(0);

    while(!(docStream.atEnd()))
    {
        QString codeLine;
        QList<QString> codeLineList;
        QString command;
        codeLine = docStream.readLine();
        codeLineList = codeLine.toUpper().split(" ", QString::SkipEmptyParts);
        for(int i = 0; i<(codeLineList.size()); i++)
        {
            command = codeLineList[i].trimmed();

            if(command[0] == '(')
            {
                drawMode = None;
                break;
            }
            else if(command[0] == 'G')
            {
                int number = command.remove(0,1).toInt();
                switch(number)
                {
                case 0:
                    drawMode = Linear;
                    drawPen.setColor(plot.moveColor);
                    break;
                case 1:
                    drawMode = Linear;
                    drawPen.setColor(plot.cutColor);
                    break;
                case 2:
                    drawMode = CWArc;
                    drawPen.setColor(plot.cutColor);
                    break;
                case 3:
                    drawMode = CCWArc;
                    drawPen.setColor(plot.cutColor);
                    break;
                case 20:
                    scaleFactor = UNITS_PER_INCH;
                    drawMode = None;
                    break;
                case 21:
                    scaleFactor = UNITS_PER_MM;
                    drawMode = None;
                    break;
                case 90:
                    coordinateMode = Absolute;
                    drawMode = None;
                    break;
                case 91:
                    coordinateMode = Incremental;
                    drawMode = None;
                    break;
                }
            }
            else if(command[0] == 'M')
            {
                int number = command.remove(0,1).toInt();
                switch(number)
                {
                case 3:
                    drawMode = None;
                    break;
                case 4:
                    drawMode = None;
                    break;
                case 5:
                    drawMode = None;
                    drawPen.setColor(plot.moveColor);
                    break;
                }
            }
            else if(command[0] == 'X')
            {
                if(coordinateMode == Absolute)
                    endCoord[0] = command.remove(0,1).toFloat();
                else
                    endCoord[0] += command.remove(0,1).toFloat();
            }
            else if(command[0] == 'Y')
            {
                if(coordinateMode == Absolute)
                    endCoord[1] = command.remove(0,1).toFloat();
                else
                    endCoord[1] += command.remove(0,1).toFloat();
            }
            else if(command[0] == 'R')
            {
                arcMode = Radius;
                radius = command.remove(0,1).toFloat();
            }
            else if(command[0] == 'I')
            {
                arcMode = Center;
                arcCenter[0] = command.remove(0,1).toFloat();
                if(arcCenterMode == Absolute)
                    arcCenter[0] -= startCoord[0];
            }
            else if(command[0] == 'J')
            {
                arcMode = Center;
                arcCenter[1] = command.remove(0,1).toFloat();
                if(arcCenterMode == Absolute)
                     arcCenter[1] -= startCoord[1];
            }
        }
        if(drawMode != None)
        {
            if(drawMode != Linear)
            {
                if(arcMode == Radius)
                    calculateArcElements(startCoord[0], startCoord[1], endCoord[0], endCoord[1], radius);
                else if(arcMode == Center)
                    calculateArcElements(startCoord[0], startCoord[1], endCoord[0], endCoord[1], arcCenter[0], arcCenter[1]);
            }
            else
                drawLine(startCoord[0], startCoord[1], endCoord[0], endCoord[1]);

            startCoord[0] = endCoord[0];
            startCoord[1] = endCoord[1];
        }
    }
}

void GCodePlot::drawLine(float x1, float y1, float x2, float y2)
{
    GCodeScene->addLine(x1*scaleFactor, (-1)*y1*scaleFactor, x2*scaleFactor, (-1)*y2*scaleFactor, drawPen);
}

void GCodePlot::calculateArcElements(float x1, float y1, float x2, float y2, float radius)
{
    float x = x2 - x1;
    float y = y2 - y1;
    if(radius < (sqrt(x*x + y*y)/2))         //check for and ignore arcs with--- bad radius values
        return;
    float h_x2_div_d = sqrt(4 * radius*radius- x*x - y*y)/sqrt(x*x + y*y);

    if(((drawMode == CWArc) && (radius > 0)) || ((drawMode == CCWArc) && (radius < 0)))
    {
        h_x2_div_d = (-1)*h_x2_div_d;
    }
    float xCenter = (x - (y * h_x2_div_d))/2;
    float yCenter = (y + (x * h_x2_div_d))/2;
    float absoluteXCenter = x1 + xCenter;
    float absoluteYCenter = y1 + yCenter;
    float relativeXStart = (-1)*xCenter;
    float relativeYStart = (-1)*yCenter;
    float relativeXEnd = x2 - absoluteXCenter;
    float relativeYEnd = y2 - absoluteYCenter;
    float thetaStart = calculateTheta(relativeXStart, relativeYStart);
    float thetaEnd = calculateTheta(relativeXEnd, relativeYEnd);
    if(radius < 0)
        radius *= -1;

    drawArc(thetaStart, thetaEnd, absoluteXCenter, absoluteYCenter, radius);
}

void GCodePlot::calculateArcElements(float x1, float y1, float x2, float y2, float xCenter, float yCenter)
{
    float absoluteXCenter = x1 + xCenter;
    float absoluteYCenter = y1 + yCenter;
    float relativeXStart = (-1)*xCenter;
    float relativeYStart = (-1)*yCenter;
    float relativeXEnd = x2 - absoluteXCenter;
    float relativeYEnd = y2 - absoluteYCenter;
    float radius = sqrt(xCenter*xCenter + yCenter*yCenter);
    float thetaStart = calculateTheta(relativeXStart, relativeYStart);
    float thetaEnd = calculateTheta(relativeXEnd, relativeYEnd);

    drawArc(thetaStart, thetaEnd, absoluteXCenter, absoluteYCenter, radius);
}


void GCodePlot::drawArc(float theta1, float theta2, float absXCenter, float absYCenter, float radius)
{
    float x1 = radius*cos(theta1);
    float y1 = radius*sin(theta1);
    float x2 = radius*cos(theta2);
    float y2 = radius*sin(theta2);

    if(drawMode == CWArc)
    {
        if(theta1 < theta2)
            theta2 -= 2*PI;
        for(float theta = theta1; theta > theta2; theta -= plot.arcPrecision)
        {
            drawLine(absXCenter+x1, absYCenter+y1, absXCenter+(radius*cos(theta)), absYCenter+(radius*sin(theta)));
            x1 = radius*cos(theta);
            y1 = radius*sin(theta);
        }
        drawLine(absXCenter+x1, absYCenter+y1, absXCenter+x2, absYCenter+y2);
    }
    else if(drawMode == CCWArc)
    {
        if(theta1 > theta2)
            theta2 += 2*PI;
        for(float theta = theta1; theta < theta2; theta += plot.arcPrecision)
        {
            drawLine(absXCenter+x1, absYCenter+y1, absXCenter+(radius*cos(theta)), absYCenter+(radius*sin(theta)));
            x1 = radius*cos(theta);
            y1 = radius*sin(theta);
        }
        drawLine(absXCenter+x1, absYCenter+y1, absXCenter+x2, absYCenter+y2);
    }
}

float GCodePlot::calculateTheta(float x, float y)
{
    float theta = atan2(y,x);
    return theta;
}
