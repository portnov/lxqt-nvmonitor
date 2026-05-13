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

#ifndef NVMONITOR_H
#define NVMONITOR_H

#include <QWidget>
#include <QColor>
#include <QImage>
#include <QVector>

class ILXQtPanelPlugin;
class PluginSettings;

/**
 * @brief GPU data collected via NVML
 *
 * Dynamically loads libnvidia-ml.so and provides GPU metrics.
 * Implementation follows the approach used in btop.
 */
class NvmlGpuData
{
public:
    NvmlGpuData() : valid(false), gpuUtil(-1), memUtil(-1), temp(-1), memTotal(0), memUsed(0) {}

    bool valid;
    QString name;
    float gpuUtil;     // GPU utilization, %
    float memUtil;     // VRAM bandwidth utilization, %
    float temp;        // temperature, °C
    qint64 memTotal;   // total VRAM, bytes
    qint64 memUsed;    // used VRAM, bytes
};

class NvmlGpu
{
public:
    NvmlGpu();
    ~NvmlGpu();

    // Disable copy/assignment (singleton-like class)
    NvmlGpu(const NvmlGpu &) = delete;
    NvmlGpu &operator=(const NvmlGpu &) = delete;

    // Initialize NVML, returns true on success
    bool init();

    // Shutdown NVML
    void shutdown();

    // Get number of GPU devices
    int deviceCount() const { return m_deviceCount; }

    // Get GPU data by index
    NvmlGpuData getDeviceData(int index) const;

    // Get GPU name by index
    QString getDeviceName(int index) const;

private:
    // NVML type aliases
    typedef void* nvmlDevice_t;
    typedef int nvmlReturn_t;
    typedef int nvmlTemperatureSensors_t;
    typedef int nvmlClockType_t;

    struct nvmlUtilization_t { unsigned int gpu, memory; };
    struct nvmlMemory_t { unsigned long long total, free, used; };

    // Function pointers
    void* m_dlHandle;
    int m_deviceCount;

    // NVML library loading
    bool loadLibrary();
    void unloadLibrary();
    bool loadSymbols();

    // Initialization state
    bool m_initialized;

    // NVML function pointers
    nvmlReturn_t (*m_nvmlInit)();
    nvmlReturn_t (*m_nvmlShutdown)();
    nvmlReturn_t (*m_nvmlDeviceGetCount)(unsigned int*);
    nvmlReturn_t (*m_nvmlDeviceGetHandleByIndex)(unsigned int, nvmlDevice_t*);
    nvmlReturn_t (*m_nvmlDeviceGetName)(nvmlDevice_t, char*, unsigned int);
    nvmlReturn_t (*m_nvmlDeviceGetUtilizationRates)(nvmlDevice_t, nvmlUtilization_t*);
    nvmlReturn_t (*m_nvmlDeviceGetTemperature)(nvmlDevice_t, nvmlTemperatureSensors_t, unsigned int*);
    nvmlReturn_t (*m_nvmlDeviceGetMemoryInfo)(nvmlDevice_t, nvmlMemory_t*);

    // NVML constants
    static const unsigned int NVML_TEMPERATURE_GPU = 0;
};

/**
 * @brief Widget displaying NVIDIA GPU metrics as a scrolling graph
 *
 * Collects GPU data and renders it as a graph,
 * similar to the sysstat plugin from lxqt-panel.
 */
class NvMonitorContent : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor graphColor READ graphColor WRITE setGraphColor)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(QColor gridColor READ gridColor WRITE setGridColor)
    Q_PROPERTY(QColor titleColor READ titleColor WRITE setTitleColor)

public:
    enum GpuMetric {
        GpuUtilization,    // GPU utilization (%) — compute unit bandwidth
        MemUtilization,    // VRAM load (%) — memory bandwidth utilization
        VramUsage,         // VRAM usage (%) — used/total capacity
        Temperature        // Temperature (°C)
    };
    Q_ENUM(GpuMetric)

    explicit NvMonitorContent(ILXQtPanelPlugin *plugin, QWidget *parent = nullptr);
    ~NvMonitorContent() override;

    void updateSettings(const PluginSettings *settings);

    QColor graphColor() const { return mGraphColor; }
    void setGraphColor(QColor value) { mGraphColor = value; update(); }

    QColor backgroundColor() const { return mBackgroundColor; }
    void setBackgroundColor(QColor value) { mBackgroundColor = value; update(); }

    QColor gridColor() const { return mGridColor; }
    void setGridColor(QColor value) { mGridColor = value; update(); }

    QColor titleColor() const { return mTitleColor; }
    void setTitleColor(QColor value) { mTitleColor = value; update(); }

public slots:
    void reset();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    bool event(QEvent *event) override;

private:
    void collectData();
    void updateGraph();
    void applyThemeColors();
    void clearHistory();
    void drawGrid(QPainter &p);
    void drawGraph(QPainter &p);
    void drawTitle(QPainter &p);
    void drawValue(QPainter &p);

    ILXQtPanelPlugin *mPlugin;

    // Settings
    GpuMetric mMetric;
    int mUpdateInterval;
    int mMinimalSize;
    int mGridLines;
    QString mTitleLabel;
    QFont mTitleFont;
    int mTitleFontPixelHeight;
    bool mShowValue;
    int mMaxHistory;
    bool mUseThemeColors;

    // Colors
    QColor mGraphColor;
    QColor mBackgroundColor;
    QColor mGridColor;
    QColor mTitleColor;

    // Data
    NvmlGpu mGpu;
    NvmlGpuData mGpuData;
    QImage mHistoryImage;
    int mHistoryOffset;

    // Timer
    int mTimerId;
    bool mTimerStarted;

    // Current value for display
    float mCurrentValue;

    // NVML state
    bool mNvmlAvailable;
};

#endif // NVMONITOR_H
