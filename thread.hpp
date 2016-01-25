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

#ifndef THREAD_HPP_
#define THREAD_HPP_

#include <thread>
#include <atomic>
#include <mutex>

class Thread
{
protected:
	std::atomic_bool running;
	std::atomic_bool stop_request;
	std::thread _thread;
	std::mutex run_mutex;

public:

	Thread()
		: running(false)
		, stop_request(0)
	{}

	virtual ~Thread() = default;

	virtual void run() = 0;

	void start()
	{
		std::lock_guard<std::mutex> lk(this->run_mutex);

		bool expected = false;
		if(this->running.compare_exchange_strong(expected, true))
		{
			this->stop_request.store(false);
			this->running.store(true);
			if(this->_thread.joinable())
			{
				this->_thread.detach();
			}

			this->_thread = std::thread(&Thread::run, this);
		}
	}

	void stop(bool wait=false)
	{
		std::lock_guard<std::mutex> lk(this->run_mutex);

		bool expected = true;
		if(this->running.compare_exchange_strong(expected, false))
		{
			this->stop_request.store(true);
			if(wait)
			{
				this->_thread.join();
			}
		}
	}

	bool is_running()
	{
		return this->running.load();
	}

	const std::atomic_bool& get_running() const
	{
		return std::ref(this->running);
	}
};

#endif /* THREAD_HPP_ */
