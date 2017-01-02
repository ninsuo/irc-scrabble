
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ChannelIRC.hpp"

ChannelIRC::ChannelIRC(std::string name)
{
	this->_name = name;
	this->_limit = -1;
}

ChannelIRC::~ChannelIRC()
{

}

ChannelIRC::ChannelIRC(const ChannelIRC& chan)
{
	_name = chan._name;
	_pass = chan._pass;
	_limit = chan._limit;
	_modes = chan._modes;
	_topic = chan._topic;
	_owner = chan._owner;
	_protect = chan._protect;
	_op = chan._op;
	_halfop = chan._halfop;
	_voice = chan._voice;
	_all = chan._all;
	_game = chan._game;
}

ChannelIRC& ChannelIRC::operator=(const ChannelIRC& chan)
{
	if (this != &chan)
	{
		this->_name = chan._name;
		this->_pass = chan._pass;
		this->_limit = chan._limit;
		this->_modes = chan._modes;
		this->_topic = chan._topic;
		this->_owner = chan._owner;
		this->_protect = chan._protect;
		this->_op = chan._op;
		this->_halfop = chan._halfop;
		this->_voice = chan._voice;
		this->_all = chan._all;
		this->_game = chan._game;
	}
	return (*this);
}

std::string	ChannelIRC::getName()
{
	return (this->_name);
}

std::string	ChannelIRC::getPass()
{
	return (this->_pass);
}

int			ChannelIRC::getLimit()
{
	return (this->_limit);
}

std::string	ChannelIRC::getModes()
{
	return (this->_modes);
}

std::string	ChannelIRC::getTopic()
{
	return (this->_topic);
}

int			ChannelIRC::nick()
{
	return (this->_all.size());
}

std::string	ChannelIRC::nick(int n)
{
	std::list<UserIRC *>::iterator	it;
	int								i;

	if ((n < 1) || (n > (int)this->_all.size()))
		return (std::string(""));
	for (i = 1, it = this->_all.begin(); (it != this->_all.end()); it++, i++)
		if (i == n)
			return ((*it)->getNick());
	return (std::string(""));
}

bool	ChannelIRC::isOn(std::string nickname)
{
	std::list<UserIRC *>::iterator	it;

	for (it = this->_all.begin(); (it != this->_all.end()); it++)
		if (lowCase((*it)->getNick()) == lowCase(nickname))
			return (true);
	return (false);
}

bool	ChannelIRC::isOwner(std::string nickname)
{
	std::list<UserIRC *>::iterator	it;

	for (it = this->_owner.begin(); (it != this->_owner.end()); it++)
		if (lowCase((*it)->getNick()) == lowCase(nickname))
			return (true);
	return (false);
}

bool	ChannelIRC::isProtect(std::string nickname)
{
	std::list<UserIRC *>::iterator	it;

	for (it = this->_protect.begin(); (it != this->_protect.end()); it++)
		if (lowCase((*it)->getNick()) == lowCase(nickname))
			return (true);
	return (false);
}

bool	ChannelIRC::isOp(std::string nickname)
{
	std::list<UserIRC *>::iterator	it;

	for (it = this->_op.begin(); (it != this->_op.end()); it++)
		if (lowCase((*it)->getNick()) == lowCase(nickname))
			return (true);
	return (false);
}

bool	ChannelIRC::isHalfop(std::string nickname)
{
	std::list<UserIRC *>::iterator	it;

	for (it = this->_halfop.begin(); (it != this->_halfop.end()); it++)
		if (lowCase((*it)->getNick()) == lowCase(nickname))
			return (true);
	return (false);
}

bool	ChannelIRC::isVoice(std::string nickname)
{
	std::list<UserIRC *>::iterator	it;

	for (it = this->_voice.begin(); (it != this->_voice.end()); it++)
		if (lowCase((*it)->getNick()) == lowCase(nickname))
			return (true);
	return (false);
}

int		ChannelIRC::level(std::string nickname)
{
	if (this->isOwner(nickname) == true)
		return (5);
	if (this->isProtect(nickname) == true)
		return (4);
	if (this->isOp(nickname) == true)
		return (3);
	if (this->isHalfop(nickname) == true)
		return (2);
	if (this->isVoice(nickname) == true)
		return (1);
	return (0);
}

bool	ChannelIRC::modeSet(std::string mode)
{
	if (mode.length() != 1)
		return (false);
	if (this->_modes.find(mode) != std::string::npos)
		return (true);
	return (false);
}

bool	ChannelIRC::_addUser(UserIRC *user)
{
	if (this->isOn(user->getNick()) == true)
		return (false);
	this->_all.push_back(user);
	return (true);
}

bool	ChannelIRC::_delUser(UserIRC *user)
{
	std::list<UserIRC *>::iterator	it;

	this->_delOwner(user);
	this->_delProtect(user);
	this->_delOp(user);
	this->_delHalfop(user);
	this->_delVoice(user);
	for (it = this->_all.begin(); (it != this->_all.end()); it++)
		if ((*it) == user)
		{
			this->_all.erase(it);
			return (true);
		}
	return (false);
}

bool	ChannelIRC::_addOwner(UserIRC *user)
{
	if (this->isOn(user->getNick()) == false)
		return (false);
	if (this->isOwner(user->getNick()) == true)
		return (false);
	this->_owner.push_back(user);
	return (true);
}

bool	ChannelIRC::_delOwner(UserIRC *user)
{
	std::list<UserIRC *>::iterator	it;

	for (it = this->_owner.begin(); (it != this->_owner.end()); it++)
		if ((*it) == user)
		{
			this->_owner.erase(it);
			return (true);
		}
	return (false);
}

bool	ChannelIRC::_addProtect(UserIRC *user)
{
	if (this->isOn(user->getNick()) == false)
		return (false);
	if (this->isProtect(user->getNick()) == true)
		return (false);
	this->_protect.push_back(user);
	return (true);
}

bool	ChannelIRC::_delProtect(UserIRC *user)
{
	std::list<UserIRC *>::iterator	it;

	for (it = this->_protect.begin(); (it != this->_protect.end()); it++)
		if ((*it) == user)
		{
			this->_protect.erase(it);
			return (true);
		}
	return (false);
}

bool	ChannelIRC::_addOp(UserIRC *user)
{
	if (this->isOn(user->getNick()) == false)
		return (false);
	if (this->isOp(user->getNick()) == true)
		return (false);
	this->_op.push_back(user);
	return (true);
}

bool	ChannelIRC::_delOp(UserIRC *user)
{
	std::list<UserIRC *>::iterator	it;

	for (it = this->_op.begin(); (it != this->_op.end()); it++)
		if ((*it) == user)
		{
			this->_op.erase(it);
			return (true);
		}
	return (false);
}

bool	ChannelIRC::_addHalfop(UserIRC *user)
{
	if (this->isOn(user->getNick()) == false)
		return (false);
	if (this->isHalfop(user->getNick()) == true)
		return (false);
	this->_halfop.push_back(user);
	return (true);
}

bool	ChannelIRC::_delHalfop(UserIRC *user)
{
	std::list<UserIRC *>::iterator	it;

	for (it = this->_halfop.begin(); (it != this->_halfop.end()); it++)
		if ((*it) == user)
		{
			this->_halfop.erase(it);
			return (true);
		}
	return (false);
}

bool	ChannelIRC::_addVoice(UserIRC *user)
{
	if (this->isOn(user->getNick()) == false)
		return (false);
	if (this->isVoice(user->getNick()) == true)
		return (false);
	this->_voice.push_back(user);
	return (true);
}

bool	ChannelIRC::_delVoice(UserIRC *user)
{
	std::list<UserIRC *>::iterator	it;

	for (it = this->_voice.begin(); (it != this->_voice.end()); it++)
		if ((*it) == user)
		{
			this->_voice.erase(it);
			return (true);
		}
	return (false);
}

bool	ChannelIRC::_setTopic(std::string topic)
{
	if (topic == this->_topic)
		return (false);
	this->_topic = topic;
	return (true);
}

bool	ChannelIRC::_setPass(std::string pass)
{
	if (pass == this->_pass)
		return (false);
	this->_pass = pass;
	return (true);
}

bool	ChannelIRC::_setLimit(std::string limit)
{
	if (::atoi(limit.c_str()) == this->_limit)
		return (false);
	this->_limit = ::atoi(limit.c_str());
	return (true);
}

bool	ChannelIRC::_setMode(std::string mode)
{
	if (mode.length() != 1)
		return (false);
	if (this->_modes.find(mode) != std::string::npos)
		return (false);
	this->_modes += mode;
	return (true);
}

bool	ChannelIRC::_unsetMode(std::string mode)
{
	if (mode.length() != 1)
		return (false);
	if (this->_modes.find(mode) == std::string::npos)
		return (false);
	this->_modes.erase(this->_modes.find(mode));
	return (true);
}
