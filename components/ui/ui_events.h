#ifndef _UI_EVENTS_H
#define _UI_EVENTS_H


#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "ui.h"


void focus_cb(lv_group_t *g);
void tv_event_cb(lv_obj_t *ta, lv_event_t e);
void ls_event_cb(lv_obj_t *ls, lv_event_t e);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
