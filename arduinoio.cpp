 #include "arduinoio.h"

ArduinoIO::ArduinoIO(QObject *parent) :
    QObject(parent)
{
    deviceState = DISCONNECTED;
    connect(this, SIGNAL(deviceStateChanged(int)), this, SLOT(SetDeviceState(int)));
}

ArduinoIO::~ArduinoIO()
{
}

bool ArduinoIO::grblSettings::operator== (grblSettings other)
{
//        if( grblVersion != other.grblVersion) qDebug() << 1;
//        if(stepsX != other.stepsX) qDebug() << 2;
//        if( stepsY != other.stepsY ) qDebug() << 3;
//        if( stepsZ != other.stepsZ ) qDebug() << 4;
//        if(feedRate != other.feedRate ) qDebug() << 5;
//        if(seekRate != other.seekRate) qDebug() << 6;
//        if( arcSegment != other.arcSegment) qDebug() << 7;
//        if(acceleration != other.acceleration ) qDebug() << 8;
//        if(cornering != other.cornering ) qDebug() << 9;
//        if(stepPulse != other.stepPulse ) qDebug() << 10;
//        if (stepInvert != other.stepInvert ) qDebug() << 11;

    if( grblVersion != other.grblVersion ||
        !CompareFloats(stepsX, other.stepsX) ||
        !CompareFloats(stepsY, other.stepsY) ||
        !CompareFloats(stepsZ, other.stepsZ) ||
        !CompareFloats(feedRate, other.feedRate) ||
        !CompareFloats(seekRate, other.seekRate) ||
        !CompareFloats(arcSegment, other.arcSegment) ||
        stepPulse != other.stepPulse ||
        stepInvert != other.stepInvert )
    {
        qDebug() << "false";
        return false;
    }
    else if(grblVersion != 0)
    {
        if(!CompareFloats(acceleration, other.acceleration) ||
           !CompareFloats(cornering, other.cornering))
        {
            qDebug() << "false";
            return false;
        }
    }
    else
        qDebug() << "true";
    return true;
}

bool ArduinoIO::grblSettings::CompareFloats(float x, float y)
{
    if (fabs(x - y) < 0.0001)
        return true;
    else
        return false;
}

void ArduinoIO::SetErrorHandler(ErrorHandler *handler)
{
    err = handler;
}

QList<QextPortInfo> ArduinoIO::GetPorts()
{
    ports.clear();
    ports = QextSerialEnumerator::getPorts();
    //qDebug() << "ed Arduinos";
    //qDebug() << "===================================";
    for (int i = 0; i < ports.size();) {
        if((ports.at(i).portName).contains("ttyUSB") || (ports.at(i).portName).contains("ttyACM") ) {
//                qDebug() << "port name:" << ports.at(i).portName;
//                qDebug() << "friendly name:" << ports.at(i).friendName;
//                qDebug() << "physical name:" << ports.at(i).physName;
//                qDebug() << "enumerator name:" << ports.at(i).enumName;
//                qDebug() << "vendor ID:" << QString::number(ports.at(i).vendorID, 16);
//                qDebug() << "product ID:" << QString::number(ports.at(i).productID, 16);
//                qDebug() << "===================================";
            i++;
        }
        else {
            ports.removeAt(i);
        }
    }
    //    qDebug() << "List of ports:";
    //    for (int i = 0; i < ports.size(); i++) {
    //        qDebug() << "port name:" << ports.at(i).portName;
    //        qDebug() << "friendly name:" << ports.at(i).friendName;
    //        qDebug() << "physical name:" << ports.at(i).physName;
    //        qDebug() << "enumerator name:" << ports.at(i).enumName;
    //        qDebug() << "vendor ID:" << QString::number(ports.at(i).vendorID, 16);
    //        qDebug() << "product ID:" << QString::number(ports.at(i).productID, 16);
    //        qDebug() << "===================================";
    //    }
    return ports;
}

int ArduinoIO::NumberConnected()
{
    return ports.count();
}

int ArduinoIO::DeviceState()
{
    return deviceState;
}

void ArduinoIO::SetDeviceState(int state)
{
    deviceState = state;
    qDebug() << "Arduino state is:" << state;
}

bool ArduinoIO::OpenPort()
{
    arduinoPort = new QextSerialPort(arduinoPortName);
    arduinoPort->setFlowControl(FLOW_XONXOFF);
    arduinoPort->setParity(PAR_NONE);
    arduinoPort->setDataBits(DATA_8);
    arduinoPort->setStopBits(STOP_1);
//    if(userArduinoSettings.baudRate == "9600")
    arduinoPort->setBaudRate(BAUD9600);
    deviceState = arduinoPort->open(QIODevice::ReadWrite);
    connect(arduinoPort, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
//    QTimer::singleShot(2000, this, SLOT(GetDeviceGrblSettings(deviceState)));
    if(deviceState) {
        emit deviceStateChanged(CONNECTED);
       // flush();
    }
    return deviceState;
}

void ArduinoIO::ClosePort()
{
    disconnect(arduinoPort, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    Flush();
    ClearGrblSettings();
    arduinoPort->close();
    deviceState = DISCONNECTED;
}

bool ArduinoIO::SetBaudRate(int baud)
{
    switch(baud)
    {
    case 9600:
        arduinoPort->setBaudRate(BAUD9600);
        return 1;
    default:
        return 0;
    }
}

void ArduinoIO::SetGrblVersion(int version)
{
    grblVersion = version;
}

void ArduinoIO::SetPortName(QString name)
{
    arduinoPortName = name;
    if(deviceState)
        arduinoPort->setPortName(name);
}

QString ArduinoIO::portName()
{
    return arduinoPortName;
}

QString ArduinoIO::getLine()
{
    if(!(dataBuffer.isEmpty()))
        return dataBuffer.takeFirst();
    else return QString("");
}

void ArduinoIO::Flush()
{
    arduinoPort->flush();
    dataBuffer.clear();
//    this << QString("\n");
}

void ArduinoIO::onReadyRead()
{
//    emit deviceStateChanged(BUSY);
    QString data = arduinoPort->readLine().trimmed();
    qDebug() << ">>" << data;
//    if(data == "")
//        return;
//    else if(data == "Stored new setting")
//        return;
    /*else*/ if(data == "ok")
    {
        emit ok();
    }
    else if(QString(data).remove(6,100) == "error:")
    {
        emit error();
        //pass to errorhandler
    }
    else if(data[0] == 'G' && data[1] == 'r' && data[2] == 'b' && data[3] == 'l' && data[4] == ' ') {
        QString version = data.remove(0,5);
        currentGrblSettings.grblVersion = 2;
        if(version == "0.7d")
            currentGrblSettings.grblVersion = 2;
        else if(version == "0.6b")
            currentGrblSettings.grblVersion = 1;
        else if(version == "0.51")
            currentGrblSettings.grblVersion = 0;
        SetGrblVersion(currentGrblSettings.grblVersion);
        return;
    }
    else if(data == "'$' to dump current settings") {
        emit deviceStateChanged(CHECKING);
        GetDeviceGrblSettings(DeviceState());
        return;
    }
    dataBuffer << data;
    emit newData();
//    emit deviceStateChanged(READY);
}

void ArduinoIO::SeekRelative(double dX, double dY, double dZ)
{
    QString command;
    command.append("G20").append(" G00").append(" X").append(QString::number(dX)).append(" Y").append(QString::number(dY)).append(" Z").append(QString::number(dZ)).append("\r\n");
    qDebug() << command;
    this << QString("G91");
    this << command;
}

void ArduinoIO::SeekAbsolute(double dX, double dY, double dZ)
{
    QString command;
    command.append("G00").append(" X").append(QString::number(dX)).append(" Y").append(QString::number(dY)).append(" Z").append(QString::number(dZ)).append("\r\n");
    this << QString("G90");
    this << command;
}

ArduinoIO* &operator <<(ArduinoIO *output, const QString string) //could probably put this in the class.Too tired to think about it.
{
    output->arduinoPort->write(string.toAscii());
    qDebug() << "<< " << string.trimmed();
    return output;
}

void ArduinoIO::GetDeviceGrblSettings(int arduinoState)
{
    qDebug() << "GetDeviceGrblSettings1";

    if(DeviceState() == READY || DeviceState() == CHECKING) {
         Flush();
//         this << QString("\r\n\r\n");     //this was causing issues in 0.7d; is it really necessary?
         this << QString("$\n");
         disconnect(this, SIGNAL(deviceStateChanged(int)), this, SLOT(GetDeviceGrblSettings(int)));
         connect(this, SIGNAL(ok()), this, SLOT(GetDeviceGrblSettings2()));
         return;
     }
     else if(DeviceState() > ArduinoIO::DISCONNECTED) {
         connect(this, SIGNAL(deviceStateChanged(int)), this, SLOT(GetDeviceGrblSettings(int)));
         return;
     }
     else {
         err->pushError(ErrorHandler::NoArduinoConnected);
         return;
     }
}

void ArduinoIO::GetDeviceGrblSettings2()
{
    qDebug() << "GetDeviceGrblSettings2";

    QString line = getLine();
    int escape = 0;
    disconnect(this, SIGNAL(ok()), this, SLOT(GetDeviceGrblSettings2()));

    while(line != "ok")
    {
        if(line != "")
            qDebug() << line << " -- from GDGS2";
        if(line.startsWith('$'))
        {
            switch(line.at(1).digitValue())
            {
            case 0:
//                qDebug() << line;
                currentGrblSettings.stepsX = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
                break;
            case 1:
//                qDebug() << line;
                currentGrblSettings.stepsY = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
                break;
            case 2:
//                qDebug() << line;
                currentGrblSettings.stepsZ = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
                break;
            case 3:
//                qDebug() << line;
                currentGrblSettings.stepPulse = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
                break;
            case 4:
//                qDebug() << line;
                currentGrblSettings.feedRate = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
                break;
            case 5:
//                qDebug() << line;
                currentGrblSettings.seekRate = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
                break;
            case 6:
//                qDebug() << line;
                currentGrblSettings.arcSegment = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
                break;
            case 7:
//                qDebug() << line;
                currentGrblSettings.stepInvert = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
                break;
            case 8:
//                qDebug() << line;
                currentGrblSettings.acceleration = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
                break;
            case 9:
//                qDebug() << line;
                currentGrblSettings.cornering = line.remove(line.indexOf('('), 75).remove(0,5).toFloat();
                break;
            }
        }
        line = getLine();
        if(escape > 1000)
            break;
        escape++;
//        qDebug() << escape;
    }
    qDebug() << "GetDeviceGrblSettings2End";
    emit deviceStateChanged(READY);
    emit getDeviceGrblSettingsFinished();
//    qDebug() << deviceGrblSettings.stepsX;
//    qDebug() << deviceGrblSettings.stepsY;
//    qDebug() << deviceGrblSettings.stepsZ;
//    qDebug() << deviceGrblSettings.stepPulse;
//    qDebug() << deviceGrblSettings.feedRate;
//    qDebug() << deviceGrblSettings.seekRate;
//    qDebug() << deviceGrblSettings.arcSegment;
//    qDebug() << deviceGrblSettings.stepInvert;
//    qDebug() << deviceGrblSettings.acceleration;
//    qDebug() << deviceGrblSettings.cornering;
}

void ArduinoIO::ClearGrblSettings()
{
    currentGrblSettings.stepsX = 0;
    currentGrblSettings.stepsY = 0;
    currentGrblSettings.stepsZ = 0;
    currentGrblSettings.stepPulse = 0;
    currentGrblSettings.feedRate = 0;
    currentGrblSettings.seekRate = 0;
    currentGrblSettings.arcSegment = 0;
    currentGrblSettings.stepInvert = 0;
    currentGrblSettings.acceleration = 0;
    currentGrblSettings.cornering = -1;
}


