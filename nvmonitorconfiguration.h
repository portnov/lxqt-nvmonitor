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

#ifndef NVMONITORCONFIGURATION_H
#define NVMONITORCONFIGURATION_H

#include "lxqtpanelpluginconfigdialog.h"
#include "pluginsettings.h"

#include <QColor>

namespace Ui {
    class NvMonitorConfiguration;
}

class NvMonitorConfiguration : public LXQtPanelPluginConfigDialog
{
    Q_OBJECT

public:
    explicit NvMonitorConfiguration(PluginSettings *settings, QWidget *parent = nullptr);
    ~NvMonitorConfiguration() override;

private:
    Ui::NvMonitorConfiguration *ui;
    bool mLockSettingChanges;
    QColor mGraphColor;
    QColor mGridColor;
    QColor mTitleColor;

private slots:
    void loadSettings() override;
    void metricChanged(int index);
    void updateIntervalChanged(double value);
    void showValueChanged(bool value);
    void useThemeColorsChanged(bool checked);
    void graphColorChanged(const QColor &color);
    void gridColorChanged(const QColor &color);
    void titleColorChanged(const QColor &color);
    void titleTextChanged(const QString &text);
    void gridLinesChanged(int value);
    void maxHistoryChanged(int value);
    void minimalSizeChanged(int value);
};

#endif // NVMONITORCONFIGURATION_H
