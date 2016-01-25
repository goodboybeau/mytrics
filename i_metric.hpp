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

#ifndef I_METRIC_HPP_
#define I_METRIC_HPP_

#include <string>

class IMetric
{
public:
	enum UNIT
	{
		GAUGE=0,
		COUNT=1,
		TIMER=2
	};

public:
	IMetric() = default;
	virtual ~IMetric() = default;

	virtual int64_t count() = 0;

	virtual std::string to_str() = 0;

	virtual UNIT unit() const = 0;
};


template<typename T>
class TMetric : public IMetric
{
public:
	TMetric() = default;
	virtual ~TMetric() = default;

	virtual T get() const = 0;

	virtual std::string to_str()
	{
		return std::to_string(this->count());
	}
};


#endif /* I_METRIC_HPP_ */
