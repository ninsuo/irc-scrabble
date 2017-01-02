
#include "UserIRC.hpp"
#include "ChannelIRC.hpp"

UserIRC::UserIRC(std::string nickname)
{
	_nickname = nickname;
	_ircop = false;
}

UserIRC::~UserIRC()
{
	
}

UserIRC::UserIRC(const UserIRC& user)
{
	_nickname = user._nickname;
	_username = user._username;
	_hostname = user._hostname;
	_fullname = user._fullname;
	_channels = user._channels;
	_ircop = user._ircop;
}

UserIRC& UserIRC::operator=(const UserIRC& user)
{
	if (this != &user)
	{
		this->_nickname = user._nickname;
		this->_username = user._username;
		this->_hostname = user._hostname;
		this->_fullname = user._fullname;
		this->_channels = user._channels;
		this->_ircop = user._ircop;
	}
	return (*this);
}

std::string	UserIRC::getNick()
{
	return (this->_nickname);
}

std::string	UserIRC::getUser()
{
	return (this->_username);
}

std::string	UserIRC::getHost()
{
	return (this->_hostname);
}

std::string	UserIRC::getFull()
{
	return (this->_fullname);
}

bool		UserIRC::isIrcop()
{
	return (this->_ircop);
}

int			UserIRC::commonChannels()
{
	return (this->_channels.size());
}

std::string	UserIRC::commonChannel(int n)
{
	std::list<ChannelIRC *>::iterator	it;
	int									i;

	if ((n < 1) || (n > (int)this->_channels.size()))
		return ("");
	for (i = 1, it = this->_channels.begin();
		(it != this->_channels.end()); it++, i++)
		if (i == n)
			return (lowCase((*it)->getName()));
	return ("");
}

bool		UserIRC::isOn(std::string channel)
{
	std::list<ChannelIRC *>::iterator	it;

	for (it = this->_channels.begin();
		(it != this->_channels.end()); it++)
		if (lowCase((*it)->getName()) == lowCase(channel))
			return (true);
	return (false);
}

bool		UserIRC::isOwner(std::string channel)
{
	std::list<ChannelIRC *>::iterator	it;

	for (it = this->_channels.begin();
		(it != this->_channels.end()); it++)
		if (lowCase((*it)->getName()) == lowCase(channel))
			return ((*it)->isOwner(this->_nickname));
	return (false);
}

bool		UserIRC::isProtect(std::string channel)
{
	std::list<ChannelIRC *>::iterator	it;

	for (it = this->_channels.begin();
		(it != this->_channels.end()); it++)
		if (lowCase((*it)->getName()) == lowCase(channel))
			return ((*it)->isProtect(this->_nickname));
	return (false);	
}

bool		UserIRC::isOp(std::string channel)
{
	std::list<ChannelIRC *>::iterator	it;

	for (it = this->_channels.begin();
		(it != this->_channels.end()); it++)
		if (lowCase((*it)->getName()) == lowCase(channel))
			return ((*it)->isOp(this->_nickname));
	return (false);
}

bool		UserIRC::isHalfop(std::string channel)
{
	std::list<ChannelIRC *>::iterator	it;

	for (it = this->_channels.begin();
		(it != this->_channels.end()); it++)
		if (lowCase((*it)->getName()) == lowCase(channel))
			return ((*it)->isHalfop(this->_nickname));
	return (false);
}

bool		UserIRC::isVoice(std::string channel)
{
	std::list<ChannelIRC *>::iterator	it;

	for (it = this->_channels.begin();
		(it != this->_channels.end()); it++)
		if (lowCase((*it)->getName()) == lowCase(channel))
			return ((*it)->isVoice(this->_nickname));
	return (false);
}

bool		UserIRC::_setNickname(std::string nickname)
{
	if (this->_nickname == nickname)
		return (false);
	this->_nickname = nickname;
	return (true);
}

bool		UserIRC::_setUsername(std::string username)
{
	if (this->_username == username)
		return (false);
	this->_username = username;
	return (true);
}

bool		UserIRC::_setHostname(std::string hostname)
{
	if (this->_hostname == hostname)
		return (false);
	this->_hostname = hostname;
	return (true);
}

bool		UserIRC::_setFullname(std::string fullname)
{
	if (this->_fullname == fullname)
		return (false);
	this->_fullname = fullname;
	return (true);	
}

bool		UserIRC::_setIrcop(bool ircop)
{
	if (this->_ircop == ircop)
		return (false);
	this->_ircop = ircop;
	return (true);	
}

bool		UserIRC::_addChannel(ChannelIRC *chan)
{
	if (this->isOn(chan->getName()) == true)
		return (false);
	this->_channels.push_back(chan);
	return (true);
}

bool		UserIRC::_delChannel(ChannelIRC *chan)
{
	std::list<ChannelIRC *>::iterator	it;

	for (it = this->_channels.begin();
		(it != this->_channels.end()); it++)
		if ((*it) == chan)
		{
			this->_channels.erase(it);
			return (true);
		}
	return (false);
}
