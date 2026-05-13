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

    connect(ui->buttons, &QDialogButtonBox::clicked, this, &NvMonitorConfiguration::dialogButtonsAction);

    loadSettings();

    // Подключение сигналов
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
            ui->graphColorPB->setPalette(QPalette(color));
            ui->graphColorPB->setStyleSheet("QPushButton { border: 1px solid palette(midlight); }");
            if (!mLockSettingChanges) {
                this->settings().setValue(QStringLiteral("graph/color"), color.name());
            }
        }
    });
    connect(ui->gridColorPB, &QPushButton::clicked, this, [this]() {
        QColor color = QColorDialog::getColor(mGridColor, this, tr("Select grid color"));
        if (color.isValid()) {
            mGridColor = color;
            ui->gridColorPB->setPalette(QPalette(color));
            ui->gridColorPB->setStyleSheet("QPushButton { border: 1px solid palette(midlight); }");
            if (!mLockSettingChanges) {
                this->settings().setValue(QStringLiteral("grid/color"), color.name());
            }
        }
    });
    connect(ui->titleColorPB, &QPushButton::clicked, this, [this]() {
        QColor color = QColorDialog::getColor(mTitleColor, this, tr("Select title color"));
        if (color.isValid()) {
            mTitleColor = color;
            ui->titleColorPB->setPalette(QPalette(color));
            ui->titleColorPB->setStyleSheet("QPushButton { border: 1px solid palette(midlight); }");
            if (!mLockSettingChanges) {
                this->settings().setValue(QStringLiteral("title/color"), color.name());
            }
        }
    });
    connect(ui->titleLE, &QLineEdit::textChanged,
            this, &NvMonitorConfiguration::titleTextChanged);
    connect(ui->gridLinesSB, qOverload<int>(&QSpinBox::valueChanged),
            this, &NvMonitorConfiguration::gridLinesChanged);
    connect(ui->maxHistorySB, qOverload<int>(&QSpinBox::valueChanged),
            this, &NvMonitorConfiguration::maxHistoryChanged);
    connect(ui->minimalSizeSB, qOverload<int>(&QSpinBox::valueChanged),
            this, &NvMonitorConfiguration::minimalSizeChanged);
}

NvMonitorConfiguration::~NvMonitorConfiguration()
{
    delete ui;
}

void NvMonitorConfiguration::loadSettings()
{
    mLockSettingChanges = true;

    // Заполняем список метрик
    ui->metricCOB->clear();
    ui->metricCOB->addItem(tr("GPU Utilization"), QStringLiteral("gpuUtilization"));
    ui->metricCOB->addItem(tr("VRAM Utilization"), QStringLiteral("memUtilization"));
    ui->metricCOB->addItem(tr("Temperature"), QStringLiteral("temperature"));

    QString metric = settings().value(QStringLiteral("data/metric"), QStringLiteral("gpuUtilization")).toString();
    int metricIndex = ui->metricCOB->findData(metric);
    ui->metricCOB->setCurrentIndex(metricIndex >= 0 ? metricIndex : 0);

    ui->updateIntervalDSB->setValue(settings().value(QStringLiteral("graph/updateInterval"), 1.0).toDouble());
    ui->showValueCB->setChecked(settings().value(QStringLiteral("graph/showValue"), false).toBool());
    ui->useThemeColorsCB->setChecked(settings().value(QStringLiteral("graph/useThemeColors"), true).toBool());

    QString graphColorName = settings().value(QStringLiteral("graph/color"), QStringLiteral("#ff0000")).toString();
    mGraphColor = QColor(graphColorName);
    ui->graphColorPB->setPalette(QPalette(mGraphColor));
    ui->graphColorPB->setStyleSheet("QPushButton { border: 1px solid palette(midlight); }");

    QString gridColorName = settings().value(QStringLiteral("grid/color"), QStringLiteral("#c0c0c0")).toString();
    mGridColor = QColor(gridColorName);
    ui->gridColorPB->setPalette(QPalette(mGridColor));
    ui->gridColorPB->setStyleSheet("QPushButton { border: 1px solid palette(midlight); }");

    QString titleColorName = settings().value(QStringLiteral("title/color"), QStringLiteral("#ffffff")).toString();
    mTitleColor = QColor(titleColorName);
    ui->titleColorPB->setPalette(QPalette(mTitleColor));
    ui->titleColorPB->setStyleSheet("QPushButton { border: 1px solid palette(midlight); }");

    ui->titleLE->setText(settings().value(QStringLiteral("title/label"), QString()).toString());
    ui->gridLinesSB->setValue(settings().value(QStringLiteral("grid/lines"), 1).toInt());
    ui->maxHistorySB->setValue(settings().value(QStringLiteral("graph/maxHistory"), 100).toInt());
    ui->minimalSizeSB->setValue(settings().value(QStringLiteral("graph/minimalSize"), 30).toInt());

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

void NvMonitorConfiguration::showValueChanged(bool value)
{
    if (!mLockSettingChanges) {
        settings().setValue(QStringLiteral("graph/showValue"), value);
    }
}

void NvMonitorConfiguration::useThemeColorsChanged(bool checked)
{
    if (!mLockSettingChanges) {
        settings().setValue(QStringLiteral("graph/useThemeColors"), checked);
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

void NvMonitorConfiguration::maxHistoryChanged(int value)
{
    if (!mLockSettingChanges) {
        settings().setValue(QStringLiteral("graph/maxHistory"), value);
    }
}

void NvMonitorConfiguration::minimalSizeChanged(int value)
{
    if (!mLockSettingChanges) {
        settings().setValue(QStringLiteral("graph/minimalSize"), value);
    }
}
