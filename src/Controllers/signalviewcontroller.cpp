#include "signalviewcontroller.h"

namespace {
const char *kLeadNames[] = {"I", "II", "V", "None"};

QString leadLabelFromIndex(int index)
{
    if (index >= 0 && index < 3) {
        return QString::fromLatin1(kLeadNames[index]);
    }
    return QStringLiteral("None");
}
}

SignalViewController::SignalViewController(QWidget *parent)
    : QWidget(parent)
    , m_signalViewWidget(new SignalViewWidget(this))
{
}

SignalViewController::~SignalViewController()
{
}

SignalViewWidget *SignalViewController::signalWidget() const
{
    return m_signalViewWidget;
}

void SignalViewController::setData(const MIT_BIH_ECGData &data)
{
    if (data.nsigs.isEmpty()) {
        m_signalViewWidget->clearLeads();
        return;
    }

    QStringList names;
    const int leadCount = qMin(SignalViewWidget::MaxLeads, data.nsigs.size());
    for (int i = 0; i < leadCount; ++i) {
        names.append(leadLabelFromIndex(data.selectedLead[i]));
    }

    m_signalViewWidget->setLeads(data, names);
}
