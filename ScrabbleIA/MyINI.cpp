
#include <iostream>
#include <fstream>
#include <map>
#include <stdio.h>
#include <string.h>
#include "MyINI.hpp"

MyINI::MyINI(const char *file)
{
	std::ifstream	ifs;
	char			line[MYINI_SIZE];
	bool			correct;

	current = "";
	ifs.open(file);
	memset(line, 0, MYINI_SIZE);
	if (ifs.is_open() == true)
	{
		while (ifs.good() != false)
		{
			ifs.getline(line, MYINI_SIZE);
			if (line[0] == '#')
				continue;
			correct = (addTag(line) || addVar(line));
		}
		ifs.close();
	}
}

MyINI::~MyINI()
{
	MyINItags::iterator	it;

	for (it = tags.begin(); (it != tags.end()); it++)
		delete (*it).second;
}

MyINI::MyINI(const MyINI& ini)
{
	MyINItags::iterator	it;
	MyINIvars			*elem;
	MyINItags			itags;

	itags = ini.tags;
	for (it = itags.begin(); (it != itags.end()); it++)
	{
		elem = new MyINIvars(*((*it).second));
		tags[((*it).first)] = elem;
	}
}

MyINI&			MyINI::operator=(const MyINI& ini)
{
	MyINItags::iterator	it;
	MyINIvars			*elem;
	MyINItags			itags;

	if (this != &ini)
	{
		for (it = tags.begin(); (it != tags.end()); it++)
			delete (*it).second;
		itags = ini.tags;
		for (it = itags.begin(); (it != itags.end()); it++)
		{
			elem = new MyINIvars(*((*it).second));
			tags[((*it).first)] = elem;
		}
	}
	return (*this);
}

MyINIvars&		MyINI::operator[](std::string tag)
{
	MyINIvars	*elem;

	if (tags[tag] == NULL)
	{
		elem = new MyINIvars;
		tags[tag] = elem;
		return (*elem);
	}
	return (*(tags[tag]));
}

MyINI&		MyINI::operator>>(const char *file)
{
	MyINItags::iterator	it_tags;
	MyINIvars::iterator	it_vars;
	MyINIvars			ivars;
	std::ofstream		ofs;

	ofs.open(file, std::ios_base::out | std::ios_base::trunc );
	if (ofs.is_open() == true)
	{
		for (it_tags = tags.begin(); (it_tags != tags.end()); it_tags++)
		{
			ofs << "[" << ((*it_tags).first) << "]" << std::endl;
			ivars = (*((*it_tags).second));
			for (it_vars = ivars.begin(); (it_vars != ivars.end()); it_vars++)
				ofs << ((*it_vars).first) << "=" << ((*it_vars).second) << std::endl;
		}
		ofs.close();
	}
	return (*this);
}

void	MyINI::del(std::string tag, std::string key)
{
	if (tags[tag] != NULL)
		(*(tags[tag])).erase(key);
}

void	MyINI::del(std::string tag)
{
	if (tags[tag] != NULL)
	{
		delete tags[tag];
		tags.erase(tag);
	}
}

bool			MyINI::addTag(char *line)
{
	MyINIvars	*elem;
	int			i;
	int			recur;

	if (line[0] == '[')
	{
		for (recur = 0, i = 0; (line[i] != '\0'); i++)
		{
			if (line[i] == '[')
				recur++;
			else if (line[i] == ']')
			{
				recur--;
				if (recur == 0)
				{
					if (line[(i + 1)] != '\0')
						return (false);
					line[i] = '\0';
					if (tags[(line + 1)] == NULL)
					{
						elem = new MyINIvars;
						tags[(line + 1)] = elem;
						current = (line + 1);
					}
					return (true);
				}
			}
		}
	}
	return (false);
}

bool			MyINI::addVar(char *line)
{
	int			i;

	if (current == "")
		return (false);
	for (i = 0; (line[i] != '\0'); i++)
		if (line[i] == '=')
		{
			line[i] = '\0';
			(*(tags[current]))[(line)] = (line + i + 1);
			return (true);
		}
	return (false);
}

std::ostream&   operator<<(std::ostream& out, const MyINI& ini)
{
	MyINItags::iterator	it_tags;
	MyINIvars::iterator	it_vars;
	MyINItags			itags;
	MyINIvars			ivars;

	itags = ini.tags;
	for (it_tags = itags.begin(); (it_tags != itags.end()); it_tags++)
	{
		out << "[" << ((*it_tags).first) << "]" << std::endl;
		ivars = (*((*it_tags).second));
		for (it_vars = ivars.begin(); (it_vars != ivars.end()); it_vars++)
			out << ((*it_vars).first) << "=" << ((*it_vars).second) << std::endl;
	}
	return (out);
}
