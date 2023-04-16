#include "countermanager.h"
#include <QDesktopServices>
#include <QUrl>

CounterManager::CounterManager(QObject *parent) : QObject(parent)
{
    counterLabel = new QLabel();
    timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this, [this]() {
        remainingSeconds--;

        if (remainingSeconds % 60 == 0) {
            settings->SetTimeToPlay(remainingSeconds);
        }

        int hours = remainingSeconds / 3600;
        int minutes = (remainingSeconds % 3600) / 60;
        int seconds = remainingSeconds % 60;

        counterLabel->setText(QString("Time Left: %1:%2:%3")
            .arg(hours, 2, 10, QLatin1Char('0'))
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0')));

        if (remainingSeconds == 0) {
            timer->stop();
            emit timeFinished();
        }
    });
}

void CounterManager::start(Settings *settings, int remainingSeconds)
{
    this->remainingSeconds = remainingSeconds;
    this->settings = settings;
    timer->start(1000);
}

void CounterManager::stop()
{
    timer->stop();
}
