/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org
 *
 * Copyright: 2024 LXQt Developers
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "nvmonitorconfiguration.h"
#include "ui_nvmonitorconfiguration.h"

#include <QColorDialog>
#include <QPushButton>

NvMonitorConfiguration::NvMonitorConfiguration(PluginSettings *settings, QWidget *parent)
    : LXQtPanelPluginConfigDialog(settings, parent)
    , ui(new Ui::NvMonitorConfiguration)
    , mLockSettingChanges(false)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName(QStringLiteral("NvMonitorConfigurationWindow"));
    ui->setupUi(this);

    // Dialog buttons: Close/Reset connected via .ui (accepted→accept, rejected→reject)
    connect(ui->buttons, &QDialogButtonBox::clicked, this, &NvMonitorConfiguration::dialogButtonsAction);

    loadSettings();

    // Connect signals
    connect(ui->metricCOB, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &NvMonitorConfiguration::metricChanged);
    connect(ui->updateIntervalDSB, &QDoubleSpinBox::valueChanged,
            this, &NvMonitorConfiguration::updateIntervalChanged);
    connect(ui->showValueCB, &QCheckBox::toggled,
            this, &NvMonitorConfiguration::showValueChanged);
    connect(ui->useThemeColorsCB, &QCheckBox::toggled,
            this, &NvMonitorConfiguration::useThemeColorsChanged);
    connect(ui->graphColorPB, &QPushButton::clicked, this, [this]() {
        QColor color = QColorDialog::getColor(mGraphColor, this, tr("Select graph color"));
        if (color.isValid()) {
            mGraphColor = color;
            setColorButton(ui->graphColorPB, color);
            if (!mLockSettingChanges) {
                this->settings().setValue(QStringLiteral("graph/color"), color.name());
            }
        }
    });
    connect(ui->gridColorPB, &QPushButton::clicked, this, [this]() {
        QColor color = QColorDialog::getColor(mGridColor, this, tr("Select grid color"));
        if (color.isValid()) {
            mGridColor = color;
            setColorButton(ui->gridColorPB, color);
            if (!mLockSettingChanges) {
                this->settings().setValue(QStringLiteral("grid/color"), color.name());
            }
        }
    });
    connect(ui->titleColorPB, &QPushButton::clicked, this, [this]() {
        QColor color = QColorDialog::getColor(mTitleColor, this, tr("Select title color"));
        if (color.isValid()) {
            mTitleColor = color;
            setColorButton(ui->titleColorPB, color);
            if (!mLockSettingChanges) {
                this->settings().setValue(QStringLiteral("title/color"), color.name());
            }
        }
    });
    connect(ui->valueColorPB, &QPushButton::clicked, this, [this]() {
        QColor color = QColorDialog::getColor(mValueColor, this, tr("Select value color"));
        if (color.isValid()) {
            mValueColor = color;
            setColorButton(ui->valueColorPB, color);
            if (!mLockSettingChanges) {
                this->settings().setValue(QStringLiteral("graph/valueColor"), color.name());
            }
        }
    });
    connect(ui->titleLE, &QLineEdit::textChanged,
            this, &NvMonitorConfiguration::titleTextChanged);
    connect(ui->gridLinesSB, qOverload<int>(&QSpinBox::valueChanged),
            this, &NvMonitorConfiguration::gridLinesChanged);
    connect(ui->minimalSizeSB, qOverload<int>(&QSpinBox::valueChanged),
            this, &NvMonitorConfiguration::minimalSizeChanged);
}

NvMonitorConfiguration::~NvMonitorConfiguration()
{
    delete ui;
}

void NvMonitorConfiguration::setColorButton(QPushButton *button, const QColor &color)
{
    // Qt6 styles (Breeze, Fusion) ignore setPalette for button backgrounds.
    // Using stylesheet with background-color is the reliable approach.
    button->setStyleSheet(
        QString("QPushButton { "
                "background-color: %1; "
                "border: 1px solid #888888; "
                "border-radius: 2px; "
                "min-width: 24px; "
                "min-height: 24px; "
                "}")
        .arg(color.name(QColor::HexRgb)));
}

void NvMonitorConfiguration::loadSettings()
{
    mLockSettingChanges = true;

    // Populate metric list
    ui->metricCOB->clear();
    ui->metricCOB->addItem(tr("GPU Utilization"), QStringLiteral("gpuUtilization"));
    ui->metricCOB->addItem(tr("VRAM Load"), QStringLiteral("memUtilization"));
    ui->metricCOB->addItem(tr("VRAM Usage Percent"), QStringLiteral("vramUsage"));
    ui->metricCOB->addItem(tr("Temperature"), QStringLiteral("temperature"));
    ui->metricCOB->addItem(tr("Power Usage"), QStringLiteral("powerUsage"));

    QString metric = settings().value(QStringLiteral("data/metric"), QStringLiteral("gpuUtilization")).toString();
    int metricIndex = ui->metricCOB->findData(metric);
    ui->metricCOB->setCurrentIndex(metricIndex >= 0 ? metricIndex : 0);

    // Stored in ms, spinbox shows seconds — convert back
    ui->updateIntervalDSB->setValue(settings().value(QStringLiteral("graph/updateInterval"), 1000).toDouble() / 1000.0);
    ui->showValueCB->setChecked(settings().value(QStringLiteral("graph/showValue"), false).toBool());
    ui->useThemeColorsCB->setChecked(settings().value(QStringLiteral("graph/useThemeColors"), true).toBool());

    QString graphColorName = settings().value(QStringLiteral("graph/color"), QStringLiteral("#ff0000")).toString();
    mGraphColor = QColor(graphColorName);
    setColorButton(ui->graphColorPB, mGraphColor);

    QString gridColorName = settings().value(QStringLiteral("grid/color"), QStringLiteral("#c0c0c0")).toString();
    mGridColor = QColor(gridColorName);
    setColorButton(ui->gridColorPB, mGridColor);

    QString titleColorName = settings().value(QStringLiteral("title/color"), QStringLiteral("#ffffff")).toString();
    mTitleColor = QColor(titleColorName);
    setColorButton(ui->titleColorPB, mTitleColor);

    QString valueColorName = settings().value(QStringLiteral("graph/valueColor"), QStringLiteral("#ffffff")).toString();
    mValueColor = QColor(valueColorName);
    setColorButton(ui->valueColorPB, mValueColor);

    ui->titleLE->setText(settings().value(QStringLiteral("title/label"), QString()).toString());
    ui->gridLinesSB->setValue(settings().value(QStringLiteral("grid/lines"), 1).toInt());
    ui->minimalSizeSB->setValue(settings().value(QStringLiteral("graph/minimalSize"), 30).toInt());

    // Update initial enabled states of color buttons
    useThemeColorsChanged(ui->useThemeColorsCB->isChecked());
    showValueChanged(ui->showValueCB->isChecked());

    mLockSettingChanges = false;
}

void NvMonitorConfiguration::metricChanged(int index)
{
    if (!mLockSettingChanges) {
        settings().setValue(QStringLiteral("data/metric"), ui->metricCOB->itemData(index).toString());
    }
}

void NvMonitorConfiguration::updateIntervalChanged(double value)
{
    if (!mLockSettingChanges) {
        settings().setValue(QStringLiteral("graph/updateInterval"), value * 1000);
    }
}

void NvMonitorConfiguration::showValueChanged(bool checked)
{
    if (!mLockSettingChanges) {
        settings().setValue(QStringLiteral("graph/showValue"), checked);
    }
    // Enable/disable value color button based on show value checkbox
    ui->valueColorPB->setEnabled(checked && !ui->useThemeColorsCB->isChecked());
    ui->valueColorL->setEnabled(checked && !ui->useThemeColorsCB->isChecked());
}

void NvMonitorConfiguration::useThemeColorsChanged(bool checked)
{
    if (!mLockSettingChanges) {
        settings().setValue(QStringLiteral("graph/useThemeColors"), checked);
    }
    // Enable/disable custom color buttons based on theme colors checkbox
    bool enableCustomColors = !checked;
    ui->graphColorPB->setEnabled(enableCustomColors);
    ui->graphColorL->setEnabled(enableCustomColors);
    ui->gridColorPB->setEnabled(enableCustomColors);
    ui->gridColorL->setEnabled(enableCustomColors);
    ui->titleColorPB->setEnabled(enableCustomColors);
    ui->titleColorL->setEnabled(enableCustomColors);
    ui->valueColorPB->setEnabled(enableCustomColors && ui->showValueCB->isChecked());
    ui->valueColorL->setEnabled(enableCustomColors && ui->showValueCB->isChecked());
}

void NvMonitorConfiguration::valueColorChanged(const QColor &color)
{
    if (!mLockSettingChanges) {
        settings().setValue(QStringLiteral("graph/valueColor"), color.name());
    }
}

void NvMonitorConfiguration::titleTextChanged(const QString &text)
{
    if (!mLockSettingChanges) {
        settings().setValue(QStringLiteral("title/label"), text);
    }
}

void NvMonitorConfiguration::gridLinesChanged(int value)
{
    if (!mLockSettingChanges) {
        settings().setValue(QStringLiteral("grid/lines"), value);
    }
}

void NvMonitorConfiguration::minimalSizeChanged(int value)
{
    if (!mLockSettingChanges) {
        settings().setValue(QStringLiteral("graph/minimalSize"), value);
    }
}
