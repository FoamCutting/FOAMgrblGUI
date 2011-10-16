 #include "arduinoio.h"

ArduinoIO::ArduinoIO(QObject *parent) :
    QObject(parent)
{
    deviceState = DISCONNECTED;
    connect(this, SIGNAL(deviceStateChanged(int)), this, SLOT(SetDeviceState(int)));
}

void ArduinoIO::SetErrorHandler(ErrorHandler *handler)
{
    err = handler;
}

QList<QextPortInfo> ArduinoIO::GetPorts()
{
    ports.clear();
    ports = QextSerialEnumerator::getPorts();
    //qDebug() << "Connected Arduinos";
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
    if(deviceState) {
        emit deviceStateChanged(CONNECTED);
       // flush();
    }
    return deviceState;
}

void ArduinoIO::ClosePort()
{
    disconnect(arduinoPort, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    dataBuffer.clear();
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

void ArduinoIO::flush()
{
    arduinoPort->flush();
    dataBuffer.clear();
}

void ArduinoIO::onReadyRead()
{
    QString data = arduinoPort->readLine().trimmed();
    if(data == "")
        return;
    else if(data == "Stored new setting")
        return;
    else if(data == "Grbl 0.6b") {
        SetGrblVersion(1);
        return;
    }
    else if(data == "Grbl 0.51") {
        SetGrblVersion(0);
        return;
    }
    else if(data == "'$' to dump current settings") {
        emit deviceStateChanged(READY);
        return;
    }
    dataBuffer << data;
    qDebug() << data << " -- from oRR";
    emit newData();
}

void ArduinoIO::SeekRelative(int dX, int dY, int dZ)
{
    QString command;
    command.append("G00").append(" X").append(dX).append(" Y").append(dY).append(" Z").append(dZ);
    qDebug() << command;
    this << QString("G91");
    this << command;
}

void ArduinoIO::SeekAbsolute(int dX, int dY, int dZ)
{
    QString command;
    command.append("G00").append(" X").append(dX).append(" Y").append(dY).append(" Z").append(dZ);
    this << QString("G90");
    this << command;
}

ArduinoIO* &operator <<(ArduinoIO *output, const QString string) //could probably put this in the class.Too tired to think about it.
{
    output->arduinoPort->write(string.toAscii());
    return output;
}

