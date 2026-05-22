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

#include "nvmonitor.h"
#include "ilxqtpanelplugin.h"
#include "pluginsettings.h"

#include <QPainter>
#include <QPalette>
#include <QResizeEvent>
#include <QTimer>
#include <QCoreApplication>
#include <qmath.h>
#include <cmath>
#include <algorithm>

// ============================================================================
// NvmlGpu — NVML wrapper
// ============================================================================

NvmlGpu::NvmlGpu()
    : m_deviceCount(0)
    , m_nvmlInit(nullptr)
    , m_nvmlShutdown(nullptr)
    , m_nvmlDeviceGetCount(nullptr)
    , m_nvmlDeviceGetHandleByIndex(nullptr)
    , m_nvmlDeviceGetName(nullptr)
    , m_nvmlDeviceGetUtilizationRates(nullptr)
    , m_nvmlDeviceGetTemperature(nullptr)
    , m_nvmlDeviceGetMemoryInfo(nullptr)
    , m_nvmlDeviceGetPowerUsage(nullptr)
    , m_nvmlDeviceGetPowerManagementLimit(nullptr)
    , m_nvmlDeviceGetEnforcedPowerLimit(nullptr)
    , m_initialized(false)
{
}

NvmlGpu::~NvmlGpu()
{
    unloadLibrary();
}

bool NvmlGpu::loadLibrary()
{
    // Try to load the NVML library
    m_lib.setFileName("libnvidia-ml.so");
    if (!m_lib.load()) {
        m_lib.setFileName("libnvidia-ml.so.1");
        return m_lib.load();
    }
    return true;
}

void NvmlGpu::unloadLibrary()
{
    if (m_lib.isLoaded()) {
        m_lib.unload();
    }
}

bool NvmlGpu::loadSymbols()
{
    if (!(m_nvmlInit = (decltype(m_nvmlInit))m_lib.resolve("nvmlInit"))) return false;
    if (!(m_nvmlShutdown = (decltype(m_nvmlShutdown))m_lib.resolve("nvmlShutdown"))) return false;
    if (!(m_nvmlDeviceGetCount = (decltype(m_nvmlDeviceGetCount))m_lib.resolve("nvmlDeviceGetCount"))) return false;
    if (!(m_nvmlDeviceGetHandleByIndex = (decltype(m_nvmlDeviceGetHandleByIndex))m_lib.resolve("nvmlDeviceGetHandleByIndex"))) return false;
    if (!(m_nvmlDeviceGetName = (decltype(m_nvmlDeviceGetName))m_lib.resolve("nvmlDeviceGetName"))) return false;
    if (!(m_nvmlDeviceGetUtilizationRates = (decltype(m_nvmlDeviceGetUtilizationRates))m_lib.resolve("nvmlDeviceGetUtilizationRates"))) return false;
    if (!(m_nvmlDeviceGetTemperature = (decltype(m_nvmlDeviceGetTemperature))m_lib.resolve("nvmlDeviceGetTemperature"))) return false;
    if (!(m_nvmlDeviceGetMemoryInfo = (decltype(m_nvmlDeviceGetMemoryInfo))m_lib.resolve("nvmlDeviceGetMemoryInfo"))) return false;

    // Power functions are optional — some GPUs/drivers may not support them
    m_nvmlDeviceGetPowerUsage = (decltype(m_nvmlDeviceGetPowerUsage))m_lib.resolve("nvmlDeviceGetPowerUsage");
    m_nvmlDeviceGetPowerManagementLimit = (decltype(m_nvmlDeviceGetPowerManagementLimit))m_lib.resolve("nvmlDeviceGetPowerManagementLimit");
    m_nvmlDeviceGetEnforcedPowerLimit = (decltype(m_nvmlDeviceGetEnforcedPowerLimit))m_lib.resolve("nvmlDeviceGetEnforcedPowerLimit");

    return true;
}

bool NvmlGpu::init()
{
    if (m_initialized) {
        return m_deviceCount > 0;  // Already initialized
    }

    if (!loadLibrary()) {
        return false;
    }

    if (!loadSymbols()) {
        unloadLibrary();
        m_nvmlShutdown = nullptr;  // Prevent destructor call
        return false;
    }

    nvmlReturn_t result = m_nvmlInit();
    if (result != 0) {
        // nvmlInit failed — call shutdown for proper cleanup
        if (m_nvmlShutdown) {
            m_nvmlShutdown();
            m_nvmlShutdown = nullptr;
        }
        unloadLibrary();
        return false;
    }

    unsigned int devCount = 0;
    result = m_nvmlDeviceGetCount(&devCount);
    if (result != 0) {
        if (m_nvmlShutdown) {
            m_nvmlShutdown();
            m_nvmlShutdown = nullptr;
        }
        devCount = 0;
        unloadLibrary();
        return false;
    }

    m_deviceCount = static_cast<int>(devCount);
    m_initialized = true;
    return m_deviceCount > 0;
}

void NvmlGpu::shutdown()
{
    if (!m_initialized) {
        return;
    }
    if (m_nvmlShutdown) {
        m_nvmlShutdown();
        m_nvmlShutdown = nullptr;
    }
    unloadLibrary();
    m_deviceCount = 0;
    m_initialized = false;
}

QString NvmlGpu::getDeviceName(int index) const
{
    if (!m_nvmlDeviceGetName || index < 0) {
        return QString();
    }

    nvmlDevice_t device;
    nvmlReturn_t result = m_nvmlDeviceGetHandleByIndex(static_cast<unsigned int>(index), &device);
    if (result != 0) {
        return QString();
    }

    char name[64];
    result = m_nvmlDeviceGetName(device, name, sizeof(name));
    if (result != 0) {
        return QString();
    }

    return QString(name);
}

NvmlGpuData NvmlGpu::getDeviceData(int index) const
{
    NvmlGpuData data;

    if (!m_nvmlDeviceGetHandleByIndex || !m_nvmlDeviceGetUtilizationRates ||
        !m_nvmlDeviceGetTemperature || !m_nvmlDeviceGetMemoryInfo) {
        return data;
    }

    nvmlDevice_t device;
    nvmlReturn_t result = m_nvmlDeviceGetHandleByIndex(static_cast<unsigned int>(index), &device);
    if (result != 0) {
        return data;
    }

    // Get device name
    char name[64];
    if (m_nvmlDeviceGetName) {
        result = m_nvmlDeviceGetName(device, name, sizeof(name));
        if (result == 0) {
            data.name = QString(name);
        }
    }

    // Get GPU and memory utilization
    nvmlUtilization_t util;
    result = m_nvmlDeviceGetUtilizationRates(device, &util);
    if (result == 0) {
        data.gpuUtil = static_cast<float>(util.gpu);
        data.memUtil = static_cast<float>(util.memory);
        data.valid = true;
    }

    // Get temperature
    unsigned int temp = 0;
    result = m_nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temp);
    if (result == 0) {
        data.temp = static_cast<float>(temp);
    }

    // Get memory info
    nvmlMemory_t mem;
    result = m_nvmlDeviceGetMemoryInfo(device, &mem);
    if (result == 0) {
        data.memTotal = static_cast<qint64>(mem.total);
        data.memUsed = static_cast<qint64>(mem.used);
    }

    // Get power usage (current draw in microwatts)
    if (m_nvmlDeviceGetPowerUsage) {
        unsigned long long power = 0;
        result = m_nvmlDeviceGetPowerUsage(device, &power);
        if (result == 0) {
            data.powerUsage = static_cast<qint64>(power);
        }
    }

    // Get power management limit (max power in microwatts)
    // GetPowerManagementLimit is not supported on laptop GPUs, fall back to GetEnforcedPowerLimit
    if (m_nvmlDeviceGetPowerManagementLimit) {
        unsigned int maxPower = 0;
        result = m_nvmlDeviceGetPowerManagementLimit(device, &maxPower);
        if (result == 0) {
            data.powerMax = static_cast<qint64>(maxPower);
        } else if (m_nvmlDeviceGetEnforcedPowerLimit) {
            // Fallback: enforced power limit (works on laptop GPUs)
            result = m_nvmlDeviceGetEnforcedPowerLimit(device, &maxPower);
            if (result == 0) {
                data.powerMax = static_cast<qint64>(maxPower);
            }
        }
    } else if (m_nvmlDeviceGetEnforcedPowerLimit) {
        unsigned int maxPower = 0;
        result = m_nvmlDeviceGetEnforcedPowerLimit(device, &maxPower);
        if (result == 0) {
            data.powerMax = static_cast<qint64>(maxPower);
        }
    }

    return data;
}

// ============================================================================
// NvMonitorContent — graph widget
// ============================================================================

NvMonitorContent::NvMonitorContent(ILXQtPanelPlugin *plugin, QWidget *parent)
    : QWidget(parent)
    , mPlugin(plugin)
    , mMetric(NvMonitorContent::GpuUtilization)
    , mUpdateInterval(1000)
    , mMinimalSize(30)
    , mGridLines(1)
    , mTitleLabel()
    , mShowValue(false)
    , mUseThemeColors(true)
    , mGraphColor(Qt::red)
    , mBackgroundColor(Qt::transparent)
    , mGridColor(QColor(192, 192, 192))
    , mTitleColor(Qt::white)
    , mValueColor(Qt::white)
    , mGpu()
    , mGpuData()
    , mHistoryImage()
    , mHistoryOffset(0)
    , mTimerId(-1)
    , mTimerStarted(false)
    , mCurrentValue(0)
    , mNvmlAvailable(false)
{
    setObjectName(QStringLiteral("NvMonitor_Graph"));
    setMouseTracking(true);

    // Initialize NVML
    mNvmlAvailable = mGpu.init();

    if (mNvmlAvailable && mGpu.deviceCount() > 0) {
        mGpuData = mGpu.getDeviceData(0);
    }
}

NvMonitorContent::~NvMonitorContent()
{
    if (mTimerId != -1) {
        killTimer(mTimerId);
        mTimerId = -1;
    }
    mGpu.shutdown();
}

void NvMonitorContent::updateSettings(const PluginSettings *settings)
{
    int oldMetric = mMetric;
    bool oldShowValue = mShowValue;

    mUseThemeColors = settings->value(QStringLiteral("graph/useThemeColors"), true).toBool();
    mUpdateInterval = settings->value(QStringLiteral("graph/updateInterval"), 1000).toInt();
    mMinimalSize = settings->value(QStringLiteral("graph/minimalSize"), 30).toInt();
    mGridLines = settings->value(QStringLiteral("grid/lines"), 1).toInt();
    mTitleLabel = settings->value(QStringLiteral("title/label"), QString()).toString();
    mShowValue = settings->value(QStringLiteral("graph/showValue"), false).toBool();

    // Apply minimum size based on panel orientation
    setMinimumSize(mPlugin->panel()->isHorizontal() ? mMinimalSize : 2,
                   mPlugin->panel()->isHorizontal() ? 2 : mMinimalSize);

    // Apply theme or custom colors
    if (mUseThemeColors) {
        applyThemeColors();
    } else {
        mGraphColor = QColor(settings->value(QStringLiteral("graph/color"), QStringLiteral("#ff0000")).toString());
        mGridColor = QColor(settings->value(QStringLiteral("grid/color"), QStringLiteral("#c0c0c0")).toString());
        mTitleColor = QColor(settings->value(QStringLiteral("title/color"), QStringLiteral("#ffffff")).toString());
        mValueColor = QColor(settings->value(QStringLiteral("graph/valueColor"), QStringLiteral("#ffffff")).toString());
    }

    // Metric
    QString metricStr = settings->value(QStringLiteral("data/metric"), QStringLiteral("gpuUtilization")).toString();
    if (metricStr == QLatin1String("memUtilization")) {
        mMetric = NvMonitorContent::MemUtilization;
    } else if (metricStr == QLatin1String("vramUsage")) {
        mMetric = NvMonitorContent::VramUsage;
    } else if (metricStr == QLatin1String("temperature")) {
        mMetric = NvMonitorContent::Temperature;
    } else if (metricStr == QLatin1String("powerUsage")) {
        mMetric = NvMonitorContent::PowerUsage;
    } else {
        mMetric = NvMonitorContent::GpuUtilization;
    }

    // Update title font
    if (!mTitleLabel.isEmpty()) {
        if (mTitleFontPixelHeight <= 0) {
            // Font not yet initialized — use system font
            mTitleFont = QFont();
            mTitleFont.setBold(true);
            mTitleFont.setPixelSize(12);
        }
        QFontMetrics fm(mTitleFont);
        mTitleFontPixelHeight = fm.height();
    } else {
        mTitleFontPixelHeight = 0;
    }

    // Start timer if not already running
    if (!mTimerStarted) {
        mTimerId = startTimer(mUpdateInterval);
        mTimerStarted = true;
    } else {
        // Restart timer when interval changes
        killTimer(mTimerId);
        mTimerId = startTimer(mUpdateInterval);
    }

    // Recreate history when size or metric changes
    if (oldMetric != mMetric) {
        reset();
    } else {
        update();
    }
}

void NvMonitorContent::reset()
{
    setMinimumSize(mPlugin->panel()->isHorizontal() ? mMinimalSize : 2,
                   mPlugin->panel()->isHorizontal() ? 2 : mMinimalSize);

    mHistoryOffset = 0;
    mHistoryImage = QImage();

    // Create new history image
    if (width() > 0) {
        mHistoryImage = QImage(width(), 100, QImage::Format_ARGB32);
        mHistoryImage.fill(Qt::transparent);
    }

    update();
}

void NvMonitorContent::applyThemeColors()
{
    // Use widget palette for system colors
    QPalette pal = palette();
    mGraphColor = pal.color(QPalette::Text);        // Graph text
    mGridColor = pal.color(QPalette::Mid);           // Grid
    mTitleColor = pal.color(QPalette::WindowText);   // Title
}

void NvMonitorContent::collectData()
{
    // NVML is already initialized in the constructor, just collect data
    if (mNvmlAvailable && mGpu.deviceCount() > 0) {
        mGpuData = mGpu.getDeviceData(0);
    }
}

void NvMonitorContent::updateGraph()
{
    if (!mGpuData.valid) {
        return;
    }

    float value = 0;
    QString tooltip;

    switch (mMetric) {
        case NvMonitorContent::GpuUtilization:
            value = mGpuData.gpuUtil;
            tooltip = tr("GPU: %1%").arg(static_cast<int>(value));
            break;
        case NvMonitorContent::MemUtilization:
            value = mGpuData.memUtil;
            tooltip = tr("VRAM load: %1%").arg(static_cast<int>(value));
            break;
        case NvMonitorContent::VramUsage: {
            // VRAM usage percentage (capacity): used / total * 100
            if (mGpuData.memTotal > 0) {
                value = static_cast<float>(mGpuData.memUsed) * 100.0f / static_cast<float>(mGpuData.memTotal);
            }
            // Format size: GB with one decimal place
            double usedGB = static_cast<double>(mGpuData.memUsed) / (1024.0 * 1024.0 * 1024.0);
            double totalGB = static_cast<double>(mGpuData.memTotal) / (1024.0 * 1024.0 * 1024.0);
            tooltip = tr("VRAM: %1% (%2 / %3 GB)")
                        .arg(static_cast<int>(value))
                        .arg(usedGB, 0, 'f', 1)
                        .arg(totalGB, 0, 'f', 1);
            break;
        }
        case NvMonitorContent::Temperature:
            value = mGpuData.temp;
            tooltip = tr("Temp: %1°C").arg(static_cast<int>(value));
            break;
        case NvMonitorContent::PowerUsage: {
            // Power usage percentage: current / max * 100
            // NVML returns power in milliwatts (mW), despite docs saying microwatts
            if (mGpuData.powerMax > 0) {
                value = static_cast<float>(mGpuData.powerUsage) * 100.0f / static_cast<float>(mGpuData.powerMax);
            }
            // Format power: Watts with one decimal place (mW → W: divide by 1000)
            double usedW = static_cast<double>(mGpuData.powerUsage) / 1000.0;
            double maxW = static_cast<double>(mGpuData.powerMax) / 1000.0;
            tooltip = tr("Power: %1% (%2 / %3 W)")
                        .arg(static_cast<int>(value))
                        .arg(usedW, 0, 'f', 1)
                        .arg(maxW, 0, 'f', 1);
            break;
        }
    }

    mCurrentValue = value;

    // Clamp value to [0, 100]
    value = std::clamp(value, 0.0f, 100.0f);

    // Update history image
    clearHistory();

    QPainter painter(&mHistoryImage);
    float y_float = 100.0f - value;
    painter.setPen(mGraphColor);
    painter.drawLine(mHistoryOffset, static_cast<int>(y_float), mHistoryOffset, 100);

    // Guard against division by zero: mHistoryImage.width() may be 0
    // if resizeEvent was called with zero width
    if (mHistoryImage.width() > 0) {
        mHistoryOffset = (mHistoryOffset + 1) % mHistoryImage.width();
    }

    setToolTip(QStringLiteral("<b>%1</b><br>%2")
            .arg(mGpuData.name.isEmpty() ? tr("NVIDIA GPU") : mGpuData.name)
            .arg(tooltip));

    update(0, mTitleFontPixelHeight, width(), height() - mTitleFontPixelHeight);
}

void NvMonitorContent::clearHistory()
{
    // Clear one column of pixels in mHistoryImage at mHistoryOffset position
    // Uses setPixel for safety (independent of pixel format)
    QRgb bg = QColor(Qt::transparent).rgba();
    for (int i = 0; i < 100 && i < mHistoryImage.height(); ++i) {
        mHistoryImage.setPixel(mHistoryOffset, i, bg);
    }
}

void NvMonitorContent::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    qreal graphTop = 0;
    qreal graphHeight = height();

    bool hasTitle = !mTitleLabel.isEmpty();

    if (hasTitle) {
        graphTop = mTitleFontPixelHeight;
        graphHeight -= graphTop;
    }

    if (graphHeight < 1)
        graphHeight = 1;

    // Draw title
    if (hasTitle) {
        drawTitle(p);
    }

    // Draw grid
    drawGrid(p);

    // Draw graph
    drawGraph(p);

    // Draw current value
    if (mShowValue) {
        drawValue(p);
    }

    Q_UNUSED(event);
}

void NvMonitorContent::drawTitle(QPainter &p)
{
    p.setPen(mTitleColor);
    p.setFont(mTitleFont);
    p.drawText(QRectF(0, 0, width(), mTitleFontPixelHeight), Qt::AlignHCenter | Qt::AlignVCenter, mTitleLabel);
}

void NvMonitorContent::drawGrid(QPainter &p)
{
    p.setPen(mGridColor);
    qreal w = static_cast<qreal>(width());
    qreal graphTop = mTitleFontPixelHeight;
    qreal graphHeight = height() - graphTop;

    // 0% line (top of the graph)
    p.drawLine(QPointF(0.0, graphTop + 0.5), QPointF(w, graphTop + 0.5));

    for (int l = 0; l < mGridLines; ++l) {
        qreal y = graphTop + static_cast<qreal>(l + 1) * graphHeight / (static_cast<qreal>(mGridLines + 1));
        p.drawLine(QPointF(0.0, y), QPointF(w, y));
    }
}

void NvMonitorContent::drawGraph(QPainter &p)
{
    if (mHistoryImage.isNull()) {
        return;
    }

    qreal graphTop = mTitleFontPixelHeight;
    qreal graphHeight = height() - graphTop;

    p.save();
    // Move origin to the top edge of the graph area.
    // mHistoryImage already has correct orientation:
    //   y=0   → 100% value (top)
    //   y=100 → 0% value   (bottom)
    // drawImage automatically stretches source (100px) to target (graphHeight px).
    p.translate(0, graphTop);

    // Right part (start of history — most recent data)
    int visibleWidth = mHistoryImage.width() - mHistoryOffset;
    if (visibleWidth > 0) {
        p.drawImage(QRect(0, 0, visibleWidth, graphHeight), mHistoryImage,
                    QRect(mHistoryOffset, 0, visibleWidth, 100));
    }

    // Left part (end of history — wraps around)
    if (mHistoryOffset > 0) {
        p.drawImage(QRect(width() - mHistoryOffset, 0, mHistoryOffset, graphHeight), mHistoryImage,
                    QRect(0, 0, mHistoryOffset, 100));
    }

    p.restore();
}

void NvMonitorContent::drawValue(QPainter &p)
{
    QString text;
    switch (mMetric) {
        case NvMonitorContent::GpuUtilization:
            text = QString("%1%").arg(static_cast<int>(mCurrentValue));
            break;
        case NvMonitorContent::MemUtilization:
            text = QString("%1%").arg(static_cast<int>(mCurrentValue));
            break;
        case NvMonitorContent::VramUsage:
            text = QString("%1%").arg(static_cast<int>(mCurrentValue));
            break;
        case NvMonitorContent::Temperature:
            text = QString("%1°C").arg(static_cast<int>(mCurrentValue));
            break;
        case NvMonitorContent::PowerUsage:
            text = QString("%1%").arg(static_cast<int>(mCurrentValue));
            break;
    }

    p.setPen(mValueColor);
    p.setFont(mTitleFont);

    qreal graphTop = mTitleFontPixelHeight;
    QRect textRect = rect().adjusted(2, static_cast<int>(graphTop) + 2, -2, -2);
    p.drawText(textRect, Qt::AlignCenter, text);
}

void NvMonitorContent::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    reset();
}

void NvMonitorContent::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == mTimerId) {
        collectData();
        updateGraph();
    }
}

bool NvMonitorContent::event(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        // Update title font height
        if (!mTitleLabel.isEmpty() && mTitleFontPixelHeight > 0) {
            QFontMetrics fm(mTitleFont);
            mTitleFontPixelHeight = fm.height();
        }
        update();
    }
    return QWidget::event(event);
}
