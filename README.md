# the_game

## Building and Running

### Windows (Visual Studio)

The easiest way to build is to use [vcpkg](https://vcpkg.io/en/index.html) and install SFML through this.

```bash
vcpkg install sfml
vcpkg install imgui
vcpkg integrate install
```

Create a new visual studio C++ empty project, git clone the files, and copy them into the project directory.

Select the "Show All Files" options in Solution Explorer, and right-click on the src/ and deps/ directory, and choose the "include in project options"

Go into the project properties and under `C/C++ > General`, add the deps/ directory as an additional include directories.

Finally, under `Linker > Input`, add OpenGL32.lib as an additional dependency.

### Linux

Install conan

```sh
python3 -m pip install conan==1.57
```

To build, at the root of the project:

```sh
sh scripts/build.sh install
```

The `install` argument is only needed for the first time compilation as this is what grabs the libraries from Conan

To run, at the root of the project:

```sh
sh scripts/run.sh
```

To build and run in release mode, simply add the `release` suffix:

```sh
sh scripts/build.sh release
sh scripts/run.sh release
```
