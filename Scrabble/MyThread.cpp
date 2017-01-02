
#include <iostream>

#include "MyThread.hpp"

#ifdef WINDOWS
# include <windows.h>
# pragma comment(lib, "Kernel32.lib")
#else
# include <pthread.h>
# include <signal.h>
#endif

#ifdef WINDOWS
	MyThreadHandle	*MyThread::_mutex = NULL;
	MyThreadHandle	*MyThread::_sign = NULL;
#else
	MyThreadMutex	*MyThread::_mutex = NULL;
	MyThreadSign	*MyThread::_sign = NULL;
#endif

int				MyThread::_count = 0;

MyThread::MyThread()
{
#ifdef WINDOWS
	if (_mutex == NULL)
		_mutex = new MyThreadHandle;
	if (_sign == NULL)
		_sign = new MyThreadHandle;
#else
	if (_mutex == NULL)
		_mutex = new MyThreadMutex;
	if (_sign == NULL)
		_sign = new MyThreadSign;
#endif
	_count += 1;
	_launched = false;
	_id = _count;
	_call = NULL;
	_param = NULL;
	#ifdef WINDOWS
		HANDLE	mutex;
		mutex = ::CreateMutex(NULL, false, NULL);
		_event = ::CreateEvent(NULL, 0, 0, NULL); 
	#else
		pthread_mutexattr_t attr;
		::pthread_mutexattr_init(&attr);
		::pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
		pthread_mutex_t mutex;
		::pthread_mutex_init(&mutex, &attr);
		::pthread_mutexattr_destroy(&attr);
	#endif
	_inside = mutex;
}

MyThread::~MyThread()
{
	if (this->_launched == true)
	{
		if (this->isThread() == true)
			this->exit();
		else
			this->wait();
	}
	#ifdef WINDOWS
		::CloseHandle(this->_inside);
		::CloseHandle(this->_event);	
		::CloseHandle(this->_thread);
	#else
		pthread_mutex_destroy(&(this->_inside));
	#endif
}

MyThread::MyThread(const MyThread& th)
{
	const MyThread	*ptr;

	ptr = &th;
	_count += 1;
	_launched = false;
	_id = _count;
	_call = NULL;
	_param = NULL;
	#ifdef WINDOWS
		HANDLE	mutex;
		mutex = ::CreateMutex(NULL, false, NULL);
		_event = ::CreateEvent(NULL, 0, 0, NULL); 
	#else
		pthread_mutexattr_t attr;
		::pthread_mutexattr_init(&attr);
		::pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
		pthread_mutex_t mutex;
		::pthread_mutex_init(&mutex, &attr);
		::pthread_mutexattr_destroy(&attr);
	#endif
	_inside = mutex;
}

MyThread& 		MyThread::operator=(const MyThread& th)
{
	if (this != &th)
	{
		if (this->_launched == true)
		{
			if (this->isThread() == false)
				this->wait();
			else
				this->exit();
		}
		this->_launched = false;
		this->_call = NULL;
		this->_param = NULL;
	}
	return (*this);
}

int		MyThread::id()
{
	return (this->_id);
}

bool	MyThread::launched()
{
	return (this->_launched);
}

bool	MyThread::isThread()
{
	if (this->_launched == false)
		return (false);
	#ifdef WINDOWS
		if (::ReleaseMutex(this->_inside) == 0)
			return (false);
		::WaitForSingleObject(this->_inside, INFINITE);
	#else
		if (::pthread_mutex_unlock(&(this->_inside)) != 0)
			return (false);
		::pthread_mutex_lock(&(this->_inside));
	#endif
	return (true);
}

#ifdef WINDOWS
DWORD WINAPI		MyThread::_launch(LPVOID param)
#else
void				*MyThread::_launch(void *param)
#endif
{
	MyThread		*obj;
	unsigned int	ret;

	obj = (MyThread *)param;
	#ifdef WINDOWS
		::ResetEvent(obj->_event);
		::WaitForSingleObject(obj->_inside, INFINITE);
	#else
		::pthread_mutex_lock(&(obj->_inside));
	#endif
	ret = obj->_call((*obj), obj->_param);
	#ifdef WINDOWS
		::SetEvent(obj->_event);
		::ReleaseMutex(&(obj->_inside));
	#else
		::pthread_mutex_unlock(&(obj->_inside));
	#endif
	obj->_launched = false;
	#ifdef WINDOWS
		return (1);
	#else
		return ((void *)ret);
	#endif
}

bool			MyThread::start(int (*call)(MyThread& obj, void *param), void *param)
{
	if (this->_launched == true)
		return (false);
	this->_call = call;
	this->_param = param;
	#ifdef WINDOWS
		DWORD id;
		if (::CreateThread(NULL, 0, this->_launch, this, 0, &id) == 0)
			return (false);
	#else
		if (::pthread_create(&(this->_thread), NULL, this->_launch, this) != 0)
			return (false);
	#endif
	this->_launched = true;
	return (true);
}

bool			MyThread::wait()
{
	if (this->_launched == false)
		return (false);
	if (this->isThread() == true)
		return (false);
	#ifdef WINDOWS
		if (::WaitForSingleObject(this->_event, INFINITE) == 0)
			return (false);
	#else
		if (::pthread_join(this->_thread, NULL) != 0)
			return (false);
	#endif
	return (true);
}

bool			MyThread::exit()
{
	if (this->_launched == false)
		return (false);
	if (this->isThread() == false)
		return (false);
	#ifdef WINDOWS
		::SetEvent(this->_event);
		::ReleaseMutex(&(this->_inside));
	#else
		::pthread_mutex_unlock(&(this->_inside));
	#endif
	this->_launched = false;
	this->_call = NULL;
	this->_param = NULL;
	#ifdef WINDOWS
		DWORD	status;
		status = 1;
		::ExitThread(status);
	#else
		::pthread_exit(NULL);
	#endif
	return (true);
}

bool						MyThread::mutex_add(int id)
{
	MyThreadMutex::iterator	it;

	#ifdef WINDOWS
		HANDLE			*mutex;
	#else
		pthread_mutex_t *mutex;
	#endif

	if ((*_mutex)[id] == NULL)
		return (false);
	#ifdef WINDOWS
		mutex = new HANDLE;
		if (((*mutex) = ::CreateMutex(NULL, false, NULL)) == NULL)
	#else
		mutex = new pthread_mutex_t;
		if (::pthread_mutex_init(mutex, NULL) != 0)
	#endif
		{
			delete mutex;
			return (false);
		}
	(*_mutex)[id] = mutex;
	return (true);
}

bool			MyThread::mutex_del(int id)
{
	if ((*_mutex)[id] == NULL)
		return (false);
	#ifdef WINDOWS
		::ReleaseMutex(*((*_mutex)[id]));
		::CloseHandle(*((*_mutex)[id]));
	#else
		::pthread_mutex_unlock((*_mutex)[id]);
		::pthread_mutex_destroy((*_mutex)[id]);
	#endif
	delete (*_mutex)[id];
	(*_mutex).erase(id);
	return (true);
}

void						MyThread::mutex_clear()
{
	#ifdef WINDOWS
		MyThreadHandle::iterator mut;
	#else
		MyThreadMutex::iterator mut;
	#endif

	for (mut = (*_mutex).begin(); (mut != (*_mutex).end()); mut++)
	{
		#ifdef WINDOWS
			::ReleaseMutex(*((*mut).second));
			::CloseHandle(*((*mut).second));
		#else
			::pthread_mutex_unlock((*mut).second);
			::pthread_mutex_destroy((*mut).second);
		#endif
		delete ((*mut).second);
	}
	(*_mutex).clear();
}

bool			MyThread::mutex_trylock(int id)
{
	if ((*_mutex)[id] == NULL)
		return (false);
	#ifdef WINDOWS
		if (::WaitForSingleObject(*((*_mutex)[id]), 0) != WAIT_OBJECT_0)
	#else
		if (::pthread_mutex_trylock((*_mutex)[id]) != 0)
	#endif
		return (false);
	return (true);
}

bool			MyThread::mutex_lock(int id)
{
	if ((*_mutex)[id] == NULL)
		return (false);
	#ifdef WINDOWS
		if (::WaitForSingleObject(*((*_mutex)[id]), INFINITE) != WAIT_OBJECT_0)
	#else
		if (::pthread_mutex_lock((*_mutex)[id]) != 0)
	#endif
		return (false);
	return (true);
}

bool			MyThread::mutex_unlock(int id)
{
	if ((*_mutex)[id] == NULL)
		return (false);
	#ifdef WINDOWS
		if (::ReleaseMutex(*((*_mutex)[id])) == 0)
	#else
		if (::pthread_mutex_unlock((*_mutex)[id]) != 0)
	#endif
		return (false);
	return (true);
}

bool				MyThread::sign_add(int id)
{
	#ifdef WINDOWS
		HANDLE			*signal;
	#else
		pthread_cond_t	*signal;
	#endif

	if ((*_sign)[id] != NULL)
		return (false);
	#ifdef WINDOWS
		signal = new HANDLE;
		if (((*signal) = ::CreateEvent(NULL, 0, 0, NULL)) == NULL)
	#else
		signal = new pthread_cond_t;
		if (::pthread_cond_init(signal, NULL) != 0)
	#endif
	{
		delete signal;
		return (false);
	}
	(*_sign)[id] = signal;
	return (true);
}

bool			MyThread::sign_del(int id)
{
	if ((*_sign)[id] == NULL)
		return (false);
	#ifdef WINDOWS
		::SetEvent(*((*_sign)[id]));
		::CloseHandle(*((*_sign)[id]));
	#else
    	::pthread_cond_broadcast((*_sign)[id]);
		::pthread_cond_destroy((*_sign)[id]);
	#endif
	delete (*_sign)[id];
	(*_sign).erase(id);
	return (true);
}

void						MyThread::sign_clear()
{
	#ifdef WINDOWS
		MyThreadHandle::iterator	sign;
	#else
		MyThreadSign::iterator	sign;
	#endif

	for (sign = (*_sign).begin(); (sign != (*_sign).end()); sign++)
	{
		#ifdef WINDOWS
			::SetEvent(*((*sign).second));
			::CloseHandle(*((*sign).second));
		#else
	   		::pthread_cond_broadcast((*sign).second);
			::pthread_cond_destroy((*sign).second);
		#endif
		delete ((*sign).second);
	}
	(*_sign).clear();
}

bool				MyThread::sign_wait(int id)
{
	#ifndef WINDOWS
		pthread_mutex_t mutex;
	#endif

	if (_launched == false)
		return (false);
	if ((*_sign)[id] == NULL)
		return (false);
	#ifdef WINDOWS
		::WaitForSingleObject(*((*_sign)[id]), INFINITE);
	#else
		::pthread_mutex_init(&mutex, NULL);
		::pthread_mutex_lock(&mutex);
		::pthread_cond_wait((*_sign)[id], &mutex);
	#endif
	return (true);
}

bool			MyThread::sign_signal(int id)
{
	if (_launched == false)
		return (false);
	if ((*_sign)[id] == NULL)
		return (false);
	#ifdef WINDOWS
		::SetEvent(*((*_sign)[id]));
	#else
		::pthread_cond_broadcast((*_sign)[id]);
	#endif
	return (true);
}

