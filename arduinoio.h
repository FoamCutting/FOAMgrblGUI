#ifndef ARDUINOIO_H
#define ARDUINOIO_H

#include <QObject>
#include <QDebug>
#include "qextserialenumerator.h"
#include "qextserialport.h"
#include "errorhandler.h"
#include <QTimer>
#include "inttypes.h"
#include "math.h"

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
    void ClearGrblSettings();

public:
    ~ArduinoIO();
    struct grblSettings {
        int grblVersion;
        float stepsX;
        float stepsY;
        float stepsZ;
        float feedRate;
        float seekRate;
        float arcSegment;
        float acceleration;
        float cornering;
        uint8_t stepInvert;
        uint8_t stepPulse;
        //uint8_t microsteps;

        bool operator==(grblSettings);
        bool CompareFloats(float, float);
    };
    grblSettings currentGrblSettings;
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
    void Flush();
    void SetGrblVersion(int);
    int NumberConnected();

    int DeviceState();
    enum{DISCONNECTED, CONNECTED, CHECKING, BUSY, READY};

    void SeekRelative(double, double, double);
    void SeekAbsolute(double, double, double);

//    ArduinoIO operator <<(QString);

signals:
    void ok();
    void error();
    void newData();
    void portReady();
    void deviceStateChanged(int);
    void getDeviceGrblSettingsFinished();

public slots:
    void onReadyRead();
    void GetDeviceGrblSettings(int);

private slots:
    void SetDeviceState(int);
    void GetDeviceGrblSettings2();
};

#endif // ARDUINOIO_H
