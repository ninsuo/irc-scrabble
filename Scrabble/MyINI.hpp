
#ifndef __MYINI_HPP__
#define __MYINI_HPP__

#define	MYINI_SIZE (64 * 1024)	// maximum size of a line

#include <iostream>
#include <map>
#include <string>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

typedef std::map<std::string, std::string>	MyINIvars;
typedef std::map<std::string, MyINIvars *>	MyINItags;

class MyINI
{

public:

	MyINI(const char *file);
	MyINI(const MyINI& ini);
	~MyINI();
	MyINI&		operator=(const MyINI& ini);
	MyINIvars&	operator[](std::string tag);
	MyINI&		operator>>(const char *file);

	friend std::ostream&   operator<<(std::ostream& out, const MyINI& ini);

	void	del(std::string tag, std::string key);
	void	del(std::string tag);

private:

	bool			addTag(char *line);
	bool			addVar(char *line);

	MyINItags		tags;
	std::string		current;

};


#endif // __MYINI_HPP__
