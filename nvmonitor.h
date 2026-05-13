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
 * @brief Класс для получения данных с NVIDIA GPU через NVML
 *
 * Динамически загружает libnvidia-ml.so и предоставляет метрики GPU.
 * Реализация основана на подходе, используемом в btop.
 */
class NvmlGpuData
{
public:
    NvmlGpuData() : valid(false), gpuUtil(-1), memUtil(-1), temp(-1), memTotal(0), memUsed(0) {}

    bool valid;
    QString name;
    float gpuUtil;     // загрузка GPU, %
    float memUtil;     // загрузка VRAM, %
    float temp;        // температура, °C
    qint64 memTotal;   // общий объём VRAM, bytes
    qint64 memUsed;    // использованная VRAM, bytes
};

class NvmlGpu
{
public:
    NvmlGpu();
    ~NvmlGpu();

    // Удалить копию/присваивание (singleton-подобный класс)
    NvmlGpu(const NvmlGpu &) = delete;
    NvmlGpu &operator=(const NvmlGpu &) = delete;

    // Инициализация NVML, возвращает true при успехе
    bool init();

    // Закрыть NVML
    void shutdown();

    // Получить количество GPU
    int deviceCount() const { return m_deviceCount; }

    // Получить данные GPU по индексу
    NvmlGpuData getDeviceData(int index) const;

    // Получить имя GPU по индексу
    QString getDeviceName(int index) const;

private:
    // Типы функций NVML
    typedef void* nvmlDevice_t;
    typedef int nvmlReturn_t;
    typedef int nvmlTemperatureSensors_t;
    typedef int nvmlClockType_t;

    struct nvmlUtilization_t { unsigned int gpu, memory; };
    struct nvmlMemory_t { unsigned long long total, free, used; };

    // Функциональные указатели
    void* m_dlHandle;
    int m_deviceCount;

    // Загрузка NVML
    bool loadLibrary();
    void unloadLibrary();
    bool loadSymbols();

    // Состояние инициализации
    bool m_initialized;

    // NVML функции
    nvmlReturn_t (*m_nvmlInit)();
    nvmlReturn_t (*m_nvmlShutdown)();
    nvmlReturn_t (*m_nvmlDeviceGetCount)(unsigned int*);
    nvmlReturn_t (*m_nvmlDeviceGetHandleByIndex)(unsigned int, nvmlDevice_t*);
    nvmlReturn_t (*m_nvmlDeviceGetName)(nvmlDevice_t, char*, unsigned int);
    nvmlReturn_t (*m_nvmlDeviceGetUtilizationRates)(nvmlDevice_t, nvmlUtilization_t*);
    nvmlReturn_t (*m_nvmlDeviceGetTemperature)(nvmlDevice_t, nvmlTemperatureSensors_t, unsigned int*);
    nvmlReturn_t (*m_nvmlDeviceGetMemoryInfo)(nvmlDevice_t, nvmlMemory_t*);

    // Константы NVML
    static const unsigned int NVML_TEMPERATURE_GPU = 0;
};

/**
 * @brief Виджет отображения графика метрик NVIDIA GPU
 *
 * Анализирует данные GPU и отображает их в виде графика,
 * аналогично плагину sysstat из lxqt-panel.
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
        GpuUtilization,  // Загрузка GPU (%)
        MemUtilization,  // Загрузка VRAM (%)
        Temperature      // Температура (°C)
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

    // Настройки
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

    // Цвета
    QColor mGraphColor;
    QColor mBackgroundColor;
    QColor mGridColor;
    QColor mTitleColor;

    // Данные
    NvmlGpu mGpu;
    NvmlGpuData mGpuData;
    QVector<float> mHistory;
    int mHistoryOffset;
    QImage mHistoryImage;

    // Таймер
    int mTimerId;
    bool mTimerStarted;

    // Текущее значение для отображения
    float mCurrentValue;

    // Состояние NVML
    bool mNvmlAvailable;
};

#endif // NVMONITOR_H
