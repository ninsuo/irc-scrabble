
#ifndef __CHANNELIRC_HPP__
#define __CHANNELIRC_HPP__

#include <string>
#include <list>

#include "CliIRC.hpp"
#include "UserIRC.hpp"
#include "Game.hpp"

class UserIRC;

class ChannelIRC
{

	friend class CliIRC;
	friend class Game;

public:

	ChannelIRC(std::string name);
	~ChannelIRC();
	ChannelIRC(const ChannelIRC& chan);
	ChannelIRC& operator=(const ChannelIRC& chan);
	
	std::string	getName();
	std::string	getPass();
	int			getLimit();
	std::string	getModes();
	std::string	getTopic();

	int			nick();
	std::string	nick(int n);

	bool	isOn(std::string nickname);
	bool	isOwner(std::string nickname);
	bool	isProtect(std::string nickname);
	bool	isOp(std::string nickname);
	bool	isHalfop(std::string nickname);
	bool	isVoice(std::string nickname);
	int		level(std::string nickname);

	bool	modeSet(std::string mode);

private:

	bool	_addUser(UserIRC *user);
	bool	_delUser(UserIRC *user);
	bool	_addOwner(UserIRC *user);
	bool	_delOwner(UserIRC *user);
	bool	_addProtect(UserIRC *user);
	bool	_delProtect(UserIRC *user);
	bool	_addOp(UserIRC *user);
	bool	_delOp(UserIRC *user);
	bool	_addHalfop(UserIRC *user);
	bool	_delHalfop(UserIRC *user);
	bool	_addVoice(UserIRC *user);
	bool	_delVoice(UserIRC *user);

	bool	_setTopic(std::string topic);
	bool	_setPass(std::string pass);
	bool	_setLimit(std::string limit);
	bool	_setMode(std::string mode);
	bool	_unsetMode(std::string mode);

	std::string	_name;
	std::string	_pass;
	int			_limit;
	std::string	_modes;
	std::string	_topic;

	std::list<UserIRC *>	_owner;
	std::list<UserIRC *>	_protect;
	std::list<UserIRC *>	_op;
	std::list<UserIRC *>	_halfop;
	std::list<UserIRC *>	_voice;
	std::list<UserIRC *>	_all;

	Game					_game;

};

#endif // __CHANNELIRC_HPP__
