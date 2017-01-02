
#include <string>
#include <list>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "MyString.hpp"


std::string	left(std::string str, int n)
{
	std::string ret;
	int			i;

	if ((n <= 0) || (n > (int)str.length()))
		return (ret);
	for (i = 0; ((i < (int)str.length()) && (i < n)); i++)
		ret += str.at(i);
	return (ret);
}

std::string	right(std::string str, int n)
{
	std::string	ret;
	int			i;

	if ((n <= 0) || (n > (int)str.length()))
		return (ret);
	for (i = ((int)str.length() - n); (i < (int)str.length()); i++)
		ret += str.at(i);
	return (ret);
}

std::string	trim(std::string str, char token = ' ')
{
	while ((str.size() > 0) && (str.at(0) == token))
		str = str.substr(1, (str.size() - 1));
	while ((str.size() > 0) && (str.at(str.length() - 1) == token))
		str = str.substr(0, (str.size() - 2));
	return (str);
}

bool		is_numeric(std::string str)
{
	int		i;
	bool	s;

	s = false;
	for (i = 0; (i < (int)str.length()); i++)
	{
		if ((str.at(i) == '-') || (str.at(i) == '+'))
		{
			if (s == true)
				return (false);
		}
		else if ((str.at(i) < '0') || (str.at(i) > '9'))
				return (false);
		else
			s = true;
	}
	return (true);
}

int			numtok(std::string str, char token)
{
	int		i;
	int		size;
	int		num;

	str = trim(str, token);
	size = (int)str.size();
	for (i = 0; ((i < size) && (str.at(i) == token)); i++);
	for (num = 0; (i < size); i++)
	{
		if ((str.at(i) == token) || ((i + 1) == size))
		{
			num++;
			if ((i + 1) == size)
				break ;
			while ((i < size) && (str.at(i) == token))
				i++;
			i--;
		}
	}
	return (num);
}

std::string		gettok(std::string str, int n, char token)
{
	std::string	result;
	int			i;
	int			size;
	int			num;
	int			start;

	str = trim(str, token);
	size = (int)str.size();
	for (i = 0; ((i < size) && (str.at(i) == token)); i++);
	for (num = 0, start = i; (i < size); i++)
	{
		if ((str.at(i) == token) || ((i + 1) == size))
		{
			num++;
			if ((i + 1) == size)
				i++;
			if (num == n)
			{
				result = left(right(str, size - start), (i - start));
				break ;
			}
			while ((i < size) && (str.at(i) == token))
				i++;
			start = i;
			i--;
		}
	}
	return (result);
}

std::string		fulltok(std::string str, int n, char token)
{
	std::string	result;
	int			i;
	int			size;
	int			num;

	size = (int)str.size();
	for (i = 0; ((i < size) && (str.at(i) == token)); i++);
	for (num = 1; (i < size); i++)
	{
		if ((str.at(i) == token) || ((i + 1) == size))
		{
			num++;
			if (num == n)
			{
				i++;
				result = right(str, size - i);
				break ;
			}
			while ((i < size) && (str.at(i) == token))
				i++;
			i--;
		}
	}
	return (result);
}

std::string		lowCase(std::string str)
{
	std::string	ret;
	int			i;

	for (i = 0; (i < (int)str.length()); i++)
		if ((str.at(i) >= 'A') && (str.at(i) <= 'Z'))
			ret += (str.at(i) - 'A' + 'a');
		else
			ret += str.at(i);
	return (ret);
}

std::string		upCase(std::string str)
{
	std::string	ret;
	int			i;

	for (i = 0; (i < (int)str.length()); i++)
		if ((str.at(i) >= 'a') && (str.at(i) <= 'z'))
			ret += (str.at(i) - 'a' + 'A');
		else
			ret += str.at(i);
	return (ret);
}

std::string	intToString(int nbr)
{
	std::string	ret;
	char		str[12];
	int			power;
	int			key;

	::memset(str, 0, 12);
	if (nbr == 0)
	{
		ret = "0";
		return (ret);
	}
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
	ret = str;
	return (ret);
}
