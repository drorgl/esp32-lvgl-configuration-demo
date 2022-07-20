# ESP32 LVGL Workflow Demo

This is a demo and an example how to work with LVGL.

# Setup
[ ] git init
[ ] pio init
[ ] Extract latest release from https://github.com/lvgl/lvgl/releases into lib/lvgl
[ ] Extract latest source from https://github.com/lvgl/lv_drivers into lib/lv_drivers
[ ] Extract latest source from https://github.com/lvgl/lvgl_esp32_drivers into lib/lvgl_esp32_drivers
[ ] Add lvgl kconfig script (run_lvgl_kconfig.py) and custom_lvgl_kconfig_save_settings, custom_lvgl_kconfig_output_header configuration sections to each relevant environment in platformio.ini 
[ ] Add lvgl esp32 drivers kconfig script (run_lvgl_esp32_drivers_kconfig.py) and custom_lvgl_esp32_drivers_kconfig_save_settings, custom_lvgl_esp32_drivers_kconfig_output_header configuration section to each relevant environment in platformio.ini
[ ] Add specific include folder in build_flags for each environment in platformio.ini (-I include/esp32 and -I include/native)
[ ] Add the runner library from this project, it simplifies cross execution environments.


# Configuration
to see which targets and configurations are configured:
```
pio run --list-targets
```


to configure lvgl for each environment, for esp32:
```
pio run -e esp32 -t lvgl-kconfig
```

and for native
```
pio run -e native -t lvgl-kconfig
```

to configure lvgl esp32 drivers:
```
pio run -e esp32 -t lvgl-esp32-drivers-kconfig
```
