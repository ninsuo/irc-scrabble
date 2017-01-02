
#include <iostream>
#include <string>
#include <fstream>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "MyUser.hpp"

#if defined(WINDOWS)
	#include <winsock2.h>
	#pragma comment(lib, "ws2_32.lib")
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
#endif

int	MyUser::_count = 0;

MyUser::MyUser()
{
	_count += 1;
	_id = _count;
	_server = NULL;
	_port = -1;
	_protocol = -1;
	_ips = NULL;
	_host = NULL;
	_alive = false;
	_uptime = 0;
	_iddle = 0;
	_size_read = 0;
	_size_wrote = 0;
	_call = NULL;
	_ptr = NULL;
}

MyUser::MyUser(const MyUser& client)
{
	int			i;
	const char	**ptr;

	_count += 1;
	_id = _count;
	_server = NULL;
	if (client.getServer() != NULL)
	{
		_server = new char [(::strlen(client.getServer()) + 1)];
		::strcpy(_server, client.getServer());
	}
	_port = client.getPort();
	_ips = NULL;
	if (client.getIPs() != NULL)
	{
		for (ptr = client.getIPs(), i = 0; (*ptr != NULL); ptr++, i++);
		_ips = new char * [(i + 1)];
		for (ptr = client.getIPs(), i = 0; (*ptr != NULL); ptr++, i++)
		{
			(*(_ips + i)) = new char [(::strlen(*ptr) + 1)];
			::strcpy((*(_ips + i)), *ptr);
		}
		(*(this->_ips + i)) = NULL;
	}
	_host = NULL;
	if (client.getHost() != NULL)
	{
		_host = new char [(::strlen(client.getHost()) + 1)];
		::strcpy(_host, client.getHost());
	}
	_alive = false;
	_uptime = 0;
	_iddle = 0;
	_size_read = 0;
	_size_wrote = 0;
	_call = NULL;
	_ptr = NULL;
}

MyUser::~MyUser()
{
	this->_clean();
}

void			MyUser::_clean()
{
	char	**ptr;

	if (this->_server != NULL)
	{
		delete [] this->_server;
		this->_server = NULL;
	}
	this->_port = 0;
	if (this->_ips != NULL)
	{
		for (ptr = this->_ips; (*ptr != NULL); ptr++)
			delete [] *ptr;
		delete [] this->_ips;
		this->_ips = NULL;
	}
	if (this->_host != NULL)
	{
		delete [] this->_host;
		this->_host = NULL;
	}
	this->_stream.clear();
	this->_alive = false;
	if (this->_sock >= 0)
	{
		#ifdef WINDOWS
			::closesocket(this->_sock);
		#else
			::close(this->_sock);
		#endif
		this->_sock = -1;
	}
	this->_uptime = 0;
	this->_iddle = 0;
	this->_size_read = 0;
	this->_size_wrote = 0;
	this->_call = NULL;
	this->_ptr = NULL;
}

MyUser&		MyUser::operator=(const MyUser& client)
{
	int			i;
	const char	**ptr;

	if (this != &client)
	{
		this->_clean();
		if (client.getServer() != NULL)
		{
			this->_server = new char [(::strlen(client.getServer()) + 1)];
			::strcpy(this->_server, client.getServer());
		}
		this->_port = client.getPort();
		if (client.getIPs() != NULL)
		{
			for (ptr = client.getIPs(), i = 0; (*ptr != NULL); ptr++, i++);
			this->_ips = new char * [(i + 1)];
			for (ptr = client.getIPs(), i = 0; (*ptr != NULL); ptr++, i++)
			{
				(*(this->_ips + i)) = new char [(::strlen(*ptr) + 1)];
				::strcpy((*(this->_ips + i)), *ptr);
			}
			(*(this->_ips + i)) = NULL;
		}
		if (client.getHost() != NULL)
		{
			this->_host = new char[(::strlen(client.getHost()) + 1)];
			::strcpy(this->_host, client.getHost());
		}
	}
	return (*this);
}

MyUser&	MyUser::operator<<(const std::string& str)
{
	this->send(str.c_str());
	return (*this);
}

MyUser&	MyUser::operator<<(const char *str)
{
	this->send(str);
	return (*this);
}

MyUser&	MyUser::operator<<(int n)
{
	this->send(n);
	return (*this);
}

MyUser&	MyUser::operator<<(char c)
{
	this->send(c);
	return (*this);
}

char		MyUser::operator[](int key) const
{
	return (this->charAt(key));
}

const char	*MyUser::getServer() const
{
	return ((const char *)this->_server);
}

int			MyUser::getPort() const
{
	return (this->_port);
}

const char	**MyUser::getIPs() const
{
	return ((const char **)this->_ips);
}

const char	*MyUser::getHost() const
{
	return ((const char *)this->_host);
}

bool		MyUser::connect(const char *server, int port)
{
	if (this->_alive == true)
		return (false);
	if ((port <= 0) || (port > 65536))
		return (false);
	this->_port = port;
	if (this->_getProtocolIP() == false)
		return (false);
	if ((this->_server == NULL) || (::strcmp(server, this->_server) != 0))
	{
		if (server == NULL)
			server = "127.0.0.1";
		this->_server = new char [(::strlen(server) + 1)];
		strcpy(this->_server, server);
		if (this->_getHostAndIPs() == false)
			return (false);
	}
	if ((this->_ips == NULL) || (this->_sockConnect() == false))
	{
		delete [] this->_server;
		this->_server = NULL;
		return (false);
	}
	this->_stream.clear();
	this->_alive = true;
	this->_uptime = (unsigned int)time(0);
	this->_iddle = this->_uptime;
	this->_size_read = 0;
	this->_size_wrote = 0;
	return (true);
}

bool					MyUser::_getProtocolIP()
{
	#ifdef WINDOWS
		PROTOENT			*proto;
	#else
		struct protoent		*proto;
  	#endif

	proto = ::getprotobyname("ip");
	if (proto == NULL)
		return (false);
	_protocol = proto->p_proto;
	return (true);
}

bool				MyUser::_getHostAndIPs()
{
	struct hostent	*hostinfo;
	unsigned int	addr;
	char			**ptr;
	char			*tmp;
	int				i;

	if ((int)::inet_addr(this->_server) < 0)
	{
		hostinfo = ::gethostbyname(this->_server);
		if (hostinfo == NULL)
			return (false);
	}
	else
	{
		addr = ::inet_addr(this->_server);
      	hostinfo = ::gethostbyaddr((char *)&addr, sizeof(addr), AF_INET);
		if (hostinfo == NULL)
		{
			this->_ips = new char * [2];
			(*this->_ips) = new char [(::strlen(this->_server) + 1)];
				::strcpy((*this->_ips), this->_server);
			(*(this->_ips + 1)) = NULL;
		}
	}
	if (hostinfo->h_addr_list == NULL)
		return (false);
	for (ptr = hostinfo->h_addr_list, i = 0; (*ptr != NULL); ptr++, i++);
	this->_ips = new char * [(i + 1)];
	for (ptr = hostinfo->h_addr_list, i = 0; (*ptr != NULL); ptr++, i++)
	{
		tmp = ::inet_ntoa(*(struct in_addr *)((*(hostinfo->h_addr_list + i))));
		(*(this->_ips + i)) = new char [(::strlen(tmp) + 1)];
		::strcpy((*(this->_ips + i)), tmp);
	}
	(*(this->_ips + i)) = NULL;
	if (hostinfo->h_name != NULL)
	{
		this->_host = new char [(::strlen(hostinfo->h_name) + 1)];
		::strcpy(this->_host, hostinfo->h_name);
	}
	return (true);
}

bool					MyUser::_sockConnect()
{
	#ifdef WINDOWS
		SOCKADDR_IN			sd_name;
	#else
		struct sockaddr_in	sd_name;
	#endif
	char				**ptr;

	this->_sock = ::socket(PF_INET, SOCK_STREAM, this->_protocol);
	#ifdef WINDOWS
		if (this->_sock == INVALID_SOCKET)
	#else
		if (this->_sock < 0)
	#endif
		return (false);
	sd_name.sin_family = AF_INET;
	sd_name.sin_port = htons(this->_port);
	for (ptr = this->_ips; (*ptr != NULL); ptr++)
	{
		sd_name.sin_addr.s_addr = ::inet_addr(*ptr);
		#ifdef WINDOWS
			if (::connect(this->_sock, (SOCKADDR *)&sd_name, sizeof(sd_name)) == 0)
		#else
			if (::connect(this->_sock, (struct sockaddr *)&sd_name, sizeof(sd_name)) == 0)
		#endif
			return (true);
	}
	return (false);
}

bool		MyUser::disconnect()
{
	if (this->_alive == false)
		return (false);
	this->_alive = false;
	#ifdef WINDOWS
		::closesocket(this->_sock);
	#else
		::close(this->_sock);
	#endif
	this->_sock = -1;
	return (true);
}

bool		MyUser::alive() const
{
	return (this->_alive);
}

int			MyUser::send(const char *str, int size)
{
	int		k;

	if (this->_alive == false)
		return (-1);
	k = ::send(this->_sock, str, size, 0x0);
	if (k < 0)
	{
		this->disconnect();
		return (-1);
	}
	this->_iddle = (unsigned int)::time(0);
	this->_size_wrote += k;
	return (k);
}

int		MyUser::send(const char *str)
{
	if (this->_alive == false)
		return (-1);
	return (this->send(str, (int)strlen(str)));
}

int		MyUser::send(const std::string& str)
{
	if (this->_alive == false)
		return (-1);
	return (this->send(str.c_str(), (int)str.size()));
}

int		MyUser::send(char c)
{
	if (this->_alive == false)
		return (-1);
	return (this->send(&c, 1));
}

int				MyUser::send(int nbr)
{
	char		str[12];
	int			power;
	int			key;

	if (this->_alive == false)
		return (-1);
	::memset(str, 0, 12);
	if (nbr == 0)
		return (this->send("0", 1));
	key = 0;
	if (nbr < 0)
    {
		*str = '-';
		nbr *= -1;
		key++;
    }
	power = 1000000000;
	while ((nbr / power) == 0)
		power /= 10;
	for (power = power; (power != 0); power /= 10, key++)
		(*(str + key)) = (((nbr / power) % 10) + '0');
	return (this->send(str, (int)::strlen(str)));
}

int			MyUser::recv(int size)
{
	char	*buff;
	int		k;

	if (this->_alive == false)
		return (-1);
	if (size < 0)
		return (-1);
	if (size == 0)
		return (0);
	buff = new char [(size + 1)];
	buff[size] = '\0';
	k = ::recv(this->_sock, buff, size, 0x0);
	if (k <= 0)
	{
		delete [] buff;
		this->disconnect();
		return (-1);
	}
	this->_stream.append(buff, k);
	delete [] buff;
	this->_size_read += k;
	return (k);
}

int			MyUser::recv()
{
	if (this->_alive == false)
		return (-1);
	return (this->recv(1024));
}

bool			MyUser::streamReady()
{
	if (this->_stream.size() > 0)
		return (true);
	return (false);
}

std::string		MyUser::peekStream()
{
	std::string	ret;

	ret = this->_stream;
	return (ret);
}

std::string		MyUser::readStream()
{
	std::string	ret;

	ret = this->_stream;
	this->_stream.clear();
	return (ret);
}

bool			MyUser::lineReady()
{
	const char	*ptr;
	int			start;
	int			i;

	ptr = this->_stream.c_str();
	for (start = 0; ((*(ptr + start) == '\r') ||
			(*(ptr + start) == '\n')); start++);
	for (i = start; (*(ptr + i) != '\0'); i++)
		if ((*(ptr + i) == '\r') || (*(ptr + i) == '\n'))
			return (true);
	return (false);
}

std::string 	MyUser::peekLine(int n)
{
	std::string	line;
	const char	*ptr;
	int			count;
	int			start;
	int			i;
	int			j;

	if (n < 1)
		return (line);
	ptr = this->_stream.c_str();
	for (start = 0; ((*(ptr + start) == '\r') ||
			(*(ptr + start) == '\n')); start++);
	for (count = 0, i = start; (*(ptr + i) != '\0'); i++)
		if ((*(ptr + i) == '\r') || (*(ptr + i) == '\n'))
		{
			count += 1;
			if (count == n)
			{
				for (j = start; (j < i); j++)
					line += *(ptr + j);
				return (line);
			}
			for (start = i; ((*(ptr + start) == '\r') ||
					(*(ptr + start) == '\n')); start++)
				;
			i = (start - 1);
		}
	return (line);
}

std::string 	MyUser::readLine()
{
	std::string	line;
	const char	*ptr;
	int			start;
	int			i;
	int			j;

	ptr = this->_stream.c_str();
	for (start = 0; ((*(ptr + start) == '\r') ||
			(*(ptr + start) == '\n')); start++);
	for (i = start; (*(ptr + i) != '\0'); i++)
		if ((*(ptr + i) == '\r') || (*(ptr + i) == '\n'))
		{
			for (j = start; (j < i); j++)
				line += *(ptr + j);
			this->_stream.erase(0, (j + 1));
			while ((this->_stream.size() != 0) &&
					((this->_stream.at(0) == '\r') ||
					(this->_stream.at(0) == '\n')))
				this->readChar();
			return (line);
		}
	return (line);
}

bool			MyUser::wordReady()
{
	const char	*ptr;
	int			start;
	int			i;

	ptr = this->_stream.c_str();
	for (start = 0; ((*(ptr + start) == ' ') ||
			(*(ptr + start) == '\r') || (*(ptr + start) == '\n') ||
			(*(ptr + start) == '\t')); start++);
	for (i = start; (*(ptr + i) != '\0'); i++)
		if ((*(ptr + i) == ' ') || (*(ptr + i) == '\t') ||
			(*(ptr + i) == '\r') || (*(ptr + i) == '\n'))
			return (true);
	return (false);
}

std::string		MyUser::peekWord(int n)
{
	std::string	word;
	const char	*ptr;
	int			count;
	int			start;
	int			i;
	int			j;

	ptr = this->_stream.c_str();
	for (start = 0; ((*(ptr + start) == ' ') ||
			(*(ptr + start) == '\r') || (*(ptr + start) == '\n') ||
			(*(ptr + start) == '\t')); start++);
	for (count = 0, i = start; (*(ptr + i) != '\0'); i++)
		if ((*(ptr + i) == ' ') || (*(ptr + i) == '\t') ||
			(*(ptr + i) == '\r') || (*(ptr + i) == '\n'))
		{
			count += 1;
			if (count == n)
			{
				for (j = start; (j < i); j++)
					word += *(ptr + j);
				return (word);
			}
			for (start = i; ((*(ptr + start) == ' ') ||
					(*(ptr + start) == '\t')); start++);
			i = (start - 1);
		}
	return (word);
}

std::string		MyUser::peekWord(int begin, int last)
{
	std::string	word;
	int			i;

	for (i = begin; (i <= last); i++)
		word += this->peekWord(i);
	return (word);
}

std::string		MyUser::readWord()
{
	std::string	word;
	const char	*ptr;
	int			start;
	int			i;
	int			j;

	ptr = this->_stream.c_str();
	for (start = 0; ((*(ptr + start) == ' ') ||
			(*(ptr + start) == '\r') || (*(ptr + start) == '\n') ||
			(*(ptr + start) == '\t')); start++);
	for (i = start; (*(ptr + i) != '\0'); i++)
		if ((*(ptr + i) == ' ') || (*(ptr + i) == '\t') ||
			(*(ptr + i) == '\r') || (*(ptr + i) == '\n'))
		{
			for (j = start; (j < i); j++)
				word += *(ptr + j);
			this->_stream.erase(0, j);
			return (word);
		}
	return (word);
}

std::string		MyUser::readWord(int begin, int last)
{
	std::string	word;
	int			i;

	for (i = begin; (i <= last); i++)
		word += this->readWord();
	return (word);
}

bool			MyUser::charReady()
{
	if (this->_stream.size() > 0)
		return (true);
	return (false);
}

char		MyUser::peekChar()
{
	if (this->_stream.size() == 0)
		return ('\0');
	return (this->_stream.at(0));
}

char		MyUser::readChar()
{
	char	c;

	if (this->_stream.size() == 0)
		return ('\0');
	c = this->_stream.at(0);
	this->_stream.erase(0, 1);
	return (c);
}

char		MyUser::at(int key) const
{
	if ((key < 0) || (key >= (int)this->_stream.size()))
		return ('\0');
	return (this->_stream.at(key));
}

char		MyUser::charAt(int key) const
{
	return (this->at(key));
}

int			MyUser::size()
{
	return ((int)this->_stream.size());
}

int			MyUser::length()
{
	return ((int)this->_stream.size());
}

void		MyUser::clear()
{
	this->_stream.clear();
}

unsigned int	MyUser::getUptime() const
{
	if (this->_alive == false)
		return (0);
	return ((unsigned int)::time(0) - this->_uptime);
}

unsigned int	MyUser::getIddle() const
{
	if (this->_alive == false)
		return (0);
	return ((unsigned int)::time(0) - this->_iddle);
}

unsigned int	MyUser::getSizeRead() const
{
	if (this->_alive == false)
		return (0);
	return (this->_size_read);
}

unsigned int	MyUser::getSizeWrote() const
{
	if (this->_alive == false)
		return (0);
	return (this->_size_wrote);
}

int					MyUser::sendFile(std::string path)
{
    std::ifstream	ifs;
	char			line[1024];
	int				ret;
	int				act;
	int				total;

	memset(line, 0, 1024);
	ifs.open(path.c_str(), std::ios_base::binary);

	if (ifs.is_open() == true)
	{
		ifs.seekg(0, std::ios_base::end);
		act = 0;
		total = ifs.tellg();
		ifs.seekg(0, std::ios_base::beg);
		while ((act != total) && (ifs.good() != false) && (ifs.eof() != true))
		{
			ret = (((total - act) >= 1) ? (1) : (total - act));
			ifs.read(line, ret);
			ret = this->send(line, ret);
			if (ret == -1)
			{
				ifs.close();
				return (act);
			}
			act += ret;
		}
		ifs.close();
		return (total);
	}
	return (-1);
}

int					MyUser::recvFile(std::string path, int size)
{
	std::ofstream	ofs;
	int				act;
	int				ret;

	ofs.open(path.c_str(), (std::ios_base::out | std::ios_base::trunc | std::ios_base::binary));
	if (ofs.is_open() == true)
	{
		act = size;
		while (act != 0)
		{
			if (this->size() == 0)
				ret = this->recv((act > 1024) ? (1024) : (act));
			if (ret == -1)
			{
				ofs.close();
				return (size - act);
			}
			ret = ((act > (int)this->_stream.size()) ? (this->_stream.size()) : (act));
			ofs.write(this->_stream.c_str(), ret);
			this->_stream = this->_stream.substr(ret);
			act -= ret;
		}
		ofs.close();
		return (act);
	}
	return (-1);
}

int				MyUser::getId() const
{
	return (this->_id);
}

void			MyUser::thread(void (*call)(MyUser&, void *), void *ptr)
{
	this->_call = call;
	this->_ptr = ptr;
	this->_thread.start(this->_run, this);
}

void			MyUser::wait()
{
	this->_thread.wait();
}

int				MyUser::_run(MyThread& obj, void *ptr)
{
	MyThread	*o;
	o = &obj;
	MyUser	*cli = (MyUser *)ptr;
	cli->_call(*cli, cli->_ptr);
	return (0);
}

std::ostream&   operator<<(std::ostream& out, const MyUser& client)
{
	const char	**ptr;

	out << "--------------------- Client Status ---------------------" << std::endl;
	if (client.alive() == true)
	{
		out << "Status : connected on server " << client.getServer() << " (port " << client.getPort() << ")." << std::endl;
		out << "Host info: " << client.getServer() << " is " << client.getHost() << std::endl;
		out << "IPs:";
		for (ptr = client.getIPs(); (*ptr != NULL); ptr++)
			out << " " << *ptr << std::endl;
		out << "Uptime: " << (client.getUptime() / (60 * 60)) << " hours, " << ((client.getUptime() / 60) % 60) << " minutes, " << (client.getUptime() % 60) << " seconds." << std::endl;
		out << "Iddle: " << (client.getIddle() / (60 * 60)) << " hours, " << ((client.getIddle() / 60) % 60) << " minutes, " << (client.getIddle() % 60) << " seconds." << std::endl;
		out << "Read " << (client.getSizeRead()) << " bytes (" << (client.getSizeRead() / (1024 * 1024)) << "Mb, " << ((client.getSizeRead() / 1024) % 1024) << "Kb, " << (client.getSizeRead() % 1024) << "b" ")." << std::endl;
		out << "Wrote " << (client.getSizeWrote()) << " bytes (" << (client.getSizeWrote() / (1024 * 1024)) << "Mb, " << ((client.getSizeWrote() / 1024) % 1024) << "Kb, " << (client.getSizeWrote() % 1024) << "b" ")." << std::endl;
	}
	else
	{
		out << "Not connected." << std::endl;
	}
	out << "---------------------------------------------------------" << std::endl;
	return (out);
}
