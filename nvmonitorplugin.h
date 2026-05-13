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

#ifndef NVMONITORPLUGIN_H
#define NVMONITORPLUGIN_H

#include "ilxqtpanelplugin.h"
#include "nvmonitorconfiguration.h"

class QWidget;
class NvMonitorContent;

class NvMonitor : public QObject, public ILXQtPanelPlugin
{
    Q_OBJECT
public:
    explicit NvMonitor(const ILXQtPanelPluginStartupInfo &startupInfo);
    ~NvMonitor() override;

    // Required methods
    QWidget *widget() override { return mWidget; }
    QString themeId() const override { return QStringLiteral("NvMonitor"); }

    // Optional methods
    Flags flags() const override { return PreferRightAlignment | HaveConfigDialog; }
    bool isSeparate() const override { return true; }
    QDialog *configureDialog() override;

protected:
    void settingsChanged() override;

private:
    QWidget *mWidget;
    NvMonitorContent *mContent;
};

class NvMonitorLibrary: public QObject, public ILXQtPanelPluginLibrary
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "lxqt.org/Panel/PluginInterface/3.0")
    Q_INTERFACES(ILXQtPanelPluginLibrary)
public:
    ILXQtPanelPlugin *instance(const ILXQtPanelPluginStartupInfo &startupInfo) const override
    {
        return new NvMonitor(startupInfo);
    }
};

#endif // NVMONITORPLUGIN_H
