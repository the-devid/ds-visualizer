# ds-visualizer
Это визуализатор структуры данных [2-3 Дерево](docs/2-3-tree.md).
## Требования
+ Компилятор `C++`, поддерживающий стандарт `C++20`.
+ Система сборки CMake версии хотя бы 3.16.
+ Библиотеки `Qt6 Widgets` и `Qt6 GUI`. В теории можно использовать эти библиотекии версий `Qt5`, заменив в `CMakeLists.txt` все вхождения `Qt6` на `Qt5` перед сборкой.
## Сборка
Находясь в сборочной директории, если `path/to/dir` - путь до файла `CMakeLists.txt` из данного проекта, выполнить следующее:
```bash
cmake path/to/dir -DCMAKE_BUILD_TYPE=RELEASE
make ds_visualizer
```
После чего будет собран исполняемый файл `ds_visualizer`.

Находясь в корне склонированного репозитория можно сделать
```bash
mkdir build && cd $_
cmake ..  -DCMAKE_BUILD_TYPE=RELEASE
make ds_visualizer
```
