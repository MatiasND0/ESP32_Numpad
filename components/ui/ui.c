#include <stdio.h>
#include "ui.h"
#include "ui_events.h"

/**********************
 *  STATIC VARIABLES
 **********************/
lv_group_t *g;
lv_obj_t *tv;
lv_obj_t *t1;
lv_obj_t *t2;

/**********************
 *  GLOBAL FUNCTIONS
 **********************/

void create_ui(void)
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

/**********************
 *  STATIC FUNCTIONS
 **********************/

void selectors_create(lv_obj_t *parent)
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

void calculator_create(lv_obj_t *parent)
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

