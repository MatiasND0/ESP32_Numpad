/**
 * @file mousewheel.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "encoder.h"

/*********************
 *      DEFINES
 *********************/
#define TAG "INDEV_ENCODER"

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static int16_t enc_diff = 0;
static lv_indev_state_t state = LV_INDEV_STATE_REL;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the mousewheel
 */
void encoder_init(void)
{
	gpio_set_direction(25, GPIO_MODE_INPUT);
	gpio_pullup_en(25);
    xTaskCreate(encoderReadTask, "encoderRead", 4096, NULL, 5, NULL);
}

/**
 * Get encoder (i.e. mouse wheel) ticks difference and pressed state
 * @param indev_drv pointer to the related input device driver
 * @param data store the read data here
 * @return false: all ticks and button state are handled
 */
bool encoder_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
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

/**
 * It is called periodically from the SDL thread to check mouse wheel state
 * @param event describes the event
 */
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
