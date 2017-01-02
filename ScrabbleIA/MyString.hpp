
#ifndef __MYSTRING_HPP__
#define __MYSTRING_HPP__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <list>

std::string left(std::string str, int n);
std::string right(std::string str, int n);
std::string	trim(std::string str, char token);
bool		is_numeric(std::string str);
int			numtok(std::string str, char token);
std::string	gettok(std::string str, int n, char token);
std::string	fulltok(std::string str, int n, char token);
std::string	lowCase(std::string str);
std::string	upCase(std::string str);
std::string	intToString(int nbr);

#ifndef __removeRandElem__
#define __removeRandElem__

template <class T>
T									removeRandElem(std::list<T>& list)
{
	class std::list<T>::iterator	it;
	T								ret;
	int								rnd;
	int								nbr;

	srand(time(0));
	rnd = (rand() % list.size());
	nbr = 0;
	for (it = list.begin(); (it != list.end()); it++)
	{
		if (nbr == rnd)
		{
			ret = (*it);
			list.erase(it);
			return (ret);
		}
		nbr++;
	}
	return (ret);
}

#endif // __removeRandElem__

#endif // __MYSTRING_HPP__
