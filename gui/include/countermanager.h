#ifndef COUNTERMANAGER_H
#define COUNTERMANAGER_H

#include <settings.h>

#include <QObject>
#include <QTimer>
#include <QLabel>

class CounterManager : public QObject
{
    Q_OBJECT
public:
    explicit CounterManager(QObject *parent = nullptr);

    void start(Settings *settings, int remainingSeconds);
    void stop();

    QLabel *counterLabel;

signals:
    void timeFinished();

private:
    QTimer *timer;
    int remainingSeconds;
    Settings *settings;
};

#endif // COUNTERMANAGER_H
