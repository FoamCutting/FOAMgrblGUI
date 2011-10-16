#ifndef EVENTFILTERIZER_H
#define EVENTFILTERIZER_H

#include <QObject>
#include <QEvent>
#include <QDebug>
#include <QWheelEvent>

class EventFilterizer : public QObject
{
    Q_OBJECT
public:
    explicit EventFilterizer(QObject *parent = 0);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

signals:
    void scrollIntercept(float, QPoint);
    void mouseMoveIntercept(QPoint);
    void mouseMiddleClickIntercept(QPoint);
    void mouseMiddleReleaseIntercept();
    void mouseLeftClickIntercept();
    void mouseLeftReleaseIntercept();
    void resizeIntercept();

public slots:

};

#endif // EVENTFILTERIZER_H
