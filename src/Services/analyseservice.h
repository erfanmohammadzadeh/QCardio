#ifndef ANALYSESERVICE_H
#define ANALYSESERVICE_H

#include <QObject>
#include "Models/sheetanalyser.h"

class AnalyseService : public QObject
{
    Q_OBJECT
public:
    explicit AnalyseService(QObject *parent = nullptr, AnalyseCfg cfg = AnalyseCfg());
    bool analyse();

signals:
    void sigAppendLog(const QString& text);
    void sigProgress(int current, int total);

private:
    AnalyseCfg m_analyseCfg;
    bool saveProcessFileResult(const QVector<FileProcessResult>& fileProcessRes);
};

#endif // ANALYSESERVICE_H
