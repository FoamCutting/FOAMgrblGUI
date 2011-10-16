#ifndef ARDUINOIO_H
#define ARDUINOIO_H

#include <QObject>
#include <QDebug>
#include "qextserialenumerator.h"
#include "qextserialport.h"
#include "errorhandler.h"

class ArduinoIO : public QObject
{
    Q_OBJECT

private:
    QextSerialPort *arduinoPort;
    QString arduinoPortName;
    QList<QString> dataBuffer;
    int deviceState;
    int grblVersion;
    ErrorHandler *err;

public:
    void SetErrorHandler(ErrorHandler*);
    QList<QextPortInfo> ports;
    bool IsReady();
    explicit ArduinoIO(QObject *parent = 0);
    QList<QextPortInfo> GetPorts();
    bool OpenPort();
    void OpenPort2();
    void ClosePort();
    bool SetBaudRate(int);
    void SetPortName(QString);
    QString portName();
    friend ArduinoIO* &operator<<(ArduinoIO*, QString);
    QString getLine();
    void flush();
    void SetGrblVersion(int);
    int NumberConnected();

    int DeviceState();
    enum{DISCONNECTED, CONNECTED, PORTOPEN, SENDING, RECIEVING, READY};

    void SeekRelative(int, int, int);
    void SeekAbsolute(int, int, int);

//    ArduinoIO operator <<(QString);

signals:
    void newData();
    void portReady();
    void deviceStateChanged(int);

public slots:
    void onReadyRead();

private slots:
    void SetDeviceState(int);
};

#endif // ARDUINOIO_H
