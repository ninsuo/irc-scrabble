
#ifndef __MYUSER_HPP__
#define __MYUSER_HPP__

#if defined(WIN32) || defined(WIN64)
# define WINDOWS
#endif

#include <string>
#include <time.h>

#ifdef WINDOWS
	#include <winsock2.h>
#endif

#include "MyThread.hpp"

class MyUser
{

public:

	MyUser();
	MyUser(const MyUser& client);
	~MyUser();

	MyUser&		operator=(const MyUser& client);
	MyUser&		operator<<(const std::string& str);
	MyUser&		operator<<(const char *str);
	MyUser&		operator<<(int n);
	MyUser&		operator<<(char c);
	char		operator[](int key) const;

	const char		*getServer() const;
	int				getPort() const;
	const char		**getIPs() const;
	const char		*getHost() const;

	bool			connect(const char *server, int port);
	bool			disconnect();
	bool			alive() const;
	
	int				send(const char *str, int size);
	int				send(const char *str);
	int				send(const std::string& str);
	int				send(char c);
	int				send(int nbr);

	int				recv(int size);
	int				recv();

	bool			streamReady();
	std::string		peekStream();
	std::string		readStream();

	bool			lineReady();
	std::string 	peekLine(int n);
	std::string 	readLine();

	bool			wordReady();
	std::string		peekWord(int n);
	std::string		peekWord(int begin, int last);
	std::string		readWord();
	std::string		readWord(int begin, int last);

	bool			charReady();
	char			peekChar();
	char			readChar();

	char			at(int key) const;
	char			charAt(int key) const;

	int				size();
	int				length();

	void			clear();
	
	unsigned int	getUptime() const;
	unsigned int	getIddle() const;

	unsigned int	getSizeRead() const;
	unsigned int	getSizeWrote() const;

	int				sendFile(std::string path);
	int				recvFile(std::string path, int size);

	int				getId() const;
	
	void			thread(void (*call)(MyUser&, void *), void *ptr);
	void			wait();

private:

	static int		_run(MyThread& obj, void *ptr);

	void			_clean();
	bool			_getProtocolIP();
	bool			_getHostAndIPs();
	bool			_sockConnect();

	char 			*_server;
	int				_port;
	int				_protocol;
	char			**_ips;
	char			*_host;
	std::string		_stream;
	bool			_alive;

	unsigned int	_uptime;
	unsigned int	_iddle;

	unsigned int	_size_read;
	unsigned int	_size_wrote;
	
	static int		_count;
	int				_id;

	#ifdef WINDOWS
		SOCKET			_sock;
	#else
		int				_sock;
	#endif

	MyThread		_thread;
	void			(*_call)(MyUser&, void *);
	void			*_ptr;

};

std::ostream&   operator<<(std::ostream& out, const MyUser& client);

#endif // __MYUSER_HPP__
