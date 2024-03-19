#ifndef UI_H
#define UI_H

#include "lvgl.h"
#include "esp_log.h"
#include "ui_events.h"
#include "encoder.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CALC_STACK_MAX 9
//#define TAG "UI"


extern lv_group_t *g;
extern lv_obj_t *tv;
extern lv_obj_t *t1;
extern lv_obj_t *t2;

struct
{
    lv_obj_t *list;
} selector_objs;

struct
{
	lv_obj_t *label;
}textinput_objs;

void create_ui(void);

void selectors_create(lv_obj_t *parent);
void calculator_create(lv_obj_t *parent);

#ifdef __cplusplus
}
#endif

#endif  // UI_H
