# cth
simple library with unit tests

## install process
1. either select "install" in your ide or compile the custom "install" target for all configurations. 
2. reference "cth/install/config_name/" in your cmake targets and ```find_package(cth)``` should discover everything
3. provides ```cth::cth```, ```cth::coro``` and ```cth::win``` (if on windows) 


## example features:
- cth_log:
  - stable & debug only macros
  - severities (log, info, warning, error, critical)
  - automatic printing (included location details based on log lvl)
  - std::format support integrated
  - trailing braces block to execute before error / abort fires.
    ```cpp
    CTH_STABLE_THROW("asdf {}", someint) {
      details->add("detail: {}", 0);
      cleanup();
    } <- throws here
    ```
- cth::coro
  - async native handle wait on windows & linux (untested xD)
  - coroutine task system with this_coro::executor (like boost)
- cth::win (wraps and compacts features of windows.h without leaking it)
  - hidden console cmd executions (contrary to windows.h system by default)
  - screen size / monitors / screenshots
  - more that i am too lazy to write here
- cth_console: fully color supported console

absoloutly no backwards compartibility guaranteed
