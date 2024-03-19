#ifndef COMPONENTS_UI_DRIVER_ENCODER_H_
#define COMPONENTS_UI_DRIVER_ENCODER_H_


#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "ui.h"
#include "lvgl.h"
#include "rotary_encoder.h"
#include "driver/gpio.h"
#include "esp_log.h"

/*********************
 *      DEFINES
 *********************/

#define ROT_ENC_A_GPIO 26
#define ROT_ENC_B_GPIO 27
#define ROT_ENC_SW_GPIO 25

#define ENABLE_HALF_STEPS false // Set to true to enable tracking of rotary encoder at half step resolution
#define RESET_AT 0              // Set to a positive non-zero number to reset the position if this value is exceeded
#define FLIP_DIRECTION false    // Set to true to reverse the clockwise/counterclockwise sense
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize the encoder
 */
void encoder_init(void);

/**
 * Get encoder (i.e. mouse wheel) ticks difference and pressed state
 * @param indev_drv pointer to the related input device driver
 * @param data store the read data here
 * @return false: all ticks and button state are handled
 */
bool encoder_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);

/**
 * It is called periodically from the SDL thread to check a key is pressed/released
 * @param event describes the event
 */
void encoderReadTask(void *pvParameters);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* COMPONENTS_UI_DRIVER_ENCODER_H_ */
