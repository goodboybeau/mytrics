#ifndef _UTILS_HPP_
#define _UTILS_HPP_

// #include "global.hpp"
#include <iostream>
#define DEBUG(s) std::cout << s << std::endl; std::flush(std::cout);

template <class T, typename R>
struct meta_call
{
	std::function<R()> f;
	std::reference_wrapper<T> ref;

	meta_call(const std::reference_wrapper<T>& a)
		: ref(a)
	{
		f = std::bind([](){return R();});
		DEBUG("meta_call()");
	}

	operator T ()
	{
		DEBUG("meta_call operator T");
		return this->ref;
	}

	R operator() ()
	{
		DEBUG("meta_call operator()");
		return this->f();
	}

	virtual T* operator->()
	{
		DEBUG("meta_call operator->");
		return &this->ref.get();
	}

	~meta_call()
	{
		DEBUG("~meta_call()");
		this->operator()();
	}
};

template <class T>
class Accessed : public T
{

public:

	Accessed() { DEBUG("new Accessed()"); }

	Accessed(const std::string& name)
		: T(name)
	{}

	virtual ~Accessed()
	{
		DEBUG("~Accessed()");
	}

	virtual void post_operation()
	{
		DEBUG("post_operation");
	}

	virtual meta_call<Accessed<T>, void> operator->()
	{
		DEBUG("Accessed operator->");

		meta_call<Accessed<T>, void> m(std::move(std::ref(*this)));
		m.f = std::bind(&Accessed<T>::post_operation, this);

		return m;
	}
};

#endif /* _UTILS_HPP_ */
