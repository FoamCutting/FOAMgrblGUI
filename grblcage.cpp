#include "grblcage.h"
#include "ui_grblcage.h"

GrblCage::GrblCage(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GrblCage)
{
    ui->setupUi(this);
    plotter = new GCodePlot(this);
    editor = new GCodeText;
    err = new ErrorHandler;
    interceptor = new EventFilterizer;
    settings = new Settings;
    arduino = new ArduinoIO;
    plotter->setSettings(settings);
    settings->SetArduino(arduino);
    settings->SetErrorHandler(err);
    settings->FindMachine1();
    streamInProgress = 0;

    GCodeFile = new QFile;
    GCodeDocument = new QTextDocument;
//    connect(GCodeDocument, SIGNAL(contentsChanged()), this, SLOT(refreshPlot()));     //caused way too many refreshes
    panning = false;
    ui->gcodePlotter->setMouseTracking(true);
    ui->gcodePlotter->viewport()->installEventFilter(interceptor);
    ui->gcodePlotter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QObject::connect(interceptor, SIGNAL(scrollIntercept(float,QPoint)),this,SLOT(scaleView(float,QPoint)));
    QObject::connect(interceptor, SIGNAL(mouseMoveIntercept(QPoint)),this,SLOT(mouseMovement(QPoint)));
    QObject::connect(interceptor, SIGNAL(mouseMiddleClickIntercept(QPoint)),this,SLOT(panStart(QPoint)));
    QObject::connect(interceptor, SIGNAL(mouseMiddleReleaseIntercept()),this,SLOT(panEnd()));
    QObject::connect(interceptor, SIGNAL(resizeIntercept()),this,SLOT(AdjustForResize()));

    QObject::connect(settings,SIGNAL(settingsHidden()),this,SLOT(EnableMainWindow()));
    QObject::connect(settings, SIGNAL(plotSettingsChanged()), this, SLOT(refreshPlot()));
    QObject::connect(settings, SIGNAL(plotSettingsChanged()), this, SLOT(GetGridScale()));
    GetGridScale();

    statusBar()->showMessage("GrblCage initiated");
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
    if(settings->PlotSettings().gridUnits == 0)
        gridScale = 254;
    else
        gridScale = 100;
}

void GrblCage::mouseMovement(QPoint pos)
{
    if(GCodeFile->isOpen())
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

void GrblCage::refreshPlot()
{
    plotter->processGCodes(GCodeDocument->toPlainText());
}

/** ******************** (Pan/Zoom Functions) ******************** **/

void GrblCage::SetCenter(const QPointF& centerPoint)
{
    //Get the rectangle of the visible area in scene coords
    QRectF visibleArea = ui->gcodePlotter->mapToScene(ui->gcodePlotter->rect()).boundingRect();

    //Get the scene areag53
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
    if(CheckForUnsavedChanges()) {

    }
    GCodeFile->setFileName("tmp");
    GCodeFile->open(QIODevice::ReadWrite);
    GCodeDocument->setPlainText("");
    editor->setDocument(GCodeDocument);
    plotter->processGCodes(GCodeDocument->toPlainText());
    emit GCodeDocumentAltered();

    ui->gcodePlotter->setScene(plotter->scene());
    ui->gcodePlotter->fitInView(plotter->scene()->sceneRect(), Qt::KeepAspectRatioByExpanding);
    ui->gcodeEditor->setDocument(editor->document());
    SetCenter(QPointF((ui->gcodePlotter->sceneRect().width())/2, -(ui->gcodePlotter->sceneRect().height())/2));
}

void GrblCage::on_actionSave_GCode_File_triggered()
{
    if(GCodeFile->fileName() == "tmp") {
        on_actionSave_as_triggered();
    }
    else {
        err->pushError(saveGCode(GCodeFile->fileName()));
    }
}

void GrblCage::on_actionSave_as_triggered()
{
    QString FileName;
    if(GCodeFile->isOpen()) {
        FileName = QFileDialog::getSaveFileName(this,
                                                "Save Files",
                                                GCodeFile->fileName(),
                                                "Gcode Files (*.ngc *.txt *.nc *.GC)",
                                                0,
                                                QFileDialog::DontUseNativeDialog);
        int success = saveGCode(FileName);
        if(success == ErrorHandler::Okay && GCodeFile->fileName() == "tmp")
            GCodeFile->remove();
        GCodeFile->close();
        GCodeFile->setFileName(FileName);
        GCodeFile->open(QIODevice::ReadWrite);
        GCodeDocument->setPlainText(GCodeFile->readAll());

        editor->setFile(GCodeFile);
        editor->setDocument(GCodeDocument);
        plotter->processGCodes(GCodeDocument->toPlainText());
        emit GCodeDocumentAltered();
        ui->gcodePlotter->setScene(plotter->scene());
    }
}

int GrblCage::saveGCode(QString fileName)
{
   QTextDocumentWriter GCodeSaver(fileName);
   GCodeSaver.setFormat("plaintext");
    if((!GCodeSaver.write(ui->gcodeEditor->document())))
        return ErrorHandler::SaveFailed;
    else
        return ErrorHandler::Okay;
}

int GrblCage::openGCode(QString fileName)
{
    if(GCodeFile->isOpen() && (GCodeFile->fileName() == "tmp"))
        GCodeFile->remove();
    GCodeFile->setFileName(fileName);
    if(!GCodeFile->open(QIODevice::ReadWrite))
        return 0;
    GCodeDocument->setPlainText(GCodeFile->readAll());

    editor->setFile(GCodeFile);
    editor->setDocument(GCodeDocument);
    plotter->processGCodes(GCodeDocument->toPlainText());
    emit GCodeDocumentAltered();
    ui->gcodePlotter->setScene(plotter->scene());
    ui->gcodePlotter->fitInView(plotter->scene()->sceneRect(), Qt::KeepAspectRatioByExpanding);
    ui->gcodeEditor->setDocument(editor->document());
    SetCenter(QPointF((ui->gcodePlotter->sceneRect().width())/2, -(ui->gcodePlotter->sceneRect().height())/2));
    return 1;
}

void GrblCage::on_actionOpen_GCode_File_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open Files",
                                                    0,
                                                    "Gcode Files (*.ngc *.txt *.nc *.GC *.gc)",
                                                    0,
                                                    QFileDialog::DontUseNativeDialog);
    if(fileName.isEmpty() || fileName.isNull()) {
        return;
    }
    else {
        openGCode(fileName);
    }
    qDebug() << "File name is " << GCodeFile->fileName();
}

void GrblCage::on_actionSettings_triggered()
{
    if(arduino->DeviceState() == ArduinoIO::READY);
    {
        settings->SetArduino(arduino);
    }
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
    plotter->processGCodes(GCodeDocument->toPlainText());
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
    connect(sDialog, SIGNAL(GCodeDocumentAltered()), this, SLOT(refreshPlot()));
    sDialog->show();
}

void GrblCage::on_actionOffset_GCode_triggered()
{
    OffsetDialog *oDialog = new OffsetDialog(editor, plotter);
    connect(oDialog, SIGNAL(GCodeDocumentAltered()), this, SLOT(refreshPlot()));
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


/** ******************** (Manual Movement buttons) ******************** **/

void GrblCage::on_needleStartStop_toggled(bool checked)
{
    if(arduino->DeviceState() == ArduinoIO::READY && !streamInProgress) {
        if(checked)
            arduino << QString("G91\n G00 X0 Y0 Z-0.3\n");
        else
            arduino << QString("G91\n G00 X0 Y0 Z1.0\n");
    }
}

void GrblCage::on_zero_pButton_clicked()
{
    //won't work with 0.51
    if(arduino->DeviceState() == ArduinoIO::READY && !streamInProgress)
        arduino << QString("G92 X0\n G92 Y0\n G92 Z0\n");
}

void GrblCage::on_jogYpositive_clicked()
{
    if(arduino->DeviceState() == ArduinoIO::READY && !streamInProgress)
        arduino->SeekRelative(0, _JogIncrement, 0);
}

void GrblCage::on_jogXYpositive_clicked()
{
    if(arduino->DeviceState() == ArduinoIO::READY && !streamInProgress)
     arduino->SeekRelative(_JogIncrement, _JogIncrement, 0);
}

void GrblCage::on_jogXnegativeYpositive_clicked()
{
    if(arduino->DeviceState() == ArduinoIO::READY && !streamInProgress)
        arduino->SeekRelative(-_JogIncrement, _JogIncrement, 0);
}

void GrblCage::on_jogXnegative_clicked()
{
    if(arduino->DeviceState() == ArduinoIO::READY && !streamInProgress)
        arduino->SeekRelative(-_JogIncrement, 0, 0);
}

void GrblCage::on_jogXpositive_clicked()
{
    if(arduino->DeviceState() == ArduinoIO::READY && !streamInProgress)
        arduino->SeekRelative(_JogIncrement, 0, 0);
}

void GrblCage::on_jogXYnegative_clicked()
{
    if(arduino->DeviceState() == ArduinoIO::READY && !streamInProgress)
        arduino->SeekRelative(-_JogIncrement, -_JogIncrement, 0);
}

void GrblCage::on_jogYnegative_clicked()
{
    if(arduino->DeviceState() == ArduinoIO::READY && !streamInProgress)
        arduino->SeekRelative(0, -_JogIncrement, 0);
}

void GrblCage::on_jogXpositiveYnegative_clicked()
{
    if(arduino->DeviceState() == ArduinoIO::READY && !streamInProgress)
        arduino->SeekRelative(_JogIncrement, -_JogIncrement, 0);
}


/** ******************** (Automatic Streaming buttons) ******************** **/

void GrblCage::on_autoStart_pButton_clicked()
{
    StreamFile();
}

void GrblCage::on_autoStop_pButton_clicked()
{
    StreamFileTerminate();
}

void GrblCage::StreamFile()
{
    if(streamInProgress)
        return;
    if(GCodeFile->isOpen()) {
        if(CheckForUnsavedChanges())
            return;
        StreamList = editor->Preprocess().split('\n', QString::SkipEmptyParts);
        int count = StreamList.length();
        ui->cutProgress_pBar->setMaximum(count);
        arduino << QString("(starting transfer)\n");
        lineCount = 0;
        connect(arduino, SIGNAL(ok()), this, SLOT(StreamFileLoop()));
        connect(arduino, SIGNAL(error()), this, SLOT(StreamFileTerminate()));
        streamInProgress = 1;
    }
    else
        err->pushError(ErrorHandler::GCodeFileNotOpen);
}

void GrblCage::StreamFileLoop()
{
    ui->cutProgress_pBar->setValue(ui->cutProgress_pBar->value()+1);
    if(lineCount < StreamList.size()) {
        arduino << (StreamList[lineCount]+'\n');
        ui->informational_tBrowser->insertPlainText("<< " + StreamList[lineCount] +'\n');
        ui->informational_tBrowser->verticalScrollBar()->setValue(ui->informational_tBrowser->verticalScrollBar()->maximum());
        lineCount++;
    }
    else {
        qDebug() << "StreamingFinished";
        StreamFileTerminate();
    }
//    ui->xPos_lcdNum       //update current head position
//    ui->yPos_lcdNum
}

void GrblCage::StreamFileTerminate()
{
    disconnect(arduino, SIGNAL(ok()), this, SLOT(StreamFileLoop()));
    disconnect(arduino, SIGNAL(error()), this, SLOT(StreamFileTerminate()));
    ui->cutProgress_pBar->reset();
    streamInProgress = 0;
}

bool GrblCage::CheckForUnsavedChanges()
{
    GCodeFile->seek(0);
    qDebug() << GCodeFile->readAll();
    qDebug() << ui->gcodeEditor->toPlainText();
    GCodeFile->seek(0);
    if(QTextDocument(QString(GCodeFile->readAll())).toPlainText() != ui->gcodeEditor->toPlainText())        //this is awkward, but fixes disappearing \r\n
    {
        err->pushError(ErrorHandler::UnsavedChanges);
        err->assessErrorList();
        qDebug() << "Streaming will not continue while there are unsaved changes to the gcode file.\n";
        return 1;
    }
    else
        qDebug() << "All changes are saved. Streaming will continue!\n";
    return 0;
}
