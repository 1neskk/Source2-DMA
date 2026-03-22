# Source2 DMA 
This project is built on top of my [CS2 External Base](https://github.com/1neskk/CS2-External) and it is a DMA (Direct Memory Access) software targeting CS2 and Deadlock, I recommend using this project to start learning DMA attacks using a FPGA device in a "fun" (considering that you do not disrupt other players' experience) way since it has considerable abstraction due to [DMALibrary](https://github.com/Metick/DMALibrary).

This project was created with the goal to assist in the learning experience regarding game hacking, DMA attacks, FPGA. It is meant to be used for educational purposes only, because of that offsets are most likely outdated and keep in mind that this project should not be used for malicious purposes.

> [!WARNING]
> The game Deadlock ("deadlock.exe") is not yet fully supported.

### Disclaimer
This project is for educational purposes only. I do not condone cheating in any form.

I did not test the project thoroughly when creating the repo (late March 2026) but it should work fine. Last tested in February to early March 2026.

## Table of Contents
- [Features](#features)
- [Requirements](#requirements)
- [Dependencies](#dependencies)
- [Get Started (Windows Only)](#get-started-windows-only)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [License](#license)
- [Contributing](#contributing)

## Features
- Physical memory reading through the PCIe Bus lane (abstracted)
- ESP (Name, Health, Box, Bones, Team Check)
- Fuser compatible overlay
- [PCILeech](https://github.com/ufrisk/pcileech) compatible DMA
- Separate worker thread for DMA operations with a double-buffer + mutex approach to avoid race conditions, "making" the main thread for GUI render (most intensive task in main thread)

## Requirements
- FPGA board (tested on XC7A75T) with DMA capable firmware ([PCILeech](https://github.com/ufrisk/pcileech-fpga)-based is encouraged)
- Secondary machine running the software
- [Visual Studio 2022](https://visualstudio.microsoft.com/) with the **Desktop development with C++** workload (MSVC v143, C++20)

## Dependencies
- [DirectX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812)
- [CMake](https://cmake.org/)
- [Python 3](https://www.python.org/) (optional, used by the setup script to invoke CMake)
- [GLM](https://github.com/g-truc/glm), [ImGui](https://github.com/ocornut/imgui) and [DMALibrary](https://github.com/Metick/DMALibrary) (included as git submodules)
- The following DLLs inside `vendor-bin` must be placed in the same directory as the compiled executable:
  - `FTD3XX.dll` — [FTDI D3XX driver](https://ftdichip.com/drivers/d3xx-drivers/)
  - `leechcore.dll` — [LeechCore](https://github.com/ufrisk/LeechCore)
  - `vmm.dll` — [MemProcFS](https://github.com/ufrisk/MemProcFS)

## Get Started (Windows Only)
1. Clone the repository recursively
```bash
git clone --recursive git@github.com:1neskk/Source2-DMA.git
```
2. Run `scripts/setup.bat` — this invokes a Python script that creates a `build/` directory and generates a Visual Studio 2022 x64 solution via CMake
3. Open the solution file inside the `build` directory
4. Build the solution
5. Run the resulting executable (considering all the [requirements](#requirements) and [dependencies](#dependencies))

## Usage
- Press 'INSERT' to toggle the imgui menu
- The 'Attach' button will attach the software to the selected game's process (the game must be open)
- When attached to the game's process the 'Detach' button will detach the program from the game's process
- The 'Exit' button will close the program

## Project Structure
```
Source2-DMA/
├── src/              # Application source code (entry point, cheat logic, overlay, offsets)
│   ├── imgui/        # Fonts used by the ImGui menu/overlay
│   └── offsets/      # Game offset definitions
├── thirdparty/       # Git submodules (DMALibrary, GLM, ImGui)
├── lib/              # Pre-built static libraries (.lib)
├── vendor-bin/       # Runtime DLLs required next to the executable
├── scripts/          # Build helper scripts (setup.bat / setup.py)
└── CMakeLists.txt    # Build configuration
```

## License
This project is licensed under the Apache 2.0 License - see the [LICENSE](LICENSE) file for details

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.
