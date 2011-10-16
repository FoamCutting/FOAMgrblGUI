 #include "arduinoio.h"

ArduinoIO::ArduinoIO(QObject *parent) :
    QObject(parent)
{
    portOpen = false;
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

bool ArduinoIO::OpenPort()
{
    arduinoPort = new QextSerialPort(arduinoPortName);
    arduinoPort->setFlowControl(FLOW_XONXOFF);
    arduinoPort->setParity(PAR_NONE);
    arduinoPort->setDataBits(DATA_8);
    arduinoPort->setStopBits(STOP_1);
//    if(userArduinoSettings.baudRate == "9600")
    arduinoPort->setBaudRate(BAUD9600);
    portOpen = arduinoPort->open(QIODevice::ReadWrite);
    connect(arduinoPort, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    if(portOpen) {
        flush();
    }
    return portOpen;
}

void ArduinoIO::ClosePort()
{
    disconnect(arduinoPort, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    arduinoPort->close();
    portOpen = false;
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
    if(portOpen)
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
    if(data == "Stored new setting")
        return;
    dataBuffer << data;
    qDebug() << data;
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

