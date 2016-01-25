#ifndef _COLLECTOR_HPP_
#define _COLLECTOR_HPP_

#include "singleton.hpp"
#include "metrics.hpp"
#include "network.hpp"

namespace
{
	typedef std::function<std::string(const std::string&)> StatFormation;
	typedef std::shared_ptr<StatFormation> SharedStatFormation;
}

class Collector
{
public:
	Collector()
		: reporting(std::make_shared<std::atomic_bool>())
	{}
	virtual ~Collector() {}

	virtual void register_metric( std::shared_ptr<IMetric> metric)
	{
		std::unique_lock<std::mutex> thread_common_lock(this->m);

		// bind this Collector's reporting flag to the condition variable's wake condition
		std::function<bool()> wake_okay = std::bind([](std::shared_ptr<std::atomic_bool> ab){ return !ab->load(); }, this->reporting);

		this->cv.wait(thread_common_lock, wake_okay);

		this->metrics.insert(metric);
	}

	virtual std::string report()
	{
		std::stringstream ss;

		this->reporting->store(true);

		{
			std::lock_guard<std::mutex> thread_common_lock(this->m);

			for(auto iter = this->metrics.begin(); iter != this->metrics.end(); iter++)
			{
				ss << (*iter)->count() << " " << ( (*iter)->unit() == IMetric::UNIT::TIMER ? "ms" : "c" ) << std::endl;
			}
		}

		this->reporting->store(false);
		this->cv.notify_one();

		return ss.str();
	}

	virtual uint8_t* buffer(size_t& buff_size)
	{
		uint8_t* byte_stream;
		uint8_t* ptr;

		this->reporting->store(true);

		{
			std::lock_guard<std::mutex> thread_common_lock(this->m);

			buff_size = this->metrics.size() * (8 + 8) + 1; // data-size + data-type + NULL
			byte_stream = new uint8_t[buff_size]; /* @TODO don't assume architecture
			 	 	 	 	 	 	 	 	 	 	 @TODO don't assume "new" succeeds */
			ptr = byte_stream;
			memset((void*)ptr, 0, buff_size);

			for(auto iter = this->metrics.begin(); iter != this->metrics.end(); iter++)
			{
				*((uint64_t*)(ptr)) = (*iter)->count();
				ptr += 8; /* @TODO don't assume architecture */
				*((uint64_t*)(ptr)) = (uint64_t)(*iter)->unit();
				ptr += 8;
			}

			ptr = nullptr;
		}

		this->reporting->store(false);
		this->cv.notify_one();

		return byte_stream;
	}

private:
	std::set<std::shared_ptr<IMetric>> metrics;
	std::mutex m;
	std::condition_variable cv;
	std::shared_ptr<std::atomic_bool> reporting;
};


class Repository
{
protected:
	std::string ns;
	std::queue<SharedStatFormation> queue;

	virtual ~Repository() {}

public:
	Repository(const std::string& _namespace = "")
		: ns(_namespace)
	{

	}

	virtual void push(SharedStatFormation& formatter)
	{
		this->queue.push(formatter);
	}

	virtual const std::string _namespace() const
	{
		return this->ns;
	}
};

class ThreadSafeRepo : public Repository
{
protected:

	std::mutex write_mutex;
	std::condition_variable cv;
	std::shared_ptr<std::atomic_bool> has_stats;

protected:
	ThreadSafeRepo(const std::string& _namespace)
			: Repository(_namespace)
			, has_stats(std::make_shared<std::atomic_bool>(false))
		{

		}
public:

	virtual ~ThreadSafeRepo() {}

	virtual void push(SharedStatFormation& formatter)
	{
		std::lock_guard<std::mutex> lk(this->write_mutex);
		this->Repository::push(formatter);
		this->has_stats->store(true);
		this->cv.notify_one();
	}

	virtual SharedStatFormation pop()
	{
		std::unique_lock<std::mutex> lk(this->write_mutex);
		this->cv.wait(lk, std::bind([](std::shared_ptr<std::atomic_bool> con){return con->load(); }, this->has_stats));

		auto func = this->queue.front();
		this->queue.pop();

		return func;
	}
};

class StatRepo : public ThreadSafeRepo, public Singleton<StatRepo>, public Thread
{
	int pcount;
	UDPSocket sock;
public:
	StatRepo(const std::string& _namespace)
		: ThreadSafeRepo(_namespace)
		, pcount(0)
		, sock("localhost", 12345)
	{
	}

	virtual void process()
	{
		while(!this->queue.empty())
		{
			pcount++;
			auto f = this->pop();
			std::string fstat = f->operator ()(this->ns);
			sock.send(fstat);
		}
		this->has_stats->store(false);
	}

	virtual void run()
	{
		int count = 0;
		while(!this->stop_request.load() || !this->queue.empty())
		{
			count ++;
			this->process();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		this->running.store(false);

		std::cout << "ran " << std::to_string(count) << " times " << std::endl;
		std::cout << "popped " << std::to_string(pcount) << " times " << std::endl;
	}
};

#endif /* _COLLECTOR_HPP_ */
