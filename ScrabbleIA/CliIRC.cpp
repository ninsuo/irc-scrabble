
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CliIRC.hpp"
#include "MyString.hpp"
#include "UserIRC.hpp"
#include "ChannelIRC.hpp"

/* general functions */

CliIRC::CliIRC(std::string config)
{
	_connected = false;
	_ircop = false;
	_away = false;
	_file = config;
	_log = false;
	_output = false;
	_config = new MyINI(config.c_str());
	if (_config != NULL)
	{
		if ((*_config)["USER"]["nickname"] == "")
			(*_config)["USER"]["nickname"] = "Game";
		_nickname = (*_config)["USER"]["nickname"];
		if ((*_config)["OPTIONS"]["log"] == "yes")
		{
			_log = true;
			_logFile = "robot.log";
		}
		if ((*_config)["OPTIONS"]["output"] == "yes")
			_output = true;
		_loadLangage(*_config);
	}
	else
	{
		std::cerr << "Could not load configuration file '" + config + "'." << std::endl;
		::exit(1);
	}
	_uevent_ptr = NULL;
	_init_events();
}

CliIRC::~CliIRC()
{
	if (this->alive() == true)
		this->disconnect();
	if (this->_logStream.is_open())
		this->_logStream.close();
	delete _config;
	delete _lang;
}

CliIRC::CliIRC(const CliIRC& client)
{
	_connected = false;
	_ircop = false;
	_away = false;
	_log = client._log;
	_logFile = client._logFile;
	_output = client._output;
	_config = new MyINI(*(client._config));
	_lang = new MyINI(*(client._lang));
	_uevent_ptr = NULL;
}

CliIRC& CliIRC::operator=(const CliIRC& client)
{
	if (this != &client)
	{
		this->_connected = false;
		this->_ircop = false;
		this->_away = false;
		this->_log = client._log;
		this->_logFile = client._logFile;
		this->_output = client._output;
		this->_config = new MyINI(*(client._config));
		this->_lang = new MyINI(*(client._lang));
		this->_uevent_ptr = NULL;
	}
	return (*this);
}

CliIRC&		CliIRC::operator<<(const std::string& str)
{
	this->_client << str;
	return (*this);
}

CliIRC&		CliIRC::operator<<(const char *str)
{
	this->_client << str;
	return (*this);
}

CliIRC&		CliIRC::operator<<(int n)
{
	this->_client << n;
	return (*this);
}

CliIRC&		CliIRC::operator<<(char c)
{
	this->_client << c;
	return (*this);
}

bool	CliIRC::addEvent(std::string name, void (*fct)(CliIRC& client, void *ptr, UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string msg))
{
	if (this->_uevents[name] != NULL)
		return (false);
	this->_uevents[name] = fct;
	return (true);
}

bool	CliIRC::delEvent(std::string name)
{
	if (this->_uevents[name] == NULL)
		return (false);
	this->_uevents.erase(name);
	return (true);
}

void	CliIRC::setEventPtr(void *ptr)
{
	this->_uevent_ptr = ptr;
}

/* configuration functions */

std::string		CliIRC::configIsOnList(std::string list, std::string elem)
{
	MyINIvars::iterator	it;

	if (this->_config == NULL)
		return ("-1");
	if ((*this->_config)[list].size() == 0)
		return ("-1");
	for (it = (*this->_config)[list].begin(); (it != (*this->_config)[list].end()); it++)
	{
		if (lowCase((*it).second) == lowCase(elem))
			return ((*it).first);
	}
	return ("-1");
}

std::string		CliIRC::configIsOnList(std::string list, std::string elem, int numtok, char token)
{
	MyINIvars::iterator	it;

	if (this->_config == NULL)
		return ("-1");
	if ((*this->_config)[list].size() == 0)
		return ("-1");
	for (it = (*this->_config)[list].begin(); (it != (*this->_config)[list].end()); it++)
	{
		if (gettok((*it).second, numtok, token) == elem)
			return ((*it).first);
	}
	return ("-1");
}

bool	CliIRC::configIsKey(std::string tag, std::string key)
{
	MyINIvars::iterator	it;
	std::string			request;

	if (this->_config == NULL)
		return (false);
	if ((*this->_config)[tag].size() == 0)
		return (false);
	for (it = (*this->_config)[tag].begin(); (it != (*this->_config)[tag].end()); it++)
	{
		if ((*it).first == key)
			return (true);
	}
	return (false);
}

bool		CliIRC::configAddToList(std::string list, std::string elem)
{
	int		i;

	if (this->_config == NULL)
		return (false);
	if (this->configIsOnList(list, elem) != "-1")
		return (false);
	i = 1;
	while (24400)
	{
		if (this->configIsKey(list, intToString(i)) == false)
		{
			(*(this->_config))[list][intToString(i)] = elem;
			break ;
		}
		i++;
	}
	return (true);
}

bool					CliIRC::configDelFromList(std::string list, std::string elem)
{
	MyINIvars::iterator	it;
	std::string			request;

	if (this->_config == NULL)
		return (false);
	if ((*this->_config)[list].size() == 0)
		return (false);
	for (it = (*this->_config)[list].begin(); (it != (*this->_config)[list].end()); it++)
	{
		if ((*it).second == elem)
		{
			this->_config->del(list, (*it).first);
			return (true);
		}
	}
	return (false);
}

bool					CliIRC::configApplyToList(std::string list, std::string start, std::string end)
{
	MyINIvars::iterator	it;
	std::string			request;

	if (this->_config == NULL)
		return (false);
	if ((*this->_config)[list].size() == 0)
		return (false);
	for (it = (*this->_config)[list].begin(); (it != (*this->_config)[list].end()); it++)
	{
		request = start + (*it).second + end + "\n";
		this->send(request);
	}
	return (true);
}

bool	CliIRC::configReload()
{
	if (this->_config != NULL)
	{
		delete this->_config;
		this->_config = new MyINI(this->_file.c_str());
		return (true);
	}
	return (false);
}

bool	CliIRC::configSave()
{
	if (this->_config != NULL)
	{
		(*this->_config) >> this->_file.c_str();
		return (true);
	}
	return (false);
}

/* action functions */

bool			CliIRC::connect()
{
	std::string	server;
	int			port;
	std::string	password;
	std::string	nickname;
	std::string	username;
	std::string	fullname;
	std::string	randnick;

	if (this->_connected == true)
		return (false);
	if (this->_config == NULL)
		return (false);
	server = (*(this->_config))["SERVER"]["address"];
	if (server == "")
		return (false);
	port = ::atoi((*(this->_config))["SERVER"]["port"].c_str());
	if ((port < 1) || (port > 65535))
		return (false);
	if (this->_client.connect(server.c_str(), port) == false)
		return (false);
	password = (*(this->_config))["SERVER"]["password"];
	nickname = this->_nickname;
	if (nickname.length() == 0)
		nickname = "Game";
	username = (*(this->_config))["USER"]["username"];
	if (username.length() == 0)
		username = "Game";
	fullname = (*(this->_config))["USER"]["fullname"];
	if (fullname.length() == 0)
		fullname = "";
	if (password.length() > 0)
		this->_client << "PASS " << password << "\n";
	this->_client << "NICK " << nickname << "\n";
	this->_client << "USER " << username << " " << username << " " << username << " :" << fullname << "\n";
	this->_connected = true;
	return (true);
}

bool									CliIRC::disconnect()
{
	std::list<ChannelIRC *>::iterator	it_c;
	std::list<UserIRC *>::iterator		it_u;

	if (this->_connected == false)
		return (false);
	this->_client.disconnect();
	for (it_c = this->_channels.begin();
		(it_c != this->_channels.end()); it_c++)
			delete (*it_c);
	for (it_u = this->_users.begin();
		(it_u != this->_users.end()); it_u++)
			delete (*it_u);
	this->_users.clear();
	this->_channels.clear();
	this->_connected = false;
	this->_ircop = false;
	this->_away = false;
	Game::_timer.clear();
	return (true);
}

bool	CliIRC::alive()
{
	return (this->_connected);
}

bool	CliIRC::isIrcop()
{
	if (this->_connected == false)
		return (false);
	return (this->_ircop);
}

bool	CliIRC::isAway()
{
	if (this->_connected == false)
		return (false);
	return (this->_away);
}

bool				CliIRC::say(std::string dest, std::string msg)
{
	std::string		query;
	unsigned int	i;

	if (this->_connected == false)
		return (false);
	if (dest.length() == 0)
		return (false);
	if ((this->_isChannel(dest)) && (this->_couldSpeak(dest) == false))
		return (false);
	for (i = 0; (i < msg.size()); i++)
	{
		if ((msg.at(i) == '$') && ((i + 1) < msg.size()) && (msg.at(i + 1) == 'u'))
		{
			query += "\037";
			i++;
		}
		else if ((msg.at(i) == '$') && ((i + 1) < msg.size()) && (msg.at(i + 1) == 'b'))
		{
			query += "\02";
			i++;
		}
		else if ((msg.at(i) == '$') && ((i + 1) < msg.size()) && (msg.at(i + 1) == 'k'))
		{
			query += "\03";
			i++;
		}
		else
			query += msg.at(i);
	}
	this->_client << "PRIVMSG " << dest << " :" << query << "\n";
	return (true);
}

bool	CliIRC::action(std::string dest, std::string msg)
{
	std::string		query;
	unsigned int	i;

	if (this->_connected == false)
		return (false);
	if (dest.length() == 0)
		return (false);
	if ((this->_isChannel(dest)) && (this->_couldSpeak(dest) == false))
		return (false);
	for (i = 0; (i < msg.size()); i++)
	{
		if ((msg.at(i) == '$') && ((i + 1) < msg.size()) && (msg.at(i + 1) == 'u'))
		{
			query += "\037";
			i++;
		}
		else if ((msg.at(i) == '$') && ((i + 1) < msg.size()) && (msg.at(i + 1) == 'b'))
		{
			query += "\02";
			i++;
		}
		else if ((msg.at(i) == '$') && ((i + 1) < msg.size()) && (msg.at(i + 1) == 'k'))
		{
			query += "\03";
			i++;
		}
		else
			query += msg.at(i);
	}
	this->_client << "PRIVMSG " << dest << " :" << (char)0x1 << "ACTION " << query << (char)0x1 << "\n";
	return (true);
}

bool	CliIRC::notice(std::string dest, std::string msg)
{
	std::string		query;
	unsigned int	i;

	if (this->_connected == false)
		return (false);
	if (dest.length() == 0)
		return (false);
	if ((this->_isChannel(dest)) && (this->_couldSpeak(dest) == false))
		return (false);
	for (i = 0; (i < msg.size()); i++)
	{
		if ((msg.at(i) == '$') && ((i + 1) < msg.size()) && (msg.at(i + 1) == 'u'))
		{
			query += "\037";
			i++;
		}
		else if ((msg.at(i) == '$') && ((i + 1) < msg.size()) && (msg.at(i + 1) == 'b'))
		{
			query += "\02";
			i++;
		}
		else if ((msg.at(i) == '$') && ((i + 1) < msg.size()) && (msg.at(i + 1) == 'k'))
		{
			query += "\03";
			i++;
		}
		else
			query += msg.at(i);
	}
	this->_client << "NOTICE " << dest << " :" << query << "\n";
	return (true);
}

bool	CliIRC::ctcp(std::string dest, std::string msg)
{
	if (this->_connected == false)
		return (false);
	if (dest.length() == 0)
		return (false);
	if ((this->_isChannel(dest)) && (this->_couldSpeak(dest) == false))
		return (false);
	this->_client << "PRIVMSG " << dest << " :" << (char)0x1 << msg << (char)0x1 << "\n";
	return (true);
}

bool	CliIRC::ctcpreply(std::string dest, std::string msg)
{
	if (this->_connected == false)
		return (false);
	if (dest.length() == 0)
		return (false);
	if ((this->_isChannel(dest)) && (this->_couldSpeak(dest) == false))
		return (false);
	this->_client << "NOTICE " << dest << " :" << (char)0x1 << msg << (char)0x1 << "\n";
	return (true);
}

bool			CliIRC::ban(std::string chan, std::string user)
{
	ChannelIRC	*pchan;
	UserIRC		*pnick;

	if (this->_connected == false)
		return (false);
	pchan = this->_getChanByName(chan);
	if (pchan == NULL)
		return (false);
	if ((this->_ircop == false) && (pchan->level(this->_nickname) < 2))
		return (false);
	if (pchan->isOn(user) == false)
		return (false);
	pnick = this->_getUserByName(user);
	if (pnick == NULL)
		return (false);
	if (pnick->getHost().length() == 0)
		return (false);
	this->_client << "MODE " << chan << " +b *!*@" << pnick->getHost() << "\n";
	return (true);
}

bool			CliIRC::banNick(std::string chan, std::string user)
{
	ChannelIRC	*pchan;

	if (this->_connected == false)
		return (false);
	pchan = this->_getChanByName(chan);
	if (pchan == NULL)
		return (false);
	if ((this->_ircop == false) && (pchan->level(this->_nickname) < 2))
		return (false);
	this->_client << "MODE " << chan << " +b " << user << "!*@*" << "\n";
	return (true);
}

bool			CliIRC::kick(std::string chan, std::string user, std::string reason)
{
	ChannelIRC	*pchan;
	UserIRC		*pnick;

	if (this->_connected == false)
		return (false);
	pchan = this->_getChanByName(chan);
	if (pchan == NULL)
		return (false);
	if ((this->_ircop == false) && (pchan->level(this->_nickname) < 2))
		return (false);
	if (pchan->isOn(user) == false)
		return (false);
	pnick = this->_getUserByName(user);
	if (pnick == NULL)
		return (false);
	this->_client << "KICK " << chan << " " << lowCase(pnick->getNick()) << " :" << reason << "\n";
	return (true);
}

bool	CliIRC::bankick(std::string chan, std::string user, std::string reason)
{
	if (this->_connected == false)
		return (false);
	if (this->ban(chan, user))
		return (this->kick(chan, user, reason));
	return (false);
}

bool	CliIRC::kickban(std::string chan, std::string user, std::string reason)
{
	if (this->_connected == false)
		return (false);
	if (this->kick(chan, user, reason))
		return (this->ban(chan, user));
	return (false);
}

bool	CliIRC::kill(std::string user, std::string reason)
{
	if (this->_connected == false)
		return (false);
	if (this->_ircop == false)
		return (false);
	this->_client << "KILL " << user << " :" << reason << "\n";
	return (true);
}

bool			CliIRC::topic(std::string chan, std::string title)
{
	ChannelIRC	*pchan;

	if (this->_connected == false)
		return (false);
	pchan = this->_getChanByName(chan);
	if (pchan == NULL)
		return (false);
	if ((this->_ircop == false) && (pchan->level(this->_nickname) < 2) && (pchan->modeSet("t") == true))
		return (false);
	this->_client << "TOPIC " << chan << " :" << title << "\n";
	return (true);
}

bool			CliIRC::mode(std::string chan, std::string modes)
{
	ChannelIRC	*pchan;

	if (this->_connected == false)
		return (false);
	pchan = this->_getChanByName(chan);
	if (pchan == NULL)
		return (false);
	if ((this->_ircop == false) && (pchan->level(this->_nickname) < 2))
		return (false);
	this->_client << "MODE " << chan << " " << modes << "\n";
	return (true);
}

bool	CliIRC::mode(std::string priv_modes)
{
	if (this->_connected == false)
		return (false);
	this->_client << "MODE " << this->_nickname << " " << priv_modes << "\n";
	return (true);
}

bool			CliIRC::invite(std::string user, std::string chan)
{
	ChannelIRC	*pchan;

	if (this->_connected == false)
		return (false);
	pchan = this->_getChanByName(chan);
	if (pchan == NULL)
		return (false);
	if ((pchan->modeSet("i") == true) && (this->_ircop == false) && (pchan->level(this->_nickname) < 2))
		return (false);
	if (pchan->isOn(user) == true)
		return (false);
	this->_client << "INVITE " << user << " " << chan << "\n";
	return (true);
}

bool	CliIRC::join(std::string chan, std::string pass)
{
	if (this->_connected == false)
		return (false);
	this->_client << "JOIN :" << chan << " " << pass << "\n";
	return (true);
}

bool	CliIRC::join(std::string chan)
{
	if (this->_connected == false)
		return (false);
	this->_client << "JOIN :" << chan << "\n";
	return (true);
}

bool	CliIRC::part(std::string chan, std::string reason)
{
	ChannelIRC	*pchan;

	if (this->_connected == false)
		return (false);
	pchan = this->_getChanByName(chan);
	if (pchan == NULL)
		return (false);
	this->_client << "PART " << chan << " :" << reason << "\n";
	return (true);
}

bool	CliIRC::part(std::string chan)
{
	ChannelIRC	*pchan;

	if (this->_connected == false)
		return (false);
	pchan = this->_getChanByName(chan);
	if (pchan == NULL)
		return (false);
	this->_client << "PART " << chan << "\n";
	return (true);
}

bool	CliIRC::hop(std::string chan)
{
	return ((this->part(chan)) && (this->join(chan)));
}

bool	CliIRC::nick(std::string nickname)
{
	if (this->_connected == false)
		return (false);
	if (this->_nickname == nickname)
		return (false);
	this->_client << "NICK " << nickname << "\n";
	return (true);
}

bool	CliIRC::quit(std::string reason)
{
	if (this->_connected == false)
		return (false);
	this->_client << "QUIT :" << reason << "\n";
	return (true);
}

bool	CliIRC::owner(std::string chan, std::string user)
{
	ChannelIRC	*pchan;

	if (this->_connected == false)
		return (false);
	pchan = this->_getChanByName(chan);
	if (pchan == NULL)
		return (false);
	if ((this->_ircop == false) && (pchan->level(this->_nickname) < 5))
		return (false);
	if (pchan->isOn(user) == false)
		return (false);
	if (pchan->isOwner(user) == true)
		return (false);
	this->_client << "MODE " << chan << " +q " << user << "\n";
	return (true);
}

bool	CliIRC::deowner(std::string chan, std::string user)
{
	ChannelIRC	*pchan;

	if (this->_connected == false)
		return (false);
	pchan = this->_getChanByName(chan);
	if (pchan == NULL)
		return (false);
	if ((this->_ircop == false) && (pchan->level(this->_nickname) < 5))
		return (false);
	if (pchan->isOn(user) == false)
		return (false);
	if (pchan->isOwner(user) == false)
		return (false);
	this->_client << "MODE " << chan << " -q " << user << "\n";
	return (true);
}

bool	CliIRC::protect(std::string chan, std::string user)
{
	ChannelIRC	*pchan;

	if (this->_connected == false)
		return (false);
	pchan = this->_getChanByName(chan);
	if (pchan == NULL)
		return (false);
	if ((this->_ircop == false) && (pchan->level(this->_nickname) < 5))
		return (false);
	if (pchan->isOn(user) == false)
		return (false);
	if (pchan->isProtect(user) == true)
		return (false);
	this->_client << "MODE " << chan << " +a " << user << "\n";
	return (true);
}

bool	CliIRC::deprotect(std::string chan, std::string user)
{
	ChannelIRC	*pchan;

	if (this->_connected == false)
		return (false);
	pchan = this->_getChanByName(chan);
	if (pchan == NULL)
		return (false);
	if ((this->_ircop == false) && (pchan->level(this->_nickname) < 5))
		return (false);
	if (pchan->isOn(user) == false)
		return (false);
	if (pchan->isProtect(user) == false)
		return (false);
	this->_client << "MODE " << chan << " -a " << user << "\n";
	return (true);
}

bool	CliIRC::op(std::string chan, std::string user)
{
	ChannelIRC	*pchan;

	if (this->_connected == false)
		return (false);
	pchan = this->_getChanByName(chan);
	if (pchan == NULL)
		return (false);
	if ((this->_ircop == false) && (pchan->level(this->_nickname) < 3))
		return (false);
	if (pchan->isOn(user) == false)
		return (false);
	if (pchan->isOp(user) == true)
		return (false);
	this->_client << "MODE " << chan << " +o " << user << "\n";
	return (true);
}

bool	CliIRC::deop(std::string chan, std::string user)
{
	ChannelIRC	*pchan;

	if (this->_connected == false)
		return (false);
	pchan = this->_getChanByName(chan);
	if (pchan == NULL)
		return (false);
	if ((this->_ircop == false) && (pchan->level(this->_nickname) < 3))
		return (false);
	if (pchan->isOn(user) == false)
		return (false);
	if (pchan->isOp(user) == false)
		return (false);
	this->_client << "MODE " << chan << " -o " << user << "\n";
	return (true);
}

bool	CliIRC::halfop(std::string chan, std::string user)
{
	ChannelIRC	*pchan;

	if (this->_connected == false)
		return (false);
	pchan = this->_getChanByName(chan);
	if (pchan == NULL)
		return (false);
	if ((this->_ircop == false) && (pchan->level(this->_nickname) < 3))
		return (false);
	if (pchan->isOn(user) == false)
		return (false);
	if (pchan->isHalfop(user) == true)
		return (false);
	this->_client << "MODE " << chan << " +h " << user << "\n";
	return (true);
}

bool	CliIRC::dehalfop(std::string chan, std::string user)
{
	ChannelIRC	*pchan;

	if (this->_connected == false)
		return (false);
	pchan = this->_getChanByName(chan);
	if (pchan == NULL)
		return (false);
	if ((this->_ircop == false) && (pchan->level(this->_nickname) < 3))
		return (false);
	if (pchan->isOn(user) == false)
		return (false);
	if (pchan->isHalfop(user) == false)
		return (false);
	this->_client << "MODE " << chan << " -h " << user << "\n";
	return (true);
}

bool	CliIRC::voice(std::string chan, std::string user)
{
	ChannelIRC	*pchan;

	if (this->_connected == false)
		return (false);
	pchan = this->_getChanByName(chan);
	if (pchan == NULL)
		return (false);
	if ((this->_ircop == false) && (pchan->level(this->_nickname) < 2))
		return (false);
	if (pchan->isOn(user) == false)
		return (false);
	if (pchan->isVoice(user) == true)
		return (false);
	this->_client << "MODE " << chan << " +v " << user << "\n";
	return (true);
}

bool	CliIRC::devoice(std::string chan, std::string user)
{
	ChannelIRC	*pchan;

	if (this->_connected == false)
		return (false);
	pchan = this->_getChanByName(chan);
	if (pchan == NULL)
		return (false);
	if ((this->_ircop == false) && (pchan->level(this->_nickname) < 2))
		return (false);
	if (pchan->isOn(user) == false)
		return (false);
	if (pchan->isVoice(user) == false)
		return (false);
	this->_client << "MODE " << chan << " -v " << user << "\n";
	return (true);
}

bool	CliIRC::oper(std::string login, std::string pass)
{
	if (this->_connected == false)
		return (false);
	if (this->_ircop == true)
		return (false);
	this->_client << "OPER " << login << " " << pass << "\n";
	return (true);
}

bool	CliIRC::sajoin(std::string user, std::string chan)
{
	if (this->_connected == false)
		return (false);
	if (this->_ircop == false)
		return (false);
	this->_client << "SAJOIN " << user << " " << chan << "\n";
	return (true);
}

bool	CliIRC::sapart(std::string user, std::string chan)
{
	if (this->_connected == false)
		return (false);
	if (this->_ircop == false)
		return (false);
	this->_client << "SAPART " << user << " " << chan << "\n";
	return (true);
}

bool	CliIRC::chghost(std::string user, std::string host)
{
	if (this->_connected == false)
		return (false);
	if (this->_ircop == false)
		return (false);
	this->_client << "CHGHOST " << user << " " << host << "\n";
	return (true);
}

bool	CliIRC::chgident(std::string user, std::string ident)
{
	if (this->_connected == false)
		return (false);
	if (this->_ircop == false)
		return (false);
	this->_client << "CHGIDENT " << user << " " << ident << "\n";
	return (true);
}

bool	CliIRC::chgfull(std::string user, std::string full)
{
	if (this->_connected == false)
		return (false);
	if (this->_ircop == false)
		return (false);
	this->_client << "CHGFULL " << user << " :" << full << "\n";
	return (true);
}

bool	CliIRC::away(std::string reason)
{
	if (this->_connected == false)
		return (false);
	if (this->_away == true)
		return (false);
	if (reason.length() == 0)
		return (false);
	this->_client << "AWAY :" << reason << "\n";
	return (true);
}

bool	CliIRC::back()
{
	if (this->_connected == false)
		return (false);
	if (this->_away == true)
		return (false);
	this->_client << "AWAY :" << "\n";
	return (true);
}

bool	CliIRC::who(std::string target)
{
	this->_client << "WHO " << target << "\n";
	return (true);
}

bool	CliIRC::send(std::string request)
{
	this->_client << request << "\n";
	return (true);
}

std::string	CliIRC::me()
{
	return (this->_nickname);
}

bool			CliIRC::loop()
{
	std::string	query;
	std::string	event;

	while (this->_client.alive())
	{
		this->_client.recv();
		while (this->_client.lineReady())
		{
			if (this->_log == true)
				this->_logQuery(this->_client.peekLine(1));
			if (this->_output == true)
				std::cout << this->strip(this->_client.peekLine(1)) << std::endl;
			if (this->_client.peekWord(1) == "PING")
			{
				this->_client << "PONG " << this->_client.peekWord(2) << "\n";
				this->_client.readLine();
				continue ;
			}
			event = this->_client.peekWord(2);
			if (is_numeric(event) == true)
			{
				this->_e_raw(this->_client.readLine());
				continue ;
			}
			query = this->_client.readLine();
			if (this->_events[(event)] != NULL)
				(this->*(this->_events[(event)]))(query);
			else
			{
				event = gettok(query, 1, ' ');
				if (event == "ERROR")
				{
					if (this->_events[(event)] != NULL)
						(this->*(this->_events[(event)]))(query);
				}
			}
		}
	}
	this->disconnect();
	return (true);
}

void	CliIRC::thread(void (*call)(CliIRC&, void *ptr), void *ptr)
{
	this->_call = call;
	this->_ptr = ptr;
	this->_client.thread(this->_run, (void *)this);
}

void	CliIRC::wait()
{
	this->_client.wait();
}

/* event functions */

std::string		CliIRC::_getRandString(int length)
{
	std::string	ret;
	int			i;

	::srandom(::time(0));
	for (i = 0; (i < length); i++)
		ret += (char)((::random() % 10) + '0');
	return (ret);
}

bool	CliIRC::_isChannel(std::string chan)
{
	if (chan.length() == 0)
		return (false);
	if ((chan.at(0) == '#') || (chan.at(0) == '&'))
		return (true);
	return (false);
}

bool		CliIRC::_isNickname(std::string nick)
{
	int		i;

	if (nick.length() == 0)
		return (false);
	for (i = 0; (i < (int)nick.length()); i++)
	{
		if ((i == 0) && ((this->_isLetter(nick.at(i)) == false) && (this->_isSpecial(nick.at(i)) == false)))
			return (false);
		if ((this->_isLetter(nick.at(i)) == false) && (this->_isNumber(nick.at(i)) == false) && (this->_isSpecial(nick.at(i)) == false))
			return (false);
	}
	return (true);
}

bool			CliIRC::_isLetter(char c)
{
	if (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')))
		return (true);
	return (false);
}

bool			CliIRC::_isNumber(char c)
{
	if ((c >= '0') && (c <= '9'))
		return (true);
	return (false);
}

bool			CliIRC::_isSpecial(char c)
{
	const char	*special;
	int			i;

	special = "-_[]\\`^{}|";
	for (i = 0; (*(special + i)); i++)
		if (c == (*(special + i)))
			return (true);
	return (false);
}

bool			CliIRC::_couldSpeak(std::string chan)
{
	ChannelIRC	*pchan;

	if (this->_ircop == true)
		return (true);
	pchan = this->_getChanByName(chan);
	if (pchan == NULL)
		return (false);
	if ((pchan->modeSet("m") == false) ||
		(pchan->level(this->_nickname) > 0))
		return (true);
	return (false);
}

bool			CliIRC::_userMatch(UserIRC *user, std::string pattern)
{
	std::string	fulluser;

	fulluser = lowCase(user->getNick()) + '!' + lowCase(user->getUser()) + '@' + lowCase(user->getHost());
	pattern = lowCase(pattern);
	if (this->_userMatchRec(fulluser.c_str(), pattern.c_str()) > 0)
		return (true);
	return (false);
}

int		CliIRC::_userMatchRec(const char *user, const char *pattern)
{
	if (!*user && !*pattern)
		return (1);
	if ((*pattern == '?') && ((*user)))
		return (this->_userMatchRec((user + 1), (pattern + 1)));
	if (*pattern == '*')
	{
		if (*user)
			return (this->_userMatchRec(user + 1, pattern) + this->_userMatchRec(user, pattern + 1));
		else
			return (this->_userMatchRec(user, pattern + 1));
	}
	if (*user == *pattern)
		return (this->_userMatchRec(user + 1, pattern + 1));
	return (0);
}

ChannelIRC	*CliIRC::_getChanByName(std::string chan)
{
	std::list<ChannelIRC *>::iterator	it;

	for (it = this->_channels.begin(); (it != this->_channels.end()); it++)
		if (lowCase((*it)->getName()) == lowCase(chan))
			return (*it);
	return (NULL);
}

UserIRC	*CliIRC::_getUserByName(std::string nick)
{
	std::list<UserIRC *>::iterator	it;

	for (it = this->_users.begin(); (it != this->_users.end()); it++)
		if (lowCase(lowCase((*it)->getNick())) == lowCase(nick))
			return (*it);
	return (NULL);
}

void		CliIRC::_run(MyUser& obj, void *ptr)
{
	CliIRC	*client = (CliIRC *)ptr;
	MyUser	*tmp;

	tmp = &obj;
	client->_call(*client, client->_ptr);
}

bool	CliIRC::applyToChannels(std::string begin, std::string end)
{
	std::list<ChannelIRC *>::iterator	it;
	std::string							request;

	if (this->_channels.size() == 0)
		return (false);
	for (it = this->_channels.begin(); (it != this->_channels.end()); it++)
	{
		request = begin + (*it)->getName() + end + '\n';
		this->send(request);
	}
	return (true);
}

bool	CliIRC::applyToUsers(std::string begin, std::string end)
{
	std::list<UserIRC *>::iterator	it;
	std::string						request;

	if (this->_users.size() == 0)
		return (false);
	for (it = this->_users.begin(); (it != this->_users.end()); it++)
	{
		request = begin + lowCase((*it)->getNick()) + end + '\n';
		this->send(request);
	}
	return (true);
}

std::string		CliIRC::strip(std::string msg)
{
	std::string	ret;
	int			i;

	for (i = 0; (i < (int)msg.size()); i++)
	{
		if (((unsigned char)msg.at(i) < (unsigned char)32) && (msg.at(i) != 0x3))
			continue ;
		else if (msg.at(i) == 0x3)
		{
			i++;
			if ((i < (int)msg.size()) && ((msg.at(i) >= '0') && (msg.at(i) <= '9')))
				i++;
			if ((i < (int)msg.size()) && ((msg.at(i) >= '0') && (msg.at(i) <= '9')))
				i++;
			if ((i < (int)msg.size()) && (msg.at(i) == ','))
			{
				i++;
				if ((i < (int)msg.size()) && ((msg.at(i) >= '0') && (msg.at(i) <= '9')))
					i++;
				if ((i < (int)msg.size()) && ((msg.at(i) >= '0') && (msg.at(i) <= '9')))
					i++;
			}
			i--;
		}
		else
		{
			ret += msg.at(i);
		}
	}
	return (ret);
}

void	CliIRC::_init_events()
{
	_events["JOIN"] = &CliIRC::_e_join;
	_events["PART"] = &CliIRC::_e_part;
	_events["QUIT"] = &CliIRC::_e_quit;
	_events["NICK"] = &CliIRC::_e_nick;
	_events["KICK"] = &CliIRC::_e_kick;
	_events["TOPIC"] = &CliIRC::_e_topic;
	_events["MODE"] = &CliIRC::_e_mode;
	_events["PRIVMSG"] = &CliIRC::_e_privmsg;
	_events["NOTICE"] = &CliIRC::_e_notice;
	_events["ERROR"] = &CliIRC::_e_error;
}

void			CliIRC::_e_join(std::string query)
{
	std::string	code("genjoin");
	std::string	nick;
	std::string	chan;
	UserIRC		*pnick;
	ChannelIRC	*pchan;

	nick = lowCase(gettok(gettok(gettok(query, 1, ':'), 1, ' '), 1, '!'));
	chan = lowCase(gettok(gettok(query, 3, ' '), 1, ':'));
	if (this->_getUserByName(nick) == false)
	{
		if (this->_isNickname(nick) == false)
			return ;
		pnick = new UserIRC(gettok(gettok(gettok(query, 1, ':'), 1, ' '), 1, '!'));
		this->_users.push_back(pnick);
		this->who(nick);
	}
	if (this->_getChanByName(chan) == false)
	{
		pchan = new ChannelIRC(chan);
		this->_channels.push_back(pchan);
	}
	pnick = this->_getUserByName(nick);
	pchan = this->_getChanByName(chan);
	pchan->_addUser(pnick);
	pnick->_addChannel(pchan);
	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, pnick, pchan, NULL, "");
	if (nick == lowCase(this->_nickname))
		this->_e_join_bot(pchan);
	else
		this->_e_join_oth(pchan, pnick);
	return ;
}

void			CliIRC::_e_join_bot(ChannelIRC *chan)
{
	std::string	code("botjoin");

	this->_client << "MODE " << chan->getName() << '\n';
	this->who(chan->getName());
	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, this->_getUserByName(lowCase(this->_nickname)), chan, NULL, "");
	return ;
}

void			CliIRC::_e_join_oth(ChannelIRC *chan, UserIRC *nick)
{
	std::string	code("othjoin");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, nick, chan, NULL, "");
	return ;
}

void			CliIRC::_e_part(std::string query)
{
	std::string	code("genpart");
	std::string	nick;
	std::string	chan;
	UserIRC		*pnick;
	ChannelIRC	*pchan;

	nick = lowCase(gettok(gettok(gettok(query, 1, ':'), 1, ' '), 1, '!'));
	chan = lowCase(gettok(gettok(query, 3, ' '), 1, ':'));
	pnick = this->_getUserByName(nick);
	pchan = this->_getChanByName(chan);
	if ((pchan == NULL) || (pnick == NULL))
		return ;
	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, pnick, pchan, NULL, "");
	if (nick == lowCase(this->_nickname))
		this->_e_part_bot(query, pchan);
	else
		this->_e_part_oth(query, pchan, pnick);
	return ;
}

void			CliIRC::_e_part_bot(std::string query, ChannelIRC *chan)
{
	std::string	code("botpart");
	std::list<UserIRC *>::iterator		it_u;
	std::list<ChannelIRC *>::iterator	it_c;
	std::string	msg;
	int									e;

	msg = fulltok(query, 2, ':');
	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, this->_getUserByName(this->_nickname), chan, NULL, msg);
	e = 1;
	while (e)
	{
		e = 0;
		for (it_u = this->_users.begin(); (it_u != this->_users.end()); it_u++)
		{
			if ((*it_u)->_delChannel(chan) == true)
				if ((*it_u)->commonChannels() == 0)
				{
					delete (*it_u);
					this->_users.erase(it_u);
					e = 1;
					break ;
				}
		}
	}
	for (it_c = this->_channels.begin(); (it_c != this->_channels.end()); it_c++)
		if (*it_c == chan)
		{
			this->_channels.erase(it_c);
			break ;
		}
	if (this->configIsOnList("AUTOJOIN", chan->getName(), 1, ' ') != "-1")
		this->join(chan->getName());
	delete chan;
	return ;
}

void			CliIRC::_e_part_oth(std::string query, ChannelIRC *chan, UserIRC *nick)
{
	std::string	code("othpart");
	std::list<UserIRC *>::iterator	it;
	std::string	msg;

	msg = fulltok(query, 2, ':');
	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, nick, chan, NULL, msg);
	chan->_delUser(nick);
	nick->_delChannel(chan);
	if (nick->commonChannels() == 0)
	{
		for (it = this->_users.begin(); (it != this->_users.end()); it++)
			if (*it == nick)
			{
				this->_users.erase(it);
				break ;
			}
		delete nick;
	}
	return ;
}

void			CliIRC::_e_quit(std::string query)
{
	std::string	code("genquit");
	std::string	code_bis("genquitchan");
	std::list<UserIRC *>::iterator	it_u;
	std::list<ChannelIRC *>::iterator	it_c;
	std::string	nick;
	std::string	msg;
	UserIRC		*pnick;

	nick = lowCase(gettok(gettok(gettok(query, 1, ':'), 1, ' '), 1, '!'));
	msg = fulltok(query, 2, ':');
	pnick = this->_getUserByName(nick);
	if (pnick == NULL)
		return ;
	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, pnick, NULL, NULL, msg);
	for (it_c = pnick->_channels.begin(); (it_c != pnick->_channels.end()); it_c++)
	{
		if (this->_uevents[code_bis] != NULL)
			(this->_uevents[code_bis])(*this, this->_uevent_ptr, pnick, (*it_c), NULL, msg);
		(*it_c)->_delUser(pnick);
	}
	for (it_u = this->_users.begin(); (it_u != this->_users.end()); it_u++)
		if ((*it_u) == pnick)
		{
			this->_users.erase(it_u);
			break ;
		}
	delete pnick;
	return ;
}

void			CliIRC::_e_nick(std::string query)
{
	std::string	code("gennick");
	std::string	nick;
	std::string	newnick;
	UserIRC		*pnick;

	nick = lowCase(gettok(gettok(gettok(query, 1, ':'), 1, ' '), 1, '!'));
	newnick = gettok(fulltok(query, 3, ' '), 1, ':');
	pnick = this->_getUserByName(nick);
	if (pnick == NULL)
		return ;
	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, pnick, NULL, NULL, newnick);
	if (lowCase(lowCase(pnick->getNick())) == lowCase(this->_nickname))
		this->_e_nick_bot(pnick, newnick);
	else
		this->_e_nick_oth(pnick, newnick);
	pnick->_nickname = newnick;
	return ;
}

void			CliIRC::_e_nick_bot(UserIRC *user, std::string newnick)
{
	std::string	code("botnick");

	if (this->_config == NULL)
		return ;
	this->_nickname = newnick;
	(*(this->_config))["USER"]["nickname"] = newnick;
	this->configSave();
	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, user, NULL, NULL, newnick);
	return ;
}

void			CliIRC::_e_nick_oth(UserIRC *user, std::string newnick)
{
	std::string	code("othnick");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, user, NULL, NULL, newnick);
	return ;
}

void			CliIRC::_e_kick(std::string query)
{
	std::string	code("genkick");
	std::string	kicker;
	std::string	channel;
	std::string	kicked;
	std::string	reason;
	UserIRC		*pkicker;
	UserIRC		*pkicked;
	ChannelIRC	*pchan;

	kicker = lowCase(gettok(gettok(gettok(query, 1, ':'), 1, ' '), 1, '!'));
	channel = lowCase(gettok(query, 3, ' '));
	kicked = lowCase(gettok(query, 4, ' '));
	reason = fulltok(query, 2, ':');
	pkicker = this->_getUserByName(kicker);
	pchan = this->_getChanByName(channel);
	pkicked = this->_getUserByName(kicked);
	if ((pkicker == NULL) || (pkicked == NULL) || (pchan == NULL))
		return ;
	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, pkicker, pchan, pkicked, reason);
	if ((kicker == lowCase(this->_nickname)) && (kicked == lowCase(this->_nickname)))
		this->_e_kick_him(pkicker, pchan, pkicked, reason);
	else if ((kicker == lowCase(this->_nickname)) && (kicked != lowCase(this->_nickname)))
		this->_e_kick_botkicker(pkicker, pchan, pkicked, reason);
	else if ((kicker != lowCase(this->_nickname)) && (kicked == lowCase(this->_nickname)))
		this->_e_kick_botkicked(pkicker, pchan, pkicked, reason);
	else
		this->_e_kick_oth(pkicker, pchan, pkicked, reason);
	return ;
}

void			CliIRC::_e_kick_him(UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string reason)
{
	std::string	code("botkickhim");
	std::list<UserIRC *>::iterator		it_u;
	std::list<ChannelIRC *>::iterator	it_c;
	int									e;

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, reason);
	e = 1;
	while (e)
	{
		e = 0;
		for (it_u = this->_users.begin(); (it_u != this->_users.end()); it_u++)
			if ((*it_u)->_delChannel(chan) == true)
				if ((*it_u)->commonChannels() == 0)
				{
					this->_users.erase(it_u);
					delete (*it_u);
					e = 1;
					break ;
				}
	}
	for (it_c = this->_channels.begin(); (it_c != this->_channels.end()); it_c++)
		if (*it_c == chan)
		{
			this->_channels.erase(it_c);
			break ;
		}
	if (this->configIsOnList("AUTOJOIN", chan->getName(), 1, ' ') == "-1")
		this->join(chan->getName());
	delete chan;
	return ;
}

void			CliIRC::_e_kick_botkicker(UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string reason)
{
	std::string	code("botkicker");
	std::list<UserIRC *>::iterator	it;

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, reason);
	chan->_delUser(dst);
	dst->_delChannel(chan);
	if (dst->commonChannels() == 0)
	{
		for (it = this->_users.begin(); (it != this->_users.end()); it++)
			if (*it == dst)
			{
				this->_users.erase(it);
				break ;
			}
		delete dst;
	}
	return ;
}

void			CliIRC::_e_kick_botkicked(UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string reason)
{
	std::string	code("botkicked");
	std::list<UserIRC *>::iterator		it_u;
	std::list<ChannelIRC *>::iterator	it_c;
	int									e;

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, reason);
	e = 1;
	while (e)
	{
		e = 0;
		for (it_u = this->_users.begin(); (it_u != this->_users.end()); it_u++)
			if ((*it_u)->_delChannel(chan) == true)
				if ((*it_u)->commonChannels() == 0)
				{
					this->_users.erase(it_u);
					delete (*it_u);
					e = 1;
					break ;
				}
	}
	for (it_c = this->_channels.begin(); (it_c != this->_channels.end()); it_c++)
		if (*it_c == chan)
		{
			this->_channels.erase(it_c);
			break ;
		}
	if (this->configIsOnList("AUTOJOIN", chan->getName(), 1, ' ') != "-1")
		this->join(chan->getName());
	delete chan;
	return ;
}

void			CliIRC::_e_kick_oth(UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string reason)
{
	std::string	code("othkick");
	std::list<UserIRC *>::iterator	it;

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, reason);
	chan->_delUser(dst);
	dst->_delChannel(chan);
	if (dst->commonChannels() == 0)
	{
		for (it = this->_users.begin(); (it != this->_users.end()); it++)
			if (*it == dst)
			{
				this->_users.erase(it);
				break ;
			}
		delete dst;
	}
	return ;
}

void	CliIRC::_e_topic(std::string query)
{
	std::string	code("gentopic");
	std::string	nick;
	std::string	channel;
	std::string	topic;
	UserIRC		*pnick;
	ChannelIRC	*pchan;

	nick = lowCase(gettok(gettok(gettok(query, 1, ':'), 1, ' '), 1, '!'));
	channel = lowCase(gettok(query, 3, ' '));
	topic = fulltok(query, 2, ':');
	pnick = this->_getUserByName(nick);
	pchan = this->_getChanByName(channel);
	if ((pnick == NULL) || (pchan == NULL))
		return ;
	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, pnick, pchan, NULL, topic);
	if (nick == lowCase(this->_nickname))
		this->_e_topic_bot(pnick, pchan, topic);
	else
		this->_e_topic_oth(pnick, pchan, topic);
	pchan->_setTopic(topic);
	return ;
}

void			CliIRC::_e_topic_bot(UserIRC *user, ChannelIRC *chan, std::string topic)
{
	std::string	code("bottopic");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, user, chan, NULL, topic);
	return ;
}

void			CliIRC::_e_topic_oth(UserIRC *user, ChannelIRC *chan, std::string topic)
{
	std::string	code("othtopic");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, user, chan, NULL, topic);
	return ;
}


void	CliIRC::_e_mode(std::string query)
{
	std::list<UserIRC *>::iterator	it;
	std::string	code("genmode");
	std::string	nick;
	std::string	chan;
	std::string	modes;
	std::string	mode;
	std::string	param;
	UserIRC		*pnick;
	UserIRC		*pdest;
	ChannelIRC	*pchan;
	int			sign;
	int			arg;
	int			i;

	chan = lowCase(gettok(query, 3, ' '));
	modes = gettok(gettok(query, 4, ' '), 1, ':');
	if (this->_isChannel(chan) == false)
	{
		nick = lowCase(gettok(gettok(query, 1, ' '), 1, ':'));
		pnick = this->_getUserByName(nick);
		if (pnick == NULL)
			return ;
		this->_e_mode_user(pnick, modes);
		return ;
	}
	nick = lowCase(gettok(gettok(gettok(query, 1, ':'), 1, ' '), 1, '!'));
	pnick = this->_getUserByName(nick);
	if (pnick == NULL)
		return ;
	pchan = this->_getChanByName(chan);
	if (pchan == NULL)
		return ;
	sign = 1;
	arg = 5;
	for (i = 0; (i < (int)modes.length()); i++)
	{
		if (modes.at(i) == '-')
			sign = -1;
		else if (modes.at(i) == '+')
			sign = 1;
		else if (modes.at(i) == 'p')
		{
			if (sign > 0)
				this->_e_mode_private(pnick, pchan);
			else
				this->_e_mode_deprivate(pnick, pchan);
		}
		else if (modes.at(i) == 's')
		{
			if (sign > 0)
				this->_e_mode_secret(pnick, pchan);
			else
				this->_e_mode_desecret(pnick, pchan);
		}
		else if (modes.at(i) == 'i')
		{
			if (sign > 0)
				this->_e_mode_invite(pnick, pchan);
			else
				this->_e_mode_deinvite(pnick, pchan);
		}
		else if (modes.at(i) == 't')
		{
			if (sign > 0)
				this->_e_mode_topic(pnick, pchan);
			else
				this->_e_mode_detopic(pnick, pchan);
		}
		else if (modes.at(i) == 'n')
		{
			if (sign > 0)
				this->_e_mode_extern(pnick, pchan);
			else
				this->_e_mode_deextern(pnick, pchan);
		}
		else if (modes.at(i) == 'k')
		{
			if (sign > 0)
			{
				param = gettok(query, arg, ' ');
				this->_e_mode_key(pnick, pchan, param);
				arg++;
			}
			else
				this->_e_mode_dekey(pnick, pchan);
		}
		else if (modes.at(i) == 'l')
		{
			if (sign > 0)
			{
				param = gettok(query, arg, ' ');
				this->_e_mode_limit(pnick, pchan, param);
				arg++;
			}
			else
				this->_e_mode_delimit(pnick, pchan);
		}
		else if (modes.at(i) == 'm')
		{
			if (sign > 0)
				this->_e_mode_moderate(pnick, pchan);
			else
				this->_e_mode_demoderate(pnick, pchan);
		}
		else if (modes.at(i) == 'q')
		{
			param = gettok(query, arg, ' ');
			arg++;
			pdest = this->_getUserByName(param);
			if (pdest == NULL)
				continue ;
			if (sign > 0)
			{
				this->_e_mode_owner_gen(pchan, pnick, pdest);
				if ((lowCase(lowCase(pnick->getNick())) == lowCase(this->_nickname)) && (lowCase(pdest->getNick()) == lowCase(this->_nickname)))
					this->_e_mode_owner_him(pchan, pnick);
				else if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase(pdest->getNick()) != lowCase(this->_nickname)))
					this->_e_mode_owner_ownerer(pchan, pnick, pdest);
				else if ((lowCase(pnick->getNick()) != lowCase(this->_nickname)) && (lowCase(pdest->getNick()) == lowCase(this->_nickname)))
					this->_e_mode_owner_ownered(pchan, pnick, pdest);
				else
					this->_e_mode_owner_oth(pchan, pnick, pdest);
				pchan->_addOwner(pdest);
			}
			else
			{
				this->_e_mode_deowner_gen(pchan, pnick, pdest);
				if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase(pdest->getNick()) == lowCase(this->_nickname)))
					this->_e_mode_deowner_him(pchan, pnick);
				else if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase(pdest->getNick()) != lowCase(this->_nickname)))
					this->_e_mode_deowner_deownerer(pchan, pnick, pdest);
				else if ((lowCase(pnick->getNick()) != lowCase(this->_nickname)) && (lowCase(pdest->getNick()) == lowCase(this->_nickname)))
					this->_e_mode_deowner_deownered(pchan, pnick, pdest);
				else
					this->_e_mode_deowner_oth(pchan, pnick, pdest);
				pchan->_delOwner(pdest);
			}
		}
		else if (modes.at(i) == 'a')
		{
			param = gettok(query, arg, ' ');
			arg++;
			pdest = this->_getUserByName(param);
			if (pdest == NULL)
				continue ;
			if (sign > 0)
			{
				this->_e_mode_protect_gen(pchan, pnick, pdest);
				if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase(pdest->getNick()) == lowCase(this->_nickname)))
					this->_e_mode_protect_him(pchan, pnick);
				else if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase(pdest->getNick()) != lowCase(this->_nickname)))
					this->_e_mode_protect_protecter(pchan, pnick, pdest);
				else if ((lowCase(pnick->getNick()) != lowCase(this->_nickname)) && (lowCase(pdest->getNick()) == lowCase(this->_nickname)))
					this->_e_mode_protect_protected(pchan, pnick, pdest);
				else
					this->_e_mode_protect_oth(pchan, pnick, pdest);
				pchan->_addProtect(pdest);
			}
			else
			{
				this->_e_mode_deprotect_gen(pchan, pnick, pdest);
				if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase(pdest->getNick()) == lowCase(this->_nickname)))
					this->_e_mode_deprotect_him(pchan, pnick);
				else if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase(pdest->getNick()) != lowCase(this->_nickname)))
					this->_e_mode_deprotect_deprotecter(pchan, pnick, pdest);
				else if ((lowCase(pnick->getNick()) != lowCase(this->_nickname)) && (lowCase(pdest->getNick()) == lowCase(this->_nickname)))
					this->_e_mode_deprotect_deprotected(pchan, pnick, pdest);
				else
					this->_e_mode_deprotect_oth(pchan, pnick, pdest);
				pchan->_delProtect(pdest);
			}
		}
		else if (modes.at(i) == 'o')
		{
			param = gettok(query, arg, ' ');
			arg++;
			pdest = this->_getUserByName(param);
			if (pdest == NULL)
				continue ;
			if (sign > 0)
			{
				this->_e_mode_op_gen(pchan, pnick, pdest);
				if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase(pdest->getNick()) == lowCase(this->_nickname)))
					this->_e_mode_op_him(pchan, pnick);
				else if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase(pdest->getNick()) != lowCase(this->_nickname)))
					this->_e_mode_op_oper(pchan, pnick, pdest);
				else if ((lowCase(pnick->getNick()) != lowCase(this->_nickname)) && (lowCase(pdest->getNick()) == lowCase(this->_nickname)))
					this->_e_mode_op_oped(pchan, pnick, pdest);
				else
					this->_e_mode_op_oth(pchan, pnick, pdest);
				pchan->_addOp(pdest);
			}
			else
			{
				this->_e_mode_deop_gen(pchan, pnick, pdest);
				if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase(pdest->getNick()) == lowCase(this->_nickname)))
					this->_e_mode_deop_him(pchan, pnick);
				else if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase(pdest->getNick()) != lowCase(this->_nickname)))
					this->_e_mode_deop_deoper(pchan, pnick, pdest);
				else if ((lowCase(pnick->getNick()) != lowCase(this->_nickname)) && (lowCase(pdest->getNick()) == lowCase(this->_nickname)))
					this->_e_mode_deop_deoped(pchan, pnick, pdest);
				else
					this->_e_mode_deop_oth(pchan, pnick, pdest);
				pchan->_delOp(pdest);
			}
		}
		else if (modes.at(i) == 'h')
		{
			param = gettok(query, arg, ' ');
			arg++;
			pdest = this->_getUserByName(param);
			if (pdest == NULL)
				continue ;
			if (sign > 0)
			{
				this->_e_mode_halfop_gen(pchan, pnick, pdest);
				if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase(pdest->getNick()) == lowCase(this->_nickname)))
					this->_e_mode_halfop_him(pchan, pnick);
				else if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase(pdest->getNick()) != lowCase(this->_nickname)))
					this->_e_mode_halfop_halfoper(pchan, pnick, pdest);
				else if ((lowCase(pnick->getNick()) != lowCase(this->_nickname)) && (lowCase(pdest->getNick()) == lowCase(this->_nickname)))
					this->_e_mode_halfop_halfoped(pchan, pnick, pdest);
				else
					this->_e_mode_halfop_oth(pchan, pnick, pdest);
				pchan->_addHalfop(pdest);
			}
			else
			{
				this->_e_mode_dehalfop_gen(pchan, pnick, pdest);
				if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase(pdest->getNick()) == lowCase(this->_nickname)))
					this->_e_mode_dehalfop_him(pchan, pnick);
				else if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase(pdest->getNick()) != lowCase(this->_nickname)))
					this->_e_mode_dehalfop_dehalfoper(pchan, pnick, pdest);
				else if ((lowCase(pnick->getNick()) != lowCase(this->_nickname)) && (lowCase(pdest->getNick()) == lowCase(this->_nickname)))
					this->_e_mode_dehalfop_dehalfoped(pchan, pnick, pdest);
				else
					this->_e_mode_dehalfop_oth(pchan, pnick, pdest);
				pchan->_delHalfop(pdest);
			}
		}
		else if (modes.at(i) == 'v')
		{
			param = gettok(query, arg, ' ');
			arg++;
			pdest = this->_getUserByName(param);
			if (pdest == NULL)
				continue ;
			if (sign > 0)
			{
				this->_e_mode_voice_gen(pchan, pnick, pdest);
				if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase(pdest->getNick()) == lowCase(this->_nickname)))
					this->_e_mode_voice_him(pchan, pnick);
				else if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase(pdest->getNick()) != lowCase(this->_nickname)))
					this->_e_mode_voice_voiceer(pchan, pnick, pdest);
				else if ((lowCase(pnick->getNick()) != lowCase(this->_nickname)) && (lowCase(pdest->getNick()) == lowCase(this->_nickname)))
					this->_e_mode_voice_voiceed(pchan, pnick, pdest);
				else
					this->_e_mode_voice_oth(pchan, pnick, pdest);
				pchan->_addVoice(pdest);
			}
			else
			{
				this->_e_mode_devoice_gen(pchan, pnick, pdest);
				if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase(pdest->getNick()) == lowCase(this->_nickname)))
					this->_e_mode_devoice_him(pchan, pnick);
				else if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase(pdest->getNick()) != lowCase(this->_nickname)))
					this->_e_mode_devoice_devoiceer(pchan, pnick, pdest);
				else if ((lowCase(pnick->getNick()) != lowCase(this->_nickname)) && (lowCase(pdest->getNick()) == lowCase(this->_nickname)))
					this->_e_mode_devoice_devoiceed(pchan, pnick, pdest);
				else
					this->_e_mode_devoice_oth(pchan, pnick, pdest);
				pchan->_delVoice(pdest);
			}
		}
		else if (modes.at(i) == 'b')
		{
			param = gettok(query, arg, ' ');
			arg++;
			if (sign > 0)
			{
				this->_e_mode_ban_gen(pchan, pnick, param);
				for (it = pchan->_all.begin(); (it != pchan->_all.end()); it++)
					if (this->_userMatch((*it), param) == true)
					{
						this->_e_mode_ban_gen(pchan, pnick, (*it), param);
						if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase((*it)->getNick()) == lowCase(this->_nickname)))
							this->_e_mode_ban_him(pchan, pnick, param);
						else if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase((*it)->getNick()) != lowCase(this->_nickname)))
							this->_e_mode_ban_baner(pchan, pnick, (*it), param);
						else if ((lowCase(pnick->getNick()) != lowCase(this->_nickname)) && (lowCase((*it)->getNick()) == lowCase(this->_nickname)))
							this->_e_mode_ban_baned(pchan, pnick, (*it), param);
						else
							this->_e_mode_ban_oth(pchan, pnick, (*it), param);
					}
			}
			else
			{
				this->_e_mode_deban_gen(pchan, pnick, param);
				for (it = pchan->_all.begin(); (it != pchan->_all.end()); it++)
					if (this->_userMatch((*it), param) == true)
					{
						this->_e_mode_deban_gen(pchan, pnick, (*it), param);
						if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase((*it)->getNick()) == lowCase(this->_nickname)))
							this->_e_mode_deban_him(pchan, pnick, param);
						else if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase((*it)->getNick()) != lowCase(this->_nickname)))
							this->_e_mode_deban_debaner(pchan, pnick, (*it), param);
						else if ((lowCase(pnick->getNick()) != lowCase(this->_nickname)) && (lowCase((*it)->getNick()) == lowCase(this->_nickname)))
							this->_e_mode_deban_debaned(pchan, pnick, (*it), param);
						else
							this->_e_mode_deban_oth(pchan, pnick, (*it), param);
					}
			}
		}
		else if (modes.at(i) == 'e')
		{
			param = gettok(query, arg, ' ');
			arg++;
			if (sign > 0)
			{
				this->_e_mode_except_gen(pchan, pnick, param);
				for (it = pchan->_all.begin(); (it != pchan->_all.end()); it++)
					if (this->_userMatch((*it), param) == true)
					{
						this->_e_mode_except_gen(pchan, pnick, (*it), param);
						if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase((*it)->getNick()) == lowCase(this->_nickname)))
							this->_e_mode_except_him(pchan, pnick, param);
						else if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase((*it)->getNick()) != lowCase(this->_nickname)))
							this->_e_mode_except_excepter(pchan, pnick, (*it), param);
						else if ((lowCase(pnick->getNick()) != lowCase(this->_nickname)) && (lowCase((*it)->getNick()) == lowCase(this->_nickname)))
							this->_e_mode_except_excepted(pchan, pnick, (*it), param);
						else
							this->_e_mode_except_oth(pchan, pnick, (*it), param);
					}
			}
			else
			{
				this->_e_mode_deexcept_gen(pchan, pnick, param);
				for (it = pchan->_all.begin(); (it != pchan->_all.end()); it++)
					if (this->_userMatch((*it), param) == true)
					{
						this->_e_mode_deexcept_gen(pchan, pnick, (*it), param);
						if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase((*it)->getNick()) == lowCase(this->_nickname)))
							this->_e_mode_deexcept_him(pchan, pnick, param);
						else if ((lowCase(pnick->getNick()) == lowCase(this->_nickname)) && (lowCase((*it)->getNick()) != lowCase(this->_nickname)))
							this->_e_mode_deexcept_deexcepter(pchan, pnick, (*it), param);
						else if ((lowCase(pnick->getNick()) != lowCase(this->_nickname)) && (lowCase((*it)->getNick()) == lowCase(this->_nickname)))
							this->_e_mode_deexcept_deexcepted(pchan, pnick, (*it), param);
						else
							this->_e_mode_deexcept_oth(pchan, pnick, (*it), param);
					}
			}
		}
		if ((modes.at(i) == 'p') || (modes.at(i) == 's') || (modes.at(i) == 'i') ||
			(modes.at(i) == 't') || (modes.at(i) == 'n') || (modes.at(i) == 'k') ||
			(modes.at(i) == 'l') || (modes.at(i) == 'm'))
		{
			mode = ((sign > 0) ? std::string("+") : std::string("-"));
			mode += modes.at(i);
			if (param.size() > 0)
			{
				mode += " ";
				mode += param;
			}
			if (this->_uevents[code] != NULL)
				(this->_uevents[code])(*this, this->_uevent_ptr, pnick, pchan, NULL, mode);
			if (nick == lowCase(this->_nickname))
				this->_e_mode_bot(pnick, pchan, mode);
			else
				this->_e_mode_oth(pnick, pchan, mode);
		}
		param.clear();
		mode.clear();
	}
	return ;
}

void			CliIRC::_e_mode_user(UserIRC *nick, std::string modes)
{
	std::string	code("usermode");
	std::string	mode;
	int			sign;
	int			i;

	sign = 1;
	for (i = 0; (i < (int)modes.length()); i++)
	{
		if (modes.at(i) == '-')
			sign = -1;
		else if (modes.at(i) == '+')
			sign = 1;
		else
		{
			mode = ((sign > 0) ? std::string("+") : std::string("-"));
			mode += modes.at(i);
			if (mode == "+o")
				this->_ircop = true;
			if (mode == "-o")
				this->_ircop = false;
			if (this->_uevents[code] != NULL)
				(this->_uevents[code])(*this, this->_uevent_ptr, nick, NULL, NULL, mode);
		}
	}
	return ;
}

void			CliIRC::_e_mode_bot(UserIRC *nick, ChannelIRC *chan, std::string mode)
{
	std::string	code("botmode");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, nick, chan, NULL, mode);
	return ;
}

void			CliIRC::_e_mode_oth(UserIRC *nick, ChannelIRC *chan, std::string mode)
{
	std::string	code("othmode");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, nick, chan, NULL, mode);
	return ;
}

void			CliIRC::_e_mode_private(UserIRC *nick, ChannelIRC *chan)
{
	std::string	code("modeprivate");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, nick, chan, NULL, "");
	return ;
}

void			CliIRC::_e_mode_deprivate(UserIRC *nick, ChannelIRC *chan)
{
	std::string	code("modedeprivate");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, nick, chan, NULL, "");
	return ;
}

void			CliIRC::_e_mode_secret(UserIRC *nick, ChannelIRC *chan)
{
	std::string	code("modesecret");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, nick, chan, NULL, "");
	return ;
}

void			CliIRC::_e_mode_desecret(UserIRC *nick, ChannelIRC *chan)
{
	std::string	code("modedesecret");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, nick, chan, NULL, "");
	return ;
}

void			CliIRC::_e_mode_invite(UserIRC *nick, ChannelIRC *chan)
{
	std::string	code("modeinvite");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, nick, chan, NULL, "");
	return ;
}

void			CliIRC::_e_mode_deinvite(UserIRC *nick, ChannelIRC *chan)
{
	std::string	code("modedeinvite");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, nick, chan, NULL, "");
	return ;
}

void			CliIRC::_e_mode_topic(UserIRC *nick, ChannelIRC *chan)
{
	std::string	code("modetopic");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, nick, chan, NULL, "");
	return ;
}

void			CliIRC::_e_mode_detopic(UserIRC *nick, ChannelIRC *chan)
{
	std::string	code("modedetopic");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, nick, chan, NULL, "");
	return ;
}

void			CliIRC::_e_mode_extern(UserIRC *nick, ChannelIRC *chan)
{
	std::string	code("modeextern");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, nick, chan, NULL, "");
	return ;
}

void			CliIRC::_e_mode_deextern(UserIRC *nick, ChannelIRC *chan)
{
	std::string	code("modedeextern");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, nick, chan, NULL, "");
	return ;
}

void			CliIRC::_e_mode_key(UserIRC *nick, ChannelIRC *chan, std::string pass)
{
	std::string	code("modekey");

	chan->_setPass(pass);
	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, nick, chan, NULL, pass);
	return ;
}

void			CliIRC::_e_mode_dekey(UserIRC *nick, ChannelIRC *chan)
{
	std::string	code("modedekey");

	chan->_setPass("");
	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, nick, chan, NULL, "");
	return ;
}

void			CliIRC::_e_mode_limit(UserIRC *nick, ChannelIRC *chan, std::string limit)
{
	std::string	code("modelimit");

	chan->_setLimit(limit);
	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, nick, chan, NULL, limit);
	return ;
}

void			CliIRC::_e_mode_delimit(UserIRC *nick, ChannelIRC *chan)
{
	std::string	code("modedelimit");

	chan->_setLimit("-1");
	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, nick, chan, NULL, "");
	return ;
}

void			CliIRC::_e_mode_moderate(UserIRC *nick, ChannelIRC *chan)
{
	std::string	code("modemoderate");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, nick, chan, NULL, "");
	return ;
}

void			CliIRC::_e_mode_demoderate(UserIRC *nick, ChannelIRC *chan)
{
	std::string	code("modedemoderate");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, nick, chan, NULL, "");
	return ;
}

void			CliIRC::_e_mode_owner_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("genowner");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_owner_him(ChannelIRC *chan, UserIRC *user)
{
	std::string	code("ownerhim");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, user, chan, NULL, "");
	return ;
}

void			CliIRC::_e_mode_owner_ownerer(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("ownerownerer");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_owner_ownered(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("ownerownered");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_owner_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("othowner");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_deowner_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("gendeowner");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_deowner_him(ChannelIRC *chan, UserIRC *user)
{
	std::string	code("deownerhim");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, user, chan, NULL, "");
	return ;
}

void			CliIRC::_e_mode_deowner_deownerer(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("deownerdeownerer");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_deowner_deownered(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("deownerdeownered");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_deowner_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("othdeowner");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_protect_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("genprotect");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_protect_him(ChannelIRC *chan, UserIRC *user)
{
	std::string	code("protecthim");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, user, chan, NULL, "");
	return ;
}

void			CliIRC::_e_mode_protect_protecter(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("protectprotecter");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_protect_protected(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("protectprotected");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_protect_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("othprotect");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_deprotect_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("gendeprotect");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_deprotect_him(ChannelIRC *chan, UserIRC *user)
{
	std::string	code("deprotecthim");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, user, chan, NULL, "");
	return ;
}

void			CliIRC::_e_mode_deprotect_deprotecter(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("deprotectdeprotecter");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_deprotect_deprotected(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("deprotectdeprotected");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_deprotect_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("othdeprotect");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_op_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("genop");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_op_him(ChannelIRC *chan, UserIRC *user)
{
	std::string	code("ophim");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, user, chan, NULL, "");
	return ;
}

void			CliIRC::_e_mode_op_oper(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("opoper");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_op_oped(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("opoped");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_op_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("othop");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_deop_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("gendeop");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_deop_him(ChannelIRC *chan, UserIRC *user)
{
	std::string	code("deophim");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, user, chan, NULL, "");
	return ;
}

void			CliIRC::_e_mode_deop_deoper(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("deopdeoper");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_deop_deoped(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("deopdeoped");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_deop_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("othdeop");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_halfop_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("genhalfop");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_halfop_him(ChannelIRC *chan, UserIRC *user)
{
	std::string	code("halfophim");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, user, chan, NULL, "");
	return ;
}

void			CliIRC::_e_mode_halfop_halfoper(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("halfophalfoper");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_halfop_halfoped(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("halfophalfoped");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_halfop_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("othhalfop");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_dehalfop_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("gendehalfop");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_dehalfop_him(ChannelIRC *chan, UserIRC *user)
{
	std::string	code("dehalfophim");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, user, chan, NULL, "");
	return ;
}

void			CliIRC::_e_mode_dehalfop_dehalfoper(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("dehalfopdehalfoper");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_dehalfop_dehalfoped(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("dehalfopdehalfoped");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_dehalfop_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("othdehalfop");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_voice_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("genvoice");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_voice_him(ChannelIRC *chan, UserIRC *user)
{
	std::string	code("voicehim");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, user, chan, NULL, "");
	return ;
}

void			CliIRC::_e_mode_voice_voiceer(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("voicevoiceer");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_voice_voiceed(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("voicevoiceed");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_voice_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("othvoice");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_devoice_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("gendevoice");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_devoice_him(ChannelIRC *chan, UserIRC *user)
{
	std::string	code("devoicehim");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, user, chan, NULL, "");
	return ;
}

void			CliIRC::_e_mode_devoice_devoiceer(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("devoicedevoiceer");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_devoice_devoiceed(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("devoicedevoiceed");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void		CliIRC::_e_mode_devoice_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst)
{
	std::string	code("othdevoice");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, "");
	return ;
}

void			CliIRC::_e_mode_ban_gen(ChannelIRC *chan, UserIRC *src, std::string pattern)
{
	std::string	code("genbanpattern");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, NULL, pattern);
	return ;
}

void			CliIRC::_e_mode_ban_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern)
{
	std::string	code("genban");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, pattern);
	return ;
}

void			CliIRC::_e_mode_ban_him(ChannelIRC *chan, UserIRC *user, std::string pattern)
{
	std::string	code("banhim");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, user, chan, NULL, pattern);
	return ;
}

void			CliIRC::_e_mode_ban_baner(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern)
{
	std::string	code("banbaner");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, pattern);
	return ;
}

void			CliIRC::_e_mode_ban_baned(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern)
{
	std::string	code("banbaned");
	std::string	command;

	if ((this->_ircop == true) || (dst->isOp(chan->getName()) == true))
	{
		command = "-b " + pattern;
		this->mode(chan->getName(), command);
	}
	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, pattern);
	return ;
}

void			CliIRC::_e_mode_ban_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern)
{
	std::string	code("othban");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, pattern);
	return ;
}

void			CliIRC::_e_mode_deban_gen(ChannelIRC *chan, UserIRC *src, std::string pattern)
{
	std::string	code("gendebanpattern");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, NULL, pattern);
	return ;
}

void			CliIRC::_e_mode_deban_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern)
{
	std::string	code("gendeban");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, pattern);
	return ;
}

void			CliIRC::_e_mode_deban_him(ChannelIRC *chan, UserIRC *user, std::string pattern)
{
	std::string	code("debanhim");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, user, chan, NULL, pattern);
	return ;
}

void			CliIRC::_e_mode_deban_debaner(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern)
{
	std::string	code("debandebaner");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, pattern);
	return ;
}

void			CliIRC::_e_mode_deban_debaned(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern)
{
	std::string	code("debandebaned");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, pattern);
	return ;
}

void		CliIRC::_e_mode_deban_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern)
{
	std::string	code("othdeban");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, pattern);
	return ;
}

void			CliIRC::_e_mode_except_gen(ChannelIRC *chan, UserIRC *src, std::string pattern)
{
	std::string	code("genexceptpattern");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, NULL, pattern);
	return ;
}

void			CliIRC::_e_mode_except_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern)
{
	std::string	code("genexcept");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, pattern);
	return ;
}

void			CliIRC::_e_mode_except_him(ChannelIRC *chan, UserIRC *user, std::string pattern)
{
	std::string	code("excepthim");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, user, chan, NULL, pattern);
	return ;
}

void			CliIRC::_e_mode_except_excepter(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern)
{
	std::string	code("exceptexcepter");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, pattern);
	return ;
}

void			CliIRC::_e_mode_except_excepted(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern)
{
	std::string	code("exceptexcepted");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, pattern);
	return ;
}

void			CliIRC::_e_mode_except_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern)
{
	std::string	code("othexcept");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, pattern);
	return ;
}

void			CliIRC::_e_mode_deexcept_gen(ChannelIRC *chan, UserIRC *src, std::string pattern)
{
	std::string	code("gendeexceptpattern");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, NULL, pattern);
	return ;
}

void			CliIRC::_e_mode_deexcept_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern)
{
	std::string	code("gendeexcept");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, pattern);
	return ;
}

void			CliIRC::_e_mode_deexcept_him(ChannelIRC *chan, UserIRC *user, std::string pattern)
{
	std::string	code("deexcepthim");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, user, chan, NULL, pattern);
	return ;
}

void			CliIRC::_e_mode_deexcept_deexcepter(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern)
{
	std::string	code("deexceptdeexcepter");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, pattern);
	return ;
}

void			CliIRC::_e_mode_deexcept_deexcepted(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern)
{
	std::string	code("deexceptdeexcepted");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, pattern);
	return ;
}

void			CliIRC::_e_mode_deexcept_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern)
{
	std::string	code("othdeexcept");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, chan, dst, pattern);
	return ;
}

void			CliIRC::_e_privmsg(std::string query)
{
	std::string	src;
	std::string	dst;
	std::string	msg;
	UserIRC		*psrc;
	UserIRC		*pdst;
	ChannelIRC	*chan;

	src = lowCase(gettok(gettok(gettok(query, 1, ':'), 1, ' '), 1, '!'));
	dst = lowCase(gettok(query, 3, ' '));
	msg = fulltok(query, 2, ':');
	psrc = this->_getUserByName(src);
	if (psrc == NULL)
		return ;
	if (this->_isChannel(dst) == false)
	{
		if (lowCase(dst) != lowCase(this->_nickname))
			return ;
		pdst = this->_getUserByName(dst);
		if (pdst == NULL)
			return ;
		if ((msg.at(0) == (char)0x1) && (msg.at(msg.length() - 1) == (char)0x1))
		{
			msg = right(msg, (msg.length() - 1));
			msg = left(msg, (msg.length() - 1));
			this->_e_privmsg_botctcp(psrc, pdst, msg);
			return ;
		}
		this->_e_privmsg_bot(psrc, pdst, msg);
		return ;
	}
	chan = this->_getChanByName(dst);
	if (chan == NULL)
		return ;
	if ((msg.at(0) == (char)0x1) && (msg.at(msg.length() - 1) == (char)0x1))
	{
		msg = right(msg, (msg.length() - 1));
		msg = left(msg, (msg.length() - 1));
		this->_e_privmsg_othctcp(psrc, chan, msg);
		return ;
	}
	this->_e_privmsg_oth(psrc, chan, msg);
	return ;
}

void			CliIRC::_e_privmsg_bot(UserIRC *src, UserIRC *dst, std::string msg)
{
	std::string	code("botmsg");
	std::string	reason;

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, NULL, dst, msg);
	return ;
}

void			CliIRC::_e_privmsg_oth(UserIRC *user, ChannelIRC *chan, std::string msg)
{
	std::string	code("othmsg");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, user, chan, NULL, msg);
	return ;
}

void			CliIRC::_e_privmsg_botctcp(UserIRC *src, UserIRC *dst, std::string msg)
{
	std::string	code("botctcp");
	std::string	reply;
	std::string	asctm;
    struct tm	*ptime;
    time_t		actime;

	if (gettok(msg, 1, ' ') == "PING")
	{
		if (numtok(msg, ' ') == 1)
			reply = "PING " + intToString((int)::time(0));
		else
			reply = "PING " + gettok(msg, 2, ' ');
		this->ctcpreply(src->getNick(), reply);
	}
	if (msg == "TIME")
	{
		::time(&actime);
		ptime = ::localtime(&actime);
		asctm = asctime(ptime);
		reply = "TIME " + left(asctm, (asctm.length() - 1));
		this->ctcpreply(src->getNick(), reply);
	}
	if (msg == "VERSION")
	{
		reply = "VERSION IRC Game Engine - Written in C++ by Ninsuo, Alain TIEMBLO ( alain@ocarina.fr ).";
		this->ctcpreply(src->getNick(), reply);
	}
	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, NULL, dst, msg);
	return ;
}

void			CliIRC::_e_privmsg_othctcp(UserIRC *user, ChannelIRC *chan, std::string msg)
{
	std::string	code("othctcp");
	std::string	reply;
	std::string	asctm;
    struct tm	*ptime;
    time_t		actime;

	if (gettok(msg, 1, ' ') == "PING")
	{
		if (numtok(msg, ' ') == 1)
			reply = "PING " + intToString((int)::time(0));
		else
			reply = "PING " + gettok(msg, 2, ' ');
		this->ctcpreply(lowCase(user->getNick()), reply);
	}
	if (msg == "TIME")
	{
		::time(&actime);
		ptime = ::localtime(&actime);
		asctm = asctime(ptime);
		reply = "TIME " + left(asctm, (asctm.length() - 1));
		this->ctcpreply(lowCase(user->getNick()), reply);
	}
	if (msg == "VERSION")
	{
		reply = "VERSION IBot - Written in C++ by Ninsuo, Alain TIEMBLO ( alain@ocarina.fr ).";
		this->ctcpreply(lowCase(user->getNick()), reply);
	}
	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, user, chan, NULL, msg);
	return ;
}

void			CliIRC::_e_notice(std::string query)
{
	std::string	src;
	std::string	dst;
	std::string	notice;
	UserIRC		*psrc;
	UserIRC		*pdst;
	ChannelIRC	*chan;

	src = gettok(gettok(gettok(query, 1, ':'), 1, ' '), 1, '!');
	dst = gettok(query, 3, ' ');
	if ((numtok(src, '.') > 1) && (dst == "AUTH"))
		return ;
	notice = fulltok(query, 2, ':');
	psrc = this->_getUserByName(src);
	if (psrc == NULL)
		return ;
	if (this->_isChannel(dst) == false)
	{
		if (lowCase(dst) != lowCase(this->_nickname))
			return ;
		pdst = this->_getUserByName(dst);
		if (pdst == NULL)
			return ;
		if ((notice.at(0) == (char)0x1) && (notice.at(notice.length() - 1) == (char)0x1))
		{
			notice = right(notice, (notice.length() - 1));
			notice = left(notice, (notice.length() - 1));
			this->_e_notice_botctcpreply(psrc, pdst, notice);
			return ;
		}
		this->_e_notice_bot(psrc, pdst, notice);
		return ;
	}
	chan = this->_getChanByName(dst);
	if (chan == NULL)
		return ;
	if ((notice.at(0) == (char)0x1) && (notice.at(notice.length() - 1) == (char)0x1))
	{
		notice = right(notice, (notice.length() - 1));
		notice = left(notice, (notice.length() - 1));
		this->_e_notice_othctcpreply(psrc, chan, notice);
		return ;
	}
	this->_e_notice_oth(psrc, chan, notice);
	return ;
}

void			CliIRC::_e_notice_bot(UserIRC *src, UserIRC *dst, std::string notice)
{
	std::string	code("botnotice");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, NULL, dst, notice);
	return ;
}

void			CliIRC::_e_notice_oth(UserIRC *user, ChannelIRC *chan, std::string notice)
{
	std::string	code("othnotice");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, user, chan, NULL, notice);
	return ;
}

void			CliIRC::_e_notice_botctcpreply(UserIRC *src, UserIRC *dst, std::string notice)
{
	std::string	code("botctcpreply");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, src, NULL, dst, notice);
	return ;
}

void			CliIRC::_e_notice_othctcpreply(UserIRC *user, ChannelIRC *chan, std::string notice)
{
	std::string	code("othctcpreply");

	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, user, chan, NULL, notice);
	return ;
}

void			CliIRC::_e_error(std::string query)
{
	std::string	code("error");

	query = fulltok(query, 2, ':');
	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, NULL, NULL, NULL, query);
	return ;
}

void			CliIRC::_e_raw(std::string query)
{
	std::string	code("raw");
	std::string	small;
	int			raw;

	raw = ::atoi(gettok(query, 2, ' ').c_str());
	small = gettok(query, 2, ' ') + " " + fulltok(query, 4, ' ');
	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, NULL, NULL, NULL, small);
	switch (raw)
	{
		case 001:
			this->_e_raw_welcome(query);
			break ;
		case 305:
			this->_e_raw_away_back();
			break;
		case 306:
			this->_e_raw_away_now();
			break;
		case 324:
			this->_e_raw_modes(query);
			break ;
		case 332:
			this->_e_raw_topic(query);
			break ;
		case 352:
			this->_e_raw_who(query);
			break ;
		case 353:
			this->_e_raw_names(query);
			break ;
		case 433:
			this->_e_raw_alreadyused(query);
			break ;
		case 471:
			this->_e_raw_limit(query);
			break ;
		case 473:
			this->_e_raw_invite(query);
			break ;
		case 474:
			this->_e_raw_banned(query);
			break ;
		case 475:
			this->_e_raw_key(query);
			break ;
		default:
			break ;
	}
	return ;
}

void			CliIRC::_e_raw_welcome(std::string query)
{
	std::string	code("welcome");
	std::string	msg;
	std::string	nick;
	UserIRC		*pnick;

	nick = lowCase(gettok(query, 3, ' '));
	if (this->_getUserByName(nick) == NULL)
	{
		if (this->_isNickname(nick) == false)
			return ;
		pnick = new UserIRC(gettok(query, 3, ' '));
		this->_users.push_back(pnick);
	}
	if (this->_uevents[code] != NULL)
		(this->_uevents[code])(*this, this->_uevent_ptr, NULL, NULL, NULL, "");
	if (this->_config == NULL)
		return ;
	if (((*this->_config)["OPTIONS"]["ircop"]) == "yes")
	{
		this->oper(((*this->_config)["IRCOP"]["login"]), ((*this->_config)["IRCOP"]["password"]));
	}
	this->_client << "NICK " << (*this->_config)["USER"]["nickname"] << "\n";
	if (((*this->_config)["OPTIONS"]["register"]) == "yes")
	{
		msg = "IDENTIFY " + ((*this->_config)["REGISTER"]["password"]);
		this->say(((*this->_config)["REGISTER"]["nickname"]), msg);
	}
	if (((*this->_config)["OPTIONS"]["autojoin"]) == "yes")
	{
		this->configApplyToList("AUTOJOIN", "JOIN ", "");
	}
	return ;
}

void			CliIRC::_e_raw_away_back(void)
{
	this->_away = false;
	return ;
}

void			CliIRC::_e_raw_away_now(void)
{
	this->_away = true;
	return ;
}

void			CliIRC::_e_raw_who(std::string query)
{
	UserIRC		*pnick;
	ChannelIRC	*pchan;
	std::string	nick;
	std::string	chan;
	std::string	modes;

	modes = gettok(query, 9, ' ');
	nick = lowCase(gettok(query, 8, ' '));
	pnick = this->_getUserByName(nick);
	if (pnick == NULL)
	{
		if (this->_isNickname(nick) == false)
			return ;
		pnick = new UserIRC(gettok(query, 8, ' '));
		this->_users.push_back(pnick);
	}
	pnick->_setUsername(gettok(query, 5, ' '));
	pnick->_setHostname(gettok(query, 6, ' '));
	pnick->_setFullname(fulltok(fulltok(query, 2, ':'), 2, ' '));
	if (modes.find("*") != std::string::npos)
		pnick->_setIrcop(true);
	else
		pnick->_setIrcop(false);
	chan = lowCase(gettok(query, 4, ' '));
	pchan = this->_getChanByName(chan);
	if (pchan != NULL)
	{
		pnick->_addChannel(pchan);
		pchan->_addUser(pnick);
		if (modes.find("~") != std::string::npos)
			pchan->_addOwner(pnick);
		if (modes.find("&") != std::string::npos)
			pchan->_addProtect(pnick);
		if (modes.find("@") != std::string::npos)
			pchan->_addOp(pnick);
		if (modes.find("%") != std::string::npos)
			pchan->_addHalfop(pnick);
		if (modes.find("+") != std::string::npos)
			pchan->_addVoice(pnick);
	}
	return ;
}

void			CliIRC::_e_raw_topic(std::string query)
{
	std::string	chan;
	std::string	topic;
	ChannelIRC	*pchan;

	chan = lowCase(gettok(query, 4, ' '));
	pchan = this->_getChanByName(chan);
	if (pchan == NULL)
		return ;
	topic = fulltok(query, 2, ':');
	pchan->_setTopic(topic);
	return ;
}

void			CliIRC::_e_raw_names(std::string query)
{
	std::string	nick;
	std::string	chan;
	UserIRC		*pnick;
	ChannelIRC	*pchan;
	int			num;
	char		c;
	int			i;

	chan = lowCase(gettok(query, 5, ' '));
	pchan = this->_getChanByName(chan);
	if (pchan == NULL)
		return ;
	num = numtok(gettok(query, 2, ':'), ' ');
	for (i = 1; (i <= num); i++)
	{
		nick = gettok(query, i, ' ');
		if (nick.size() == 0)
			continue ;
		c = left(nick, 1).c_str()[0];
		if ((c == '~') || (c == '&') || (c == '@') || (c == '%') || (c == '+'))
			nick = lowCase(right(nick, (nick.size() - 1)));
		pnick = this->_getUserByName(nick);
		if (pnick == NULL)
		{
			if (this->_isNickname(nick) == false)
				return ;
			pnick = new UserIRC(gettok(query, i, ' '));
			this->_users.push_back(pnick);
		}
		pnick->_addChannel(pchan);
		pchan->_addUser(pnick);
		if (c == '~')
			pchan->_addOwner(pnick);
		if (c == '&')
			pchan->_addProtect(pnick);
		if (c == '@')
			pchan->_addOp(pnick);
		if (c == '%')
			pchan->_addHalfop(pnick);
		if (c == '+')
			pchan->_addVoice(pnick);
	}
	return ;
}

void			CliIRC::_e_raw_alreadyused(std::string query)
{
	std::string	randnick;

	query = query;
	//randnick = this->_nickname.at(0) + this->_getRandString(9);
	randnick = this->_nickname + "_";
	this->_nickname = randnick;
	this->_client << "NICK " << randnick << "\n";
}

void			CliIRC::_e_raw_modes(std::string query)
{
	std::string	chan;
	std::string	modes;
	std::string	mode;
	ChannelIRC	*pchan;
	int			arg;
	int			i;

	chan = lowCase(gettok(query, 4, ' '));
	pchan = this->_getChanByName(chan);
	if (pchan == NULL)
		return ;
	modes = gettok(query, 5, ' ');
	arg = 6;
	for (i = 0; (i < (int)modes.length()); i++)
	{
		if ((modes.at(i) == '-') || (modes.at(i) == '+'))
			continue ;
		else
		{
			mode = modes.at(i);
			if (mode == "k")
			{
				pchan->_setPass(gettok(query, arg, ' '));
				arg++;
			}
			if (mode == "l")
			{
				pchan->_setLimit(gettok(query, arg, ' '));
				arg++;
			}
			pchan->_setMode(mode);
		}
	}
	return ;
}

void			CliIRC::_e_raw_limit(std::string query)
{
	std::string	chan;
	std::string	*save;

	chan = gettok(query, 4, ' ');
	if (this->configIsOnList("AUTOJOIN", chan, 1, ' ') != "-1")
	{
		save = new std::string(chan);
		this->_timer.set(1, 5, this->_rejoin, (void *)this, (void *)save, NULL);
	}
	return ;
}

void			CliIRC::_e_raw_invite(std::string query)
{
	std::string	chan;
	std::string	*save;

	chan = gettok(query, 4, ' ');
	if (this->configIsOnList("AUTOJOIN", chan, 1, ' ') != "-1")
	{
		save = new std::string(chan);
		this->_timer.set(1, 5, this->_rejoin, (void *)this, (void *)save, NULL);
	}
	return ;
}

void			CliIRC::_e_raw_banned(std::string query)
{
	std::string	chan;
	std::string	*save;

	chan = gettok(query, 4, ' ');
	if (this->configIsOnList("AUTOJOIN", chan, 1, ' ') != "-1")
	{
		save = new std::string(chan);
		this->_timer.set(1, 5, this->_rejoin, (void *)this, (void *)save, NULL);
	}
	return ;
}

void			CliIRC::_e_raw_key(std::string query)
{
	std::string	chan;
	std::string	*save;

	chan = gettok(query, 4, ' ');
	if (this->configIsOnList("AUTOJOIN", chan, 1, ' ') != "-1")
	{
		save = new std::string(chan);
		this->_timer.set(1, 5, this->_rejoin, (void *)this, (void *)save, NULL);
	}
	return ;
}

void			CliIRC::_rejoin(void *p1, void *p2, void *p3)
{
	CliIRC		*cli;
	std::string	*chan;

	cli = (CliIRC *)p1;
	chan = (std::string *)p2;
	cli->join(*chan);
	delete chan;
	p3 = p3;
	return ;
}

void			CliIRC::_logQuery(std::string query)
{
	if (this->_logStream.is_open() == false)
		this->_logStream.open(this->_logFile.c_str(), std::ios_base::app );
	if (this->_logStream.is_open() == true)
		this->_logStream << this->strip(query) << "\n";
	return ;
}

/* main command functions */

std::string			CliIRC::lang(std::string tag, const char *var, ...)
{
	std::string		origin;
	std::string		query;
	va_list			vg;
	int				i;

	if ((*this->_lang)[tag].size() == 0)
		return (query);
	if ((*this->_lang)[tag][var] == "")
		return (query);
	origin = (*this->_lang)[tag][var];
	va_start(vg, var);
	for (i = 0; (i < (int)origin.size()); i++)
	{
		if ((origin.at(i) == '$') && ((i + 1) < (int)origin.size()) && (origin.at(i + 1) == '?'))
		{
			query += va_arg(vg, const char *);
			i++;
		}
		else if ((origin.at(i) == '$') && ((i + 1) < (int)origin.size()) && (origin.at(i + 1) == 'u'))
		{
			query += "\037";
			i++;
		}
		else if ((origin.at(i) == '$') && ((i + 1) < (int)origin.size()) && (origin.at(i + 1) == 'b'))
		{
			query += "\02";
			i++;
		}
		else if ((origin.at(i) == '$') && ((i + 1) < (int)origin.size()) && (origin.at(i + 1) == 'k'))
		{
			query += "\03";
			i++;
		}
		else
			query += origin.at(i);
	}
	va_end(vg);
	return (query);
}

void		CliIRC::_loadLangage(MyINI& config)
{
	MyINI	*cfg;
	cfg = &config;
	_lang = new MyINI("langage.ini");
	if (_lang == NULL)
	{
		std::cerr << "Could not load langage file 'langage.ini'." << std::endl;
		::exit(1);
	}
	return ;
}
