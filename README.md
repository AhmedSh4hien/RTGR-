# Real Time Graphics
Repository for the code used in the course Real Time Graphics at the FH Hagenberg.

Contact: [David C. Schedl](mailto:david.schedl@fh-hagenberg.at).

## Install
To compile the examples use the following steps:
### VCPKG:
The entire package uses [vcpkg](https://github.com/microsoft/vcpkg) for building. This ensures cross-platform compatibility and avoids the need to provide any thirdparty libraries (include files, libraries, dlls ...). 
Make sure that [vcpkg](https://github.com/microsoft/vcpkg) is installed and optionally define an `VCPKG_ROOT` environment variable. vcpkg also works without the environment variable; just adjust the paths accordingly. On the lab machines the path to vcpkg is `C:/public/vcpkg`.
See this [Get started guide](https://vcpkg.io/en/getting-started.html) for additional help.

Install `assimp`, `glad`, `glfw3`, `glm`, `opengl`, `stb` and `imgui` via vcpkg.
```pwsh
vcpkg install assimp glad glfw3 glm opengl stb imgui[opengl3-binding,glfw-binding] --triplet x64-windows
```

:watch: Note that installation will take a few minutes, especially if vcpkg runs for the first time!

:important: **Note**: Make sure the English language pack of Visual Studio is installed if you use Windows and Visual Studio as compiler. :unamused: 

### Working with Visual Studio
After installing the libraries working with Visual Studio is very convenient. 
Make sure to integrate vcpkg into Visual Studio with:
```pwsh
vcpkg integrate install
```
All installed libraries should now be discoverable by IntelliSense and usable in code without additional configuration.

Just open the corresponding project file in the  `VS` folder.

### Working with Other IDEs:

Most modern IDEs (e.g., Visual Studio Code or CLion) support a CMake build workflow and will support the project structure. 
However, for a flawless integration it is important to specify the `CMAKE_TOOLCHAIN_FILE` correctly. 

#### Visual Studio Code: 
1. Install C/C++ and CMake Tools extensions. 
2. Go to File -> Preferences -> Settings
3. Choose Extensions, and then "CMake Tools".
4. Locate “Cmake: Configure Settings” and select Edit in settings.json.
5. In the settings.json add: 
    ```json
        "cmake.configureSettings": {
            "CMAKE_TOOLCHAIN_FILE": "<path_to_vcpkg>/vcpkg/scripts/buildsystems/vcpkg.cmake"
        }
    ```

For further details see [this blog](https://gamefromscratch.com/vcpkg-cpp-easy-mode-step-by-step-tutorial/).

#### JetBrains CLion:
1. Go to File -> Settings -> Build, Execution, Deployment -> Toolchains
2. Make sure that the architecture is `amd64`. Note: it will also work with x86, but requires that all packages are installed without the x64 triplet in vcpkg.
3. Then go to File -> Settings -> Build, Execution, Deployment -> CMake
4. Choose one profile, and then find "CMake Options".
5. Insert value to the option: `-DCMAKE_TOOLCHAIN_FILE=<path_to_vcpkg>/scripts/buildsystems/vcpkg.cmake`

For further details see [this discussion](https://github.com/Microsoft/vcpkg/issues/3572).

### Working with CMake:

To build the module, make the repository the current directory and run the following Powershell commands:
```pwsh
mkdir build 
cd build
cmake .. "-DCMAKE_TOOLCHAIN_FILE=<path_to_vcpkg>\scripts\buildsystems\vcpkg.cmake"
```
The `build` folder now contains project files (e.g., Visual Studio Solutions on Windows).

#### Buiding with CMake

Open the project files and run or edit them (e.g., Visual Studio). 

Alternatively use cmake to build the project(s).

```pwsh
cmake --build . --config=Release
cmake --install .
```
The `bin` folder now contains compiled files.


