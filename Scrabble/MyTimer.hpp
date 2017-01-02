
#ifndef __MYTIMER_HPP__
#define __MYTIMER_HPP__

#include "MyThread.hpp"
#include "Timer.hpp"

#if defined(WIN32) || defined(WIN64)
# define WINDOWS
#endif

#ifdef WINDOWS
# include <windows.h>
# define MySleep(n) Sleep(n)
#else
# include <unistd.h>
# define MySleep(n) usleep((n * 1000))
#endif

#include <map>

typedef	std::map<int, Timer *>	MyTimerList;

class	MyTimer
{

	friend class Game;

public:

	MyTimer();
	~MyTimer();
	MyTimer(const MyTimer& tm);
	MyTimer&	operator=(const MyTimer& tm);

	void	clear();
	int		set(int times, int secs, void (*call)(void *p1, void *p2, void *p3), void *p1, void *p2, void *p3);
	bool	unset(int id);

private:

	static int		_run(MyThread& obj, void *ptr);

	MyThread		_thread;
	MyTimerList		_timers;

};

#endif // __MYTIMER_HPP__
