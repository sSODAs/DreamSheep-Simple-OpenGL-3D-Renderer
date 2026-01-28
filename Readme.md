# OpenGL First Project (GLEW + GLFW + GLM)

## Quickstart

### 0) VS Code extensions
- C/C++ (ms-vscode.cpptools)
- CMake Tools (ms-vscode.cmake-tools)

### 1) Toolchain
- **Windows**: Visual Studio 2022 (Desktop C++), Git, CMake
- **macOS**: Xcode CLT (`xcode-select --install`), Git, CMake, Ninja (`brew install cmake ninja`)
- **Ubuntu/Debian**: `sudo apt install -y build-essential git cmake ninja-build`

### 2) vcpkg (one-time)
- **Windows (PowerShell):**
  ```powershell
  git clone https://github.com/microsoft/vcpkg $env:USERPROFILE\vcpkg
  & $env:USERPROFILE\vcpkg\bootstrap-vcpkg.bat
  setx VCPKG_ROOT "$env:USERPROFILE\vcpkg"

- **macOS/Linux:**
  git clone https://github.com/microsoft/vcpkg ~/vcpkg
  ~/vcpkg/bootstrap-vcpkg.sh
  echo 'export VCPKG_ROOT="$HOME/vcpkg"' >> ~/.zshrc   # or ~/.bashrc
  export VCPKG_ROOT="$HOME/vcpkg"