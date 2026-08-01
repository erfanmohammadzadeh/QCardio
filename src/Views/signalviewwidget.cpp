#include "signalviewwidget.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainterPath>
#include <QtMath>
#include <QTime>

const QColor SignalViewWidget::s_leadColors[MaxLeads] = {
    QColor(0, 160, 80),
    QColor(220, 30, 30),
    QColor(30, 90, 200)
};

namespace {
constexpr int kLabelColumnWidth = 30;
constexpr int kTopMargin = 30;
constexpr int kBottomMargin = 25;
constexpr int kRightMargin = 30;
constexpr qreal kMinorGridMm = 1.0;
constexpr qreal kMajorGridMm = 5.0;
constexpr int maxAmplitude = 40;
constexpr int minAmplitude = -40;

}

SignalViewWidget::SignalViewWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(640, 360);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    m_scrollBar = new QScrollBar(this);
    connect(m_scrollBar, &QScrollBar::valueChanged, this, &SignalViewWidget::sltValueChanged);
    m_scrollBar->setVisible(false);
}

void SignalViewWidget::setLeads(const MIT_BIH_ECGData& data,
                                const QStringList &names)
{
    QVector<QVector<qreal>> leads = data.nsigs;
    QVector<int> startIndexList = data.getStartIndex();
    QStringList labelList = data.getAnotLable();
    int sampleRate = data.sampling;

    m_leads.clear();
    m_leadNames.clear();
    m_beatLableList = labelList;
    m_startIndexList = startIndexList;
    m_auxList = data.getAuxList();

    configScrollBar(leads);

    const int count = qMin(MaxLeads, leads.size());
    for (int i = 0; i < count; ++i) {
        if (leads[i].isEmpty()) {
            continue;
        }
        m_leads.append(leads[i]);
        if (i < names.size()) {
            m_leadNames.append(names.at(i));
        } else {
            m_leadNames.append(QStringLiteral("Lead %1").arg(i + 1));
        }
    }

    m_sampleRate = qMax(1, sampleRate);
    m_adcPerMv = qMax<qreal>(1.0, data.adcPerMv);
    m_timeOffsetSec = 0.0;
    update();
}

void SignalViewWidget::clearLeads()
{
    m_leads.clear();
    m_leadNames.clear();
    m_timeOffsetSec = 0.0;
    update();
}

void SignalViewWidget::setPaperSpeed(qreal mmPerSecond)
{
    m_paperSpeedMmPerSec = qMax<qreal>(1.0, mmPerSecond);
    update();
}

void SignalViewWidget::setSensitivity(qreal mmPerMillivolt)
{
    m_sensitivityMmPerMv = qMax<qreal>(1.0, mmPerMillivolt);
    update();
}

void SignalViewWidget::setVisibleDuration(qreal seconds)
{
    m_visibleDurationSec = qMax<qreal>(1.0, seconds);
    m_timeOffsetSec = qBound(0.0, m_timeOffsetSec, maxTimeOffset());
    update();
}

int SignalViewWidget::activeLeadCount() const
{
    return m_leads.size();
}

qreal SignalViewWidget::totalDurationSec() const
{
    if (m_leads.isEmpty() || m_sampleRate <= 0) {
        return 0.0;
    }

    int maxSamples = 0;
    for (const QVector<qreal> &lead : m_leads) {
        maxSamples = qMax(maxSamples, lead.size());
    }
    return maxSamples / static_cast<qreal>(m_sampleRate);
}

qreal SignalViewWidget::maxTimeOffset() const
{
    return qMax(0.0, totalDurationSec() - m_visibleDurationSec);
}

QRectF SignalViewWidget::plotRect() const
{
    const int leads = qMax(1, activeLeadCount());
    const qreal plotHeight = leads * m_stripHeightMm * m_pixelsPerMm;
    return QRectF(kLabelColumnWidth,
                  kTopMargin,
                  width() - kLabelColumnWidth - kRightMargin,
                  plotHeight);
}

QRectF SignalViewWidget::leadStripRect(int leadIndex) const
{
    const QRectF plot = plotRect();
    const qreal stripHeightPx = m_stripHeightMm * m_pixelsPerMm;
    return QRectF(plot.left(),
                  plot.top() + leadIndex * stripHeightPx,
                  plot.width(),
                  stripHeightPx);
}

qreal SignalViewWidget::sampleToMillivolts(qreal sample) const
{
    return sample / m_adcPerMv;
}

qreal SignalViewWidget::sampleIndexToTime(int index) const
{
    return index / static_cast<qreal>(m_sampleRate);
}

qreal SignalViewWidget::timeToX(qreal timeSec, const QRectF &plotArea) const
{
    // Fixed mapping: time to pixels using paper speed
    // The visible duration is now determined by the widget width
    const qreal relativeTime = timeSec - m_timeOffsetSec;
    return plotArea.left() + relativeTime * pixelsPerSecond();
}

qreal SignalViewWidget::millivoltsToY(qreal millivolts, qreal stripCenterY, double signalDC) const
{
    return stripCenterY - millivolts * m_sensitivityMmPerMv * m_pixelsPerMm + signalDC/5;
}

void SignalViewWidget::drawGrid(QPainter &painter, const QRectF &plotArea)
{
    const QColor minorColor(255, 182, 193, 180);
    const QColor majorColor(255, 105, 135, 220);

    const qreal minorPx = kMinorGridMm * m_pixelsPerMm;
    const qreal secondsPerMinor = kMinorGridMm / m_paperSpeedMmPerSec;
    const qreal mvPerMinor = kMinorGridMm / m_sensitivityMmPerMv;

    painter.fillRect(plotArea, QColor(255, 245, 245));

    const int leadCount = qMax(1, activeLeadCount());
    for (int leadIndex = 0; leadIndex < leadCount; ++leadIndex) {
        const QRectF strip = leadStripRect(leadIndex);
        const qreal centerY = strip.center().y();

        for (int col = 0; ; ++col) {
            const qreal x = plotArea.left() + col * minorPx;
            if (x > plotArea.right()) {
                break;
            }
            const bool isMajor = (col % static_cast<int>(kMajorGridMm)) == 0;
            painter.setPen(QPen(isMajor ? majorColor : minorColor, isMajor ? 1.2 : 0.8));
            painter.drawLine(QPointF(x, strip.top()), QPointF(x, strip.bottom()));
        }

        for (int step = minAmplitude; step <= maxAmplitude; ++step) {
            const qreal mv = step * mvPerMinor;
            const qreal y = millivoltsToY(mv, centerY);
            if (y < strip.top() || y > strip.bottom()) {
                continue;
            }

            const bool isMajor = (step % static_cast<int>(kMajorGridMm)) == 0;
            painter.setPen(QPen(isMajor ? majorColor : minorColor, isMajor ? 1.2 : 0.8));
            painter.drawLine(QPointF(plotArea.left(), y), QPointF(plotArea.right(), y));
        }
    }

    painter.setPen(QPen(QColor(180, 80, 90), 1.5));
    for (int leadIndex = 1; leadIndex < leadCount; ++leadIndex) {
        const qreal y = leadStripRect(leadIndex).top();
        painter.drawLine(QPointF(plotArea.left(), y), QPointF(plotArea.right(), y));
    }

    const int timeTicks = qMax(1, static_cast<int>(m_visibleDurationSec / secondsPerMinor / 5));
    const int labelCount = 3;
    const int intervalTick = static_cast<int>(timeTicks/(labelCount-1));
    painter.setPen(QPen(QColor(90, 90, 90)));
    QFont tickFont = painter.font();
    tickFont.setPointSize(8);
    painter.setFont(tickFont);

    for (int i = 0; i <= timeTicks; i+= intervalTick) {
        const qreal t = m_timeOffsetSec + (i / static_cast<qreal>(timeTicks)) * m_visibleDurationSec;
        const qreal x = timeToX(t, plotArea);
        QTime time = QTime(0, 0).addSecs(static_cast<int>(t));
        painter.drawLine(QPointF(x, plotArea.bottom()), QPointF(x, plotArea.bottom() + 4));
        painter.drawText(QRectF(x - 20, plotArea.bottom() + 4, 80, 16),
                         Qt::AlignHCenter | Qt::AlignTop,
                         time.toString(DISPLAY_TIME_DURATION_FORMAT));

    }
}

void SignalViewWidget::drawLeadLabel(QPainter &painter, int leadIndex, const QRectF &stripRect)
{
    painter.setPen(QPen(s_leadColors[leadIndex % MaxLeads], 2));
    QFont labelFont = painter.font();
    labelFont.setBold(true);
    labelFont.setPointSize(10);
    painter.setFont(labelFont);

    const QString label = leadIndex < m_leadNames.size()
                              ? m_leadNames.at(leadIndex)
                              : QStringLiteral("Lead %1").arg(leadIndex + 1);
    painter.drawText(QRectF(4, stripRect.top(), kLabelColumnWidth - 8, stripRect.height()),
                     Qt::AlignVCenter | Qt::AlignRight,
                     label);
}
#include <set>
void SignalViewWidget::drawLead(QPainter &painter, int leadIndex, const QRectF &stripRect)
{
    if (leadIndex >= m_leads.size()) {
        return;
    }

    const QVector<qreal> &lead = m_leads[leadIndex];
    if (lead.isEmpty()) {
        return;
    }

#if DEBUG_SIGNALVIEW
    if (leadIndex == 0) {
        qreal minVal = *std::min_element(lead.begin(), lead.end());
        qreal maxVal = *std::max_element(lead.begin(), lead.end());
        qDebug() << "Lead" << leadIndex << "range:" << minVal << "to" << maxVal
                 << "mV:" << sampleToMillivolts(minVal) << "to" << sampleToMillivolts(maxVal);
        qDebug() << "Sample rate:" << m_sampleRate;
        qDebug() << "Total samples:" << lead.size();
        qDebug() << "Time offset:" << m_timeOffsetSec;
        qDebug() << "Visible duration:" << m_visibleDurationSec;
    }
#endif

    const QRectF plotArea(stripRect.left(), stripRect.top(), stripRect.width(), stripRect.height());
    const qreal centerY = stripRect.center().y();
    const qreal endTime = m_timeOffsetSec + m_visibleDurationSec;

    const int startIndex = qMax(0, static_cast<int>(m_timeOffsetSec * m_sampleRate));
    const int endIndex = qMin(lead.size(), static_cast<int>(endTime * m_sampleRate) + 2);

    //calc median
    std::set<double> leadSrt;
    for(int i = 0; i < 15; i++) leadSrt.insert(static_cast<double>(lead[i]));
    qreal dcSignalY = lead[7];

    painter.setPen(QPen(s_leadColors[leadIndex % MaxLeads], 1.6));
    QPainterPath path;
    bool started = false;

    for (int i = startIndex; i < endIndex; ++i) {
        const qreal t = sampleIndexToTime(i);
        if (t < m_timeOffsetSec || t > endTime) {
            continue;
        }

        const qreal x = timeToX(t, plotArea);
        const qreal y = millivoltsToY(sampleToMillivolts(lead[i]), centerY, dcSignalY);

        if (y < stripRect.top() - 2.0 || y > stripRect.bottom() + 2.0) {
            if (started) {
                painter.drawPath(path);
                path = QPainterPath();
                started = false;
            }
            continue;
        }

        if (!started) {
            path.moveTo(x, y);
            started = true;
        } else {
            path.lineTo(x, y);
        }
    }

    if (started) {
        painter.drawPath(path);
    }

    /// Draw Label
    painter.save();
    painter.setPen(Qt::blue);
    QFont font = painter.font();
    font.setPointSize(12);
    font.setBold(true);
    painter.setFont(font);


    for (int beat = 0; beat < m_startIndexList.size(); ++beat)
    {
        int sample = m_startIndexList[beat];

        // Skip beats outside the visible range
        if (sample < startIndex || sample >= endIndex)
            continue;

        qreal t = sampleIndexToTime(sample);
        qreal x = timeToX(t, plotArea);

        QString label;
        if (beat < m_beatLableList.size())
            label = m_beatLableList[beat];
        else
            label = QString::number(beat + 1);

        painter.drawText(QPointF(x + 2, stripRect.top() + 15), label);
        if(label == '"')
        {
            painter.drawText(QPointF(x + 2, stripRect.top() + 25), m_auxList[beat]);
        }
    }

    painter.restore();


    drawLeadLabel(painter, leadIndex, stripRect);
}

void SignalViewWidget::drawScaleLegend(QPainter &painter, const QRectF &plotArea)
{
    QFont legendFont = painter.font();
    legendFont.setPointSize(9);
    painter.setFont(legendFont);
    painter.setPen(QPen(QColor(70, 70, 70)));

    const QString legend = QStringLiteral("%1 mm/s  |  %2 mm/mV  |  scroll to navigate")
                               .arg(m_paperSpeedMmPerSec, 0, 'f', 0)
                               .arg(m_sensitivityMmPerMv, 0, 'f', 0);
    painter.drawText(QRectF(plotArea.left(), 0, plotArea.width(), kTopMargin - 1),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     legend);
}

void SignalViewWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.fillRect(rect(), QColor(252, 252, 252));

    if (activeLeadCount() == 0) {
        painter.setPen(QPen(QColor(120, 120, 120)));
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("No ECG data loaded"));
        return;
    }

    const QRectF plotArea = plotRect();
    drawScaleLegend(painter, plotArea);
    drawGrid(painter, plotArea);

    for (int leadIndex = 0; leadIndex < activeLeadCount(); ++leadIndex) {
        drawLead(painter, leadIndex, leadStripRect(leadIndex));
    }
}

void SignalViewWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    //scrollBar policy
    m_scrollBar->setOrientation(Qt::Horizontal);
    m_scrollBar->setGeometry(0, height() - 18, width(), 25);

    updateVisibleDuration();
    update();
}

void SignalViewWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isPanning = true;
        m_lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void SignalViewWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_isPanning) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    const QRectF plotArea = plotRect();
    if (plotArea.width() <= 0.0) {
        return;
    }

    const qreal deltaPx = event->pos().x() - m_lastMousePos.x();
    const qreal deltaSec = -(deltaPx / plotArea.width()) * m_visibleDurationSec;
    m_timeOffsetSec = qBound(0.0, m_timeOffsetSec + deltaSec, maxTimeOffset());
    m_lastMousePos = event->pos();
    update();
    event->accept();
}

void SignalViewWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isPanning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void SignalViewWidget::wheelEvent(QWheelEvent *event)
{
    const int delta = event->angleDelta().y();
    if (delta == 0) {
        event->ignore();
        return;
    }

    const qreal scrollStep = m_visibleDurationSec * 0.08;
    if (delta > 0) {
        m_timeOffsetSec += scrollStep;
    } else {
        m_timeOffsetSec -= scrollStep;
    }

    m_timeOffsetSec = qBound(0.0, m_timeOffsetSec, maxTimeOffset());
    m_scrollBar->setValue(m_timeOffsetSec*m_sampleRate);
    update();
    event->accept();
}

void SignalViewWidget::updateVisibleDuration()
{
    const QRectF plot = plotRect();
    if (plot.width() <= 0) return;

    // Calculate visible duration based on plot width and pixels per second
    qreal pixelsPerSec = pixelsPerSecond();
    if (pixelsPerSec > 0) {
        m_visibleDurationSec = plot.width() / pixelsPerSec;
        // Clamp to reasonable values
        m_visibleDurationSec = qBound(1.0, m_visibleDurationSec, 60.0);

        // Adjust time offset if needed
        m_timeOffsetSec = qBound(0.0, m_timeOffsetSec, maxTimeOffset());
    }
}

qreal SignalViewWidget::pixelsPerSecond() const {
    // Calculate based on paper speed and pixels per mm
    return m_paperSpeedMmPerSec * m_pixelsPerMm;
}

void SignalViewWidget::configScrollBar(const  QVector<QVector<qreal>>& leads)
{
    int maxSamples = 0;
    for (const auto &lead : leads)
        maxSamples = qMax(maxSamples, lead.size());
    int visibleSamples = static_cast<int>(m_visibleDurationSec * m_sampleRate);
    m_scrollBar->setMaximum(qMax(0, maxSamples - visibleSamples));
    m_scrollBar->setRange(0, qMax(0, maxSamples - visibleSamples));
    m_scrollBar->setPageStep(visibleSamples);
    m_scrollBar->setSingleStep(m_sampleRate / 10);
    if(qMax(0, maxSamples - visibleSamples) > 0)
        m_scrollBar->setVisible(true);
}

void SignalViewWidget::sltValueChanged(int value)
{
    m_timeOffsetSec = value / static_cast<double>(m_sampleRate);
    update();
}