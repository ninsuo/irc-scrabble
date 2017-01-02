
#ifndef __MYTHREAD_H__
#define __MYTHREAD_H__

#if defined(WIN32) || defined(WIN64)
# define WINDOWS
#endif

#include <map>

#ifdef WINDOWS
# include <windows.h>
# pragma comment(lib, "Kernel32.lib")
#else
# include <pthread.h>
#endif

#ifdef WINDOWS
	typedef std::map<int, HANDLE *>	MyThreadHandle;
#else
	typedef std::map<int, pthread_mutex_t *>	MyThreadMutex;
	typedef std::map<int, pthread_cond_t *>		MyThreadSign;
#endif

class MyThread
{

public:

	MyThread();
	~MyThread();
	MyThread(const MyThread& th);
	MyThread&		operator=(const MyThread& th);

	int				id();
	bool			launched();
	bool			isThread();

	bool			start(int (*call)(MyThread& obj, void *param), void *param);
	bool			wait();
	bool			exit();

	static bool		mutex_add(int id);
	static bool		mutex_del(int id);
	static void		mutex_clear();
	bool			mutex_trylock(int id);
	bool			mutex_lock(int id);
	bool			mutex_unlock(int id);

	static bool		sign_add(int id);
	static bool		sign_del(int id);
	static void		sign_clear();
	bool			sign_wait(int id);
	bool			sign_signal(int id);

private:

	#ifdef WINDOWS
		static DWORD WINAPI	_launch(LPVOID param);
	#else
		static void			*_launch(void *ptr);
	#endif

	#ifdef WINDOWS
		static MyThreadHandle	*_mutex;
		static MyThreadHandle	*_sign;
	#else
		static MyThreadMutex	*_mutex;
		static MyThreadSign		*_sign;
	#endif

	static int				_count;

	int				_id;
	int				(*_call)(MyThread& obj, void *param);
	void			*_param;
	bool			_launched;

	#ifdef WINDOWS
		HANDLE			_thread;
		HANDLE			_inside;
		HANDLE			_event;
	#else
		pthread_t		_thread;
		pthread_mutex_t	_inside;
	#endif

};

#endif // __MYTHREAD_H__
