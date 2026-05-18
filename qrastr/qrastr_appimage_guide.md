# QRastr: сборка проекта и создание AppImage

## Окружение

| Параметр | Astra Linux 1.8 | AltLinux 10.4 |
|---|---|---|
| GLIBC | 2.34 | 2.32 |
| GCC | 12 | 10 |
| C++17 | ✓ | ✓ |
| Qt (системный) | 5.15.8 | зависит от установки |
| Qt (QtCreator) | 5.15.2 | 5.15.2 (устанавливается вручную) |
| CMake | 3.30.5 (из Qt) | 3.23+ (из Qt или репозитория) |

---

## Ключевое правило сборки AppImage

> **AppImage нужно собирать на системе с самой старой версией GLIBC из всех целевых ОС.**
>
> Если бинарник собран на системе с GLIBC 2.34, он использует символы недоступные
> на системах с GLIBC 2.32. Обратное неверно: собранный на GLIBC 2.32 бинарник
> запустится везде, где GLIBC >= 2.32.
>
> **Вывод:** собирать AppImage нужно на AltLinux 10.4 (GLIBC 2.32),
> тогда он запустится и на AltLinux 10.4, и на Astra Linux 1.8.

---

## Раздел: Blacklist библиотек в linuxdeploy

`linuxdeploy` при сборке AppImage **намеренно не включает** ряд базовых библиотек ОС:

```
libc.so.6       — стандартная библиотека C
libstdc++.so.6  — стандартная библиотека C++
libm.so.6       — математическая библиотека
libgcc_s.so.1   — runtime-поддержка GCC
libpthread.so.0 — POSIX-потоки
libdl.so.2      — динамическая загрузка
```

**Что такое blacklist?**

Это встроенный список библиотек, которые linuxdeploy никогда не копирует в AppDir
независимо от того, что говорит `ldd`. Если `ldd` показывает зависимость от `libc.so.6`,
linuxdeploy её видит, но пропускает — намеренно.

**Почему они в blacklist?**

Эти библиотеки — часть ядра операционной системы. Они тесно интегрированы с конкретным
ядром Linux. Попытка включить `libc.so.6` в AppImage и использовать её вместо системной
крайне сложна: библиотека обращается к системным вызовам напрямую, и её версия должна
точно соответствовать ядру. Поэтому принято брать их из ОС.

**Что это означает на практике?**

Если AppImage собран на Astra Linux (GLIBC 2.34), а запускается на AltLinux (GLIBC 2.32),
динамический загрузчик не найдёт символы `GLIBC_2.33`, `GLIBC_2.34` и откажет в загрузке:

```
/lib64/libc.so.6: version `GLIBC_2.34' not found
/usr/lib64/libstdc++.so.6: version `GLIBCXX_3.4.29' not found
```

**Решение:** всегда собирать AppImage на самой старой целевой системе.

---

## Важные моменты перед сборкой AppImage

### Проблема двух Qt

На Astra Linux установлены два Qt: системный (5.15.8) и QtCreator (5.15.2).
CMake находил нужный Qt, но линкер записывал в бинарник только SONAME библиотек.
При запуске `ld.so` находил **системный** Qt.
Решение: В CMakeList указать свой Runtime path в папку libs:
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
    BUILD_RPATH      "\$ORIGIN/libs"
    INSTALL_RPATH    "\$ORIGIN/libs"
    BUILD_WITH_INSTALL_RPATH TRUE
)

## Поиск папки Data через APPIMAGE env

`applicationDirPath()` внутри AppImage возвращает путь внутри смонтированного squashfs
(`/tmp/.mount_XXX/usr/bin/`), а не директорию где лежит `.AppImage` файл.

AppImage runtime автоматически выставляет переменную окружения `APPIMAGE` с полным
путём к `.AppImage` файлу. Поэтому для поиска папки Data в относительных путях:

```cpp
QString baseDir;
const QString appImageEnv = qEnvironmentVariable("APPIMAGE");
if (!appImageEnv.isEmpty()) {
    // Запуск из AppImage: директория где лежит .AppImage файл
    baseDir = QFileInfo(appImageEnv).absolutePath();
} else {
    // Обычный запуск (QtCreator): бинарник в Release/,
    // Data лежит рядом с Release/ — поднимаемся на уровень выше
    baseDir = QDir::cleanPath(
        QCoreApplication::applicationDirPath() + "/..");
}

const QString confPath = QDir::cleanPath(
    baseDir + "/" +
    RastrParameters::pch_dir_data_ + "/" +
    RastrParameters::pch_fname_appsettings);

QFileInfo fi(confPath);
```
## Подготовка AltLinux 10.4 к сборке

### GCC и C++ на AltLinux 10.4

AltLinux 10.4 поставляется с GCC 10 и GLIBC 2.32.

Проект использует `CMAKE_CXX_STANDARD 17`, который поддерживается с GCC 7.

### Шаг 1: Python

```
# ── Python 3.11 Runtime ────
PYTHON_VERSION=3.11

# Копируем интерпретатор
mkdir -p ~/AppDir/usr/bin
cp /usr/bin/python${PYTHON_VERSION} ~/AppDir/usr/bin/python

# Копируем стандартную библиотеку
mkdir -p ~/AppDir/usr/lib
PYTHON_LIBDIR=/usr/lib/python${PYTHON_VERSION}
if [ -d "$PYTHON_LIBDIR" ]; then
    cp -r $PYTHON_LIBDIR ~/AppDir/usr/lib/
    echo "✓ Python standard library copied"
else
    echo "✗ Python library directory not found: $PYTHON_LIBDIR"
fi

# Копируем динамические библиотеки Python
find /usr/lib -maxdepth 1 -name "libpython${PYTHON_VERSION}*.so*" \
    -exec cp {} ~/AppDir/usr/lib/ \;
ln -sf libpython${PYTHON_VERSION}.so.1.0 ~/AppDir/usr/lib/libpython${PYTHON_VERSION}.so 2>/dev/null || true

# Копируем site-packages (если нужны дополнительные модули)
# cp -r /usr/lib/python${PYTHON_VERSION}/site-packages ~/AppDir/usr/lib/python${PYTHON_VERSION}/ 2>/dev/null || true

echo "✓ Python ${PYTHON_VERSION} integrated into AppImage"
```

### Шаг 2: FUSE (для запуска AppImage)

```bash
sudo apt-get install -y fuse libfuse-devel
```

### Шаг 3: Инструменты AppImage

```bash
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x linuxdeploy-x86_64.AppImage linuxdeploy-plugin-qt-x86_64.AppImage
```


## Зависимости и карта загрузки плагинов

### Карта зависимостей

```
qrastr
├── libqmcr.so             <- QLibrary: applicationDirPath()/libqmcr.so
│   └── libScintillaEdit.so.5
│       └── liblexilla.so
├── liblexilla.so          <- QLibrary: applicationDirPath()/liblexilla.so
└── plugins/               <- QPluginLoader: applicationDirPath()/plugins/
    ├── librastr.so
    │   └── libastra.so    <- QLibrary: applicationDirPath()/plugins/astra
    │       └── libmk4.so
    ├── libCOMCK.so
    │   └── libpqxx-7.10.so  <- нет в системе, нужно собрать из thirdparty
    ├── libbarsmdp.so      -> зависит от libCOMCK.so (некритично)
    ├── libpgdriver.so     -> зависит от libCOMCK.so (некритично)
    └── libti.so           -> зависит от libCOMCK.so (некритично)
```

### Критичность ошибок загрузки

| Плагин | Критично? | Поведение при ошибке |
|---|---|---|
| librastr.so | ДА | `return false`, приложение не запустится |
| libastra.so | ДА | ld не загрузит librastr |
| libmk4.so | ДА | ld не загрузит libastra |
| libCOMCK.so | нет | `continue` |
| libbarsmdp, libpgdriver, libti | нет | `continue` |

### Сборка libpqxx из исходников

Установка зависимсотей libpqxx (версия 7.10, т.к. требует стандарт C++17, а версия 8+ уже требует C++20):
```
sudo apt update && sudo apt install libpq-dev postgresql-server-dev-all
```


```bash
cd ~/projects/rastr/qrastr/thirdparty/libpqxx
mkdir -p build && cd build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=ON \
  -DSKIP_BUILD_TEST=ON \
  -DCMAKE_INSTALL_PREFIX=./install
cmake --build . -j$(nproc)
cmake --install .

# Проверить имя файла
find ./install -name "libpqxx*.so*"
```

---

## Создание AppImage

> Все команды выполняются на **AltLinux 10.4** (система с GLIBC 2.32).

### Шаг 0: Переменные

```bash
QT_ROOT=~/Qt/5.15.2/gcc_64
PROJ=~/projects/rastr/qrastr
THIRDPARTY=$PROJ/thirdparty/compile
ASTRA_ROOT=~/projects/astra
LIBPQXX_BUILD=$PROJ/thirdparty/libpqxx/build/install
```

### Шаг 1: Очистить AppDir

```bash
rm -rf ~/AppDir
```

### Шаг 2: Скопировать бинарник и зависимости

```bash
mkdir -p ~/AppDir/usr/{bin,lib,plugins}
mkdir -p ~/AppDir/usr/share/{applications,icons/hicolor/256x256/apps}

# Основной бинарник
cp $PROJ/Release/qrastr ~/AppDir/usr/bin/

# Библиотеки в usr/bin/ (загружаются через QLibrary с applicationDirPath())
cp $PROJ/Release/libqmcr.so ~/AppDir/usr/bin/
cp $THIRDPARTY/lexilla/linux/gcc/Release/lib/liblexilla.so ~/AppDir/usr/bin/
cp $THIRDPARTY/ScintillaEdit/linux/gcc/Release/lib/libScintillaEdit.so.5 ~/AppDir/usr/bin/
ln -sf libScintillaEdit.so.5 ~/AppDir/usr/bin/libScintillaEdit.so

# Плагины приложения (загружаются через QPluginLoader)
cp -r $PROJ/Release/plugins ~/AppDir/usr/bin/

# libmk4.so — зависимость libastra.so, должна лежать рядом с плагинами
find $ASTRA_ROOT -name "libmk4.so*" -exec cp {} ~/AppDir/usr/bin/plugins/ \; 2>/dev/null || \
find /usr/lib -name "libmk4.so*" -exec cp {} ~/AppDir/usr/bin/plugins/ \; 2>/dev/null || \
    echo "ВНИМАНИЕ: libmk4.so не найдена!"

# libpqxx — зависимость libCOMCK.so
find $LIBPQXX_BUILD -name "libpqxx*.so*" -exec cp {} ~/AppDir/usr/bin/plugins/ \; 2>/dev/null || \
    echo "ВНИМАНИЕ: libpqxx не найдена — BarsMDP/TI/PGDriver не загрузятся."

# Нестандартные .so в usr/lib/ (подтягиваются через RPATH)
cp $THIRDPARTY/qtadvanceddocking/linux/gcc/Release/lib/libqtadvanceddocking-qt5.so.4 ~/AppDir/usr/lib/
cp $THIRDPARTY/SDL3/linux/gcc/Release/lib/libSDL3.so.0 ~/AppDir/usr/lib/
cp $THIRDPARTY/SDL3_image/linux/gcc/Release/lib/libSDL3_image.so* ~/AppDir/usr/lib/ 2>/dev/null || true

cp $PROJ/lib/qtitan8_lin_5_15/lib/libQtitanBase.so.2       ~/AppDir/usr/lib/
cp $PROJ/lib/qtitan8_lin_5_15/lib/libQtitanGrid.so.8       ~/AppDir/usr/lib/
cp $PROJ/lib/qtitan8_lin_5_15/lib/libQtitanStyle.so*       ~/AppDir/usr/lib/ 2>/dev/null || true
cp $PROJ/lib/qtitan8_lin_5_15/lib/libQtitanFastInfoset.so* ~/AppDir/usr/lib/ 2>/dev/null || true

# Qt-плагины
cp -r $QT_ROOT/plugins/platforms           ~/AppDir/usr/plugins/
cp -r $QT_ROOT/plugins/xcbglintegrations   ~/AppDir/usr/plugins/
cp -r $QT_ROOT/plugins/imageformats        ~/AppDir/usr/plugins/
cp -r $QT_ROOT/plugins/printsupport        ~/AppDir/usr/plugins/

# QtWebEngine
cp $QT_ROOT/libexec/QtWebEngineProcess ~/AppDir/usr/bin/
cp -r $QT_ROOT/resources ~/AppDir/usr/
mkdir -p ~/AppDir/usr/translations
cp -r $QT_ROOT/translations/qtwebengine_locales ~/AppDir/usr/translations/

# Иконка
cp $PROJ/qrastr/resources/images/qrastr.png \
   ~/AppDir/usr/share/icons/hicolor/256x256/apps/
```

### Шаг 3: Desktop файл

```bash
cat > ~/AppDir/usr/share/applications/qrastr.desktop << 'EOF'
[Desktop Entry]
Name=QRastr
Exec=qrastr
Icon=qrastr
Type=Application
Categories=Utility;
EOF

cp ~/AppDir/usr/share/applications/qrastr.desktop ~/AppDir/
```

### Шаг 4: Запуск linuxdeploy

```bash
export QT_ROOT=~/Qt/5.15.2/gcc_64
export QTDIR=$QT_ROOT
export PATH=$QT_ROOT/bin:$PATH
export LINUXDEPLOY_PLUGIN_QT_PATH=./linuxdeploy-plugin-qt-x86_64.AppImage

export LD_LIBRARY_PATH=\
$QT_ROOT/lib:\
$HOME/AppDir/usr/lib:\
$HOME/AppDir/usr/bin:\
$HOME/AppDir/usr/bin/plugins:\
$PROJ/lib/qtitan8_lin_5_15/lib:\
$LD_LIBRARY_PATH

cd ~
./linuxdeploy-x86_64.AppImage \
    --appdir AppDir \
    --executable AppDir/usr/bin/qrastr \
    --desktop-file AppDir/qrastr.desktop \
    --plugin qt \
    --output appimage 2>&1 | tee deploy.log
```

> `qt.conf` создаётся автоматически плагином `linuxdeploy-plugin-qt`. Ручное создание не нужно.

---

## Часть 6: Запуск и структура деплоя

### Необходимая структура на целевой машине

```
~/rastr_deploy/
├── QRastr-x86_64.AppImage
└── Data/                    <- папка с данными (поставляется отдельно)
    ├── appsettings.json
    ├── SHABLON/
    │   ├── context.form     <- обязательно
    │   └── *.rg2
    └── form/
        └── *.fm
```

### Команды запуска

```bash
cd ~/rastr_deploy

# Стандартный запуск (FUSE должен быть установлен):
./QRastr-x86_64.AppImage

# Если FUSE недоступен:
./QRastr-x86_64.AppImage --appimage-extract-and-run
```

---

## Диагностика

```bash
# Распаковать AppImage и проверить зависимости
./QRastr-x86_64.AppImage --appimage-extract

ldd squashfs-root/usr/bin/qrastr
ldd squashfs-root/usr/bin/plugins/libastra.so    # проверить libmk4
ldd squashfs-root/usr/bin/plugins/libCOMCK.so    # проверить libpqxx

# Проверить максимальную версию GLIBC, которую требует бинарник
objdump -T squashfs-root/usr/bin/qrastr | grep GLIBC | \
    grep -oP 'GLIBC_[0-9]+\.[0-9]+' | sort -V | tail -5

# Проверить плагины Qt
QT_DEBUG_PLUGINS=1 ./QRastr-x86_64.AppImage 2>&1 | head -60
```
