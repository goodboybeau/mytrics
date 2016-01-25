/*
 * timer.hpp
 *
 *  Created on: Sep 11, 2015
 *      Author: jaronhalt
 */

#ifdef __cplusplus
#if __cplusplus < 201103L
#define __cplusplus 201103L
#endif
#endif

#ifndef COUNTER_HPP_
#define COUNTER_HPP_

#include <atomic>

#include "i_metric.hpp"


template<typename T>
class ITCounter : public TMetric<T>
{
public:
	ITCounter() = default;
	virtual ~ITCounter() = default;

	virtual T operator+(const T&) = 0;
	virtual T operator-(const T&) = 0;

	virtual T add(const T&) = 0;

	virtual T sub(const T&) = 0;

	virtual T set(const T&) = 0;

	virtual T get() const = 0;

	virtual IMetric::UNIT unit() const
	{
		return IMetric::UNIT::COUNT;
	}
};


template<typename T>
class TBaseCounter : public ITCounter<T>
{
public:
	TBaseCounter(T v=0)
	: val(v)
	{}

	virtual ~TBaseCounter()
	{}

	virtual T operator+(const T& v)
	{
		return this->add(v);
	}

	virtual T operator-(const T& v)
	{
		return this->sub(v);
	}

	virtual T add(const T& v = 1)
	{
		return this->val.fetch_add(v);
	}

	virtual T sub(const T& v)
	{
		return this->val.fetch_sub(v);
	}

	virtual T set(const T& v)
	{
		return this->val.exchange(v);
	}

	virtual T get() const
	{
		return this->val.load();
	}

	virtual int64_t count()
	{
		return this->get();
	}


private:
	std::atomic<T> val;
};

typedef TBaseCounter<uint64_t> UnsignedCounter;
typedef TBaseCounter<int64_t> SignedCounter;
typedef TBaseCounter<double> DoubleCounter;



#endif /* COUNTER_HPP_ */
