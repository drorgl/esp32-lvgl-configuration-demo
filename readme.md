# ESP32 LVGL Workflow Demo

This is a demo and an example how to work with LVGL.

# Setup
[ ] git init
[ ] pio init
[ ] Extract latest release from https://github.com/lvgl/lvgl/releases into lib/lvgl

    [ ] copy library.json from this project
    [ ] modify library.json build section and remove demo and examples if its not relevant to your project

[ ] Extract latest source from https://github.com/lvgl/lv_drivers into lib/lv_drivers

    [ ] copy lv_drv_conf_template.h to include/native/lv_drv_conf.h and enable the file (change #if 0 to 1)
    [ ] modify library.json to include SDL dependency
    ```
    "dependencies":[
        {
            "name":"SDL2"
        }
    ],
    ```
    [ ] modify library.json to remove an incompatible source file
    ```
    "build": {
        "srcFilter" : [
            "-<display/ILI9341.c>"
        ]
    }
    ```
    [ ] update include/native/lv_drv_conf.h in appropriate places
    ```
    #include "lvgl_native_drivers.h"
    # define USE_SDL 1
    #  define SDL_HOR_RES     CONFIG_SDL_HOR_RES
    #  define SDL_VER_RES     CONFIG_SDL_VER_RES
    ```

[ ] Extract latest source from https://github.com/lvgl/lvgl_esp32_drivers into lib/lvgl_esp32_drivers and copy the library.json from this project

    [ ] copy library.json from this project, it removes compilation to incompatible files and sets its framework to espressif only
    [ ] modify lvgl_helper.c to include "lv_conf.h' right after "sdkconfig.h", the vanilla setup assumes your lvgl is part of esp32 components which can make desktop configuration a problem.
    ```
    #include "sdkconfig.h"
    #include "lv_conf.h"
    ```

[ ] Add lvgl kconfig script (run_lvgl_kconfig.py) and custom_lvgl_kconfig_save_settings, custom_lvgl_kconfig_output_header,  custom_lvgl_kconfig_include_headers and custom_lvgl_kconfig_include_headers configuration sections to each relevant environment in platformio.ini
[ ] Add lvgl esp32 drivers kconfig script (run_lvgl_esp32_drivers_kconfig.py) and custom_lvgl_esp32_drivers_kconfig_save_settings, custom_lvgl_esp32_drivers_kconfig_output_header configuration section to each relevant environment in platformio.ini
[ ] Add specific include folder in build_flags for each environment in platformio.ini (-I include/esp32 and -I include/native)
[ ] Add the runner library from this project, it simplifies cross execution environments.
[ ] Add SDL2 to lib, see instructions below

    [ ] Copy library.json from this project to your SDL library

[ ] Add script run_lvgl_native_drivers_kconfig.py with custom_lvgl_native_drivers_kconfig_save_settings and custom_lvgl_native_drivers_kconfig_output_header configuration keys
[ ] Add build_flags to relevant environments in platformio.ini so it reflect there is a custom configuration file in include folder:
```
    -DLV_LVGL_H_INCLUDE_SIMPLE
    -DLV_CONF_INCLUDE_SIMPLE
    -DLV_CONF_PATH=lv_conf.h
```
[ ] copy lvgl_hal from this project to your lib folder, this library abstracts lvgl on esp32 and desktop, you may need to modify it to your environment / programming style.
[ ] configure esp32 menuconfig to enable PSRAM and increase its speed to 80Mhz (see sdkconfig.esp32)
[ ] run ```pio run -e esp32 -t lvgl-esp32-drivers-config``` to configure your hardware.
[ ] run ```pio run -e native -t lvgl-native-drivers-config``` to configure your desktop.
[ ] run ```pio run -e esp32 -t lvgl-config``` to configre lvgl for your hardware
[ ] run ```pio run -e native -t lvgl-config``` to configure lvgl for your desktop (can be different but really should be the same or very similar)



# Configuration
to see which targets and configurations are configured:
```
pio run --list-targets
```


to configure lvgl for each environment, for esp32:
```
pio run -e esp32 -t lvgl-config
```

and for native
```
pio run -e native -t lvgl-config
```

to configure lvgl esp32 drivers:
```
pio run -e esp32 -t lvgl-esp32-drivers-config
```

to configure lvgl native driver (SDL2):
```
pio run -e native -t lvgl-native-drivers-config
```

# Installing SDL2
SDL2 is required to display LVGL on the Desktop, SDL2 has 3 parts:
- include files needed for compilation
- the library files needed for linking
- the dlls (for Windows) and shared objects (.so for linux) for running the application
    the script in extraScripts copies the DLLS to your project output

For easy integration, copy the library.json from this project to your project SDL2 under lib, note the includeDir, flags and extraScripts and adapt to your needs.
```json
"build": {
    "includeDir": "./i686-w64-mingw32/include/",
    "flags":"-L./i686-w64-mingw32/lib/ -lSDL2 -lSDL2main",
    "extraScript":"./scripts/copy_sdl2_dll_x86.py"
},
```
## Linux
https://wiki.libsdl.org/Installation

```bash
git clone https://github.com/libsdl-org/SDL
cd SDL
mkdir build
cd build
../configure
make
sudo make install
```
## Ubuntu/Debian
sudo apt-get install libsdl2-2.0 libsdl2-dev
## Fedora
sudo yum install SDL2  SDL2-devel
## Windows
https://www.libsdl.org/download-2.0.php
Download https://www.libsdl.org/release/SDL2-devel-2.0.22-mingw.zip and install it in the lib directory
