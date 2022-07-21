#ifdef ESP_PLATFORM

#include "app_hal.h"
#include <lvgl.h>

#include <lvgl_helpers.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include <lvgl_helpers.h>

#define LV_TICK_PERIOD_MS 1

static void lv_tick_task(void *arg)
{
    lv_tick_inc(LV_TICK_PERIOD_MS);
}

void hal_setup(void)
{
    lvgl_driver_init();

    static lv_disp_draw_buf_t disp_buf;

    lv_color_t *buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DEFAULT);
    assert(buf1);
    lv_color_t *buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DEFAULT);
    assert(buf2);

    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, DISP_BUF_SIZE); /*Initialize the display buffer*/

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);           /*Basic initialization*/
    disp_drv.flush_cb = disp_driver_flush; /*Used when `LV_VDB_SIZE != 0` in lv_conf.h (buffered drawing)*/
    disp_drv.draw_buf = &disp_buf;
    disp_drv.hor_res = 480;
    disp_drv.ver_res = 320;
    lv_disp_drv_register(&disp_drv);

#if CONFIG_LV_TOUCH_CONTROLLER != TOUCH_CONTROLLER_NONE
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = touch_driver_read;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);
#endif

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"};

    static esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));
}

void hal_loop(void)
{
    while (1)
    {
        uint32_t next = lv_task_handler();
        vTaskDelay(pdMS_TO_TICKS(next));
        esp_task_wdt_reset();
    }
}

#endif