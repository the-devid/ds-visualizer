# ds-visualizer
Это визуализатор структуры данных [2-3 Дерево](docs/2-3-tree.md).
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
