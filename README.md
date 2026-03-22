# Source2 DMA 
This project is built on top of my [CS2 External Base](https://github.com/1neskk/CS2-External) and it is a DMA (Direct Memory Access) software targeting CS2 and Deadlock, I recomend using this project to start learning DMA attacks using a FPGA device in a "fun" (considering that you do not disrupt other players' experience) way since it has considerable abstraction due to [DMALibrary](https://github.com/Metick/DMALibrary).

This project was created with the goal to assist in the learning experience regarding game hacking, DMA attacks, FPGA. It is meant to be used for educational purposes only, because of that offsets are most likely outdated and keep in mind that this project should not be used for malicious purposes.

> [!WARNING]
> The support for Deadlock ("deadlock.exe") is deprecated/discouraged yet.

### Disclaimer
This project is for educational purposes only. I do not condone cheating in any form.

I did not test the project thoroughly when creating the repo (late March 2026) but it should work fine. Last tested in February to early March 2026.

## Table of Contents
- [Features](#features)
- [Dependencies](#dependencies)
- [Get Started (Windows Only)](#get-started-windows-only)
- [Usage](#usage)
- [License](#license)
- [Contributing](#contributing)

## Features
- Physical memory reading through the PCIe Bus lane (abstracted)
- ESP (Name, Health, Box, Bones, Team Check)
- Fuser compatible overlay
- [PCILeech](https://github.com/ufrisk/pcileech) compatible DMA

## Requirements
- A FPGA board (tested on XC7A75T) with DMA capable firmware ([PCILeech](https://github.com/ufrisk/pcileech-fpga)-based is encouraged)
- Secondary machine running the software

## Dependencies
- [DirectX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812)
- [CMake](https://cmake.org/)
- [GLM](https://github.com/g-truc/glm), [ImGui](https://github.com/ocornut/imgui) and [DMALibrary](https://github.com/Metick/DMALibrary) (included as git submodules)
- The dynamic libraries inside `vendor-bin` are to be in the same directory as the compiled executable (you may procure the official ones if feeling distrustful)

## Get Started (Windows Only)
1. Clone the repository recursively
```bash
git clone --recursive git@github.com:1neskk/Source2-DMA.git
```
2. Run the script `setup.bat` to build the project
3. Open the solution file inside the `build` directory
4. Build the solution
5. Run the resulting executable (considering all the [requirements](#requirements) and [dependencies](#dependencies))

## Usage
- Press 'INSERT' to toggle the imgui menu
- The 'Attach' button will attach the software to the selected game's proccess (the game must be open)
- When attached to the game's proccess the 'Detach' button will detach the program from the game's proccess
- The 'Exit' button will close the program

## License
This project is licensed under the Apache 2.0 License - see the [LICENSE](LICENSE) file for details

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.
