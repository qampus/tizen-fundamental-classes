/*
 * ControllerManager.cpp
 *
 *  Created on: Feb 15, 2016
 *      Contributor:
 *        Gilang M. Hamidy (g.hamidy@samsung.com)
 */

#include "TFC/Framework/Application.h"
#include <memory>
#include <sstream>
using namespace TFC::Framework;

void ControllerManager_FreeFactory(void* data)
{
	ControllerFactory* factory = static_cast<ControllerFactory*>(data);
	delete factory;
}

TFC::Framework::ControllerManager::ControllerManager() :
	CurrentController(this)
{

}

LIBAPI void ControllerManager::RegisterControllerFactory(ControllerFactory* ptr)
{
	std::unique_ptr<ControllerFactory> controller(ptr);
	auto iter = this->controllerTable.find(controller->controllerName);
	if(iter == this->controllerTable.end())
	{
		this->controllerTable[controller->controllerName] = controller->factoryMethod;
	}
	else
		throw TFCException("Controller already registered");
}

TFC::Framework::ControllerManager::~ControllerManager()
{
}

LIBAPI StackingControllerManager::StackingControllerManager(IAttachable* app) :
	app(app)
{

}

LIBAPI void StackingControllerManager::NavigateTo(const char* controllerName, ObjectClass* data)
{
	NavigateTo(controllerName, data, false);
}

void StackingControllerManager::PushController(ControllerBase* controller)
{
	this->controllerStack.emplace_back(controller);

	/*
	ControllerChain* newChain = new ControllerChain();
	newChain->instance = controller;
	newChain->next = this->chain;
	this->chain = newChain;
	*/
}

bool StackingControllerManager::PopController()
{
	/*
	if (this->chain != nullptr)
	{
		ControllerChain* oldChain = this->chain;
		this->chain = oldChain->next;
		delete oldChain->instance;
		delete oldChain;
	}
	*/

	this->controllerStack.pop_back();

	if (!this->controllerStack.empty())
		return true;
	else
		return false;
}

LIBAPI bool StackingControllerManager::NavigateBack()
{
	if(this->pendingNavigation)
		return true; // TODO change this to exception

	this->pendingNavigation = true;
	InvokeLater(&StackingControllerManager::DoNavigateBackward);

	// The new implementation should always return true
	// as the codes might interpret False to exit the application
	return true;
}

LIBAPI void StackingControllerManager::NavigateTo(const char* controllerName, ObjectClass* data, bool noTrail)
{
	if(this->pendingNavigation)
		return; // TODO change this to exception

	this->pendingNavigation = true;
	//this->navigateForward = true;
	//this->nextControllerName = controllerName;
	//this->data = data;
	//this->noTrail = noTrail;
	InvokeLater(&StackingControllerManager::DoNavigateForward, controllerName, data, noTrail);

}

void StackingControllerManager::OnPerformNavigation()
{

}

LIBAPI ControllerFactory::ControllerFactory(char const* controllerName, ControllerFactoryMethod factory) :
	controllerName(controllerName), factoryMethod(factory)
{
	attachedData = nullptr;
}

LIBAPI ControllerBase& TFC::Framework::StackingControllerManager::GetCurrentController() const
{
	return *(this->controllerStack.back());
}

LIBAPI
void TFC::Framework::StackingControllerManager::DoNavigateBackward()
{
	// Navigate back
	ObjectClass* returnedData = this->CurrentController->Unload();
	app->Detach();
	bool popResult = PopController();

	if (popResult)
	{

		ControllerBase& current = this->CurrentController;
		current.Reload(returnedData);
		eventNavigationProcessed(this, &current);
	}

	this->pendingNavigation = false;

	// If pop result is false, it is the end of the controller
	// It should end the application
	// TODO: Add this as event instead
	if(!popResult)
		ui_app_exit();
}

LIBAPI
void TFC::Framework::StackingControllerManager::DoNavigateForward(const char* targetControllerName, ObjectClass* data, bool noTrail)
{
	if(noTrail)
	{
		this->CurrentController->Unload();
		app->Detach();
		PopController();
	}

	// Instantiate controller
	ControllerBase* newInstance = this->Instantiate(targetControllerName);

	// Perform OnLeave on previous controller
	if(!this->controllerStack.empty())
		this->CurrentController->Leave();

	PushController(newInstance);
	app->Attach(newInstance->View);

	// Instantiated State, move to Running state
	newInstance->Load(data);
	eventNavigationProcessed(this, newInstance);

	this->pendingNavigation = false;
}

LIBAPI
void TFC::Framework::StackingControllerManager::ClearNavigationHistory()
{
	while(this->controllerStack.size() != 1)
	{
		this->controllerStack.pop_front();
	}
}

ControllerBase* TFC::Framework::ControllerFactory::Instantiate(
		ControllerManager* mgr) {
	return this->factoryMethod(mgr);
}

ControllerBase* TFC::Framework::ControllerManager::Instantiate(
		const char* controllerName) {
	try
	{
		auto method = this->controllerTable.at(controllerName);
		return method(this);
	}
	catch(std::out_of_range& ex)
	{
		std::stringstream errBuilder;
		errBuilder << "Unknown controller identifier: " << controllerName;
		throw TFCException(errBuilder.str());
	}
}
