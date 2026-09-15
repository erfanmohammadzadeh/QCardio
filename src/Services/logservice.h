#ifndef LOGSERVICE_H
#define LOGSERVICE_H

#include <QObject>
#include <QTextEdit>

#define SHOW_LOG_TERMINAL false

class LogService : public QObject
{
    Q_OBJECT
public:
    explicit LogService(QObject *parent = nullptr, QTextEdit* logfiled = nullptr);

public Q_SLOTS:
    void sltAppendLog(const QString& logText);

private:
    QTextEdit *m_logFiled = nullptr;
    int m_logCounter = 0;
    int m_maxLogCount = 1000;
};

#endif // LOGSERVICE_H
