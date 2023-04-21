#include "progressdialog.h"

#include <QScreen>
#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QPlainTextEdit>
#include <QScrollArea>

#include <windows.h>

ProgressDialog::ProgressDialog(QApplication &app, QWidget *parent)
    : QDialog(parent)
{
    // window screen size
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();
    move(0.25 * screenWidth, 0.25 * screenHeight);
    setFixedSize(0.5 * screenWidth, 0.5 * screenHeight);

    // Banner
    m_bannerLabel = new QLabel(this);
    m_bannerLabel->setPixmap(QPixmap(":/images/background.jpg"));
    m_bannerLabel->setScaledContents(true);
    m_bannerLabel->setGeometry(0, 0, width(), height());
    m_bannerLabel->move(0, 0);
    m_bannerLabel->setParent(this);

    // Message
    m_messageLabel = new QPlainTextEdit(this);
    m_messageLabel->setReadOnly(true);
    m_messageLabel->setStyleSheet("background-color: transparent; color: white; font-weight: bold;");

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(m_messageLabel);
    scrollArea->setFixedWidth(width() - 30);
    scrollArea->setFixedHeight(height() - 30);
    scrollArea->setStyleSheet("background-color: transparent; border: none;");

    // Progress bar
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->hide();

    // Cancel button
    m_cancelButton = new QPushButton("Exit", this);
    m_cancelButton->setStyleSheet("QPushButton {"
                                  "background-color: SlateGray;"
                                  "font-weight: bold;"
                                  "color: white;"
                                  "border: none;"
                                  "padding: 10px 13px;"
                                  "border-radius: 0px;"
                                  "}"
                                  "QPushButton:hover {"
                                  "background-color: #4e5965;"
                                  "}");

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(scrollArea, 0, Qt::AlignHCenter);
    mainLayout->addWidget(m_progressBar, 0, Qt::AlignHCenter);
    mainLayout->addStretch();
    mainLayout->addWidget(m_cancelButton, 0, Qt::AlignRight);

    // Signals and slots
    connect(m_cancelButton, &QPushButton::clicked, this, [&app, this]() {
        this->quit(app);
    });

    setWindowTitle("Loading...");

    setModal(true);
    setWindowModality(Qt::ApplicationModal);
    //setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
}

void ProgressDialog::setMessage(const QString &message)
{
    m_messageLabel->appendPlainText(message);
    QApplication::processEvents();
}

void ProgressDialog::setProgress(const int &progress)
{
    m_progressBar->setValue(progress);
    QApplication::processEvents();
}

void ProgressDialog::quit(QApplication &app)
{
    int pid = app.applicationPid();
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    TerminateProcess(hProcess, 1);
}
