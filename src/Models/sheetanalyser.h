#ifndef SHEETANALYSER_H
#define SHEETANALYSER_H

#include <QObject>
#include <Models/csv.h>

class SheetAnalyser : public QObject
{
    Q_OBJECT
public:
    explicit SheetAnalyser(QObject *parent = nullptr, QStringList sheetPathList = {}, QString outputPath = "", int processTime = 0);
    void processSheets();
private:
    QStringList m_sheetPathList;
    QString m_outputPath;
    CSV *m_csv1 = nullptr;
    CSV *m_csv2 = nullptr;

signals:
    void sigStartCompare();
};

#endif // SHEETANALYSER_H
