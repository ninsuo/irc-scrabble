
#ifndef __TIMER_HPP__
#define __TIMER_HPP__

class	Timer
{

public:
	
	Timer(int times, int secs, void (*call)(void *p1, void *p2, void *p3), void *p1, void *p2, void *p3);
	~Timer();
	Timer(const Timer& tm);
	Timer&	operator=(const Timer& tm);

	void	incTimes(int times);
	void	decTimes(int times);
	int		getTimes(void) const;

	void	incSecs(int secs);
	void	decSecs(int secs);
	int		getSecs(void) const;

	int		getMax(void) const;
	void	reset(void);

public:

	void	(*_call)(void *p1, void *p2, void *p3);
	void	*_p1;
	void	*_p2;
	void	*_p3;

private:

	int		_times;
	int		_secs;
	int		_max;

};

#endif // __TIMER_HPP__

