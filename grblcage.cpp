#include "grblcage.h"
#include "ui_grblcage.h"

GrblCage::GrblCage(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GrblCage)
{
    ui->setupUi(this);
    plotter = new GCodePlot(this);
    editor = new GCodeText;
    GCodeDocument = new QTextDocument;
    err = new ErrorHandler;
    interceptor = new EventFilterizer;
    settings = new Settings;
    arduino = new ArduinoIO;
    plotter->setSettings(settings);
    settings->SetArduino(arduino);
    settings->SetErrorHandler(err);

    panning = false;
    gcodeFileOpen = false;
    ui->gcodePlotter->setMouseTracking(true);
    ui->gcodePlotter->viewport()->installEventFilter(interceptor);
    ui->gcodePlotter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);


    QObject::connect(this, SIGNAL(GCodeDocumentAltered()),plotter,SLOT(refreshView()));
    QObject::connect(interceptor, SIGNAL(scrollIntercept(float,QPoint)),this,SLOT(scaleView(float,QPoint)));
    QObject::connect(interceptor, SIGNAL(mouseMoveIntercept(QPoint)),this,SLOT(mouseMovement(QPoint)));\
    QObject::connect(interceptor, SIGNAL(mouseMiddleClickIntercept(QPoint)),this,SLOT(panStart(QPoint)));
    QObject::connect(interceptor, SIGNAL(mouseMiddleReleaseIntercept()),this,SLOT(panEnd()));
    QObject::connect(interceptor, SIGNAL(resizeIntercept()),this,SLOT(AdjustForResize()));

    statusBar()->showMessage("GrblCage initiated");


    QObject::connect(settings,SIGNAL(settingsHidden()),this,SLOT(EnableMainWindow()));
    QObject::connect(settings, SIGNAL(plotSettingsChanged()), plotter, SLOT(refreshView()));
    QObject::connect(settings, SIGNAL(plotSettingsChanged()), this, SLOT(GetGridScale()));
    GetGridScale();
}

GrblCage::~GrblCage()
{
    delete ui;
    delete plotter;
    delete editor;
    delete GCodeDocument;
    delete err;
    delete interceptor;
    delete arduino;
}

void GrblCage::GetGridScale()
{
    Settings::plotSettings plotSet = settings->PlotSettings();
    if(plotSet.gridUnits == 0)
        gridScale = 254;
    else
        gridScale = 100;
}

void GrblCage::mouseMovement(QPoint pos)
{
    if(gcodeFileOpen)
    {
        if(panning && this->isEnabled())
        {
            if(!lastPanPoint.isNull())
            {
                //Get how much we panned
                QPointF delta = ui->gcodePlotter->mapToScene(lastPanPoint) - ui->gcodePlotter->mapToScene(pos);
                lastPanPoint = pos;

                //Update the center ie. do the pan
                SetCenter(GetCenter() + delta);
            }
        }
        else
        {
            QString coordinateString;
            QPointF coordinate = (ui->gcodePlotter->mapToScene(pos))/gridScale;
            coordinateString += "x:";
            coordinateString += QString::number(coordinate.x());
            coordinateString += " y:";
            coordinateString += QString::number(-coordinate.y());
            statusBar()->showMessage(coordinateString);
        }
    }
}

/** ******************** (Public Slots) ******************** **/

void GrblCage::EnableMainWindow()
{
    ui->GCodeEditorDockWidget->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable|QDockWidget::DockWidgetFloatable);
    ui->ControlsDockWidget->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable|QDockWidget::DockWidgetFloatable);
    this->setEnabled(true);
}

void GrblCage::DisableMainWindow()
{
    ui->GCodeEditorDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
    ui->ControlsDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
    this->setEnabled(false);
}

/** ******************** (Pan/Zoom Functions) ******************** **/

void GrblCage::SetCenter(const QPointF& centerPoint)
{
    //Get the rectangle of the visible area in scene coords
    QRectF visibleArea = ui->gcodePlotter->mapToScene(ui->gcodePlotter->rect()).boundingRect();

    //Get the scene area
    QRectF sceneBounds = ui->gcodePlotter->sceneRect();

    double boundX = visibleArea.width() / 2.0;
    double boundY = visibleArea.height() / 2.0 - sceneBounds.height();
    double boundWidth = sceneBounds.width() - visibleArea.width();
    double boundHeight = sceneBounds.height() - visibleArea.height();

    //The max boundary that the centerPoint can be to
    QRectF bounds(boundX, boundY, boundWidth, boundHeight);

    if(bounds.contains(centerPoint)) {
        //We are within the bounds
        currentCenterPoint = centerPoint;
    }
    else
    {
        //We need to clamp or use the center of the screen
        if(visibleArea.contains(sceneBounds))
        {
            //Use the center of scene ie. we can see the whole scene
            currentCenterPoint = sceneBounds.center();
        }
        else
        {

            currentCenterPoint = centerPoint;

            //We need to clamp the center. The centerPoint is too large
            if(centerPoint.x() > bounds.x() + bounds.width())
            {
                currentCenterPoint.setX(bounds.x() + bounds.width());
            }
            else if(centerPoint.x() < bounds.x())
            {
                currentCenterPoint.setX(bounds.x());
            }

            if(centerPoint.y() > bounds.y() + bounds.height())
            {
                currentCenterPoint.setY(bounds.y() + bounds.height());
            }
            else if(centerPoint.y() < bounds.y())
            {
                currentCenterPoint.setY(bounds.y());
            }

        }
    }

    //Update the scrollbars
    ui->gcodePlotter->centerOn(currentCenterPoint);
}

void GrblCage::scaleView(float delta, QPoint pos)
{
    if(!panning && this->isEnabled())
    {
        //Get the position of the mouse before scaling, in scene coords
        QPointF pointBeforeScale(ui->gcodePlotter->mapToScene(pos));

        //Get the original screen centerpoint
        QPointF screenCenter = GetCenter(); //CurrentCenterPoint; //(visRect.center());

        //Scale the view ie. do the zoom
        double scaleFactor = 1.15; //How fast we zoom
        if(delta > 0) {
            //Zoom in
            ui->gcodePlotter->scale(scaleFactor, scaleFactor);
        } else {
            //Zooming out
            ui->gcodePlotter->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
        }

        //Get the position after scaling, in scene coords
        QPointF pointAfterScale(ui->gcodePlotter->mapToScene(pos));

        //Get the offset of how the screen moved
        QPointF offset = pointBeforeScale - pointAfterScale;

        //Adjust to the new center for correct zooming
        QPointF newCenter = screenCenter + offset;
        SetCenter(newCenter);
    }
}

void GrblCage::panStart(QPoint point)
{
    panning = true;
    lastPanPoint = point;
    this->setCursor(Qt::SizeAllCursor);
}

void GrblCage::panEnd()
{
    panning = false;
    this->setCursor(Qt::ArrowCursor);
}

void GrblCage::AdjustForResize()
{
    //Get the rectangle of the visible area in scene coords
    QRectF visibleArea = ui->gcodePlotter->mapToScene(ui->gcodePlotter->rect()).boundingRect();
    SetCenter(visibleArea.center());
}

/** ******************** (File Menu Buttons) ******************** **/


void GrblCage::on_actionNew_GCode_File_triggered()
{
    GCodeDocument->setPlainText("");
    plotter->setDocument(GCodeDocument);
    editor->setDocument(GCodeDocument);
    emit GCodeDocumentAltered();

    ui->gcodePlotter->setScene(plotter->scene());
    ui->gcodePlotter->fitInView(plotter->scene()->sceneRect(), Qt::KeepAspectRatioByExpanding);
    ui->gcodeEditor->setDocument(editor->document());
    SetCenter(QPointF((ui->gcodePlotter->sceneRect().width())/2, -(ui->gcodePlotter->sceneRect().height())/2));
    gcodeFileOpen = true;
}

int GrblCage::saveGCode(QString fileName)
{
    GCodeSaver = new QTextDocumentWriter(fileName);
    if((!GCodeSaver->write(ui->gcodeEditor->document())))
        return ErrorHandler::SaveFailed;
    else
        return ErrorHandler::Okay;
}

void GrblCage::on_actionSave_GCode_File_triggered()
{
    if(gcodeFileOpen) {
        saveGCode(QFileDialog::getSaveFileName(this,
                                               "Save Files",
                                               "*.txt",
                                               "Gcode Files (*.ngc *.txt *.nc *.GC)",
                                               0,
                                               QFileDialog::DontUseNativeDialog));
    }
}

void GrblCage::on_actionOpen_GCode_File_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open Files",
                                                    0,
                                                    "Gcode Files (*.ngc *.txt *.nc *.GC)",
                                                    0,
                                                    QFileDialog::DontUseNativeDialog);
    if(fileName.isEmpty() || fileName.isNull())
    {
        return;
    }
    else
    {
        QFile *file = new QFile(fileName);
        file->open(QIODevice::ReadWrite);

        GCodeDocument->setPlainText(file->readAll());
        plotter->setDocument(GCodeDocument);
        editor->setDocument(GCodeDocument);
        emit GCodeDocumentAltered();

        ui->gcodePlotter->setScene(plotter->scene());
        ui->gcodePlotter->fitInView(plotter->scene()->sceneRect(), Qt::KeepAspectRatioByExpanding);
        ui->gcodeEditor->setDocument(editor->document());
        SetCenter(QPointF((ui->gcodePlotter->sceneRect().width())/2, -(ui->gcodePlotter->sceneRect().height())/2));
        gcodeFileOpen = true;
    }
}

void GrblCage::on_actionSettings_triggered()
{
    settings->show();
    DisableMainWindow();
}

/** ******************** (Gcode Viewer Dock Buttons) ******************** **/
void GrblCage::on_editMode_toggled(bool checked)
{
    if(checked) {
        ui->gcodeEditor->setReadOnly(false);
//        ui->gcodeEditor->setUndoRedoEnabled(true);
    }
    else {
        ui->gcodeEditor->setReadOnly(true);
//        ui->gcodeEditor->setUndoRedoEnabled(false);
    }
}

void GrblCage::on_refresh_clicked()
{
    emit GCodeDocumentAltered();
}

void GrblCage::on_undo_clicked()
{
    if(ui->editMode->isChecked())
        ui->gcodeEditor->undo();
}

void GrblCage::on_redo_clicked()
{
    if(ui->editMode->isChecked())
        ui->gcodeEditor->redo();
}

/** ******************** (Gcode Menu Buttons) ******************** **/

void GrblCage::on_actionScale_G_Code_triggered()
{

    ScaleDialog *sDialog = new ScaleDialog(editor, plotter);
    sDialog->show();
}

void GrblCage::on_actionOffset_GCode_triggered()
{
    OffsetDialog *oDialog = new OffsetDialog(editor, plotter);
    oDialog->show();
}

void GrblCage::on_actionConvert_to_Incremental_GCode_triggered()
{
    editor->toIncremental();
    err->assessErrorList();
}

void GrblCage::on_actionConvert_to_Absolute_GCode_triggered()
{
    editor->toAbsolute();
    err->assessErrorList();
}

void GrblCage::on_actionAdd_Line_Numbers_triggered()
{
    editor->addLineNumbers();
    err->assessErrorList();
}

void GrblCage::on_actionRemove_Line_Numbers_triggered()
{
    editor->removeLineNumbers();
    err->assessErrorList();
}

void GrblCage::on_jogYpositive_clicked()
{
    settings->FindMachine();
//    arduino->SeekRelative(0, 0.1, 0);
}

void GrblCage::on_jogXYpositive_clicked()
{
    arduino->ClosePort();
}
