#ifndef SHEETANALYSER_H
#define SHEETANALYSER_H

#include <QObject>
#include <Models/csv.h>

class SheetAnalyser : public QObject
{
    Q_OBJECT
public:
    explicit SheetAnalyser(QObject *parent = nullptr,
                           QStringList sheetPathList = {},
                           QString outputPath = "",
                           int processTime = 0);
    bool processSheets();

private:
    QStringList m_sheetPathList;
    QString m_outputPath;
    int m_processTime = 0;
};

#endif // SHEETANALYSER_H
