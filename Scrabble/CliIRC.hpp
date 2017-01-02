
#ifndef __CLIIRC_HPP__
#define __CLIIRC_HPP__

#include <map>
#include <list>
#include <string>
#include <fstream>

#include <stdarg.h>

#include "MyUser.hpp"
#include "MyINI.hpp"
#include "MyTimer.hpp"
#include "MyString.hpp"

class UserIRC;
class ChannelIRC;
class Game;

class	CliIRC
{

	friend class MyTimer;
	friend class UserIRC;
	friend class ChannelIRC;
	friend class Game;

public:

	/* general functions */

	CliIRC(std::string config);
	~CliIRC();
	CliIRC(const CliIRC& client);
	CliIRC&	operator=(const CliIRC& client);
	CliIRC&	operator<<(const std::string& str);
	CliIRC&	operator<<(const char *str);
	CliIRC&	operator<<(int n);
	CliIRC&	operator<<(char c);
	bool	addEvent(std::string name, void (*fct)(CliIRC& client, void *ptr, UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string msg));
	bool	delEvent(std::string name);
	void	setEventPtr(void *ptr);
	bool	loop();
	void	thread(void (*call)(CliIRC&, void *ptr), void *ptr);
	void	wait();

	/* configuration functions */
	
	std::string	configIsOnList(std::string list, std::string elem);
	std::string	configIsOnList(std::string list, std::string elem, int numtok, char token);
	bool		configIsKey(std::string tag, std::string key);
	bool		configAddToList(std::string list, std::string elem);
	bool		configDelFromList(std::string list, std::string elem);
	bool		configApplyToList(std::string list, std::string start, std::string end);
	bool		configReload();
	bool		configSave();

	/* action functions */

	bool	connect();
	bool	disconnect();
	bool	alive();
	bool	isIrcop();
	bool	isAway();
	bool	say(std::string dest, std::string msg);
	bool	action(std::string dest, std::string msg);
	bool	notice(std::string dest, std::string msg);
	bool	ctcp(std::string dest, std::string msg);
	bool	ctcpreply(std::string dest, std::string msg);
	bool	ban(std::string chan, std::string user);
	bool	banNick(std::string chan, std::string user);
	bool	kick(std::string chan, std::string user, std::string reason);
	bool	bankick(std::string chan, std::string user, std::string reason);
	bool	kickban(std::string chan, std::string user, std::string reason);
	bool	kill(std::string user, std::string reason);
	bool	topic(std::string chan, std::string title);
	bool	mode(std::string chan, std::string modes);
	bool	mode(std::string priv_modes);
	bool	invite(std::string user, std::string chan);
	bool	join(std::string chan);
	bool	join(std::string chan, std::string pass);
	bool	part(std::string chan);
	bool	part(std::string chan, std::string reason);
	bool	hop(std::string chan);
	bool	nick(std::string nickname);
	bool	quit(std::string reason);
	bool	owner(std::string chan, std::string user);
	bool	deowner(std::string chan, std::string user);
	bool	protect(std::string chan, std::string user);
	bool	deprotect(std::string chan, std::string user);
	bool	op(std::string chan, std::string user);
	bool	deop(std::string chan, std::string user);
	bool	halfop(std::string chan, std::string user);
	bool	dehalfop(std::string chan, std::string user);
	bool	voice(std::string chan, std::string user);
	bool	devoice(std::string chan, std::string user);
	bool	oper(std::string login, std::string pass);
	bool	sajoin(std::string user, std::string chan);
	bool	sapart(std::string user, std::string chan);
	bool	chghost(std::string user, std::string host);
	bool	chgident(std::string user, std::string ident);
	bool	chgfull(std::string user, std::string full);
	bool	away(std::string reason);
	bool	back();
	bool	who(std::string target);
	bool	send(std::string request);
	bool	applyToChannels(std::string begin, std::string end);
	bool	applyToUsers(std::string begin, std::string end);
	std::string	me();
	std::string	strip(std::string msg);
	std::string	lang(std::string tag, const char *var, ...);

private:

	/* event functions */

	std::string		_getRandString(int length);
	bool			_isChannel(std::string chan);
	bool			_isNickname(std::string nick);
	bool			_isLetter(char c);
	bool			_isNumber(char c);
	bool			_isSpecial(char c);
	ChannelIRC		*_getChanByName(std::string chan);
	UserIRC			*_getUserByName(std::string nick);
	bool			_couldSpeak(std::string chan);
	bool			_userMatch(UserIRC *user, std::string pattern);
	int				_userMatchRec(const char *user, const char *pattern);
	static void		_run(MyUser& obj, void *ptr);
	void			_init_events();
	void			_e_join(std::string query);
	void			_e_join_bot(ChannelIRC *chan);
	void			_e_join_oth(ChannelIRC *chan, UserIRC *user);
	void			_e_part(std::string query);
	void			_e_part_bot(std::string query, ChannelIRC *chan);
	void			_e_part_oth(std::string query, ChannelIRC *chan, UserIRC *nick);
	void			_e_quit(std::string query);
	void			_e_nick(std::string query);
	void			_e_nick_bot(UserIRC *user, std::string newnick);
	void			_e_nick_oth(UserIRC *user, std::string newnick);
	void			_e_kick(std::string query);
	void			_e_kick_him(UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string reason);
	void			_e_kick_botkicker(UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string reason);
	void			_e_kick_botkicked(UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string reason);
	void			_e_kick_oth(UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string reason);
	void			_e_topic(std::string query);
	void			_e_topic_bot(UserIRC *user, ChannelIRC *chan, std::string topic);
	void			_e_topic_oth(UserIRC *user, ChannelIRC *chan, std::string topic);
	void			_e_mode(std::string query);
	void			_e_mode_user(UserIRC *nick, std::string modes);
	void			_e_mode_bot(UserIRC *nick, ChannelIRC *chan, std::string mode);
	void			_e_mode_oth(UserIRC *nick, ChannelIRC *chan, std::string mode);
	void			_e_mode_private(UserIRC *nick, ChannelIRC *chan);
	void			_e_mode_deprivate(UserIRC *nick, ChannelIRC *chan);
	void			_e_mode_secret(UserIRC *nick, ChannelIRC *chan);
	void			_e_mode_desecret(UserIRC *nick, ChannelIRC *chan);
	void			_e_mode_invite(UserIRC *nick, ChannelIRC *chan);
	void			_e_mode_deinvite(UserIRC *nick, ChannelIRC *chan);
	void			_e_mode_topic(UserIRC *nick, ChannelIRC *chan);
	void			_e_mode_detopic(UserIRC *nick, ChannelIRC *chan);
	void			_e_mode_extern(UserIRC *nick, ChannelIRC *chan);
	void			_e_mode_deextern(UserIRC *nick, ChannelIRC *chan);
	void			_e_mode_key(UserIRC *nick, ChannelIRC *chan, std::string pass);
	void			_e_mode_dekey(UserIRC *nick, ChannelIRC *chan);
	void			_e_mode_limit(UserIRC *nick, ChannelIRC *chan, std::string limit);
	void			_e_mode_delimit(UserIRC *nick, ChannelIRC *chan);
	void			_e_mode_moderate(UserIRC *nick, ChannelIRC *chan);
	void			_e_mode_demoderate(UserIRC *nick, ChannelIRC *chan);
	void			_e_mode_owner_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_owner_him(ChannelIRC *chan, UserIRC *user);
	void			_e_mode_owner_ownerer(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_owner_ownered(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_owner_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_deowner_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_deowner_him(ChannelIRC *chan, UserIRC *user);
	void			_e_mode_deowner_deownerer(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_deowner_deownered(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_deowner_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_protect_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_protect_him(ChannelIRC *chan, UserIRC *user);
	void			_e_mode_protect_protecter(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_protect_protected(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_protect_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_deprotect_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_deprotect_him(ChannelIRC *chan, UserIRC *user);
	void			_e_mode_deprotect_deprotecter(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_deprotect_deprotected(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_deprotect_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_op_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_op_him(ChannelIRC *chan, UserIRC *user);
	void			_e_mode_op_oper(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_op_oped(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_op_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_deop_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_deop_him(ChannelIRC *chan, UserIRC *user);
	void			_e_mode_deop_deoper(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_deop_deoped(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_deop_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_halfop_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_halfop_him(ChannelIRC *chan, UserIRC *user);
	void			_e_mode_halfop_halfoper(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_halfop_halfoped(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_halfop_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_dehalfop_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_dehalfop_him(ChannelIRC *chan, UserIRC *user);
	void			_e_mode_dehalfop_dehalfoper(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_dehalfop_dehalfoped(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_dehalfop_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_voice_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_voice_him(ChannelIRC *chan, UserIRC *user);
	void			_e_mode_voice_voiceer(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_voice_voiceed(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_voice_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_devoice_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_devoice_him(ChannelIRC *chan, UserIRC *user);
	void			_e_mode_devoice_devoiceer(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_devoice_devoiceed(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_devoice_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst);
	void			_e_mode_ban_gen(ChannelIRC *chan, UserIRC *src, std::string pattern);
	void			_e_mode_ban_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern);
	void			_e_mode_ban_him(ChannelIRC *chan, UserIRC *user, std::string pattern);
	void			_e_mode_ban_baner(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern);
	void			_e_mode_ban_baned(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern);
	void			_e_mode_ban_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern);
	void			_e_mode_deban_gen(ChannelIRC *chan, UserIRC *src, std::string pattern);
	void			_e_mode_deban_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern);
	void			_e_mode_deban_him(ChannelIRC *chan, UserIRC *user, std::string pattern);
	void			_e_mode_deban_debaner(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern);
	void			_e_mode_deban_debaned(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern);
	void			_e_mode_deban_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern);
	void			_e_mode_except_gen(ChannelIRC *chan, UserIRC *src, std::string pattern);
	void			_e_mode_except_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern);
	void			_e_mode_except_him(ChannelIRC *chan, UserIRC *user, std::string pattern);
	void			_e_mode_except_excepter(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern);
	void			_e_mode_except_excepted(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern);
	void			_e_mode_except_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern);
	void			_e_mode_deexcept_gen(ChannelIRC *chan, UserIRC *src, std::string pattern);
	void			_e_mode_deexcept_gen(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern);
	void			_e_mode_deexcept_him(ChannelIRC *chan, UserIRC *user, std::string pattern);
	void			_e_mode_deexcept_deexcepter(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern);
	void			_e_mode_deexcept_deexcepted(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern);
	void			_e_mode_deexcept_oth(ChannelIRC *chan, UserIRC *src, UserIRC *dst, std::string pattern);
	void			_e_privmsg(std::string query);
	void			_e_privmsg_bot(UserIRC *src, UserIRC *dst, std::string msg);
	void			_e_privmsg_oth(UserIRC *user, ChannelIRC *chan, std::string msg);
	void			_e_privmsg_botctcp(UserIRC *src, UserIRC *dst, std::string msg);
	void			_e_privmsg_othctcp(UserIRC *user, ChannelIRC *chan, std::string msg);
	void			_e_notice(std::string query);
	void			_e_notice_bot(UserIRC *src, UserIRC *dst, std::string msg);
	void			_e_notice_oth(UserIRC *user, ChannelIRC *chan, std::string msg);
	void			_e_notice_botctcpreply(UserIRC *src, UserIRC *dst, std::string msg);
	void			_e_notice_othctcpreply(UserIRC *user, ChannelIRC *chan, std::string msg);
	void			_e_error(std::string query);
	void			_e_raw(std::string query);
	void			_e_raw_welcome(std::string query);
	void			_e_raw_away_back();
	void			_e_raw_away_now();
	void			_e_raw_who(std::string query);
	void			_e_raw_topic(std::string query);
	void			_e_raw_names(std::string query);
	void			_e_raw_alreadyused(std::string query);
	void			_e_raw_modes(std::string query);
	void			_e_raw_limit(std::string query);
	void			_e_raw_invite(std::string query);
	void			_e_raw_banned(std::string query);
	void			_e_raw_key(std::string query);
	static void		_rejoin(void *p1, void *p2, void *p3);
	void			_logQuery(std::string query);
	void			_loadLangage(MyINI& config);

	/* attributes */

	MyUser			_client;
	MyTimer			_timer;
	std::string		_file;
	MyINI			*_config;
	MyINI			*_lang;
	std::map<std::string, void (CliIRC::*)(std::string)>		_events;
	std::map<std::string, void (*)(CliIRC& client, void *ptr, UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string msg)>	_uevents;
	std::list<ChannelIRC *>	_channels;
	std::list<UserIRC *>	_users;
	bool			_log;
	std::string		_logFile;
	std::ofstream	_logStream;
	bool			_output;
	bool			_connected;
	bool			_ircop;
	bool			_away;
	std::string		_nickname;
	void			(*_call)(CliIRC&, void *);
	void			*_ptr;
	void			*_uevent_ptr;

};

#endif // __CLIIRC_HPP__
