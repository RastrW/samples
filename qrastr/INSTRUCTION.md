# Инструкция по сборке проекта qrastr

## Предварительные требования на Windows

Перед сборкой убедитесь, что на машине установлены:

- **Visual Studio 2022** (с компонентом MSVC v143 x64/x86 Build Tools (последняя версия) и C++ Build Insights)
- **Qt 6.7.0** и более для MSVC 2022 x64
- **CMake** (например, входящий в состав Qt: `C:\Qt\Tools\CMake_64\bin\cmake.exe`)
- **Ninja** *(опционально)* — ускоряет сборку CMake-проектов; если не установлен, скрипт автоматически переключится на `NMake Makefiles`
- **NASM 3.01** — требуется для сборки `SDL3_image` под MSVC. [Скачать](https://www.nasm.us/pub/nasm/releasebuilds/3.01/win64/)
- **Python 3.12** — требуется для генерации кода `ScintillaEdit`. Путь к интерпретатору должен быть добавлен в переменную среды `PATH`
 [Скачать](https://www.python.org/downloads/release/python-3120/)
- **PostgreSQL 18** *(опционально)* — необходим для сборки `libpqxx`

---

## Предварительные требования на AstraLinux 1.8
### Основные пакеты
```bash
sudo apt install build-essential
```
	
### Python
```bash
sudo apt install -y python3
```

### PostgreSQL (опционально)

Только если планируете использовать `BUILD_WITH_PQ=ON`:

```bash
sudo apt install -y libpq-dev
```

### SDL зависимости (X11/Wayland)

SDL3 требует системные библиотеки для работы с графикой:

```bash
sudo apt install -y \
    libx11-dev \
    libxext-dev \
    libxrandr-dev \
    libxcursor-dev \
    libxinerama-dev \
    libxi-dev \
    libxss-dev \
    libxxf86vm-dev \
    libxtst-dev \
    libxkbcommon-dev \
    libwayland-dev \
    libdbus-1-dev \
    libudev-dev \
    libgbm-dev \
    libdrm-dev
```

## Шаг 1. Загрузка submodule-зависимостей
Из корня проекта (`...\rastr`) выполните:

```bash
git submodule update --init --recursive
```

Все сторонние проекты будут загружены автоматически в папку:

```
...\rastr\qrastr\thirdparty\
```

---

## Шаг 2. Загрузка сторонних библиотек без репозитория

### Scintilla и Lexilla
Библиотеки **Scintilla** и **Lexilla** не входят в состав submodule и должны быть загружены вручную с официального сайта:

- https://scintilla.org/

Поместите их в:

```
...\rastr\qrastr\thirdparty\scintilla\
...\rastr\qrastr\thirdparty\lexilla\
```

### boost 1.80
Скачать исходники библиотеки boost 1.80 с официального сайта:
[Скачать](https://www.boost.org/releases/1.80.0/)

Поместите их в:

```
...\rastr\qrastr\thirdparty\boost_1_80_0
```

---

## Шаг 3. Сборка сторонних библиотек

Для сборки всех сторонних библиотек из thirdparty используется скрипт:

1. Под Windows:
```
...\rastr\qrastr\thirdparty\build_thirdparty_vs_msvc.bat
```
2. Под astralinux:
```
...\rastr\qrastr\thirdparty\build_thirdparty_astralinux.sh
```

**ВАЖНО: 1. При использовании скриптов указывать персональные пути к пакетам**
**ВАЖНО: 2. Не корректировать файлы формата .sh через текстовый редактор на windows

### Расположение собранных артефактов

После сборки библиотеки будут размещены по пути:

```
${THIRDPARTY_DIR}\compile\${LIB_NAME}\win\msvc\${CONFIG}\
```

---

## Шаг 4. Установка библиотек astra и qtitan

Библиотеки **astra** и **qtitan** должны быть загружены вручную и помещены в папку:

```
...\rastr\qrastr\lib\
```
---

## Шаг 5. Сборка библиотеки QtitanDataGrid (Windows / MSVC)

Для сборки **QtitanDataGrid** из исходных файлов используется скрипт:

1. Под Windows:
```
...\rastr\qrastr\lib\QtitanDataGrid8.2.0_win_qt_6_7\build_qtitan_vs_msvc.bat
```
2. Под astralinux:
```
...\rastr\qrastr\lib\qtitan8_lin_5_15\build_qtitan_astralinux.sh
```

---

## Итоговая структура проекта

```
C:\work\rastr\qrastr\
├───Data
│   ├───Files
│   ├───form
│   └───SHABLON
├── lib
│   ├── astra
│   ├── astra
│   └── QtitanDataGrid8.2.0_win_qt_6_7
├── qrastr
└── thirdparty
    ├── compile          ← предсобранные библиотеки
    │   ├── fmt
    │   ├── spdlog
    │   ├── SDL3
    │   ├── SDL3_image
    │   ├── scintilla
    │   ├── lexilla
    │   ├── ScintillaEdit
    │   ├── ScintillaEditBase
    │   ├── qtadvanceddocking
    │   └── libpqxx
    ├── fmt              ← исходники (submodule)
    ├── spdlog
    ├── SDL
    ├── SDL_image
    ├── scintilla        ← скачать вручную
    ├── lexilla          ← скачать вручную
	├── boost_1_80_0     ← скачать вручную	
    ├── Qt-Advanced-Docking-System
    └── libpqxx
```

---

## Быстрый старт (при наличии предсобранных библиотек)

Если папка `compile` с предсобранными библиотеками уже есть:

1. Поместите `compile` в `...\rastr\qrastr\thirdparty\compile\`
2. Поместите `astra` и `QtitanDataGrid8.2.0_win_qt_6_7` в `...\rastr\qrastr\lib\`
3. Установите Python 3.12 и добавьте в `PATH`
4. Откройте проект в Qt Creator и соберите его
