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

#include "nvmonitorplugin.h"
#include "nvmonitor.h"
#include "nvmonitorconfiguration.h"

#include <QVBoxLayout>

NvMonitor::NvMonitor(const ILXQtPanelPluginStartupInfo &startupInfo)
    : QObject(),
      ILXQtPanelPlugin(startupInfo),
      mWidget(new QWidget()),
      mContent(nullptr)
{
    QVBoxLayout *layout = new QVBoxLayout(mWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    mContent = new NvMonitorContent(this, mWidget);
    layout->addWidget(mContent);
    layout->setStretchFactor(mContent, 1);

    settingsChanged();
}

NvMonitor::~NvMonitor()
{
    delete mWidget;
}

QDialog *NvMonitor::configureDialog()
{
    return new NvMonitorConfiguration(settings(), mWidget);
}

void NvMonitor::settingsChanged()
{
    mContent->updateSettings(settings());
}
