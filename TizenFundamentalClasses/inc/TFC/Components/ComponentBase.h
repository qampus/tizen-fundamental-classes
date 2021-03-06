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
 *    Components/ComponentBase.h
 *
 * Base class for reusable components which utilizes EFL widgets and can
 * be integrated with TFC patterns.
 *
 * Created on: 	 Apr 27, 2016
 * Author: 		 Gilang Mentari Hamidy (g.hamidy@samsung.com)
 */

#ifndef TFCFW_COMPONENTBASE_H_
#define TFCFW_COMPONENTBASE_H_

#include "TFC/EFL.h"

#include <string>

namespace TFC {
namespace Components {


/**
 * Base class for Components. Provides basic structure that encapsulates Elementary objects into one single object in
 * C++. This pattern will enable reusability of custom composite components that comprises of multiple Elementary
 * objects.
 *
 * Deriver of this class must implement CreateComponent abstract method to implement the component creation logic which
 * interact with EFL functions. The CreateComponent method must return the created Evas_Object handle which represents
 * this components in EFL space.
 *
 * Object of this class can be directly casted to Evas_Object handle, which enables direct interfacing between this
 * class object and EFL functions.
 */
class LIBAPI ComponentBase:
		TFC::EventEmitterClass<ComponentBase>,
		public EFL::EFLProxyClass,
		public TFC::PropertyClass<ComponentBase>
{
public:
	ComponentBase();
	inline bool IsCreated()
	{
		return created;
	}
	Evas_Object* LIBAPI Create(Evas_Object* root);

	virtual LIBAPI ~ComponentBase();
	LIBAPI operator Evas_Object*() const;
protected:
	/**
	 * Field componentRoot which consists of the created component. Before method Create is called, this field must
	 * always be null.
	 */
	Evas_Object* componentRoot;

	double weightX; /**< Weight of X axis in EFL specification */
	double weightY; /**< Weight of Y axis in EFL specification */
	double alignX; 	/**< Alignment of X axis in EFL specification */
	double alignY;	/**< Alignment of Y axis in EFL specification */

	/**
	 * Abstract CreateComponent which must be overriden by inheriting class. This method must implement UI creation
	 * logics by interacting with Elementary functions within EFL framework. This method must return an Evas_Object*
	 * that will act as the main handle of this component in the EFL space.
	 *
	 * It is recommended to wrap all Elementary objects into one single Elementary containers which is then returned
	 * back to the caller of this function.
	 *
	 * @param[in] root The root/parent given for this component.
	 *
	 * @return Handle to Evas_Object that represents this component in EFL space.
	 */
	virtual LIBAPI Evas_Object* CreateComponent(Evas_Object* root) = 0;

	/**
	 * Method that will be called when the user of this component sets Enabled field. Override this method if the
	 * derived class have specific implementation when the user enable/disable this component.
	 *
	 * @note When overriding this method, always apply the changes to the EFL directly to ensure the appropriate
	 * 		 UI will be rendered. Check ComponentBase::IsCreated() method to indicate whether this component has been
	 * 		 created or not, and always call the parent method to ensure the state is stored internally.
	 *
	 * @param[in] enabled Value set by the user.
	 */
	virtual LIBAPI void SetEnabled(const bool& enabled);

	/**
	 * Method which returns the enable state of this component
	 *
	 * @return State whether this control is enabled or not
	 */
	virtual LIBAPI bool GetEnabled() const;

	/**
	 * Method that will be called when the user of this component sets Visible field. Override this method if the
	 * derived class have specific implementation when the user set visibility of this component.
	 *
	 * @note When overriding this method, always apply the changes to the EFL directly to ensure the appropriate
	 * 		 UI will be rendered. Check ComponentBase::IsCreated() method to indicate whether this component has been
	 * 		 created or not, and always call the parent method to ensure the state is stored internally.
	 *
	 * @param[in] visibility value set by the user.
	 */
	virtual LIBAPI void SetVisible(const bool& visible);

	/**
	 * Method which returns the visibility state of this component
	 *
	 * @return State whether this control is enabled or not
	 */
	virtual LIBAPI bool GetVisible() const;

	virtual LIBAPI void SetName(const std::string& name);
	virtual LIBAPI std::string const& GetName() const;
private:
	std::string name;
	bool created;
	bool enabled;
	bool visible;
public:
	Property<bool>::Get<&ComponentBase::GetEnabled>::Set<&ComponentBase::SetEnabled> Enabled;
	Property<bool>::Get<&ComponentBase::GetVisible>::Set<&ComponentBase::SetVisible> Visible;
	Property<std::string const&>::Get<&ComponentBase::GetName>::Set<&ComponentBase::SetName> Name;
};

}
}

#endif /* COMPONENTBASE_H_ */
