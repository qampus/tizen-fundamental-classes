/*
 * ApplicationBase.cpp
 *
 * Source file for class ApplicationBase
 *
 *  Created on: Feb 12, 2016
 *      Author: gilang
 */

#include "SRIN/Framework/Application.h"

using namespace SRIN::Framework;



bool ApplicationBase_AppCreateHandler(void *data)
{
	ApplicationBase* app = static_cast<ApplicationBase*>(data);
	return app->ApplicationCreate();
}

void ApplicationBase_AppControlHandler(app_control_h app_control, void* data)
{
	ApplicationBase* app = static_cast<ApplicationBase*>(data);
	app->ApplicationControl(app_control);
}

void ApplicationBase_AppPauseHandler(void* data)
{
	ApplicationBase* app = static_cast<ApplicationBase*>(data);
	app->ApplicationPause();
}

void ApplicationBase_AppResumeHandler(void* data)
{
	ApplicationBase* app = static_cast<ApplicationBase*>(data);
	app->ApplicationResume();
}

void ApplicationBase_AppTerminateHandler(void* data)
{
	ApplicationBase* app = static_cast<ApplicationBase*>(data);
	app->ApplicationTerminate();
}

void ui_app_lang_changed(app_event_info_h event_info, void* data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);

	ApplicationBase* app = static_cast<ApplicationBase*>(data);
	app->LanguageChanged(event_info, locale);

	free(locale);
	return;
}

void ui_app_orient_changed(app_event_info_h event_info, void *data)
{
	ApplicationBase* app = static_cast<ApplicationBase*>(data);
	app->OrientationChanged(event_info);
}

void ui_app_region_changed(app_event_info_h event_info, void *data)
{
	ApplicationBase* app = static_cast<ApplicationBase*>(data);
	app->RegionChanged(event_info);
}

void ui_app_low_battery(app_event_info_h event_info, void *data)
{
	ApplicationBase* app = static_cast<ApplicationBase*>(data);
	app->LowBattery(event_info);
}

void ui_app_low_memory(app_event_info_h event_info, void *data)
{
	ApplicationBase* app = static_cast<ApplicationBase*>(data);
	app->LowMemory(event_info);
}


int ApplicationBase::Main(ApplicationBase* app, int argc, char* argv[])
{

	::ui_app_lifecycle_callback_s event_callback ={
			ApplicationBase_AppCreateHandler,
			ApplicationBase_AppTerminateHandler,
			ApplicationBase_AppPauseHandler,
			ApplicationBase_AppResumeHandler,
			ApplicationBase_AppControlHandler };

	::app_event_handler_h handlers[5] =
	{ NULL, };

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, app);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, app);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, app);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, app);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, app);

	int ret = ::ui_app_main(argc, argv, &event_callback, app);
	if (ret != APP_ERROR_NONE)
	{
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	return ret;
}


ApplicationBase::ApplicationBase(CString packageName) :
		packageName(packageName)
{
	this->rootFrame = this->win = this->conform = NULL;
	this->backButtonCallback = nullptr;
	this->backButtonInstance = nullptr;
	this->haveEventBackPressed = false;
}


void ApplicationBase::ApplicationControl(app_control_h app_control) { }

void ApplicationBase::ApplicationPause() { }

void ApplicationBase::ApplicationResume() { }

void ApplicationBase::ApplicationTerminate() { }

void ApplicationBase::LanguageChanged(app_event_info_h event_info, const char* locale) { }

void ApplicationBase::OrientationChanged(app_event_info_h event_info) { }

void ApplicationBase::RegionChanged(app_event_info_h event_info) { }

void ApplicationBase::LowBattery(app_event_info_h event_info) { }

void ApplicationBase::LowMemory(app_event_info_h event_info) { }

bool ApplicationBase::OnBackButtonPressed()
{
	return true;
}

void SRIN::Framework::ApplicationBase::EnableBackButtonCallback(bool enable)
{
}

bool SRIN::Framework::ApplicationBase::AcquireExclusiveBackButtonPressed(EventClass* instance, BackButtonCallback callback)
{
}

bool SRIN::Framework::ApplicationBase::ReleaseExclusiveBackButtonPressed(EventClass* instance, BackButtonCallback callback)
{
}

void SRIN::Framework::ApplicationBase::Attach(ViewBase* view)
{
	Evas_Object* viewComponent = view->Create(this->rootFrame);

	//show to window
	if(viewComponent != NULL)
	{
		auto naviframeItem = elm_naviframe_item_push(this->rootFrame, view->viewName, NULL, NULL, viewComponent, view->GetStyle());
		auto backButton = elm_object_item_part_content_get(naviframeItem, "elm.swallow.prev_btn");
		auto style = elm_object_style_get(backButton);

		// Title button handling
		auto titleButton = dynamic_cast<ITitleButton*>(view);
		if(titleButton)
		{
			auto left = titleButton->GetTitleLeftButton();
			if(left)
			{
				auto oldObj = elm_object_item_part_content_unset(naviframeItem, "title_left_btn");
				evas_object_hide(oldObj);
				elm_object_item_part_content_set(naviframeItem, "title_left_btn", left);
				evas_object_show(left);
			}

			auto right = titleButton->GetTitleRightButton();
			if(right)
			{
				auto oldObj = elm_object_item_part_content_unset(naviframeItem, "title_right_btn");
				evas_object_hide(oldObj);
				elm_object_item_part_content_set(naviframeItem, "title_right_btn", right);
				evas_object_show(right);
			}
		}


		dlog_print(DLOG_DEBUG, LOG_TAG, "Prev Button Style: %s", style);

		evas_object_smart_callback_add(backButton, "clicked", [] (void* a, Evas_Object* b, void* c) { static_cast<ApplicationBase*>(a)->BackButtonPressed(); }, this);

		evas_object_show(viewComponent);
	}
}

void SRIN::Framework::ApplicationBase::Detach()
{
	elm_naviframe_item_pop(this->rootFrame);
}

void SRIN::Framework::ApplicationBase::Exit()
{
}

void SRIN::Framework::ApplicationBase::BackButtonPressed()
{
}

bool SRIN::Framework::ApplicationBase::ApplicationCreate()
{
	elm_config_accel_preference_set("3d");

	// Window
	// Create and initialize elm_win.
	// elm_win is mandatory to manipulate window.
	this->win = elm_win_util_standard_add(packageName, packageName);

	elm_win_autodel_set(this->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(this->win))
	{
		int rots[1] =
		{ 0 };
		elm_win_wm_rotation_available_rotations_set(this->win, (const int *) (&rots), 1);
	}

	//eext_object_event_callback_add(this->win, EEXT_CALLBACK_BACK, win_delete_request_cb, (void*)this);

	//evas_object_smart_callback_add(this->win, "delete,request", win_delete_request_cb, NULL);
	//eext_object_event_callback_add(this->win, EEXT_CALLBACK_BACK, win_delete_request_cb, this);
	EnableBackButtonCallback(true);
	// Conformant
	// Create and initialize elm_conformant.
	// elm_conformant is mandatory for base gui to have proper size
	// when indicator or virtual keypad is visible.
	this->conform = elm_conformant_add(this->win);
	elm_win_indicator_mode_set(this->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(this->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(this->conform, EVAS_HINT_EXPAND,
	EVAS_HINT_EXPAND);
	elm_win_resize_object_add(this->win, this->conform);
	evas_object_show(this->conform);

	// Naviframe
	this->rootFrame = elm_naviframe_add(this->conform);

	elm_naviframe_prev_btn_auto_pushed_set(this->rootFrame, EINA_TRUE);
	elm_naviframe_content_preserve_on_pop_set(this->rootFrame, EINA_FALSE);
	elm_object_content_set(this->conform, this->rootFrame);
	evas_object_show(this->rootFrame);

	evas_object_show(this->win);

	this->OnApplicationCreated();
	return true;
}

void SRIN::Framework::ApplicationBase::OnApplicationCreated()
{

}

ApplicationBase::~ApplicationBase()
{
}
