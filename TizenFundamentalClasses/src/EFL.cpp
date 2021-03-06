/*
 * Tizen Fundamental Classes - TFC
 * Copyright (c) 2016-2017 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *    EFL.cpp
 *
 * Created on:  Oct 27, 2016
 * Author: 		Gilang Mentari Hamidy (g.hamidy@samsung.com)
 * Contributor: Kevin Winata (k.winata@samsung.com)
 * 				Ida Bagus Putu Peradnya Dinata (ib.putu@samsung.com)
 */

#include "TFC/EFL.h"

LIBAPI
TFC::EFL::EvasObjectEventObject::EvasObjectEventObject() :
	eventType(EVAS_CALLBACK_DEL), boundObject(nullptr)
{
}

LIBAPI
TFC::EFL::EvasObjectEventObject::~EvasObjectEventObject()
{
	Unbind();
}

LIBAPI
void TFC::EFL::EvasObjectEventObject::Bind(Evas_Object* obj, Evas_Callback_Type eventType)
{
	if(this->boundObject != nullptr) {
			throw EventBoundException();
	}

	evas_object_event_callback_add(obj, eventType, Callback, this);
	evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL, Finalize, this);

	this->boundObject = obj;
	this->eventType = eventType;
}

LIBAPI
void TFC::EFL::EvasObjectEventObject::Unbind()
{
	if(this->boundObject == nullptr)
		return;

	evas_object_event_callback_del_full(boundObject, eventType, Callback, this);
	evas_object_event_callback_del_full(boundObject, EVAS_CALLBACK_DEL, Finalize, this);

	this->boundObject = nullptr;



}

LIBAPI
void TFC::EFL::EvasObjectEventObject::Callback(void* data, Evas* es,Evas_Object* obj, void* eventInfo)
{
	auto d = reinterpret_cast<EvasObjectEventObject*>(data);
	(*d)({es, obj}, eventInfo);
}

LIBAPI
void TFC::EFL::EvasObjectEventObject::Finalize(void* data, Evas* e, Evas_Object* obj, void* event_info)
{
	auto d = reinterpret_cast<EvasObjectEventObject*>(data);
	d->boundObject = nullptr;
}

LIBAPI
TFC::EFL::EdjeSignalEventObject::EdjeSignalEventObject() :
	boundObject(nullptr), emission(nullptr), source(nullptr)
{
}

LIBAPI
TFC::EFL::EdjeSignalEventObject::~EdjeSignalEventObject()
{
	Unbind();
}

LIBAPI
void TFC::EFL::EdjeSignalEventObject::Bind(Evas_Object* obj, const char* emission, const char* source)
{
	if(this->boundObject != nullptr)
		throw EventBoundException();

	edje_object_signal_callback_add(obj, emission, source, Callback, this);
	evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL, Finalize, this);

	this->boundObject = obj;
	this->emission = emission;
	this->source = source;
}

LIBAPI
void TFC::EFL::EdjeSignalEventObject::BindLayout(Elm_Layout* obj, const char* emission, const char* source)
{
	if(this->boundObject != nullptr)
		throw EventBoundException();

	elm_layout_signal_callback_add(obj, emission, source, Callback, this);
	evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL, Finalize, this);

	this->boundObject = obj;
	this->emission = emission;
	this->source = source;
}

LIBAPI
void TFC::EFL::EdjeSignalEventObject::Unbind()
{
	if(this->boundObject == nullptr)
		return;


	edje_object_signal_callback_del_full(boundObject, emission, source, Callback, this);
	evas_object_event_callback_del_full(boundObject, EVAS_CALLBACK_DEL, Finalize, this);

	this->boundObject = nullptr;
	this->emission = nullptr;
	this->source = nullptr;
}

LIBAPI
void TFC::EFL::EdjeSignalEventObject::Callback(void* data, Evas_Object* obj, const char* emission, const char* source)
{
	(*reinterpret_cast<EdjeSignalEventObject*>(data))(obj, {emission, source});
}

LIBAPI
void TFC::EFL::EdjeSignalEventObject::Finalize(void* data, Evas* es, Evas_Object* obj, void* eventInfo)
{
	auto thiz = reinterpret_cast<EdjeSignalEventObject*>(data);

	thiz->boundObject = nullptr;
	thiz->emission = nullptr;
	thiz->source = nullptr;
}

LIBAPI
TFC::EFL::ObjectItemEdjeSignalEventObject::ObjectItemEdjeSignalEventObject() :
	boundObject(nullptr), emission(nullptr), source(nullptr)
{
}

LIBAPI
TFC::EFL::ObjectItemEdjeSignalEventObject::~ObjectItemEdjeSignalEventObject()
{
	Unbind();
}

LIBAPI
void TFC::EFL::ObjectItemEdjeSignalEventObject::Bind(Elm_Object_Item* objIt, const char* emission, const char* source)
{
	if(this->boundObject != nullptr)
		throw EventBoundException();

	elm_object_item_signal_callback_add(objIt, emission, source, Callback, this);

	// TODO Check if it is correct to call evas_object_event_callback_* on Elm_Object_Item*
	// THIS MIGHT NOT WORKING
	evas_object_event_callback_add(objIt, EVAS_CALLBACK_DEL, Finalize, this);

	this->boundObject = objIt;
	this->emission = emission;
	this->source = source;
}

LIBAPI
void TFC::EFL::ObjectItemEdjeSignalEventObject::Unbind()
{
	if(this->boundObject == nullptr)
			return;

	elm_object_item_signal_callback_del(boundObject, emission, source, Callback);

	// TODO Check if it is correct to call evas_object_event_callback_* on Elm_Object_Item*
	// THIS MIGHT NOT WORKING
	evas_object_event_callback_del_full(boundObject, EVAS_CALLBACK_DEL, Finalize, this);

	this->boundObject = nullptr;
	this->emission = nullptr;
	this->source = nullptr;
}

LIBAPI
void TFC::EFL::ObjectItemEdjeSignalEventObject::Callback(void* data, Elm_Object_Item* it, const char* emission, const char* source)
{
	(*reinterpret_cast<ObjectItemEdjeSignalEventObject*>(data))(it, {emission, source});
}

LIBAPI
void TFC::EFL::ObjectItemEdjeSignalEventObject::Finalize(void* data, Evas* es, Evas_Object* obj, void* eventInfo)
{
	auto thiz = reinterpret_cast<ObjectItemEdjeSignalEventObject*>(data);

	thiz->boundObject = nullptr;
	thiz->emission = nullptr;
	thiz->source = nullptr;
}
