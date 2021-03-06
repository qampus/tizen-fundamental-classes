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
 *    Framework/Application.h
 *
 * Main header consisting the basic MVC pattern for TFC application, including
 * base classes for view, controller, and application.
 *
 * Created on: 	Feb 12, 2016
 * Author: 		Gilang Mentari Hamidy (g.hamidy@samsung.com)
 * Contributor: Ida Bagus Putu Peradnya Dinata (ib.putu@samsung.com)
 *              Kevin Winata (k.winata@samsung.com)
 */
#ifndef TFC_FW_APPLICATION_H__
#define TFC_FW_APPLICATION_H__

#define __STDBOOL_H // Remove STDBOOL
#define USE_TFC_CORE // MIGRATORY

#include <app.h>

#include <system_settings.h>
#include <efl_extension.h>
#include <deque>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include "TFC/Core.h"
#include "TFC/Core/Introspect.h"
#include "TFC/EFL.h"

/**
 * Macro to register a controller to a ControllerManager by providing a controller name and controller type
 *
 * @param CONTROLLER_NAME name of the controller to identify this controller
 * @param CONTROLLER_TYPE the class or typensme of the controller to register
 */

#define RegisterController(CONTROLLER_NAME, CONTROLLER_TYPE) \
	RegisterControllerFactory(new TFC::Framework::ControllerFactory(CONTROLLER_NAME, \
	[] (TFC::Framework::ControllerManager* m) -> \
	TFC::Framework::ControllerBase* { return new CONTROLLER_TYPE (m); }))


namespace TFC {
namespace Framework {

class ViewBase;
class ControllerManager;
class ApplicationBase;

typedef bool (EventClass::*BackButtonCallback)();

/**
 * Application base class that encapsulates a simple Tizen application. It contains a simple
 * implementation of a Tizen application, and several overridable methods that can be overriden
 * for specific Tizen system callback implementation.
 *
 * ApplicationBase class encapsulates window frame with Naviframe object as its root component.
 * Any view class inheriting ViewBase can be attached to this application and will be displayed
 * on to screen.
 */
class LIBAPI ApplicationBase
{
public:
	char const* const packageName;

	/**
	 * Constructor of ApplicationBase class
	 */
	ApplicationBase(char const* packageName);

	/**
	 * Method that will be called by Tizen system for any Control event received. Override this
	 * method to react to the event.
	 */
	virtual void ApplicationControl(app_control_h app_control);

	/**
	 * Method that will be called by Tizen system for any Pause event received. Override this
	 * method to react to the event.
	 */
	virtual void ApplicationPause();

	/**
	 * Method that will be called by Tizen system for any Resume event received. Override this
	 * method to react to the event.
	 */
	virtual void ApplicationResume();

	/**
	 * Method that will be called by Tizen system for any Terminate event received. Override
	 * this method to react to the event.
	 */
	virtual void ApplicationTerminate();

	/**
	 * Method that will be called by Tizen system for any Language Changed event received.
	 * Override this method to react to the event.
	 */
	virtual void LanguageChanged(app_event_info_h event_info, const char* locale);

	/**
	 * Method that will be called by Tizen system for any Orientation Changed event received.
	 * Override this method to react to the event.
	 */
	virtual void OrientationChanged(app_event_info_h event_info);

	/**
	 * Method that will be called by Tizen system for any Region Changed event received.
	 * Override this method to react to the event.
	 */
	virtual void RegionChanged(app_event_info_h event_info);

	/**
	 * Method that will be called by Tizen system for any Low Battery event received. Override
	 * this method to react to the event.
	 */
	virtual void LowBattery(app_event_info_h event_info);

	/**
	 * Method that will be called by Tizen system for any Low Memory event received. Override
	 * this method to react to the event.
	 */
	virtual void LowMemory(app_event_info_h event_info);

	/**
	 * Method to initiate exit the application
	 */
	void Exit();

	/**
	 * Method that will be called by Tizen system to create the application.
	 */
	virtual bool ApplicationCreate() = 0;

	/**
	 * Destructor of ApplicationBase
	 */
	virtual ~ApplicationBase();

	/**
	 * Main method to start the application loop using an ApplicationBase instance
	 */
	static int Main(ApplicationBase* app, int argc, char *argv[]);

	//TODO Give replacement for these SimpleReadOnlyProperty as these classes is no longer available
	/*
	static SimpleReadOnlyProperty<ApplicationBase, ApplicationBase*> CurrentInstance;
	static SimpleReadOnlyProperty<ApplicationBase, char const*> ResourcePath;
	*/

	static std::string GetResourcePath(char const* path);

	static ApplicationBase* GetCurrentInstance();
private:
	static char const* resourcePath;
	static ApplicationBase* currentInstance;
};

inline ApplicationBase* ApplicationBase::GetCurrentInstance() { return currentInstance; }


class LIBAPI HeadlessApplicationBase : public ApplicationBase
{
public:
	HeadlessApplicationBase(char const* packageName);

	virtual bool ApplicationCreate() final;
	virtual void ApplicationControl(app_control_h app_control) final;
	virtual void OnReceiveAppControlMessage(app_control_h app_control) = 0;
};

class LIBAPI UIApplicationBase : public ApplicationBase
{
private:

	bool haveEventBackPressed;
	bool haveEventMorePressed;
	UIApplicationBase* currentInstance;

	struct BackButtonCallbackDelegate
	{
		EventClass* instance;
		BackButtonCallback callback;
	};

	std::vector<BackButtonCallbackDelegate> backButtonStack;

protected:
	/**
	 * Root win object
	 */
	Evas_Object* win;

	/**
	 * Root conform object
	 */
	Evas_Object* conform;

	/**
	 * Root frame object. Should never be modified in any condition
	 */
	Evas_Object* rootFrame;

public:
	UIApplicationBase(char const* packageName);

	/**
	 * Method that will be called after the application creation process is finished. At this
	 * point, the application is ready to receive Attach action from any component.
	 */
	virtual void OnApplicationCreated();

	/**
	 * Method that will be called by Tizen system to create the application.
	 */
	virtual bool ApplicationCreate() final;

	void SetIndicatorVisibility(bool value);

	void SetIndicatorColor(Color color);

	Evas_Object* GetApplicationConformant();

	/**
	 * Method that will be called when the user called back button. Override this method to
	 * handle specific behavior when user clicks the back button.
	 *
	 * @return true if the application should be closed after clicking back
	 * 		   false to cancel closing the application
	 */
	virtual bool OnBackButtonPressed();

	virtual void OnMoreButtonPressed();

	void EnableBackButtonCallback(bool enable);

	void EnableMoreButtonCallback(bool enable);

	bool AcquireExclusiveBackButtonPressed(EventClass* instance, BackButtonCallback callback);

	bool ReleaseExclusiveBackButtonPressed(EventClass* instance, BackButtonCallback callback);

	void SetApplicationContent(Evas_Object* content);

	/**
	 * Method that will be called by Tizen system when the user clicks the back button
	 */
	void BackButtonPressed();

	//TODO Give replacement for these SimpleReadOnlyProperty as these classes is no longer available
	/*
	static SimpleReadOnlyProperty<UIApplicationBase, UIApplicationBase*> CurrentInstance;
	*/
};

/**
 * Controller base class for encapsulation of business logic from view logic. Each controller
 * will have one default view which will display data and receive action on behalf of this
 * controller.
 *
 * Create a class which inherits this class to create a controller for the application. A
 * controller will be managed by ControllerManager to support navigation between controllers,
 * and control passing.
 */
class LIBAPI ControllerBase
{
protected:
	/**
	 * Reference to ControllerManager
	 */
	ControllerManager* const Manager;

	/**
	 * Method that will be called during the loading of this controller. Override this method
	 * to implement specific action for controller loading.
	 *
	 * @param data Data received from other controller which calls this controller
	 *
	 * @note The data parameter must be freed by this controller as the owner of the object is
	 * 		 passed onto this controller.
	 */
	virtual void OnLoad(ObjectClass* data);

	/**
	 * Method that will be called during the unloading of this controller. This method will be
	 * called when it is navigating back from this controller to its caller-controller.
	 *
	 * @return Data returned to be passed back to the calling controller
	 *
	 * @note The data to be returned must be allocated in the heap, and the responsible party
	 * 		 to clean up the data is the receiver controller.
	 */
	virtual ObjectClass* OnUnload();

	/**
	 * Method that will be called during the reloading of this controller after returning from
	 * other controller. Override this method to implement specific action for controller
	 * reloading.
	 *
	 * @param data Data retrieved from the previously called controller (i.e. returned by
	 * 			   OnUnload method of called controller).
	 *
	 * @note The data parameter must be freed by this controller as the owner of the object is
	 * 		 passed onto this controller.
	 */
	virtual void OnReload(ObjectClass* data);


	virtual void OnLeaving();
public:
	/**
	 * The standard constructor for ControllerBase class.
	 *
	 * @param manager ControllerManager that is automatically populated during the instantiation
	 * 				  of the controller
	 * @param view ViewBase instance that will be the main view of this controller
	 * @param controllerName The unique name of this controller
	 *
	 * @note In order to follow the standardized factory pattern for controller classes, the
	 * 		 implementing class _must follow standardized constructor signature_ as follow:
	 * 		 `CustomController(Framework::ControllerManager* manager)`. If the implementing
	 * 		 class does not create the standard constructor, the factory mechanism to register
	 * 		 the controller to the ControllerManager will not work.
	 *
	 * @see {RegisterController}
	 */
	ControllerBase(ControllerManager* manager, ViewBase* view, char const* controllerName);

	/**
	 * The reference to the view instance for this controller
	 */
	ViewBase* const View;

	/**
	 * The name of this controller
	 */
	char const* const ControllerName;

	/**
	 * Method that will be called by ControllerManager to load the controller
	 *
	 * @param data Parameter to pass to this controller
	 */
	void Load(ObjectClass* data);

	/**
	 * Method that will be called by ControllerManager to unload the controller
	 *
	 * @return The returned data from this controller
	 */
	ObjectClass* Unload();

	/**
	 * Method that will be called by ControllerManager to reload the controller
	 *
	 * @param data Parameter to pass to this controller
	 */
	void Reload(ObjectClass* data);



	void Leave();

	/**
	 * Destructor of ControllerBase instance
	 */
	virtual ~ControllerBase();
};

/**
 * Function pointer to instantiate the specific controller. Should be used along the macro
 * RegisterController
 */
typedef ControllerBase* (*ControllerFactoryMethod)(ControllerManager*);

/**
 * Class that encapsulate dynamic controller instatiation which is required by ControllerManager
 */
class LIBAPI ControllerFactory
{
public:
	/**
	 * Constructor of ControllerFactory
	 *
	 * @param controllerName Name of the controller
	 * @param factory Factory method which will instantiate the controller
	 */
	ControllerFactory(char const* controllerName, ControllerFactoryMethod factory);
	void* attachedData;

	ControllerBase* Instantiate(ControllerManager* mgr);

	friend class TFC::Framework::ControllerManager;

private:
	char const* const controllerName;
	ControllerFactoryMethod const factoryMethod;
};

/**
 * Class that encapsulate controller stack within ControllerManager
 */
enum class NavigationFlag
{
	Default 	 = 0x1,
	Back 		 = 0x2,
	NoTrail 	 = 0x4,
	ClearHistory = 0x8,
	NoCallUnload = 0x10
};

inline bool operator&(NavigationFlag a, NavigationFlag b) { return static_cast<int>(a) & static_cast<int>(b); }
inline NavigationFlag operator|(NavigationFlag a, NavigationFlag b) { return static_cast<NavigationFlag>(static_cast<int>(a) | static_cast<int>(b)); }

/**
 * Class that manages Controller and provides navigation mechanism between loaded controllers
 */
class LIBAPI ControllerManager :
	public EFL::EFLProxyClass,
	public EventEmitterClass<ControllerManager>,
	PropertyClass<ControllerManager>
{
private:
	//Eina_Hash* controllerTable;
	std::unordered_map<std::string, ControllerFactoryMethod> controllerTable;

	bool pendingNavigation;
	void PerformNavigationInternal(char const* controllerName, ObjectClass* data, NavigationFlag mode);
protected:
	ControllerBase* Instantiate(char const* controllerName);

	virtual ControllerBase& GetCurrentController() const = 0;
	virtual void PerformNavigation(char const* controllerName, ObjectClass* data, NavigationFlag mode) = 0;
	virtual Evas_Object* CreateViewContainer(Evas_Object* parent) = 0;
public:
	ControllerManager();
	void NavigateTo(char const* controllerName, ObjectClass* data, NavigationFlag mode);
	void NavigateTo(char const* controllerName, ObjectClass* data) 				 { NavigateTo(controllerName, data, NavigationFlag::Default); };
	void NavigateTo(char const* controllerName, ObjectClass* data, bool noTrail) { NavigateTo(controllerName, data, NavigationFlag::NoTrail); };
	bool NavigateBack() 													     { NavigateTo(nullptr, nullptr, NavigationFlag::Back); return true; };

	Event<ControllerBase*> eventNavigationProcessed;
	Property<ControllerBase&>::Get<&ControllerManager::GetCurrentController> CurrentController;

	/**
	 * Method to register ControllerFactory to this manager so this manager can recognize
	 * the controller and instantiate it when needed
	 */
	void RegisterControllerFactory(ControllerFactory* controller);
	virtual ~ControllerManager();
};

class LIBAPI ITitleProvider: PropertyClass<ITitleProvider>
{
private:
	std::string viewTitle;
protected:
	virtual void SetTitle(const std::string& value);
	std::string const& GetTitle() const;
	ITitleProvider();
public:
	Property<std::string const&>::Get<&ITitleProvider::GetTitle>
						        ::Set<&ITitleProvider::SetTitle> Title;

	virtual ~ITitleProvider();
};

/**
 * Abstract base class for encapsulation of View definition and logic. Each view class represent
 * a user interface definition which will display data and interact directly with user.
 *
 * Create a class which inherits from this base class to create a user interface class. Instance
 * of this class can be attached into an IAttachable instance to display it to designated window
 * or placeholder.
 */
class LIBAPI ViewBase:
	virtual public EventClass,
	virtual public ITitleProvider,
	protected EFL::EFLProxyClass
{
private:
	Evas_Object* viewRoot;

protected:
	virtual Evas_Object* CreateView(Evas_Object* root) = 0;
public:
	ViewBase();
	virtual Evas_Object* Create(Evas_Object* root);
	bool IsCreated();
	Evas_Object* GetViewRoot();
	virtual ~ViewBase();
};

class LIBAPI INaviframeContent : virtual public ITitleProvider
{
private:
	Elm_Object_Item* naviframeItem;
protected:
	virtual void AfterNaviframePush(Elm_Object_Item* naviframeItem);
	virtual void SetTitle(const std::string& value);
public:
	INaviframeContent();
	virtual Evas_Object* GetTitleLeftButton(char const** buttonPart) = 0;
	virtual Evas_Object* GetTitleRightButton(char const** buttonPart) = 0;
	virtual char const* GetContentStyle() = 0;
	void RaiseAfterNaviframePush(Elm_Object_Item* naviframeItem);
	virtual ~INaviframeContent();
};

class LIBAPI IIndicatorStyle : PropertyClass<IIndicatorStyle>
{
public:
	IIndicatorStyle();

	//TODO Reimplement Auto Property
	/*
	Property<IIndicatorStyle, Color>::Auto::ReadWrite IndicatorColor;
	Property<IIndicatorStyle, bool>::Auto::ReadWrite IndicatorVisible;
	*/

	Color IndicatorColor;
	bool IndicatorVisible;
};


class LIBAPI IndicatorStyler : public EventClass
{
private:
	void OnPostNavigation(ControllerManager* manager, ControllerBase* controller);
	UIApplicationBase* app;
	ControllerManager* manager;
	Color defaultColor;
public:
	IndicatorStyler(UIApplicationBase* app, ControllerManager* manager, Color defaultColor);
	~IndicatorStyler();
};


}
}

#endif
