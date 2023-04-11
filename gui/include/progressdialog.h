#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>
#include <QProgressBar>
#include <QPlainTextEdit>
#include <QScrollArea>

class QLabel;
class QPushButton;

class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(QApplication &app, QWidget *parent = nullptr);

public slots:
    void setMessage(const QString &message);
    void setProgress(const int &progress);

private:
    void quit(QApplication &app);

    QLabel *m_bannerLabel;
    QPlainTextEdit *m_messageLabel;
    QProgressBar *m_progressBar;
    QPushButton *m_cancelButton;
};

#endif // PROGRESSDIALOG_H
