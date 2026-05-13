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
#include <dlfcn.h>
#include <qmath.h>
#include <cmath>
#include <algorithm>

// ============================================================================
// NvmlGpu - обёртка над NVML
// ============================================================================

NvmlGpu::NvmlGpu()
    : m_dlHandle(nullptr)
    , m_deviceCount(0)
    , m_nvmlInit(nullptr)
    , m_nvmlShutdown(nullptr)
    , m_nvmlDeviceGetCount(nullptr)
    , m_nvmlDeviceGetHandleByIndex(nullptr)
    , m_nvmlDeviceGetName(nullptr)
    , m_nvmlDeviceGetUtilizationRates(nullptr)
    , m_nvmlDeviceGetTemperature(nullptr)
    , m_nvmlDeviceGetMemoryInfo(nullptr)
    , m_initialized(false)
{
}

NvmlGpu::~NvmlGpu()
{
    // m_nvmlShutdown обнуляется в shutdown() и при ошибках init()
    // unloadLibrary() безопасен при m_dlHandle == nullptr
    unloadLibrary();
}

bool NvmlGpu::loadLibrary()
{
    // Пробуем загрузить библиотеку NVML
    m_dlHandle = dlopen("libnvidia-ml.so", RTLD_LAZY);
    if (!m_dlHandle) {
        m_dlHandle = dlopen("libnvidia-ml.so.1", RTLD_LAZY);
    }
    if (!m_dlHandle) {
        return false;
    }
    return true;
}

void NvmlGpu::unloadLibrary()
{
    if (m_dlHandle) {
        dlclose(m_dlHandle);
        m_dlHandle = nullptr;
    }
}

bool NvmlGpu::loadSymbols()
{
    if (!(m_nvmlInit = (decltype(m_nvmlInit))dlsym(m_dlHandle, "nvmlInit"))) return false;
    if (!(m_nvmlShutdown = (decltype(m_nvmlShutdown))dlsym(m_dlHandle, "nvmlShutdown"))) return false;
    if (!(m_nvmlDeviceGetCount = (decltype(m_nvmlDeviceGetCount))dlsym(m_dlHandle, "nvmlDeviceGetCount"))) return false;
    if (!(m_nvmlDeviceGetHandleByIndex = (decltype(m_nvmlDeviceGetHandleByIndex))dlsym(m_dlHandle, "nvmlDeviceGetHandleByIndex"))) return false;
    if (!(m_nvmlDeviceGetName = (decltype(m_nvmlDeviceGetName))dlsym(m_dlHandle, "nvmlDeviceGetName"))) return false;
    if (!(m_nvmlDeviceGetUtilizationRates = (decltype(m_nvmlDeviceGetUtilizationRates))dlsym(m_dlHandle, "nvmlDeviceGetUtilizationRates"))) return false;
    if (!(m_nvmlDeviceGetTemperature = (decltype(m_nvmlDeviceGetTemperature))dlsym(m_dlHandle, "nvmlDeviceGetTemperature"))) return false;
    if (!(m_nvmlDeviceGetMemoryInfo = (decltype(m_nvmlDeviceGetMemoryInfo))dlsym(m_dlHandle, "nvmlDeviceGetMemoryInfo"))) return false;

    return true;
}

bool NvmlGpu::init()
{
    if (m_initialized) {
        return m_deviceCount > 0;  // Уже инициализирована
    }

    if (!loadLibrary()) {
        return false;
    }

    if (!loadSymbols()) {
        unloadLibrary();
        m_nvmlShutdown = nullptr;  // Предотвращаем вызов в деструкторе
        return false;
    }

    nvmlReturn_t result = m_nvmlInit();
    if (result != 0) {
        // nvmlInit не удался — вызываем shutdown для корректного cleanup
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

    // Получаем имя
    char name[64];
    if (m_nvmlDeviceGetName) {
        m_nvmlDeviceGetName(device, name, sizeof(name));
        data.name = QString(name);
    }

    // Получаем утилизацию GPU и памяти
    nvmlUtilization_t util;
    result = m_nvmlDeviceGetUtilizationRates(device, &util);
    if (result == 0) {
        data.gpuUtil = static_cast<float>(util.gpu);
        data.memUtil = static_cast<float>(util.memory);
        data.valid = true;
    }

    // Получаем температуру
    unsigned int temp = 0;
    result = m_nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temp);
    if (result == 0) {
        data.temp = static_cast<float>(temp);
    }

    // Получаем информацию о памяти
    nvmlMemory_t mem;
    result = m_nvmlDeviceGetMemoryInfo(device, &mem);
    if (result == 0) {
        data.memTotal = static_cast<qint64>(mem.total);
        data.memUsed = static_cast<qint64>(mem.used);
    }

    return data;
}

// ============================================================================
// NvMonitorContent - виджет графика
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
    , mMaxHistory(100)
    , mUseThemeColors(true)
    , mGraphColor(Qt::red)
    , mBackgroundColor(Qt::transparent)
    , mGridColor(QColor(192, 192, 192))
    , mTitleColor(Qt::white)
    , mGpu()
    , mGpuData()
    , mHistory()
    , mHistoryOffset(0)
    , mTimerId(-1)
    , mTimerStarted(false)
    , mCurrentValue(0)
    , mNvmlAvailable(false)
{
    setObjectName(QStringLiteral("NvMonitor_Graph"));
    setMouseTracking(true);

    // Инициализируем NVML
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
    mMaxHistory = settings->value(QStringLiteral("graph/maxHistory"), 100).toInt();

    // Применяем тему или пользовательские цвета
    if (mUseThemeColors) {
        applyThemeColors();
    } else {
        mGraphColor = QColor(settings->value(QStringLiteral("graph/color"), QStringLiteral("#ff0000")).toString());
        mGridColor = QColor(settings->value(QStringLiteral("grid/color"), QStringLiteral("#c0c0c0")).toString());
        mTitleColor = QColor(settings->value(QStringLiteral("title/color"), QStringLiteral("#ffffff")).toString());
    }

    // Метрика
    QString metricStr = settings->value(QStringLiteral("data/metric"), QStringLiteral("gpuUtilization")).toString();
    if (metricStr == QLatin1String("memUtilization")) {
        mMetric = NvMonitorContent::MemUtilization;
    } else if (metricStr == QLatin1String("temperature")) {
        mMetric = NvMonitorContent::Temperature;
    } else {
        mMetric = NvMonitorContent::GpuUtilization;
    }

    // Обновляем шрифт заголовка
    if (!mTitleLabel.isEmpty()) {
        if (mTitleFontPixelHeight <= 0) {
            // Шрифт ещё не инициализирован — берём системный
            mTitleFont = QFont();
            mTitleFont.setBold(true);
            mTitleFont.setPixelSize(12);
        }
        QFontMetrics fm(mTitleFont);
        mTitleFontPixelHeight = fm.height();
    } else {
        mTitleFontPixelHeight = 0;
    }

    // Запускаем таймер, если ещё не запущен
    if (!mTimerStarted) {
        mTimerId = startTimer(mUpdateInterval);
        mTimerStarted = true;
    } else {
        // Перезапускаем таймер при изменении интервала
        killTimer(mTimerId);
        mTimerId = startTimer(mUpdateInterval);
    }

    // Пересоздаём историю при изменении размера или метрики
    if (oldMetric != mMetric || mMaxHistory != static_cast<int>(mHistory.size())) {
        reset();
    } else {
        update();
    }
}

void NvMonitorContent::reset()
{
    setMinimumSize(mPlugin->panel()->isHorizontal() ? mMinimalSize : 2,
                   mPlugin->panel()->isHorizontal() ? 2 : mMinimalSize);

    mHistory.clear();
    mHistoryOffset = 0;
    mHistoryImage = QImage();

    // Создаём новое изображение для истории
    if (width() > 0) {
        mHistoryImage = QImage(width(), 100, QImage::Format_ARGB32);
        mHistoryImage.fill(Qt::transparent);
    }

    update();
}

void NvMonitorContent::applyThemeColors()
{
    // Используем палитру виджета для получения системных цветов
    QPalette pal = palette();
    mGraphColor = pal.color(QPalette::Text);        // Текст графика
    mGridColor = pal.color(QPalette::Mid);           // Сетка
    mTitleColor = pal.color(QPalette::WindowText);   // Заголовок
}

void NvMonitorContent::collectData()
{
    // NVML уже инициализирована в конструкторе, просто получаем данные
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
            tooltip = tr("VRAM: %1%").arg(static_cast<int>(value));
            break;
        case NvMonitorContent::Temperature:
            value = mGpuData.temp;
            tooltip = tr("Temp: %1°C").arg(static_cast<int>(value));
            break;
    }

    mCurrentValue = value;

    // Ограничиваем значение в пределах [0, 100]
    value = std::clamp(value, 0.0f, 100.0f);

    // Добавляем в историю
    mHistory.append(value);

    // Ограничиваем размер истории
    while (mHistory.size() > mMaxHistory) {
        mHistory.removeFirst();
    }

    // Обновляем изображение истории
    clearHistory();

    QPainter painter(&mHistoryImage);
    int y = static_cast<int>(value);
    if (y > 0) {
        painter.setPen(mGraphColor);
        painter.drawLine(mHistoryOffset, 100 - y, mHistoryOffset, 100);
    }

    // Защита от деления на ноль: mHistoryImage.width() может быть 0
    // если resizeEvent был вызван с нулевой шириной
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
    // Очищаем один столбец пикселей в mHistoryImage на позиции mHistoryOffset
    // Используем setPixel для безопасности (не зависит от формата пикселя)
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

    // Рисуем заголовок
    if (hasTitle) {
        drawTitle(p);
    }

    // Рисуем сетку
    drawGrid(p);

    // Рисуем график
    drawGraph(p);

    // Рисуем текущее значение
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

    // Линия 0% (верх графика при вертикальной ориентации)
    p.drawLine(QPointF(0.0, graphTop + 0.5), QPointF(w, graphTop + 0.5));

    for (int l = 0; l < mGridLines; ++l) {
        qreal y = graphTop + static_cast<qreal>(l + 1) * graphHeight / (static_cast<qreal>(mGridLines + 1));
        p.drawLine(QPointF(0.0, y), QPointF(w, y));
    }
}

void NvMonitorContent::drawGraph(QPainter &p)
{
    if (mHistory.isEmpty() || mHistoryImage.isNull()) {
        return;
    }

    qreal graphTop = mTitleFontPixelHeight;
    qreal graphHeight = height() - graphTop;

    p.save();
    // Сдвигаем начало координат в верхнюю границу графика.
    // mHistoryImage уже имеет правильную ориентацию:
    //   y=0  → значение 100% (верх)
    //   y=100 → значение 0%  (низ)
    // drawImage автоматически растянёт источник (100px) до цели (graphHeight px).
    p.translate(0, graphTop);

    // Правая часть (начало истории — самые свежие данные)
    int visibleWidth = mHistoryImage.width() - mHistoryOffset;
    if (visibleWidth > 0) {
        p.drawImage(QRect(0, 0, visibleWidth, graphHeight), mHistoryImage,
                    QRect(mHistoryOffset, 0, visibleWidth, 100));
    }

    // Левая часть (конец истории — обёртка по кругу)
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
        case NvMonitorContent::Temperature:
            text = QString("%1°C").arg(static_cast<int>(mCurrentValue));
            break;
    }

    p.setPen(mGraphColor);
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
        // Обновляем высоту шрифта заголовка
        if (!mTitleLabel.isEmpty() && mTitleFontPixelHeight > 0) {
            QFontMetrics fm(mTitleFont);
            mTitleFontPixelHeight = fm.height();
        }
        update();
    }
    return QWidget::event(event);
}
