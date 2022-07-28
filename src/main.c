#include <runner.h>

#include <lv_conf.h>
#include <lvgl_hal.h>
#include <lvgl.h>

#include <stdio.h>

#include <demos/widgets/lv_demo_widgets.h>

MAIN()
{
    printf("starting...\r\n");

    /*Initialize LVGL*/
    lv_init();

    /*Initialize the HAL (display, input devices, tick) for LVGL*/
    hal_setup();


    lv_demo_widgets();

    hal_loop();

    return 0;
}