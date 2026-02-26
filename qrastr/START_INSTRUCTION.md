# Инструкция по сборке проекта qrastr

## CMakePresets

Если у вас уже есть все предсобранные библиотеки и все зависимости установлены:

1. Скопируйте `CMakePresets.json.template` в `CMakePresets.json`:
   ```bash
   cp CMakePresets.json.template CMakePresets.json
   ```

2. Отредактируйте `CMakePresets.json` и установите правильный путь к 
	ASTRA:
		- Windows: `"ASTRA_ROOT": "C:/path/to/astra"`
		- Linux: `"ASTRA_ROOT": "/path/to/astra"`
	*Qt:
		- Windows: `"QT_DIR": "C:/path/to/lib/cmake/Qt6"`
		- На Linux: `"/home/administrator/Qt/5.15.2/gcc_64/lib/cmake/Qt5"`
3. Откройте проект в Qt Creator и соберите

4. При изменении содержимого CMakePresets необходимо выполнять "Build->Reload CMake Presets"

**Проблема в том, что Qt Creator не передаёт информацию о Qt при использовании CMakePresets.json.**
**Поэтому QT_DIR необходимо прописывать CMakePresets.json.**
**Для ускорения сборки указать генератор Ninja. Иначе будет по умолчанию использоваться медленный генератор "Visual Studio 17 2022"  или "Unix Makefiles"**
---

## Полная инструкция для первого запуска

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

**ВАЖНО:** 
- При использовании скриптов указывайте персональные пути к пакетам
- Не корректируйте файлы формата `.sh` через текстовый редактор на Windows

### Расположение собранных артефактов

После сборки библиотеки будут размещены по пути:

```
${THIRDPARTY_DIR}\compile\${LIB_NAME}\win\msvc\${CONFIG}\
```

---

## Шаг 4. Установка библиотеки qtitan

Библиотека **qtitan** должна быть загружена вручную и помещена в папку:

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
