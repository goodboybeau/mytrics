
#include <thread>
#include <chrono>
#include <iostream>
#include <thread>
#include <random>


#include <counter.hpp>
#include <timer.hpp>
#include <metrics.hpp>


#include <gmock/gmock.h>
#include <gtest/gtest.h>


#define SLEEP(n) 	{ \
						auto milliwait = std::chrono::milliseconds(n); \
						std::this_thread::sleep_for(milliwait); \
					}

TEST(Timer, CanStartStop)
{
	Timer t;
	t.start();
	SLEEP(1000);
	t.stop();

	EXPECT_TRUE(t.get().count() == 1000 || t.get().count() == 1001);
}

TEST(Counter, CanCount)
{
	UnsignedCounter c;
	c.add();

	EXPECT_EQ(c.get(), 1);
}

TEST(Timer, CanAdd)
{
	Timer ta, tb;
	ta.start();
	ta.stop();

	tb.start();
	SLEEP(1000);
	tb.stop();

	std::cout << (ta - tb).count();
	std::cout << (ta + tb).count();
	ta + (ta + tb);
}

TEST(Collector, CanRegister)
{
	Collector c;
	auto t = std::make_shared<Timer>();
	auto uc = std::make_shared<UnsignedCounter>();
	c.register_metric(t);
	c.register_metric(uc);

	std::cout << c.report();
}

TEST(Collector, CanReport)
{
	Collector c;

	auto t = std::make_shared<Timer>();
	auto uc = std::make_shared<UnsignedCounter>();

	c.register_metric(t);
	c.register_metric(uc);

	uc->add();

	size_t bs;
	uint8_t* bf = c.buffer(bs);
	std::cout << "size " << bs << ": " << std::hex << (char*)bf << std::endl;
	delete [] bf;
}


TEST(StatRepo, SingletonsAreTheSame)
{
	auto repo = StatRepo::Instance("namespace");
	auto repo1 = StatRepo::Instance();

	EXPECT_EQ(repo.get(), repo1.get());

	EXPECT_EQ(repo->_namespace(), "namespace");
	EXPECT_EQ(repo1->_namespace(), "namespace");
}

TEST(StatRepo, CanProcess)
{
	auto repo = StatRepo::Instance("namespace");
	SmartUnsignedCounter ss("smart_stat");
	ss.notify();
	repo->process();
}

void create_stats(std::string name)
{
	std::default_random_engine e;
	std::uniform_int_distribution<> d(0, 1000);
	std::function<int()> rnd = std::bind(d, e); // a copy of e is stored in rnd
	auto ss = std::make_shared<SmartUnsignedCounter>(name);
	for(int n=0; n<100; ++n)
	{
		{
			ss->add(rnd());
			ss->notify();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	std::cout << '\n';
}

TEST(StatRepo, CanThreaded)
{
	StatRepo::reset();
	auto repo = StatRepo::Instance("namespace");

	repo->start();
	repo->start();

	auto creator1 = std::thread(create_stats, ("first"));
	auto creator2 = std::thread(create_stats, ("second"));
	auto creator3 = std::thread(create_stats, ("third"));
	auto creator4 = std::thread(create_stats, ("fourth"));
	auto creator5 = std::thread(create_stats, ("fifth"));

	creator1.join();
	creator2.join();
	creator3.join();
	creator4.join();
	creator5.join();

	repo->stop();

	repo->start();
	repo->stop(true);
}

TEST(StatRepo, AsItWouldGo)
{
	StatRepo::Instance("namespace")->start();

	{
		ScopedTimer s("scoped");
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	StatRepo::Instance("namespace")->stop(true);
}

TEST(UDPSocket, Send)
{
	UDPSocket s("localhost", 12345);
	std::cout << "sent: " << s.send("data") << " bytes" << std::endl;
}

TEST(Accessed, Check)
{
	UDPSocket receiver("localhost", 12345);
	auto bind_res = receiver.bind();
	std::cout << "bind() returned: " << bind_res << std::endl;

	StatRepo::Instance("namespace")->start();

	SmarterStat<UnsignedCounter> s("name");
	s->add();

	if(!bind_res)
	{
		uint8_t buff[1024];
		receiver.recv(buff, 1024, false);
		std::cout << (const char*) buff << std::endl;
	}

	StatRepo::Instance("namespace")->stop(true);
}


int main(int argc, char** argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
