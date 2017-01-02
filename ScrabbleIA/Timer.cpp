
#include "Timer.hpp"

Timer::Timer(int times, int secs, void (*call)(void *p1, void *p2, void *p3), void *p1, void *p2, void *p3)
{
	if (times < 0)
		times = 1;
	_times = times;
	if (secs < 0)
		secs = 1;
	_max = secs;
	_secs = secs;
	_call = call;
	_p1 = p1;
	_p2 = p2;
	_p3 = p3;
}

Timer::~Timer() { }

Timer::Timer(const Timer& tm)
{
	_times = tm.getTimes();
	_max = tm.getMax();
	_secs = tm.getSecs();
	_call = tm._call;
	_p1 = tm._p1;
	_p2 = tm._p2;
	_p3 = tm._p3;
}

Timer&	Timer::operator=(const Timer& tm)
{
	if (this != &tm)
	{
		this->_times = tm.getTimes();
		this->_max = tm.getMax();
		this->_secs = tm.getSecs();
		this->_call = tm._call;
		this->_p1 = tm._p1;
		this->_p2 = tm._p2;
		this->_p3 = tm._p3;
	}
	return (*this);
}

void	Timer::incTimes(int times)
{
	this->_times += times;
	if (this->_times < 0)
		this->_times = 0;
}

void	Timer::decTimes(int times)
{
	this->_times -= times;
	if (this->_times < 0)
		this->_times = 0;
}

int		Timer::getTimes(void) const
{
	return (this->_times);
}

void	Timer::incSecs(int secs)
{
	this->_secs += secs;
	if (this->_secs < 0)
		this->_secs = 0;
}

void	Timer::decSecs(int secs)
{
	this->_secs -= secs;
	if (this->_secs < 0)
		this->_secs = 0;
}

int		Timer::getSecs(void) const
{
	return (this->_secs);
}

int		Timer::getMax(void) const
{
	return (this->_max);
}

void	Timer::reset(void)
{
	this->_secs = this->_max;	
}
