# StacieEmu
Stacie Emulator for the PEGASUS Cubesat project


### Build on Windows

Download and install QT-Creator : https://www.qt.io/download/
Install Visual Studio 2015 Update 3 and select the universal c-runtime (or install https://support.microsoft.com/en-us/kb/2999226)

The 2015 Redistributable Runtime is also needed : https://www.microsoft.com/de-at/download/details.aspx?id=48145

To pack the exe on windows, you must include this files:
* Qt5SerialPort.dll
* QT5Core.dll
* QT5Gui.dll
* QT5Widgets.dll

### Build on Linux

Based on Ubuntu 16.04 LTS

sudo apt-get update
sudo apt-get install qtcreator

