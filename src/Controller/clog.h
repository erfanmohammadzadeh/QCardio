#ifndef CLOG_H
#define CLOG_H

#include <QObject>
#include <QTextEdit>
#include <QDateTime>
class CLog : public QObject
{
    Q_OBJECT
public:
    explicit CLog(QObject *parent = nullptr, QTextEdit* logfiled = nullptr);
    QTextEdit *m_logFiled;
    int m_logCounter = 0;

public Q_SLOTS:
    void sltAppendLog(const QString& logText);


signals:
};

#endif // CLOG_H
