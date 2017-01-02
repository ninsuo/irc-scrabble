
#include "MyTimer.hpp"

MyTimer::MyTimer()
{
	_thread.mutex_add(24400);
}

MyTimer::~MyTimer()
{
	MyTimerList::iterator it;

	if (_thread.launched() == true)
		_thread.wait();
	for (it = _timers.begin(); (it != _timers.end()); it++)
		delete ((*it).second);
}

MyTimer::MyTimer(const MyTimer& tm)
{
	MyTimerList::const_iterator it;
	Timer						*elem;

	for (it = tm._timers.begin(); (it != tm._timers.end()); it++)
	{
		elem = new Timer(*((*it).second));
		_timers[((*it).first)] = elem;
	}
}

MyTimer&							MyTimer::operator=(const MyTimer& tm)
{
	MyTimerList::iterator 			it;
	MyTimerList::const_iterator 	cit;
	Timer							*elem;

	if (this != &tm)
	{
		this->_thread.mutex_lock(24400);
		for (it = this->_timers.begin(); (it != this->_timers.end()); it++)
			delete ((*it).second);
		this->_timers.clear();
		for (cit = tm._timers.begin(); (cit != tm._timers.end()); cit++)
		{
			elem = new Timer(*((*cit).second));
			this->_timers[((*cit).first)] = elem;
		}
		this->_thread.mutex_unlock(24400);
	}
	return (*this);
}

void						MyTimer::clear()
{
	MyTimerList::iterator 	it;

	for (it = this->_timers.begin(); (it != this->_timers.end()); it++)
		this->unset((*it).first);
	return ;
}

int			MyTimer::set(int times, int secs, void (*call)(void *p1, void *p2, void *p3), void *p1, void *p2, void *p3)
{
	Timer	*elem;
	int		i;

	elem = new Timer(times, secs, call, p1, p2, p3);
	for (i = 1; (this->_timers[i] != NULL); i++);
	if (this->_thread.isThread() == false)
		this->_thread.mutex_lock(24400);
	this->_timers[i] = elem;
	if (this->_thread.isThread() == false)
		this->_thread.mutex_unlock(24400);
	if (this->_timers.size() == 1)
		this->_thread.start(this->_run, this);
	return (i);
}

bool						MyTimer::unset(int id)
{
	MyTimerList::iterator 	it;

	for (it = this->_timers.begin(); (it != this->_timers.end()); it++)
		if ((*it).first == id)
			break ;
	if (it == this->_timers.end())
		return (false);
	this->_thread.mutex_lock(24400);
	delete this->_timers[id];
	this->_timers.erase(id);
	this->_thread.mutex_unlock(24400);
	return (true);
}

int							MyTimer::_run(MyThread& obj, void *ptr)
{
	MyTimerList::iterator	it;
	Timer					*tm;
	MyTimer					*tms;
	MyThread				*tmp;

	tmp = (MyThread *)&obj;
	tms = (MyTimer *)ptr;
	while (24400)
	{
		MySleep(1000);
		tms->_thread.mutex_lock(24400);
		for (it = tms->_timers.begin(); (it != tms->_timers.end()); it++)
		{
			tm = ((*it).second);
			if (tm->getSecs() == 1)
			{
				tm->decTimes(1);
				tm->reset();
				tm->incSecs(1);
				tm->_call(tm->_p1, tm->_p2, tm->_p3);
				if (tm->getTimes() == 0)
				{
					delete tm;
					tms->_timers.erase((*it).first);
				}
				if (tms->_timers.size() == 0)
				{
					tms->_thread.mutex_unlock(24400);				
					tms->_thread.exit();
				}
			}
			tm->decSecs(1);
		}
		tms->_thread.mutex_unlock(24400);
	}
	return (0);
}
