
#ifndef __USERIRC_HPP__
#define __USERIRC_HPP__

#include "CliIRC.hpp"

#include <string>
#include <list>

class ChannelIRC;

class UserIRC
{

	friend class CliIRC;

public:

	UserIRC(std::string nickname);
	~UserIRC();
	UserIRC(const UserIRC& user);
	UserIRC& operator=(const UserIRC& user);
	
	std::string	getNick();
	std::string	getUser();
	std::string	getHost();
	std::string	getFull();
	bool		isIrcop();

	int			commonChannels();
	std::string	commonChannel(int n);

	bool		isOn(std::string channel);	
	bool		isOwner(std::string channel);
	bool		isProtect(std::string channel);
	bool		isOp(std::string channel);
	bool		isHalfop(std::string channel);
	bool		isVoice(std::string channel);

private:

	bool		_setNickname(std::string nickname);
	bool		_setUsername(std::string username);
	bool		_setHostname(std::string hostname);
	bool		_setFullname(std::string fullname);
	bool		_setIrcop(bool ircop);

	bool		_addChannel(ChannelIRC *chan);
	bool		_delChannel(ChannelIRC *chan);

	std::string	_nickname;
	std::string	_username;
	std::string	_hostname;
	std::string	_fullname;

	bool					_ircop;
	std::list<ChannelIRC *>	_channels;

};


#endif // __USERIRC_HPP__
