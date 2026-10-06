#include "comparesettingsdialog.h"

#include <QCheckBox>
#include <QSignalBlocker>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

namespace {

QCheckBox *addCheck(QWidget *parent, QLayout *layout, const QString &text)
{
    auto *box = new QCheckBox(text, parent);
    layout->addWidget(box);
    return box;
}

QWidget *indentBox(QWidget *parent, QVBoxLayout *host, const QList<QCheckBox *> &checks)
{
    auto *indent = new QWidget(parent);
    auto *layout = new QVBoxLayout(indent);
    layout->setContentsMargins(22, 0, 0, 0);
    layout->setSpacing(4);
    for (QCheckBox *check : checks)
        layout->addWidget(check);
    host->addWidget(indent);
    return indent;
}

} // namespace

CompareSettingsDialog::CompareSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Compare test"));
    setModal(true);
    setMinimumWidth(560);

    auto *root = new QVBoxLayout(this);
    root->setSpacing(12);
    auto *intro = new QLabel(
        QStringLiteral("Choose what to write. A subtype left unchecked is counted with its group, so a fusion beat counts as PVC and a fusion matched to a PVC is a true positive."),
        this);
    intro->setObjectName(QStringLiteral("hintLabel"));
    intro->setWordWrap(true);
    root->addWidget(intro);

    auto *columns = new QHBoxLayout;
    root->addLayout(columns);

    auto *tablesBox = new QGroupBox(QStringLiteral("Tables"), this);
    auto *tablesLayout = new QVBoxLayout(tablesBox);
    m_beats = addCheck(tablesBox, tablesLayout, QStringLiteral("Beat classification"));
    m_qrs = addCheck(tablesBox, tablesLayout, QStringLiteral("QRS detection"));
    m_shutdown = addCheck(tablesBox, tablesLayout, QStringLiteral("Missed beats"));
    m_vRuns = addCheck(tablesBox, tablesLayout, QStringLiteral("Ventricular runs"));
    m_sRuns = addCheck(tablesBox, tablesLayout, QStringLiteral("Supraventricular runs"));
    tablesLayout->addStretch();
    columns->addWidget(tablesBox);

    auto *beatsBox = new QGroupBox(QStringLiteral("Beat types"), this);
    auto *beatsLayout = new QVBoxLayout(beatsBox);

    m_groupNormal = addCheck(beatsBox, beatsLayout, QStringLiteral("Normal"));
    m_bundle = new QCheckBox(QStringLiteral("Bundle branch"), beatsBox);
    m_supra = new QCheckBox(QStringLiteral("Supraventricular"), beatsBox);
    indentBox(beatsBox, beatsLayout, {m_bundle, m_supra});

    m_groupPvc = addCheck(beatsBox, beatsLayout, QStringLiteral("PVC"));
    m_fusion = new QCheckBox(QStringLiteral("Fusion"), beatsBox);
    m_escape = new QCheckBox(QStringLiteral("Escape"), beatsBox);
    m_interpolated = new QCheckBox(QStringLiteral("Interpolated"), beatsBox);
    indentBox(beatsBox, beatsLayout, {m_fusion, m_escape, m_interpolated});

    m_unknown = addCheck(beatsBox, beatsLayout, QStringLiteral("Unknown"));
    beatsLayout->addStretch();
    columns->addWidget(beatsBox);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_ok = buttons->button(QDialogButtonBox::Ok);
    m_ok->setText(QStringLiteral("Compare"));
    m_ok->setProperty("primary", true);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    root->addWidget(buttons);

    const auto tables = {m_beats, m_qrs, m_shutdown, m_vRuns, m_sRuns};
    for (QCheckBox *box : tables)
        connect(box, &QCheckBox::toggled, this, &CompareSettingsDialog::refreshOkButton);

    bindGroup(m_groupNormal, {m_bundle, m_supra});
    bindGroup(m_groupPvc, {m_fusion, m_escape, m_interpolated});

    setProfile(BeatTestProfile());
}

void CompareSettingsDialog::bindGroup(QCheckBox *group, const QList<QCheckBox *> &children)
{
    connect(group, &QCheckBox::toggled, this, [group, children](bool checked) {
        for (QCheckBox *child : children) {
            child->setEnabled(checked);
            if (!checked)
                child->setChecked(false);
        }
    });

    for (QCheckBox *child : children) {
        connect(child, &QCheckBox::toggled, this, [group](bool checked) {
            if (checked && !group->isChecked())
                group->setChecked(true);
        });
    }
}

void CompareSettingsDialog::refreshOkButton()
{
    const bool anyTable = m_beats->isChecked() || m_qrs->isChecked() || m_shutdown->isChecked()
        || m_vRuns->isChecked() || m_sRuns->isChecked();
    m_ok->setEnabled(anyTable);
}

void CompareSettingsDialog::setProfile(const BeatTestProfile &profile)
{
    const QSignalBlocker blockBeats(m_beats);
    const QSignalBlocker blockQrs(m_qrs);
    const QSignalBlocker blockShutdown(m_shutdown);
    const QSignalBlocker blockV(m_vRuns);
    const QSignalBlocker blockS(m_sRuns);
    const QSignalBlocker blockNormal(m_groupNormal);
    const QSignalBlocker blockB(m_bundle);
    const QSignalBlocker blockSv(m_supra);
    const QSignalBlocker blockPvc(m_groupPvc);
    const QSignalBlocker blockF(m_fusion);
    const QSignalBlocker blockE(m_escape);
    const QSignalBlocker blockI(m_interpolated);
    const QSignalBlocker blockU(m_unknown);

    m_beats->setChecked(profile.reportBeats);
    m_qrs->setChecked(profile.reportQrs);
    m_shutdown->setChecked(profile.reportShutdown);
    m_vRuns->setChecked(profile.reportVentricularRuns);
    m_sRuns->setChecked(profile.reportSupraventricularRuns);

    m_groupNormal->setChecked(profile.groupNormal);
    m_bundle->setChecked(profile.separateBundle);
    m_supra->setChecked(profile.separateSupra);
    const bool normalOn = profile.groupNormal;
    m_bundle->setEnabled(normalOn);
    m_supra->setEnabled(normalOn);

    m_groupPvc->setChecked(profile.groupPvc);
    m_fusion->setChecked(profile.separateFusion);
    m_escape->setChecked(profile.separateEscape);
    m_interpolated->setChecked(profile.separateInterpolated);
    const bool pvcOn = profile.groupPvc;
    m_fusion->setEnabled(pvcOn);
    m_escape->setEnabled(pvcOn);
    m_interpolated->setEnabled(pvcOn);

    m_unknown->setChecked(profile.groupUnknown);
    refreshOkButton();
}

BeatTestProfile CompareSettingsDialog::profile() const
{
    BeatTestProfile profile;
    profile.reportBeats = m_beats->isChecked();
    profile.reportQrs = m_qrs->isChecked();
    profile.reportShutdown = m_shutdown->isChecked();
    profile.reportVentricularRuns = m_vRuns->isChecked();
    profile.reportSupraventricularRuns = m_sRuns->isChecked();

    profile.groupNormal = m_groupNormal->isChecked();
    profile.separateNormal = false;
    profile.separateBundle = m_bundle->isChecked();
    profile.separateSupra = m_supra->isChecked();

    profile.groupPvc = m_groupPvc->isChecked();
    profile.separatePvc = false;
    profile.separateFusion = m_fusion->isChecked();
    profile.separateEscape = m_escape->isChecked();
    profile.separateInterpolated = m_interpolated->isChecked();

    profile.groupUnknown = m_unknown->isChecked();
    return profile;
}
