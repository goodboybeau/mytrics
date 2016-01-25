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

#ifndef TIMER_HPP_
#define TIMER_HPP_

#include <chrono>
#include <atomic>

#include "i_metric.hpp"


class ITimer : public TMetric<std::chrono::milliseconds>
{
public:

	typedef std::chrono::time_point<std::chrono::high_resolution_clock> hres_time_point;
	typedef std::chrono::milliseconds millis;

	static hres_time_point hres_NOW()
	{
		hres_time_point n;
		NOW(n);
		return n;
	}

	template<class T>
	static void NOW(std::chrono::time_point<T>& now)
	{
		now = T::now();
	}

	hres_time_point
	NOW() const
	{
		hres_time_point n;
		NOW(n);
		return n;
	}

	ITimer() = default;
	virtual ~ITimer() = default;

	virtual void start() = 0;
	virtual void stop() = 0;

	virtual void elapsed(millis& dur) const = 0;
	virtual millis elapsed() const = 0;

	virtual millis operator+(const ITimer& ) = 0;
	virtual millis operator+(const millis& ) = 0;

	virtual millis operator-(const ITimer& ) = 0;
	virtual millis operator-(const millis& ) = 0;

	virtual IMetric::UNIT unit() const
	{
		return IMetric::UNIT::TIMER;
	}

private:

};


class Timer : public ITimer
{
public:
	Timer(bool start = false);
	Timer(const Timer& t) = default;
	Timer(const millis& m, bool running = true);

	virtual void start();
	virtual void stop();
	virtual void elapsed(millis& dur) const;
	virtual millis elapsed() const;
	virtual millis get() const;

	virtual int64_t count();


	virtual millis operator+(const ITimer& );
	virtual millis operator+(const millis& );

	virtual millis operator-(const ITimer& );
	virtual millis operator-(const millis& );


private:
	hres_time_point start_tp;
	hres_time_point stop_tp;
	std::atomic<bool> is_running;
};

#endif /* TIMER_HPP_ */
