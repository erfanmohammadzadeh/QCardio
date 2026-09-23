#include "csv.h"

CSV::CSV(QObject *parent, QString filename, CSVFormat csvData)
    : QObject{parent}, m_filename(filename), m_csvFormat(csvData)
{
}

CSVFormat CSV::csvFormat() const
{
    return m_csvFormat;
}

void CSV::loadRequested()
{
    QtConcurrent::run(QThreadPool::globalInstance(), [=]() {
        loadFromFile(m_filename);
        Q_EMIT sigReadyForRead();
    });
}

bool CSV::loadFromFile(const QString &path)
{
    m_csvFormat.sampleIndex.clear();
    m_csvFormat.type.clear();

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open CSV:" << path;
        return false;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty())
        {
            continue;
        }

        const QStringList fields = line.split(',');
        if (fields.size() < 2) {
            continue;
        }

        bool ok1 = false;
        bool ok2 = false;
        const int sampleIndex = fields[0].toInt(&ok1);
        const int type = fields[1].toInt(&ok2);

        if (!ok1 || !ok2) {
            continue;
        }

        m_csvFormat.sampleIndex.append(sampleIndex);
        m_csvFormat.type.append(static_cast<quint8>(type));
    }

    return true;
}

void CSV::loadData(const QString &path)
{
    loadFromFile(path);
}

void CSV::saveSignal()
{
    QFile file(m_filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);

    // Find minimum time for shifting
    long minTime = 0;
    int numAnnotations = m_csvFormat.sampleIndex.size();

    for (int i = 0; i < numAnnotations; ++i)
    {
        long sampleIndex = (m_csvFormat.sampleIndex.at(i) - minTime);
        out << sampleIndex << ","
            << m_csvFormat.type.at(i) << "\n";
    }

    file.close();
}

bool CSV::saveRawCSVRes(const SheetResult &res)
{
    QFile file(m_filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    int numAnnotations = res.alignedList1.size(); // Use the final aligned size

    for(int i = 0; i < numAnnotations; i++)
    {
        int diff = res.difIndexList.at(i);

        // FIX 2 & 3: Match only if it's NOT a desync marker (-1) AND within tolerance limit
        bool isMach = (diff != -1) && ((diff / 178) < 3);

        out << res.alignedList1.at(i) << ","
            << res.alignedList2.at(i) << ","
            << diff << ","
            << (isMach ? "QRS Macth" : "QRS not Macth") << ","
            << res.typeAlignList1.at(i) << ","
            << res.typeAlignList2.at(i) << ","
            << (res.difTypeList.at(i) ? "Type Macth" : "Type Not Macth")        << "\n";
    }
    file.close();
    return true;
}

bool CSV::saveProcessFileResult(const AnalyseFileProcessResult &fileProcessRes)
{
    QFile file201_103(m_filename);
    if (!file201_103.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out103(&file201_103);
    out103 << "FileName,"
              "Nn,"
              "Vn,"
              "On,"
              "Nv,"
              "Vv,"
              "Ov,"
              "Q Se,"
              "Q+P,"
              "N Se,"
              "N+P,"
              "V Se,"
              "V+P,"
              "VTN,"
              "V FPR"
              "\n";

    for (const FileProcessResult &res : fileProcessRes.fileProcessResult)
    {
        out103 << res.fileName << ","
               << res.beatTypeMap[ArrhythmiaType::NormalArr][ArrhythmiaType::NormalArr]           << ","
               << res.beatTypeMap[ArrhythmiaType::VentricularArr][ArrhythmiaType::NormalArr]      << ","
               << res.beatTypeMap[ArrhythmiaType::UnknownArr][ArrhythmiaType::NormalArr]          << ","
               << res.beatTypeMap[ArrhythmiaType::NormalArr][ArrhythmiaType::VentricularArr]      << ","
               << res.beatTypeMap[ArrhythmiaType::VentricularArr][ArrhythmiaType::VentricularArr] << ","
               << res.beatTypeMap[ArrhythmiaType::UnknownArr][ArrhythmiaType::VentricularArr]     << ","
               << res.qrsPredict.se    << ","
               << res.qrsPredict.p     << ","
               << res.normalPredict.se << ","
               << res.normalPredict.p  << ","
               << res.pvcPredict.se    << ","
               << res.pvcPredict.p     << ","
               << res.pvcPredict.tn    << ","
               << res.pvcPredict.fpr   <<
            "\n";

        if (out103.status() != QTextStream::Ok)
        {
            file201_103.close();
            return false;
        }
    }
    file201_103.close();

    //////////////////////////////////////
    QFile file201_104a(m_filename + "a.csv");
    if (!file201_104a.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out104a(&file201_104a);
    out104a << "FileName,"
               "Nx,"
               "Vx,"
               "Qx,"
               "beats missed,"
               "%N missed,"
               "%V missed,"
               "total shudown time"
               "\n";

    auto pct = [](int part, int total) -> QString {
        if (total <= 0) return QString("0.00");
        return QString::number(100.0 * static_cast<double>(part) / static_cast<double>(total), 'f', 2);
    };

    for (const FileProcessResult &res : fileProcessRes.fileProcessResult)
    {

        out104a << res.fileName                                                                << ","
                << res.beatTypeMap[ArrhythmiaType::NormalArr][ArrhythmiaType::UnknownArr]      << ","
                << res.beatTypeMap[ArrhythmiaType::VentricularArr][ArrhythmiaType::UnknownArr] << ","
                << res.beatTypeMap[ArrhythmiaType::UnknownArr][ArrhythmiaType::UnknownArr]     << ","
                << res.missedBeatCount                                                         << ","
                << pct(res.normalMissed, res.totalBeat)                                        << ","
                << pct(res.pvcMissed,    res.totalBeat)                                        << ","
                << res.totalShutdown.toString("hh:mm:ss")
                << "\n";

        if (out104a.status() != QTextStream::Ok)
        {
            file201_104a.close();
            return false;
        }
    }
    file201_104a.close();

    /////////////////////////////////
    QFile file201_104b(m_filename + "b.csv");
    if (!file201_104b.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out104b(&file201_104b);  // FIXED: was pointing to file201_103
    out104b << "FileName,"
               "N_TP(%),"
               "V_TP(%),"  // FIXED: was "%," without proper label
               "Q_TP(%),"
               "N_FN(%),"
               "V_FN(%),"
               "Q_FN(%),"
               "N_FP(%),"  // FIXED: added comma
               "V_FP(%),"
               "Q_FP(%),"
               "N_TN(%),"
               "V_TN(%),"
               "Q_TN(%)"
               "\n"; // FIXED: added comma and newline

    for (const FileProcessResult &res : fileProcessRes.fileProcessResult)
    {
        out104b << res.fileName << ","
                << QString("%1 (%2)").arg(res.normalPredict.tp).arg(res.normalPredict.rtp, 0, 'f', 2) << ","
                << QString("%1 (%2)").arg(res.pvcPredict.tp   ).arg(res.pvcPredict.rtp,    0, 'f', 2) << ","
                << QString("%1 (%2)").arg(res.qrsPredict.tp   ).arg(res.qrsPredict.rtp,    0, 'f', 2) << ","
                << QString("%1 (%2)").arg(res.normalPredict.fn).arg(res.normalPredict.rfn, 0, 'f', 2) << ","
                << QString("%1 (%2)").arg(res.pvcPredict.fn   ).arg(res.pvcPredict.rfn,    0, 'f', 2) << ","
                << QString("%1 (%2)").arg(res.qrsPredict.fn   ).arg(res.normalPredict.rfn, 0, 'f', 2) << ","
                << QString("%1 (%2)").arg(res.normalPredict.fp).arg(res.normalPredict.rfp, 0, 'f', 2) << ","
                << QString("%1 (%2)").arg(res.pvcPredict.fp   ).arg(res.pvcPredict.rfp,    0, 'f', 2) << ","
                << QString("%1 (%2)").arg(res.qrsPredict.fp   ).arg(res.qrsPredict.rfp,    0, 'f', 2) << ","
                << QString("%1 (%2)").arg(res.normalPredict.tn).arg(res.normalPredict.rtn, 0, 'f', 2) << ","
                << QString("%1 (%2)").arg(res.pvcPredict.tn   ).arg(res.pvcPredict.rtn,    0, 'f', 2) << ","
                << QString("%1 (%2)").arg(res.qrsPredict.tn   ).arg(res.qrsPredict.rtn,    0, 'f', 2)
                << "\n";

        if (out104b.status() != QTextStream::Ok)
        {
            file201_104b.close();
            return false;
        }
    }
    out104b << "\n\n\n";
    out104b << "Avg Se QRS,"
               "Avg Se PVC,"
               "Avg Se Normal,"
               "Gross QRS,"
               "Gross PVC,"
               "Gross Normal"
               "\n";
    out104b << QString("%1").arg(fileProcessRes.avgSeQrs, 0, 'f', 2) << ","
            << QString("%1").arg(fileProcessRes.avgSePvc, 0, 'f', 2) << ","
            << QString("%1").arg(fileProcessRes.avgSeNor, 0, 'f', 2) << ","
            << QString("%1").arg(fileProcessRes.avgPPQrs, 0, 'f', 2) << ","
            << QString("%1").arg(fileProcessRes.avgPPPvc, 0, 'f', 2) << ","
            << QString("%1").arg(fileProcessRes.avgPPNor, 0, 'f', 2) << ","

               ;
    file201_104b.close();
    return true;
}



QString CSV::getFilename() const
{
    return m_filename;
}
