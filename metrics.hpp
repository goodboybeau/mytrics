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

#ifndef METRICS_HPP_
#define METRICS_HPP_

#include <stdlib.h>

#include <chrono>
#include <atomic>
#include <string>
#include <cstring>
#include <memory>
#include <set>
#include <sstream>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <utility>


#include "thread.hpp"

#include "collector.hpp"
#include "accessed.hpp"
#include "counter.hpp"
#include "timer.hpp"

template<class T>
class SmartStat : public T
{
	const std::shared_ptr<std::string> name;
	std::shared_ptr<StatRepo> repo;

public:
	SmartStat(const std::string& name)
		: name(std::make_shared<std::string>(std::move(name)))
		, repo(StatRepo::Instance())
	{
	}

	~SmartStat()
	{
	}

	static std::string format(const std::shared_ptr<std::string> name, const int64_t val, ITimer::hres_time_point time, const std::string& _namespace)
	{
		std::stringstream ss;
		ss << _namespace << "." << *name.get() << ' ' << val << ' ' << time.time_since_epoch().count() << std::endl;
		return ss.str();
	}

	void notify()
	{
		using std::placeholders::_1;
		// deferred string processing...
		SharedStatFormation stat_formation(
				std::make_shared<StatFormation>(std::bind(&SmartStat::format, this->name, this->count(), ITimer::hres_NOW(), _1)));

		this->repo->push(stat_formation);
	}

};

typedef SmartStat<UnsignedCounter> SUCounter;
typedef SmartStat<SignedCounter> SSCounter;
typedef SmartStat<Timer> STimer;

template <class T>
class OnceLivedStat : public SmartStat<T>
{
public:
	OnceLivedStat(const std::string& name)
		: SmartStat<T>(name)
	  {
	  }

	~OnceLivedStat()
	{
		this->notify();
	}
};

class ScopedTimer : OnceLivedStat<Timer>
{
public:
	ScopedTimer(const std::string& name)
		: OnceLivedStat(name)
	{
		this->start();
	}

	virtual ~ScopedTimer()
	{
		this->stop();
	}
};


template <class T>
class SmarterStat : public Accessed<SmartStat<T>>
{
public:
	SmarterStat(const std::string& name)
		: Accessed<SmartStat<T>>(name)
	{}

	virtual ~SmarterStat() {}

	/**
	 * This is called after the object has been accessed via operator->()
	 */
	virtual void post_operation()
	{
		this->notify();
	}
};

typedef SmarterStat<UnsignedCounter> SmartUnsignedCounter;

#endif /* METRICS_HPP_ */
