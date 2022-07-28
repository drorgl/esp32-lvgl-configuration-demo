/**
 * @file ili9488.c
 */

/*********************
 *      INCLUDES
 *********************/
#include "ili9488.h"
#include "disp_spi.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_heap_caps.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*********************
 *      DEFINES
 *********************/
 #define TAG "ILI9488"

/**********************
 *      TYPEDEFS
 **********************/

/*The LCD needs a bunch of command/argument values to be initialized. They are stored in this struct. */
typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void ili9488_set_orientation(uint8_t orientation);

static void ili9488_send_cmd(uint8_t cmd);
static void ili9488_send_data(void * data, uint16_t length);
static void ili9488_send_color(void * data, uint16_t length);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
// From github.com/jeremyjh/ESP32_TFT_library
// From github.com/mvturnho/ILI9488-lvgl-ESP32-WROVER-B
void ili9488_init(void)
{
	lcd_init_cmd_t ili_init_cmds[]={
		{ILI9488_CMD_SOFTWARE_RESET, {0x01}, 0x80},
                {ILI9488_CMD_SLEEP_OUT, {0x00}, 0x80},
		{ILI9488_CMD_POWER_CONTROL_1, {0x01, 0x01}, 2}, //brightness (low/high 0x00 - 0x1F)
		{ILI9488_CMD_POWER_CONTROL_2, {0x41}, 1}, //brightness multiplier 0x40 - 0x47
		{ILI9488_CMD_POWER_CONTROL_NORMAL_3, {0x00},1},
		// {ILI9488_CMD_VCOM_CONTROL_1, {0x00, 0x12, 0x80}, 3},
		{ILI9488_CMD_VCOM_CONTROL_1, {0x00, 0x18, 0x80}, 3}, //brightness (2nd byte)
		{ILI9488_CMD_MEMORY_ACCESS_CONTROL, {(0x20 | 0x08)}, 1},
		{ILI9488_CMD_COLMOD_PIXEL_FORMAT_SET, {0b0110110}, 1},//24bits //24bits
		{ILI9488_CMD_INTERFACE_MODE_CONTROL, {0x00}, 1},
		{ILI9488_CMD_FRAME_RATE_CONTROL_NORMAL, {0xA0}, 1},
		{ILI9488_CMD_DISPLAY_INVERSION_CONTROL, {0x02}, 1},
		{ILI9488_CMD_DISPLAY_FUNCTION_CONTROL, {0x02, 0x02}, 2},
		{ILI9488_CMD_SET_IMAGE_FUNCTION, {0x00}, 1},
		{ILI9488_CMD_WRITE_CTRL_DISPLAY, {0x28}, 1},
		{ILI9488_CMD_WRITE_CONTENT_ADAPT_BRIGHTNESS, {0x00},1},
		{ILI9488_CMD_WRITE_DISPLAY_BRIGHTNESS, {0x0F}, 1},
		// {ILI9488_CMD_WRITE_DISPLAY_BRIGHTNESS, {0xFF}, 1},
		{ILI9488_CMD_ADJUST_CONTROL_3, {0xA9, 0x51, 0x2C, 0x02}, 4},
		{ILI9488_CMD_DISPLAY_ON, {0x00}, 0x80},
		{0, {0}, 0xff},
	};

	//Initialize non-SPI GPIOs
    gpio_pad_select_gpio(ILI9488_DC);
	gpio_set_direction(ILI9488_DC, GPIO_MODE_OUTPUT);

#if ILI9488_USE_RST
    gpio_pad_select_gpio(ILI9488_RST);
	gpio_set_direction(ILI9488_RST, GPIO_MODE_OUTPUT);

	//Reset the display
	gpio_set_level(ILI9488_RST, 0);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(ILI9488_RST, 1);
	vTaskDelay(100 / portTICK_RATE_MS);
#endif

	ESP_LOGI(TAG, "ILI9488 initialization.");

	// Exit sleep
	ili9488_send_cmd(0x01);	/* Software reset */
	vTaskDelay(100 / portTICK_RATE_MS);

	//Send all the commands
	uint16_t cmd = 0;
	while (ili_init_cmds[cmd].databytes!=0xff) {
		ili9488_send_cmd(ili_init_cmds[cmd].cmd);
		ili9488_send_data(ili_init_cmds[cmd].data, ili_init_cmds[cmd].databytes&0x1F);
		if (ili_init_cmds[cmd].databytes & 0x80) {
			vTaskDelay(100 / portTICK_RATE_MS);
		}else{
			vTaskDelay(5 / portTICK_RATE_MS);
		}
		cmd++;
	}

    ili9488_set_orientation(CONFIG_LV_DISPLAY_ORIENTATION);
}

uint8_t convert_8_to_6_bits(uint8_t val){
		return ((uint32_t)val * 253 + 512) >> 10;
}

// Flush function based on mvturnho repo
void ili9488_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_map)
{
    uint32_t size = lv_area_get_width(area) * lv_area_get_height(area);

#if LV_COLOR_DEPTH == 16
    lv_color16_t *buffer_16bit = (lv_color16_t *) color_map;
#elif  LV_COLOR_DEPTH == 32
	lv_color32_t *buffer_32bit = (lv_color32_t *) color_map;
#endif
    uint8_t *mybuf;
    do {
        mybuf = (uint8_t *) heap_caps_malloc(3 * size * sizeof(uint8_t) + 3, MALLOC_CAP_DMA);
        if (mybuf == NULL)  ESP_LOGW(TAG, "Could not allocate enough DMA memory!");
    } while (mybuf == NULL);

    uint32_t LD = 0;
    uint32_t j = 0;

    for (uint32_t i = 0; i < size; i++) {

#if LV_COLOR_DEPTH == 16
		mybuf[j] = buffer_16bit[i].ch.red << 1;
		j++;
		mybuf[j] = buffer_16bit[i].ch.green << 0;
		j++;
		mybuf[j] = buffer_16bit[i].ch.blue << 1;
		j++;

#elif  LV_COLOR_DEPTH == 32
		mybuf[j] = convert_8_to_6_bits(buffer_32bit[i].ch.red);
		j++;
		mybuf[j] = convert_8_to_6_bits(buffer_32bit[i].ch.green);
		j++;
		mybuf[j] = convert_8_to_6_bits(buffer_32bit[i].ch.blue);
		j++;
#endif

    }

	/* Column addresses  */
	uint8_t xb[] = {
	    (uint8_t) (area->x1 >> 8) & 0xFF,
	    (uint8_t) (area->x1) & 0xFF,
	    (uint8_t) (area->x2 >> 8) & 0xFF,
	    (uint8_t) (area->x2) & 0xFF,
	};

	/* Page addresses  */
	uint8_t yb[] = {
	    (uint8_t) (area->y1 >> 8) & 0xFF,
	    (uint8_t) (area->y1) & 0xFF,
	    (uint8_t) (area->y2 >> 8) & 0xFF,
	    (uint8_t) (area->y2) & 0xFF,
	};

	/*Column addresses*/
	ili9488_send_cmd(ILI9488_CMD_COLUMN_ADDRESS_SET);
	ili9488_send_data(xb, 4);

	/*Page addresses*/
	ili9488_send_cmd(ILI9488_CMD_PAGE_ADDRESS_SET);
	ili9488_send_data(yb, 4);

	/*Memory write*/
	ili9488_send_cmd(ILI9488_CMD_MEMORY_WRITE);

	ili9488_send_color((void *) mybuf, size * 3);
	heap_caps_free(mybuf);

	lv_disp_flush_ready(drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/


static void ili9488_send_cmd(uint8_t cmd)
{
    disp_wait_for_pending_transactions();
    gpio_set_level(ILI9488_DC, 0);	 /*Command mode*/
    disp_spi_send_data(&cmd, 1);
}

static void ili9488_send_data(void * data, uint16_t length)
{
    disp_wait_for_pending_transactions();
    gpio_set_level(ILI9488_DC, 1);	 /*Data mode*/
    disp_spi_send_data(data, length);
}

static void ili9488_send_color(void * data, uint16_t length)
{
    disp_wait_for_pending_transactions();
    gpio_set_level(ILI9488_DC, 1);   /*Data mode*/
    disp_spi_send_colors(data, length);
}

static void ili9488_set_orientation(uint8_t orientation)
{
    // ESP_ASSERT(orientation < 4);

    const char *orientation_str[] = {
        "PORTRAIT", "PORTRAIT_INVERTED", "LANDSCAPE", "LANDSCAPE_INVERTED"
    };

    ESP_LOGI(TAG, "Display orientation: %s", orientation_str[orientation]);

#if defined (CONFIG_LV_PREDEFINED_DISPLAY_NONE)
    uint8_t data[] = {0x48, 0x88, 0x28, 0xE8};
#endif

    ESP_LOGI(TAG, "0x36 command value: 0x%02X", data[orientation]);

    ili9488_send_cmd(0x36);
    ili9488_send_data((void *) &data[orientation], 1);
}
