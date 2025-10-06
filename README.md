# cth
simple library with unit tests

## install process
1. either select "install" in your ide or compile the custom "install" target for all configurations. 
2. reference "cth/install/config_name/" in your cmake targets and ```find_package(cth)``` should discover everything
3. provides ```cth::cth``` and (```cth::win``` if on windows) 

## example features:
- cth_log: assert extensions & other logging helpers
- cth_keybd: keyboard input logger with high and low level interfaces
- cth_win: hidden console
- cth_console: fully color supported console

absoloutly no backwards compartibility guaranteed
