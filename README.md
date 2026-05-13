# NVIDIA GPU Monitor Plugin for LXQt Panel

Плагин для панели LXQt, отображающий в виде графика метрики NVIDIA GPU:
- **Загрузка GPU** — процент использования графического процессора
- **Загрузка VRAM** — процент использования видеопамяти
- **Температура GPU** — текущая температура графического процессора

В качестве источника данных используется библиотека NVML (`libnvidia-ml.so`), аналогично тому, как это реализовано в [btop](https://github.com/aristocratos/btop).

## Сборка

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/home/portnov/soft/lxqt
make -j$(nproc)
```

## Установка

### Способ 1: Через CMake install

```bash
sudo cmake --install .
```

### Способ 2: Ручная установка

```bash
# Скопировать .so файл в директорию плагинов
sudo cp nvmonitor.so /usr/lib/lxqt-panel/

# Скопировать .desktop файл
sudo cp nvmonitor.desktop /usr/share/lxqt/lxqt-panel/
```

### Способ 3: Через переменную окружения

```bash
# Добавить директорию с плагином в LXQT_PANEL_PLUGINS_DIR
export LXQT_PANEL_PLUGINS_DIR="/путь/к/директории/с/плагином"
lxqt-panel
```

### Способ 4: Пользовательская директория

Поместите `.desktop` файл в:
```
~/.local/share/lxqt/lxqt-panel/nvmonitor.desktop
```

Поместите `.so` файл в одну из директорий, указанных в `LXQT_PANEL_PLUGINS_DIR`.

## Настройки

После добавления плагина на панель, кликните по нему правой кнопкой мыши и выберите "Настроить". Доступны следующие параметры:

### Метрика
- **GPU Utilization** — отображает загрузку GPU в процентах
- **VRAM Utilization** — отображает использование VRAM в процентах
- **Temperature** — отображает температуру GPU в градусах Цельсия

### Дисплей
- **Update interval** — интервал обновления данных (секунды)
- **Minimal size** — минимальный размер виджета (пиксели)
- **History length** — количество точек в истории графика
- **Grid lines** — количество горизонтальных линий сетки
- **Show value on graph** — показывать текущее значение на графике

### Заголовок
- **Label** — текст заголовка (пусто = без заголовка)

### Цвета
- **Use theme colors** — использовать цвета темы
- **Graph color** — цвет графика
- **Grid color** — цвет сетки
- **Title color** — цвет заголовка

## Требования

- LXQt 2.4+
- Qt 6
- NVIDIA GPU с установленными проприетарными драйверами (для работы NVML)
- Библиотека `libnvidia-ml.so` (обычно устанавливается вместе с драйвером NVIDIA)

## Зависимости сборки

- CMake 3.16+
- Qt 6 (Widgets)
- lxqt (liblxqt)
- KF6::WindowSystem
- Заголовочные файлы lxqt-panel (из дерева исходников или установленные)

## Структура проекта

```
nvmonitor/
├── CMakeLists.txt                    # Скрипт сборки
├── README.md                         # Этот файл
├── nvmonitorplugin.h                 # Класс плагина + библиотека
├── nvmonitorplugin.cpp               # Реализация плагина
├── nvmonitor.h                       # Класс графика + обёртка NVML
├── nvmonitor.cpp                     # Реализация графика и NVML
├── nvmonitorconfiguration.h          # Диалог настроек
├── nvmonitorconfiguration.cpp        # Реализация диалога настроек
├── nvmonitorconfiguration.ui         # UI диалога настроек
├── resources/
│   └── nvmonitor.desktop.in          # Файл описания плагина
└── translations/
    └── nvmonitor.ts                  # Файл перевода (опционально)
```

## Лицензия

GNU Lesser General Public License v2.1 или новее (LGPL2+)
