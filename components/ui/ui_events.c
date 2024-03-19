#include "ui.h"
#include "ui_events.h"

#define TAG "UI_Events"

void focus_cb(lv_group_t *group)
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
        }
    }
}

void tv_event_cb(lv_obj_t *ta, lv_event_t e)
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

void ls_event_cb(lv_obj_t *ls, lv_event_t e)
{
	uint8_t index = 0;
	if(e == LV_EVENT_CLICKED)
	{
		index = lv_list_get_btn_index(ls,lv_list_get_btn_selected(ls));
		ESP_LOGI(TAG,"El boton nro %d fue presionado",index);
	}

}



