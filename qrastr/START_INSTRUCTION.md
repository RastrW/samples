# CMakePresets

1. Скопируйте шаблон:
   ```bash
   cp CMakePresets.json.template CMakePresets.json
   ```

2. Отредактируйте `CMakePresets.json`, задав пути для вашей машины:

   | Параметр | Windows | Linux |
   |---|---|---|
   | `ASTRA_ROOT` | `C:/path/to/astra` | `/path/to/astra` |
   | `QT_DIR` | `C:/Qt/6.7.0/msvc2022_64/lib/cmake/Qt6` | `/home/user/Qt/5.15.2/gcc_64/lib/cmake/Qt5` |

3. Откройте проект в Qt Creator и соберите.

4. После любого изменения `CMakePresets.json` выполните **Build → Reload CMake Presets**.

> **ВАЖНО:** Qt Creator не передаёт информацию о Qt через CMakePresets.json,
> поэтому `QT_DIR` обязательно прописывать вручную.
> Для ускорения сборки укажите генератор CMake **Ninja** — иначе используется
> медленный `"Visual Studio 17 2022"` или `"Unix Makefiles"`.

---

# Предварительные требования: Windows

| Компонент | Версия | Примечание |
|---|---|---|
| Visual Studio 2022 | MSVC v143 x64 | компонент «C++ Build Insights» |
| Qt | 6.7.0+ для MSVC 2022 x64 | |
| CMake | входит в Qt | `C:\Qt\Tools\CMake_64\bin\cmake.exe` |
| Ninja | любая | опционально, ускоряет сборку |
| NASM | 3.01 | требуется для SDL3_image · [скачать](https://www.nasm.us/pub/nasm/releasebuilds/3.01/win64/) |
| Python | 3.10+ | добавить в PATH · [скачать](https://www.python.org/downloads/release/python-3120/) |
| PostgreSQL | 18 | требуется для сборки libpqxx (нужна для COMCK-плагина) |

---

# Предварительные требования: AstraLinux 1.8

## Компилятор и базовые инструменты
```bash
sudo apt install -y build-essential cmake
```

## Python
```bash
sudo apt install -y python3
```

## PostgreSQL (требуется для libpqxx)
```bash
sudo apt install -y libpq-dev
```

## SDL (X11/Wayland)
```bash
sudo apt install -y \
    libx11-dev libxext-dev libxrandr-dev libxcursor-dev \
    libxinerama-dev libxi-dev libxss-dev libxxf86vm-dev \
    libxtst-dev libxkbcommon-dev libwayland-dev \
    libdbus-1-dev libudev-dev libgbm-dev libdrm-dev
```

---

# Предварительные требования: AltLinux 10.4

## Компилятор и базовые инструменты
```bash
apt-get install -y gcc g++ make cmake
```

## Python
```bash
apt-get install -y python3.11 python3.11-dev
```
> В AltLinux p10 системный `python3` — версия 3.9.2, не подходит для кодогенерации.
> Используйте именно `python3.11`.

## PostgreSQL (требуется для libpqxx)
```bash
apt-get install -y libpq-devel postgresql-devel
```

## SDL
```bash
apt-get install -y \
    libXcursor-devel libXi-devel bzlib-devel \
    libXrandr-devel libpcre-devel libXScrnSaver-devel \
    libbrotli-devel libXtst-devel
```

## Scintilla (GTK)
```bash
apt-get install -y libgtk+3-devel
```

---

# Шаг 1. Загрузка submodule-зависимостей

Из корня проекта (`…/rastr`):

```bash
git submodule update --init --recursive
```

Все сторонние проекты загружаются в (через git submodule):
```
…/rastr/qrastr/thirdparty/
```

---

# Шаг 2. Загрузка Scintilla и Lexilla вручную

Библиотеки не входят в submodule. Скачать с https://scintilla.org/ и разместить:

```
…/rastr/qrastr/thirdparty/scintilla/
…/rastr/qrastr/thirdparty/lexilla/
```

---

# Шаг 3. Сборка сторонних библиотек

Скрипты собирают все библиотеки thirdparty.

```bash
# AstraLinux / AltLinux:
…/rastr/qrastr/thirdparty/build_thirdparty_gcc.sh

# Windows (MSVC):
…\rastr\qrastr\thirdparty\build_thirdparty_vs_msvc.bat
```

> **ВАЖНО:**
> - Укажите персональные пути к пакетам в начале скрипта перед запуском.
> - Не редактируйте `.sh`-файлы через Windows-редактор — нарушаются символы переноса строки.

После сборки артефакты размещаются в:
```
${THIRDPARTY_DIR}/compile/${LIB_NAME}/linux/gcc/${CONFIG}/
```

---

# Шаг 4. Установка библиотеки qtitan

Скачать вручную и разместить в:
```
…/rastr/qrastr/lib/
```

---

# Шаг 5. Сборка QtitanDataGrid
**ВАЖНО:** Скрипт должен лежать внутри папки qtitan.

```bash
# Windows:
…\rastr\qrastr\lib\QtitanDataGrid8.2.0_win_qt_6_7\build_qtitan_vs_msvc.bat

# AstraLinux / AltLinux:
…/rastr/qrastr/lib/qtitan8_lin_5_15/build_qtitan_astralinux.sh
```

---

# Шаг 6. Библиотеки Графики

Windows — MinIO:
```
http://msk-n9e-psu4.ntc.ntcees.ru:9001/browser/qrastr  →  бакет Alexey_graph_libs
```

---

# Итоговая структура проекта

```
…/rastr/qrastr/
├── Data/
│   ├── Files/
│   ├── form/
│   └── SHABLON/
├── lib/
│   ├── astra/
│   └── QtitanDataGrid8.2.0_win_qt_6_7/
├── qrastr/
└── thirdparty/
    ├── compile/                   ← предсобранные библиотеки
    │   ├── fmt/
    │   ├── spdlog/
    │   ├── SDL3/
    │   ├── SDL3_image/
    │   ├── scintilla/
    │   ├── lexilla/
    │   ├── ScintillaEdit/
    │   ├── ScintillaEditBase/
    │   ├── qtadvanceddocking/
    │   └── libpqxx/
    ├── fmt/                       ← исходники (submodule)
    ├── spdlog/
    ├── SDL/
    ├── SDL_image/
    ├── scintilla/                 ← скачать вручную
    ├── lexilla/                   ← скачать вручную
    ├── Qt-Advanced-Docking-System/
    └── libpqxx/
```

---

# Быстрый старт (при наличии предсобранных библиотек)

Если папка `compile/` уже есть:

1. Поместите `compile/` → `…/rastr/qrastr/thirdparty/compile/`
2. Поместите `astra/` и `QtitanDataGrid8.2.0_win_qt_6_7/` → `…/rastr/qrastr/lib/`
3. Установите Python 3.12 (Windows) или python3.11 (AltLinux), добавьте в `PATH`
4. Откройте проект в Qt Creator и соберите

---

# Известные проблемы

## Два Qt на AstraLinux 1.8

На AstraLinux присутствуют системный Qt 5.15.8 и Qt из QtCreator 5.15.2.
CMake выбирает нужный Qt при сборке, но при запуске `ld.so` находит **системный**.

**Решение** — задать RPATH в `CMakeLists.txt`:
```cmake
set_target_properties(${PROJECT_NAME} PROPERTIES
    BUILD_RPATH      "\$ORIGIN/libs"
    INSTALL_RPATH    "\$ORIGIN/libs"
    BUILD_WITH_INSTALL_RPATH TRUE
)
```

## Отладчик заблокирован на AstraLinux
1. Анализ:
При проблемах с запуском отладчика в Qt Creator сначала проверьте Debugger Log (вкладка внизу IDE) и выполните следующие команды в терминале:
1.1 — проверить текущий уровень ptrace:
	cat /proc/sys/kernel/yama/ptrace_scope
Значение 1 — норма. Значение 3 — отладка заблокирована.
1.2 — проверить активные модули безопасности ядра:
	cat /sys/kernel/security/lsm
Если в списке есть parsec — он управляет ptrace_scope.

2. Решение:
2.1 Проверить, кому принадлежит файл
	dpkg -S /etc/sysctl.d/999-astra.conf
Если файл принадлежит пакету astra-safepolicy — он будет восстанавливаться при каждой загрузке. Нужна заморозка файла.
2.2 Изменить значение и заморозить файл
	sudo chattr -i /etc/sysctl.d/999-astra.conf
	sudo sed -i 's/ptrace_scope=3/ptrace_scope=1/' /etc/sysctl.d/999-astra.conf # изменение значения
	grep ptrace_scope /etc/sysctl.d/999-astra.conf   							# проверить
	sudo chattr +i /etc/sysctl.d/999-astra.conf									# заморозить
	sudo reboot																	# Перезагрузить
	grep ptrace_scope /etc/sysctl.d/999-astra.conf   							# проверить

Примечание: chattr +i запрещает изменение файла даже для root.
Пакет astra-safepolicy при обновлении выдаст ошибку записи — это не критично. Для отката: sudo chattr -i /etc/sysctl.d/999-astra.conf
