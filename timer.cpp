#include "timer.hpp"


Timer::Timer(bool start)
	: is_running(false)
{
	if(start)
	{
		this->start();
	}
}

Timer::Timer(const millis& m, bool running)
	: is_running(running)
{
	if(!this->is_running.load())
	{
		this->stop_tp = NOW();
	}
	this->start_tp = (this->stop_tp - m);
}
void
Timer::start()
{
	bool expected = false;
	if(!this->is_running.compare_exchange_strong(expected, true))
	{
		return;
	}

	this->start_tp = NOW();
}

void
Timer::stop()
{
	bool expected = true;
	if(!this->is_running.compare_exchange_strong(expected, false))
	{
		return;
	}

	this->stop_tp = NOW();
}


void
Timer::elapsed(millis& dur) const
{
	if(this->is_running.load())
	{
		auto d = (ITimer::NOW() - this->start_tp);
		dur = std::chrono::duration_cast<millis>(d);
	}
	else
	{
		dur = std::chrono::duration_cast<millis>(this->stop_tp - this->start_tp);
	}
}

ITimer::millis
Timer::elapsed() const
{
	millis d;
	this->elapsed(d);
	return d;
}

ITimer::millis
Timer::get() const
{
	return this->elapsed();
}

int64_t
Timer::count()
{
	return this->elapsed().count();
}

ITimer::millis
Timer::operator+(const ITimer& t)
{
	return this->get() + t.get();
}

ITimer::millis
Timer::operator+(const millis& t)
{
	return this->get() + t;
}

ITimer::millis
Timer::operator-(const ITimer& t)
{
	return this->get() - t.get();
}

ITimer::millis
Timer::operator-(const millis& t)
{
	return this->get() - t;
}
