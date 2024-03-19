/* LVGL Example project
 *
 * Basic project to test LVGL on ESP32 based projects.
 *
 * This example code is in the Public Domain (or CC0 licensed, at your option.)
 *
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "rotary_encoder.h"

#include "lvgl.h"
#include "lvgl_helpers.h"
#define TAG "demo"

/*********************
 *      DEFINES
 *********************/
#define CALC_STACK_MAX 9

#define LV_TICK_PERIOD_MS 1

#define ROT_ENC_A_GPIO (CONFIG_ROT_ENC_A_GPIO)
#define ROT_ENC_B_GPIO (CONFIG_ROT_ENC_B_GPIO)
#define ROT_ENC_SW_GPIO 25

#define ENABLE_HALF_STEPS false // Set to true to enable tracking of rotary encoder at half step resolution
#define RESET_AT 0              // Set to a positive non-zero number to reset the position if this value is exceeded
#define FLIP_DIRECTION false    // Set to true to reverse the clockwise/counterclockwise sense

/**********************
 *  STATIC VARIABLES
 **********************/

static lv_group_t *g;
static lv_obj_t *tv;
static lv_obj_t *t1;
static lv_obj_t *t2;
static lv_obj_t *t3;

struct
{
    lv_obj_t *list;
} selector_objs;

struct 
{
	lv_obj_t *label;
}textinput_objs;

static int16_t enc_diff = 0;
static lv_indev_state_t state = LV_INDEV_STATE_REL;
/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_tick_task(void *arg);
static void guiTask(void *pvParameter);

static void create_demo_application(void);
static void selectors_create(lv_obj_t *parent);
static void calculator_create(lv_obj_t *parent);

static void focus_cb(lv_group_t *g);
static void tv_event_cb(lv_obj_t *ta, lv_event_t e);
static void ls_event_cb(lv_obj_t *ls, lv_event_t e);

void encoderReadTask(void *pvParameters);
bool mousewheel_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);

LV_EVENT_CB_DECLARE(dd_enc)
{
    if (e == LV_EVENT_VALUE_CHANGED)
    {
        /*printf("chg\n");*/
    }
}
/**********************
 *   APPLICATION MAIN
 **********************/
void app_main()
{
    xTaskCreatePinnedToCore(guiTask, "gui", 4096 * 2, NULL, 0, NULL, 1);
    xTaskCreate(encoderReadTask, "encoderRead", 4096, NULL, 5, NULL);
    gpio_set_direction(25, GPIO_MODE_INPUT);
    gpio_pullup_en(25);
}

SemaphoreHandle_t xGuiSemaphore;

static void guiTask(void *pvParameter)
{

    (void)pvParameter;
    xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();

    lvgl_driver_init();

    lv_color_t *buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);
    lv_color_t *buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2 != NULL);

    static lv_disp_buf_t disp_buf;
    uint32_t size_in_px = DISP_BUF_SIZE;

    lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;

    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    lv_indev_drv_t enc_drv;
    lv_indev_drv_init(&enc_drv);
    enc_drv.type = LV_INDEV_TYPE_ENCODER;
    enc_drv.read_cb = mousewheel_read;
    lv_indev_drv_register(&enc_drv);

    /* Create and start a periodic timer interrupt to call lv_tick_inc */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"};
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    /* Create the demo application */
    // lv_demo_keypad_encoder();
    create_demo_application();

    while (1)
    {
        /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
        vTaskDelay(pdMS_TO_TICKS(10));

        /* Try to take the semaphore, call lvgl related function on success */
        if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY))
        {
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
        }
    }

    /* A task should NEVER return */
    free(buf1);
    free(buf2);
    vTaskDelete(NULL);
}

static void create_demo_application(void)
{
    g = lv_group_create();
    lv_group_set_focus_cb(g, focus_cb);

    lv_indev_t *cur_drv = NULL;
    for (;;)
    {
        cur_drv = lv_indev_get_next(cur_drv);
        if (!cur_drv)
        {
            break;
        }
        if (cur_drv->driver.type == LV_INDEV_TYPE_ENCODER)
        {
            lv_indev_set_group(cur_drv, g);
        }
    }

    tv = lv_tabview_create(lv_scr_act(), NULL);
    lv_obj_set_event_cb(tv, tv_event_cb);

    t1 = lv_tabview_add_tab(tv, "Selectors");
    t2 = lv_tabview_add_tab(tv, "Text input");

    lv_group_add_obj(g, tv);

    selectors_create(t1);
    calculator_create(t2);

    lv_obj_reset_style_list(lv_layer_top(), LV_OBJ_PART_MAIN);
    lv_obj_set_click(lv_layer_top(), false);
    lv_event_send(tv, LV_EVENT_REFRESH, NULL);
}

static void lv_tick_task(void *arg)
{
    (void)arg;

    lv_tick_inc(LV_TICK_PERIOD_MS);
}

//---------------------------------------------------------//
bool mousewheel_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    (void)indev_drv; /*Unused*/
    data->enc_diff = enc_diff;

    if (gpio_get_level(25) == 1 && state != LV_INDEV_STATE_REL)
        state = LV_INDEV_STATE_REL;
    else if (gpio_get_level(25) == 0 && state != LV_INDEV_STATE_PR)
        state = LV_INDEV_STATE_PR;

    if (state)
        data->state = LV_INDEV_STATE_PR;
    else
        data->state = LV_INDEV_STATE_REL;

    enc_diff = 0;
    return false; /*No buffering now so no more data read*/
}

void encoderReadTask(void *pvParameters)
{
    ESP_ERROR_CHECK(gpio_install_isr_service(0));

    rotary_encoder_info_t info = {0};
    ESP_ERROR_CHECK(rotary_encoder_init(&info, ROT_ENC_A_GPIO, ROT_ENC_B_GPIO));
    ESP_ERROR_CHECK(rotary_encoder_enable_half_steps(&info, ENABLE_HALF_STEPS));
#ifdef FLIP_DIRECTION
    ESP_ERROR_CHECK(rotary_encoder_flip_direction(&info));
#endif

    QueueHandle_t event_queue = rotary_encoder_create_queue();
    ESP_ERROR_CHECK(rotary_encoder_set_queue(&info, event_queue));

    while (1)
    {
        // Wait for incoming events on the event queue.
        rotary_encoder_event_t event = {0};
        if (xQueueReceive(event_queue, &event, 1000 / portTICK_PERIOD_MS) == pdTRUE)
        {
            switch (event.state.direction)
            {
            case ROTARY_ENCODER_DIRECTION_CLOCKWISE:
                enc_diff = 1;
                break;
            case ROTARY_ENCODER_DIRECTION_COUNTER_CLOCKWISE:
                enc_diff = -1;
                break;
            default:
                break;
            }
        }
        else
        {
            // Poll current position and direction
            rotary_encoder_state_t state = {0};
            ESP_ERROR_CHECK(rotary_encoder_get_state(&info, &state));
            // Reset the device
            if (RESET_AT && (state.position >= RESET_AT || state.position <= -RESET_AT))
            {
                ESP_LOGI(TAG, "Reset");
                ESP_ERROR_CHECK(rotary_encoder_reset(&info));
            }
        }
    }
    ESP_LOGE(TAG, "queue receive failed");

    ESP_ERROR_CHECK(rotary_encoder_uninit(&info));
}

/**********************
 *   STATIC FUNCTIONS
 **********************/


static void focus_cb(lv_group_t *group)
{
    lv_obj_t *obj = lv_group_get_focused(group);
    if (obj != tv)
    {
        uint16_t tab = lv_tabview_get_tab_act(tv);
        switch (tab)
        {
        case 0:
            lv_page_focus(t1, obj, LV_ANIM_ON);
            break;
        case 1:
            lv_page_focus(t2, obj, LV_ANIM_ON);
            break;
        case 2:
            lv_page_focus(t3, obj, LV_ANIM_ON);
            break;
        }
    }
}

static void tv_event_cb(lv_obj_t *ta, lv_event_t e)
{
    if (e == LV_EVENT_VALUE_CHANGED || e == LV_EVENT_REFRESH)
    {
        lv_group_remove_all_objs(g);

        uint16_t tab = lv_tabview_get_tab_act(tv);
        size_t size = 0;
        lv_obj_t **objs = NULL;
        if (tab == 0)
        {
            size = sizeof(selector_objs);
            objs = (lv_obj_t **)&selector_objs;
        }
        else if (tab == 1)
        {
            size = sizeof(textinput_objs);
            objs = (lv_obj_t **)&textinput_objs;
        }

        lv_group_add_obj(g, tv);

        uint32_t i;
        for (i = 0; i < size / sizeof(lv_obj_t *); i++)
        {
            if (objs[i] == NULL)
                continue;
            lv_group_add_obj(g, objs[i]);
        }
    }
}

static void ls_event_cb(lv_obj_t *ls, lv_event_t e)
{
	uint8_t index = 0;
	if(e == LV_EVENT_CLICKED)
	{
		index = lv_list_get_btn_index(ls,lv_list_get_btn_selected(ls));
		ESP_LOGI(TAG,"El boton nro %d fue presionado.\n",index);
	}

}

static void selectors_create(lv_obj_t *parent)
{
    selector_objs.list = lv_list_create(parent, NULL);
    lv_obj_set_size(selector_objs.list, 200, 180);
    lv_obj_align(selector_objs.list, NULL, LV_ALIGN_IN_TOP_MID, 0, 13);
    if (lv_obj_get_height(selector_objs.list) > lv_page_get_height_fit(parent))
    {
        lv_obj_set_height(selector_objs.list, lv_page_get_height_fit(parent));
    }
    lv_obj_t *bt;

    bt = lv_list_add_btn(selector_objs.list, LV_SYMBOL_KEYBOARD, "Profile 1");    lv_group_add_obj(g, bt);
    bt = lv_list_add_btn(selector_objs.list, LV_SYMBOL_KEYBOARD, "Profile 2");    lv_group_add_obj(g, bt);
    bt = lv_list_add_btn(selector_objs.list, LV_SYMBOL_KEYBOARD, "Profile 3");    lv_group_add_obj(g, bt);
    bt = lv_list_add_btn(selector_objs.list, LV_SYMBOL_KEYBOARD, "Profile 4");    lv_group_add_obj(g, bt);
    bt = lv_list_add_btn(selector_objs.list, LV_SYMBOL_KEYBOARD, "Profile 5");    lv_group_add_obj(g, bt);
    bt = lv_list_add_btn(selector_objs.list, LV_SYMBOL_KEYBOARD, "Profile 6");    lv_group_add_obj(g, bt);
    lv_obj_set_event_cb(selector_objs.list, ls_event_cb);
}

static void calculator_create(lv_obj_t *parent)
{
    char text_buff[50];
    for (int i = 0; i < CALC_STACK_MAX; i++)
    {
        textinput_objs.label = lv_label_create(parent, NULL);
        lv_obj_align(textinput_objs.label, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 10 + i * 20);
        sprintf(text_buff, "%d:", CALC_STACK_MAX - i);
        lv_label_set_text(textinput_objs.label, text_buff);
    }

    for (int i = 0; i < CALC_STACK_MAX; i++)
    {
        textinput_objs.label = lv_label_create(parent, NULL);
        lv_obj_align(textinput_objs.label, NULL, LV_ALIGN_IN_TOP_RIGHT, -20, 10 + i * 20);
        lv_label_set_text(textinput_objs.label, "-");
    }
}
