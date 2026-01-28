# QRastr
Кроссплатформенный Rastr на основе QT.

Сборка проекта **QRastr** в QTCreator с помощью CMake:

## 1. Требуется наличие установленного пакета **Astra**, для обеспечения:
```cmake
find_package(Astra REQUIRED)
target_link_libraries( ${PROJECT_NAME} PRIVATE astra  )
```

Инcталяция **Astra** примеры:
### Windows
```console
C:\Projects\tfs\rastr\RastrWin> cmake --install build --config Debug
C:\Projects\tfs\rastr\RastrWin> cmake --install build --config Release
```
В каталоге build должен появиться каталог install - это и есть пакет **Astra**

Требуется указать
```cmake
RASTR_PREFIX_PATH C:/Projects/tfs/rastr/RastrWin/build/install
```

### Linux
```console
/projects/git_main/rastr/build-RastrWin-Desktop-Release$ sudo cmake --install .
```
Пакет должен записаться в /usr/local

## 2. Требуемые пакеты (Third-party) с использованием find_package
- fmt - Форматирование и кодировка ввода/ввывода строк
- Qt - компоненты Qt
- -	  Core
- -    Widgets
- -    LinguistTools
- -    PrintSupport
- -    Xml
- -    Test

fmt уже входит в RastrWin/thirdparty поэтому используем его. Требуется указать:
```cmake
COMPILE_DIR C:\Projects\compile_cmake  #(который по умолчанию C:\projects\rastr\RastrWin\thirdparty\_deps\compile)
```
## 3. Требуемые пакеты (Third-party) собранные вручную
- [*scintilla*](https://github.com/ScintillaOrg/lexilla) - текстовые редактор, нужен для компонента протокол
- [*Qt-Advanced-Docking-System*](https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System) - Докинг окон, красивый
- [*spdlog*](https://github.com/gabime/spdlog) - Логи
- QtitanDataGrid - собирается из исходников, контрол для грида
- [*SDL*](https://github.com/libsdl-org/SDL) - зарезервирован для отображения/рендеринга однолинейной графики , пока не используется

Для них требуется указать
```cmake
DEPENDENCIES_LOCATION c:/projects/compile/ # win: "c:/projects/compile/ lin: /home/ustas/projects/compile
```



    
