# Инструкция по деплою QRastr: AppImage

> Скрипт деплоя: `deploy_appimage.sh`. Данный документ объясняет теорию,
> архитектурные решения и логику каждого шага скрипта. Сами команды — в скрипте.

---

# Часть 1. Теория AppImage

## Окружение

| Параметр | AstraLinux 1.8 | AltLinux 10.4 |
|---|---|---|
| GLIBC | 2.34 | 2.32 |
| GCC | 12 | 10 |
| C++17 | ✓ | ✓ |
| Qt (системный) | 5.15.8 | зависит от установки |
| Qt (QtCreator) | 5.15.2 | 5.15.2 (устанавливается вручную) |
| CMake | 3.30.5 (из Qt) | 3.23+ (из Qt или репозитория) |

## Что такое AppImage

AppImage — формат дистрибуции Linux-приложений без установки. Один файл содержит
внутри себя squashfs-образ с бинарником, всеми его `.so`-зависимостями и
Qt-плагинами. При запуске AppImage монтирует этот образ через FUSE и запускает
приложение из него. Пользователю достаточно сделать файл исполняемым и запустить.

## Почему сборка только на AltLinux 10.4

AppImage не включает стандартные библиотеки C (`libc.so.6`, `libstdc++.so.6` и др.)
— они берутся из ОС при запуске. Если бинарник собран на системе с GLIBC 2.34
(AstraLinux 1.8), он использует символы `GLIBC_2.33`, `GLIBC_2.34`, которых нет
на системах с GLIBC 2.32 (AltLinux 10.4). Обратное неверно.

```
AltLinux 10.4   GLIBC 2.32  ← собирать здесь
AstraLinux 1.8  GLIBC 2.34  ← запустится, т.к. 2.34 > 2.32
```

**Вывод:** всегда собирать AppImage на AltLinux 10.4. Тогда он запустится
и на AltLinux 10.4, и на AstraLinux 1.8.

## Blacklist библиотек в linuxdeploy

`linuxdeploy` при сборке AppImage намеренно **не включает** ряд системных библиотек:

```
libc.so.6       — стандартная библиотека C
libstdc++.so.6  — стандартная библиотека C++
libm.so.6       — математическая библиотека
libgcc_s.so.1   — runtime-поддержка GCC
libpthread.so.0 — POSIX-потоки
libdl.so.2      — динамическая загрузка
```

Эти библиотеки тесно интегрированы с конкретным ядром Linux и должны браться из ОС.
Попытка упаковать `libc.so.6` внутрь AppImage крайне сложна и ненадёжна. Именно
поэтому работает правило «собирай на самой старой целевой системе».

## Проблема двух Qt на Linux

На Linux могут быть установлены одновременно системный Qt5 и Qt из Qt5, скачанный из QtMaintananceTools.
CMake находит нужный Qt по `QT_DIR`, но записывает в бинарник только SONAME библиотек
(например, `libQt5Core.so.5`). При запуске `ld.so` находит **системный** Qt.

Решение — задать RPATH в `CMakeLists.txt`:

```cmake
set_target_properties(${PROJECT_NAME} PROPERTIES
    BUILD_RPATH      "\$ORIGIN/libs"
    INSTALL_RPATH    "\$ORIGIN/libs"
    BUILD_WITH_INSTALL_RPATH TRUE
)
```

## Переменная APPIMAGE и поиск папки Data

`applicationDirPath()` внутри AppImage возвращает путь внутри смонтированного
squashfs (`/tmp/.mount_XXX/usr/bin/`), а не директорию с файлом `.AppImage`.

AppImage runtime автоматически выставляет переменную `APPIMAGE` с полным путём
к файлу. Через неё и находим `Data/`:

```cpp
const QString appImageEnv = qEnvironmentVariable("APPIMAGE");
if (!appImageEnv.isEmpty()) {
    baseDir = QFileInfo(appImageEnv).absolutePath();  // рядом с .AppImage
} else {
    baseDir = QDir::cleanPath(
        QCoreApplication::applicationDirPath() + "/..");  // Qt Creator
}
```

## Папка AppDir

`AppDir` — временный staging-каталог, из которого `linuxdeploy` упаковывает squashfs.
После создания `.AppImage` он не нужен. Скрипт полностью удаляет `AppDir` в начале
каждого запуска — это гарантирует отсутствие артефактов от предыдущих сборок.

## Подготовка инструментов linuxdeploy (один раз)

Скачать и сделать исполняемыми в папку `LINUXDEPLOY_DIR` (из скрипта):
```bash
mkdir -p ~/tools/linuxdeploy
cd ~/tools/linuxdeploy
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x linuxdeploy-x86_64.AppImage linuxdeploy-plugin-qt-x86_64.AppImage
```

---

# Часть 2. Карта зависимостей плагинов

```
qrastr
├── libqmcr.so             ← QLibrary: applicationDirPath()/libqmcr.so
│   └── libScintillaEdit.so.5
│       └── liblexilla.so
├── liblexilla.so          ← QLibrary: applicationDirPath()/liblexilla.so
└── plugins/               ← QPluginLoader: applicationDirPath()/plugins/
    ├── librastr.so
    │   └── libastra.so    ← QLibrary: applicationDirPath()/plugins/astra
    │       └── libmk4.so
    ├── libCOMCK.so
    │   └── libpqxx-7.10.so
    ├── libbarsmdp.so      → зависит от libCOMCK.so
    ├── libpgdriver.so     → зависит от libCOMCK.so
    └── libti.so           → зависит от libCOMCK.so
```

## Почему часть .so идёт в usr/bin/, а часть в usr/lib/

Размещение определяется механизмом загрузки:

**`usr/bin/`** — библиотеки, загружаемые через `QLibrary` с явным путём
`applicationDirPath() + "/libXxx.so"`. Внутри AppImage `applicationDirPath()`
указывает на `usr/bin/`, поэтому эти файлы должны лежать там же.

**`usr/bin/plugins/`** — плагины, загружаемые через `QPluginLoader` с путём
`applicationDirPath() + "/plugins/"`, и их прямые зависимости (`libmk4.so`,
`libpqxx`). Зависимости кладутся рядом с плагинами, чтобы `ld` нашёл их через
RPATH `$ORIGIN`.

**`usr/lib/`** — библиотеки, загружаемые через RPATH бинарника (`$ORIGIN/libs`
или `$ORIGIN/../lib`). `linuxdeploy` находит их через `LD_LIBRARY_PATH` при
упаковке и сам укладывает в `usr/lib/`.

---

# Часть 3. Логика шагов deploy_appimage.sh

## Конфигурация в начале скрипта

Все пути вынесены в один блок в начало файла: `QT_ROOT`, `PROJ`, `THIRDPARTY`,
`ASTRA_ROOT`, `LINUXDEPLOY_DIR`, `OUTPUT_DIR`. При смене окружения (новая машина,
другой путь Qt) редактируется только этот блок.

## Шаг 0: Проверки окружения

Перед началом работы скрипт проверяет наличие всех необходимых файлов: Qt, бинарника
проекта, предсобранной libpqxx, инструментов linuxdeploy, иконки. Если чего-то нет —
выход с перечнем проблем, без частичного выполнения.

## Шаг 1: Подготовка AppDir

AppDir полностью удаляется и создаётся заново при каждом запуске. `linuxdeploy`
при повторном запуске дополняет существующий AppDir, а не перезаписывает — старые
версии библиотек накапливаются. Полная очистка исключает этот сценарий.

Структура `usr/bin`, `usr/lib`, `usr/share` соответствует стандарту FHS, который
ожидает linuxdeploy.

## Шаг 2: Копирование файлов

Файлы копируются в три группы согласно механизму загрузки (см. Часть 2).

`libScintillaEdit.so.5` копируется с суффиксом версии, но рядом создаётся
симлинк без суффикса — именно его ищет `QLibrary` по умолчанию.

`libmk4.so` является критичной зависимостью: при её отсутствии скрипт завершается
с ошибкой, не доходя до linuxdeploy.

`libpqxx` берётся из `THIRDPARTY/libpqxx/linux/gcc/Release/` — результата работы
`build_thirdparty_gcc.sh`. При отсутствии скрипт также завершается с ошибкой
и подсказывает, как пересобрать.

## Шаг 3: Интеграция Python 3.11

Приложение использует Python-модуль `astra_py`. На целевых машинах Python 3.11
может отсутствовать или иметь другую версию, поэтому интерпретатор, его stdlib
и `libpython.so` упаковываются внутрь AppImage.

Интерпретатор копируется без суффикса версии (`python`, не `python3.11`) — именно
такое имя используется при вызове через `Py_Initialize`.

При отсутствии Python скрипт продолжает работу с предупреждением: функциональность
`astra_py` будет недоступна, но AppImage соберётся.

## Шаг 4: linuxdeploy

Перед запуском `linuxdeploy` выставляются переменные окружения:

`QTDIR`, `PATH`, `QMAKE` — указывают инструменту, какой Qt использовать.

`LD_LIBRARY_PATH` расширяется путями из AppDir (`usr/lib` и `usr/bin`). Это
необходимо, чтобы `linuxdeploy` при анализе зависимостей через `ldd` видел
вручную скопированные библиотеки (Qtitan, SDL3, qtadvanceddocking) и включил
их в AppImage.

`LINUXDEPLOY_PLUGIN_QT_PATH` — путь к плагину Qt для linuxdeploy. Плагин
автоматически копирует Qt-библиотеки, Qt-плагины платформы (xcb, imageformats
и т.д.) и создаёт `qt.conf`. Ручное создание `qt.conf` не требуется.

Вывод linuxdeploy дублируется в файл `deploy.log` в `OUTPUT_DIR`.

## Шаг 5: Итог

Скрипт проверяет факт создания файла, выводит его размер и путь. При ошибке
указывает на `deploy.log`.

---

# Часть 4. Структура на целевой машине

```
~/rastr_deploy/
├── QRastr-x86_64.AppImage
└── Data/                    ← поставляется отдельно
    ├── appsettings.json
    ├── SHABLON/
    │   ├── context.form
    │   └── *.rg2
    └── form/
        └── *.fm
```

AppImage ищет `Data/` рядом с собой через переменную окружения `APPIMAGE`.
Папка `Data/` должна лежать в той же директории, что и `.AppImage` файл.

Запуск:
```bash
./QRastr-x86_64.AppImage                        # стандартный (требует FUSE)
./QRastr-x86_64.AppImage --appimage-extract-and-run   # если FUSE недоступен
```

---

# Часть 5. Диагностика

## Зависимости бинарника

Распаковать AppImage без FUSE (`--appimage-extract`) и проверить через `ldd`:
- основной бинарник `usr/bin/qrastr`
- критичные плагины: `usr/bin/plugins/librastr.so`, `usr/bin/plugins/libCOMCK.so`

Строки `not found` в выводе `ldd` — отсутствующие зависимости.

## Требуемая версия GLIBC

Через `objdump -T` на бинарнике отфильтровать символы `GLIBC_*` и взять
максимальный. Если результат выше `GLIBC_2.32` — AppImage собран не на AltLinux 10.4
и не запустится на целевой системе. Нужна пересборка.

## Загрузка Qt-плагинов

Запуск с переменной `QT_DEBUG_PLUGINS=1` выводит в stderr список загружаемых
Qt-плагинов. Строки `Cannot load library` указывают на конкретную проблему.

## Поиск папки Data

Лог приложения (через `spdlog`) содержит вывод `PathHelper`. Если путь к `Data/`
не выводится — переменная `APPIMAGE` не установлена. Это происходит, когда
`.AppImage` запускается через shell-обёртку, а не напрямую. Решение: запускать
файл напрямую.
