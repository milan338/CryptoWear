#ifndef POPUPVIEW_H
#define POPUPVIEW_H

#include "main.h"

Elm_Object_Item *show_popup_view_ok(appdata_s *ad, char *title, char *text, void *cb, void *cb_data);
void show_popup_view_yesno(appdata_s *ad, char *title, char *text, void *cb, void *ok_cb_data, void *cancel_cb_data);

#endif // POPUPVIEW_H
