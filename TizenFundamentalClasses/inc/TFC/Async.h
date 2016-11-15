/*
 * Async.h
 *
 *  Created on: Nov 2, 2016
 *      Author: Gilang M. Hamidy
 */

#pragma once

#ifndef TFC_CORE_ASYNC_NEW_H_
#define TFC_CORE_ASYNC_NEW_H_

#include <functional>

#include "TFC/Core.h"
#include "TFC/Core/Introspect.h"

namespace TFC {

template<typename TReturnValue>
struct Async;


namespace Core {
namespace Async {

template<typename TReturnValue>
struct AsyncTask;

struct AsyncTypeTag
{
	struct AsyncVoid 	{ };
	struct AsyncNonVoid { };
};

template<typename T>
struct AsyncTypeSelector
{
	typedef AsyncTypeTag::AsyncNonVoid Tag;
	typedef T EventDataType;
	typedef std::function<void(T)> CompleteFunction;
};

template<>
struct AsyncTypeSelector<void>
{
	typedef AsyncTypeTag::AsyncVoid Tag;
	typedef void* EventDataType;
	typedef std::function<void()> CompleteFunction;
};


template<typename TReturnValue, typename TEventType = void>
struct AsyncOperand
{
	typedef TReturnValue											  ReturnType;
	typedef AsyncTask<TReturnValue>									  AsyncTaskType;
	typedef SharedEventObject<AsyncTask<TReturnValue>*, TReturnValue> EventType;
	std::function<ReturnType(void)> asyncFunc;

	AsyncOperand(std::function<ReturnType(void)>&& asyncFunc) :
		asyncFunc(asyncFunc)
	{ }

	template<typename TLambda,
			 typename TIntrospect = Introspect::CallableObject<TLambda>,
			 typename 			  = typename std::enable_if<TIntrospect::Arity == 0>::type>
	AsyncOperand(TLambda aLambda) :
		asyncFunc(aLambda)
	{ }
};



template<>
struct AsyncOperand<void, void>
{
	typedef void											ReturnType;
	typedef AsyncTask<void>									AsyncTaskType;
	typedef SharedEventObject<AsyncTask<void>*, void*> 		EventType;

	std::function<void(void)> asyncFunc;

	AsyncOperand(std::function<void(void)>&& func) :
		asyncFunc(func)
	{ }

	template<typename TLambda,
				 typename TIntrospect = Introspect::CallableObject<TLambda>,
				 typename 			  = typename std::enable_if<TIntrospect::Arity == 0>::type>
	AsyncOperand(TLambda aLambda) :
		asyncFunc(aLambda)
	{ }
};

template<>
struct AsyncOperand<void, SharedEventObject<AsyncTask<void>*, void*>> :
		AsyncOperand<void, void>
{

	AsyncOperand<void, void>::EventType&	eventRef;

	AsyncOperand(std::function<void(void)>&& func, AsyncOperand<void, void>::EventType& eventRef) :
		AsyncOperand<void, void>(std::move(func)),
		eventRef(eventRef)
	{ }
};

template<typename TReturnValue>
struct AsyncOperand<TReturnValue, SharedEventObject<AsyncTask<TReturnValue>*, TReturnValue>> :
		AsyncOperand<TReturnValue, void>
{

	typename AsyncOperand<TReturnValue, void>::EventType&	eventRef;

	AsyncOperand(std::function<TReturnValue(void)>&& func, typename AsyncOperand<TReturnValue, void>::EventType& eventRef) :
		AsyncOperand<TReturnValue, void>(std::move(func)),
		eventRef(eventRef)
	{ }
};

template<typename TLambda,
		 typename TIntrospect = Introspect::CallableObject<TLambda>,
		 bool TNonVoidParam   = TIntrospect::Arity == 1,
		 bool TParamOneOrZero = TIntrospect::Arity <= 1>
struct AsyncCompleteOperand
{
	typedef typename Introspect::CallableObject<TLambda>::template Args<0> ArgumentType;
	static constexpr bool Valid = true;
	static constexpr bool IsVoid = false;


	typedef typename AsyncTypeSelector<ArgumentType>::CompleteFunction Function;
	Function CompleteLambda;

	AsyncCompleteOperand(Function&& CompleteLambda) :
			CompleteLambda(CompleteLambda)
	{

	}

	static AsyncCompleteOperand<TLambda> MakeOperand(TLambda&& lambda)
	{
		return { lambda };
	}

	template<typename TLambdaAsync,
			 typename TIntrospectAsync = Introspect::CallableObject<TLambdaAsync>,
			 bool 	  RuleValidation   = std::is_same<typename TIntrospect::template Args<0>,
								  	  	  	  	  	  typename TIntrospectAsync::ReturnType>::value>
	struct Match
	{
		static constexpr bool Valid = RuleValidation;
	};
};

template<typename TLambda, typename TIntrospect>
struct AsyncCompleteOperand<TLambda, TIntrospect, false, true>
{
	static constexpr bool Valid = true;
	static constexpr bool IsVoid = true;


	typedef typename AsyncTypeSelector<void>::CompleteFunction Function;
	Function CompleteLambda;

	AsyncCompleteOperand(Function&& CompleteLambda) :
				CompleteLambda(CompleteLambda)
	{

	}

	static AsyncCompleteOperand<TLambda> MakeOperand(TLambda&& lambda)
	{
		return { lambda };
	}

	template<typename TLambdaAsync,
			 typename TIntrospectAsync = Introspect::CallableObject<TLambdaAsync>,
			 bool 	  RuleValidation   = std::is_same<void,
									  	  	  	  	  typename TIntrospectAsync::ReturnType>::value>
	struct Match
	{
		static constexpr bool Valid = RuleValidation;
	};
};

template<typename TLambda, typename TIntrospect>
struct AsyncCompleteOperand<TLambda, TIntrospect, false, false>
{
	static constexpr bool Valid = false;

	explicit AsyncCompleteOperand()
	{

	}

	template<typename TAny>
	static AsyncCompleteOperand<TLambda> MakeOperand(TAny lambda)
	{
		return AsyncCompleteOperand<TLambda>();
	}
};



template<typename TReturnValue, typename TLambdaComplete>
struct AsyncOperand<TReturnValue, AsyncCompleteOperand<TLambdaComplete>> :
		AsyncOperand<TReturnValue, void>
{
	typedef typename AsyncTypeSelector<TReturnValue>::CompleteFunction Function;
	Function funcComplete;

	AsyncOperand(std::function<TReturnValue(void)>&& func, Function&& funcComplete) :
		AsyncOperand<TReturnValue, void>(std::move(func)),
		funcComplete(funcComplete)
	{ }
};

template<typename T>
struct AsyncResult
{
	typedef T ReturnType;
	T value;

	AsyncResult() : value(T()) { };
};

template<>
struct AsyncResult<void>
{

};

template<typename T>
struct AsyncPackage
{
	std::function<T(void)> function;

	SharedEventObject<AsyncTask<T>*, typename AsyncTypeSelector<T>::EventDataType> event;
	typename AsyncTypeSelector<T>::CompleteFunction funcComplete;


	void(*completeFunc)(void*);
	AsyncResult<T> result;
	void* taskHandle;
	bool awaitable;

	AsyncPackage() : taskHandle(nullptr), awaitable(true), completeFunc(nullptr) { };
};




template<typename T>
void AsyncWorker(void* package)
{
	auto ptr = reinterpret_cast<AsyncPackage<T>*>(package);
	ptr->result.value = std::move(ptr->function());
}

template<>
inline void AsyncWorker<void>(void* package)
{
	auto ptr = reinterpret_cast<AsyncPackage<void>*>(package);
	ptr->function();
}

template<typename T>
void AsyncCompleteHandler(void* package)
{
	AsyncPackage<T>* ptr = reinterpret_cast<AsyncPackage<T>*>(package);
	ptr->event(reinterpret_cast<AsyncTask<T>*>(ptr->taskHandle), ptr->result.value);
}

template<>
inline void AsyncCompleteHandler<void>(void* package)
{
	AsyncPackage<void>* ptr = reinterpret_cast<AsyncPackage<void>*>(package);
	ptr->event(reinterpret_cast<AsyncTask<void>*>(ptr->taskHandle), nullptr);

}

template<typename T>
void AsyncCompleteHandlerComplete(void* package)
{
	AsyncPackage<T>* ptr = reinterpret_cast<AsyncPackage<T>*>(package);
	ptr->funcComplete(ptr->result.value);
}

template<>
inline void AsyncCompleteHandlerComplete<void>(void* package)
{
	AsyncPackage<void>* ptr = reinterpret_cast<AsyncPackage<void>*>(package);
	ptr->funcComplete();
}

template<typename T>
void AsyncFinalizeHandler(void* package)
{
	auto ptr = reinterpret_cast<AsyncPackage<T>*>(package);
	delete ptr;
}


template<typename TReturnValue, typename TEvent>
auto PackOperand(AsyncOperand<TReturnValue, TEvent> const& operand)
	-> typename std::enable_if<!std::is_void<TEvent>::value, AsyncPackage<TReturnValue>*>::type
{
	AsyncPackage<TReturnValue>* ret = new AsyncPackage<TReturnValue>;
	ret->function = operand.asyncFunc;
	ret->event = operand.eventRef;
	ret->completeFunc = AsyncCompleteHandler<TReturnValue>;
	ret->awaitable = false;
	return ret;
}

template<typename TReturnValue, typename TLambdaComplete>
auto PackOperand(AsyncOperand<TReturnValue, AsyncCompleteOperand<TLambdaComplete>> const& operand)
	-> AsyncPackage<TReturnValue>*
{
	AsyncPackage<TReturnValue>* ret = new AsyncPackage<TReturnValue>;
	ret->function = operand.asyncFunc;
	ret->funcComplete = operand.funcComplete;
	ret->completeFunc = AsyncCompleteHandlerComplete<TReturnValue>;
	ret->awaitable = false;
	return ret;
}

template<typename TReturnValue>
auto PackOperand(AsyncOperand<TReturnValue, void> const& operand)
	-> AsyncPackage<TReturnValue>*
{
	AsyncPackage<TReturnValue>* ret = new AsyncPackage<TReturnValue>;
	ret->function = operand.asyncFunc;
	return ret;
}

struct AsyncHandlerPayload
{
	typedef void(*FunctionType)(void*);
	FunctionType taskFunc;
	FunctionType completeFunc;
	FunctionType finalizeFunc;
	void*		 internalData;
	void**		 taskHandleRef;
	bool		 awaitable;
};

void* RunAsyncTask(AsyncHandlerPayload payload);

void AwaitAsyncTask(void* handle, void*& package, bool& doFinalize);

struct AwaitBuilder
{
	template<typename TReturnValue>
	TReturnValue operator&(AsyncTask<TReturnValue>* taskHandle)
	{
		void* packagePtr = nullptr;
		bool doFinalize = false;
		AwaitAsyncTask(taskHandle, packagePtr, doFinalize);

		if(doFinalize)
		{
			auto package = reinterpret_cast<AsyncPackage<TReturnValue>*>(packagePtr);
			TReturnValue ret = package->result.value;
			delete package;
			return ret;
		}
		else
		{
			return reinterpret_cast<AsyncPackage<TReturnValue>*>(packagePtr)->result.value;
		}
	}

	void operator&(AsyncTask<void>* taskHandle)
	{
		bool doFinalize = false;
		void* packagePtr = nullptr;
		AwaitAsyncTask(taskHandle, packagePtr, doFinalize);

		if(doFinalize)
		{
			delete reinterpret_cast<AsyncPackage<void>*>(packagePtr);
		}
	}
};

struct AsyncBuilder
{
	template<typename TReturnValue, typename TEvent>
	auto operator& (AsyncOperand<TReturnValue, TEvent> operand)
		-> AsyncTask<TReturnValue>*
	{
		auto packed = PackOperand(operand);

		AsyncHandlerPayload payload = {
			AsyncWorker<TReturnValue>,
			packed->completeFunc,
			AsyncFinalizeHandler<TReturnValue>,
			packed,
			&packed->taskHandle,
			packed->awaitable
		};
		return reinterpret_cast<AsyncTask<TReturnValue>*>(RunAsyncTask(payload));
	}

	template<typename TLambda,
			 typename TIntrospect = Introspect::CallableObject<TLambda>>
	auto operator&(TLambda lambda)
		-> AsyncTask<typename TIntrospect::ReturnType>*
	{
		return operator&(AsyncOperand<typename TIntrospect::ReturnType>(lambda));
	}
};

struct CompleteBuilder
{
	template<typename TLambda>
	auto operator*(TLambda&& lambda)
		-> AsyncCompleteOperand<TLambda>
	{
		static_assert(AsyncCompleteOperand<TLambda>::Valid, "tfc_async_complete block has invalid result-capture declaration");
		return AsyncCompleteOperand<TLambda>::MakeOperand(std::move(lambda));
	}
};

template<typename TLambda,
		 typename TIntrospect = Introspect::CallableObject<TLambda>,
		 typename 			  = typename std::enable_if<TIntrospect::Arity == 0>::type>
auto operator>>(TLambda lambda, typename AsyncOperand<typename TIntrospect::ReturnType>::EventType& event)
	-> AsyncOperand<typename TIntrospect::ReturnType, typename AsyncOperand<typename TIntrospect::ReturnType>::EventType>
{
	return { lambda, event };
}

/*
The following template form :
	template<typename TLambdaAsync,
			 typename TLambdaAfter,
			 typename TIntrospectAsync = Introspect::CallableObject<TLambdaAsync>,
			 typename TIntrospectAfter = Introspect::CallableObject<TLambdaAfter>,
			 typename				   = typename std::enable_if<TIntrospectAsync::Arity == 0
																 && TIntrospectAfter::Arity == 1
																 && std::is_same<typename TIntrospectAfter::template Args<0>,
																				 typename TIntrospectAsync::ReturnType>::value>::type>
	auto operator>>(TLambdaAsync async, AsyncCompleteOperand<TLambdaAfter> after)
		-> AsyncOperand<typename TIntrospectAsync::ReturnType, AsyncCompleteOperand<TLambdaAfter>>
	{
		return { async, std::move(after.CompleteLambda) };
	}
is not applicable if we want to use std::enable_if and will result in [template parameter redefines default argument] error.
See : https://stackoverflow.com/questions/29502052/template-specialization-and-enable-if-problems
*/

template<typename TLambdaAsync,
		 typename TLambdaAfter,
		 typename TIntrospectAsync = Introspect::CallableObject<TLambdaAsync>,
		 typename TIntrospectAfter = Introspect::CallableObject<TLambdaAfter>,
		 typename std::enable_if<AsyncCompleteOperand<TLambdaAfter>::template Match<TLambdaAsync>::Valid, int>::type* = nullptr>
auto operator>>(TLambdaAsync async, AsyncCompleteOperand<TLambdaAfter> after)
	-> AsyncOperand<typename TIntrospectAsync::ReturnType, AsyncCompleteOperand<TLambdaAfter>>
{
	return { async, std::move(after.CompleteLambda) };
}

// Assert that CompleteBuilder >> operator will receive lambda with appropriate parameter
template<typename TLambdaAsync,
		 typename TLambdaAfter,
		 typename TIntrospectAsync = Introspect::CallableObject<TLambdaAsync>,
		 typename TIntrospectAfter = Introspect::CallableObject<TLambdaAfter>,
		 typename std::enable_if<!AsyncCompleteOperand<TLambdaAfter>::Valid, int>::type* = nullptr>
auto operator>>(TLambdaAsync async, AsyncCompleteOperand<TLambdaAfter> after)
	-> AsyncOperand<typename TIntrospectAsync::ReturnType, AsyncCompleteOperand<TLambdaAfter>>
{

	return { nullptr, nullptr };
}


template<typename TLambdaAsync,
		 typename TLambdaAfter,
		 typename TIntrospectAsync = Introspect::CallableObject<TLambdaAsync>,
		 typename TIntrospectAfter = Introspect::CallableObject<TLambdaAfter>,
		 typename std::enable_if<!AsyncCompleteOperand<TLambdaAfter>::template Match<TLambdaAsync>::Valid, int>::type* = nullptr>
auto operator>>(TLambdaAsync async, AsyncCompleteOperand<TLambdaAfter> after)
	-> AsyncOperand<typename TIntrospectAsync::ReturnType, AsyncCompleteOperand<TLambdaAfter>>
{
	static_assert(std::is_same<typename TIntrospectAfter::template Args<0>, typename TIntrospectAsync::ReturnType>::value,
			"Parameter for tfc_async_complete lambda must match with the return value from tfc_async.");
	return { nullptr, nullptr };
}



}}}

template<typename TReturnValue>
struct TFC::Async
{
	typedef Core::Async::AsyncTask<TReturnValue> Task;
	typedef Core::SharedEventObject<Task*,
									typename std::conditional<std::is_void<TReturnValue>::value,
															  void*,
															  TReturnValue>::type
									> Event;
	typedef typename Event::Type BaseEvent;

};


#define tfc_async TFC::Core::Async::AsyncBuilder() & [=] ()
#define tfc_await TFC::Core::Async::AwaitBuilder() &
#define tfc_try_await
#define tfc_async_complete >> TFC::Core::Async::CompleteBuilder() * [=]
#define tfc_if_abort_return

#endif /* ASYNC_NEW_H_ */