
#include <time.h>
#include "Game.hpp"
#include "UserIRC.hpp"
#include "ChannelIRC.hpp"
#include "MyString.hpp"

MyTimer	Game::_timer;

void	empty(void *p1, void *p2, void *p3)
{
	p1 = p1;
	p2 = p2;
	p3 = p3;
}

Game::Game()
{
	_state = 1;
	_time = (unsigned int)time(0);
	_timer.set(1, 1000000000, &empty, NULL, NULL, NULL);
	_scrabble_last = NULL;
	_cmds["rule"] = &Game::_game_rules;
	_cmds["rules"] = &Game::_game_rules;
	_cmds["regle"] = &Game::_game_rules;
	_cmds["regles"] = &Game::_game_rules;
	_cmds["suscribe"] = &Game::_game_suscribe;
	_cmds["inscription"] = &Game::_game_suscribe;
	_cmds["go"] = &Game::_game_suscribe;
	_cmds["list"] = &Game::_game_list;
	_cmds["liste"] = &Game::_game_list;
	_cmds["cmd"] = &Game::_game_list;
	_cmds["cmds"] = &Game::_game_list;
	_cmds["command"] = &Game::_game_list;
	_cmds["commands"] = &Game::_game_list;
	_cmds["commande"] = &Game::_game_list;
	_cmds["commandes"] = &Game::_game_list;
	_cmds["help"] = &Game::_game_list;
	_cmds["aide"] = &Game::_game_list;
	_cmds["time"] = &Game::_game_time;
	_cmds["temp"] = &Game::_game_time;
	_cmds["temps"] = &Game::_game_time;
	_cmds["player"] = &Game::_game_players;
	_cmds["players"] = &Game::_game_players;
	_cmds["joueur"] = &Game::_game_players;
	_cmds["joueurs"] = &Game::_game_players;
	_cmds["order"] = &Game::_game_players;
	_cmds["ordre"] = &Game::_game_players;
	_cmds["reg"] = &Game::_game_players;
	_cmds["registered"] = &Game::_game_players;
	_cmds["inscrit"] = &Game::_game_players;
	_cmds["inscrits"] = &Game::_game_players;
	_cmds["unsuscribe"] = &Game::_game_unsuscribe;
	_cmds["cancel"] = &Game::_game_unsuscribe;
	_cmds["annule"] = &Game::_game_unsuscribe;
	_cmds["abandon"] = &Game::_game_unsuscribe;
	_cmds["start"] = &Game::_game_start;
	_cmds["commence"] = &Game::_game_start;
	_cmds["lance"] = &Game::_game_start;
	_cmds["end"] = &Game::_game_start;
	_cmds["fin"] = &Game::_game_start;
	_cmds["trash"] = &Game::_game_trash;
	_cmds["vire"] = &Game::_game_trash;
	_cmds["del"] = &Game::_game_trash;
	_cmds["reset"] = &Game::_game_reset;
	_cmds["reinitialise"] = &Game::_game_reset;
	_cmds["stop"] = &Game::_game_stop;
	_cmds["disable"] = &Game::_game_stop;
	_cmds["arrete"] = &Game::_game_stop;
	_cmds["desactive"] = &Game::_game_stop;
	_cmds["score"] = &Game::_game_score;
	_cmds["point"] = &Game::_game_score;
	_cmds["points"] = &Game::_game_score;
	_cmds["status"] = &Game::_game_status;
	_cmds["statut"] = &Game::_game_status;
	_cmds["top"] = &Game::_game_top;
	_play_init();
}

Game::Game(const Game& game)
{
	_cmds = game._cmds;
	_state = game._state;
	_players = game._players;
	_turn = game._turn;
	_time = game._time;
	_timers = game._timers;
	_scrabble_word = game._scrabble_word;
	_scrabble_last = game._scrabble_last;
}

Game::~Game()
{
	std::list<int>::iterator	it;
	for (it = this->_timers.begin(); (it != this->_timers.end()); it++)
		this->_timer.unset(*it);
}

Game&							Game::operator=(const Game& game)
{
	std::list<int>::iterator	it;

	if (this != &game)
	{
		for (it = this->_timers.begin(); (it != this->_timers.end()); it++)
			this->_timer.unset(*it);
		this->_cmds = game._cmds;
		this->_state = game._state;
		this->_players = game._players;
		this->_turn = game._turn;
		this->_time = game._time;
		this->_timers = game._timers;
		this->_scrabble_word = game._scrabble_word;
		this->_scrabble_last = game._scrabble_last;
	}
	return (*this);
}

void	Game::init(CliIRC& client)
{
	client.addEvent("botjoin", &Game::bot_join);
	client.addEvent("othjoin", &Game::oth_join);
	client.addEvent("othpart", &Game::oth_part);
	client.addEvent("botpart", &Game::bot_part);
	client.addEvent("genquitchan", &Game::oth_part);
	client.addEvent("botkicker", &Game::oth_part_rev);
	client.addEvent("othkick", &Game::oth_part_rev);
	client.addEvent("botkickhim", &Game::bot_part);
	client.addEvent("botkicked", &Game::bot_part);
	client.addEvent("othmsg", &Game::oth_msg);
	client.addEvent("othnotice", &Game::oth_msg);
	client.addEvent("othaction", &Game::oth_msg);
	client.addEvent("othnick", &Game::oth_nick);
	return ;
}

void	Game::bot_join(CliIRC& cli, void *ptr, UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string msg)
{
	if (cli.configIsOnList("AUTOJOIN", chan->getName()) != "-1")
	{
		cli.say(chan->getName(), cli.lang("GAME", "join_hello_1"));
		cli.say(chan->getName(), cli.lang("GAME", "join_hello_2", GAME_NAME));
		chan->_game._timers.push_back(Game::_timer.set(1, 3, &Game::_countdown_devoice, (void *)&cli, (void *)chan, NULL));
	}
	else
		cli.part(chan->getName());
	ptr = ptr; src = src; dst = dst; msg = msg;
}

void	Game::oth_join(CliIRC& cli, void *ptr, UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string msg)
{
	if (chan->_game._state == 1)
	{
		cli.notice(src->getNick(), cli.lang("GAME", "join_go_1", chan->getName().c_str(), src->getNick().c_str()));
		cli.notice(src->getNick(), cli.lang("GAME", "join_go_2", GAME_NAME));
	}
	if (chan->_game._state == 2)
	{
		cli.notice(src->getNick(), cli.lang("GAME", "join_started_1", chan->getName().c_str(), src->getNick().c_str()));	
		cli.notice(src->getNick(), cli.lang("GAME", "join_started_2", GAME_NAME, (*(chan->_game._turn)).nick.c_str()));				
	}
	ptr = ptr; dst = dst; msg = msg;
	return ;
}

void							Game::bot_part(CliIRC& cli, void *ptr, UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string msg)
{
	if (cli.configIsOnList("AUTOJOIN", chan->getName()) != "-1")
		cli.join(chan->getName());
	chan->_game.timersOff();
	ptr = ptr; src = src; dst = dst; msg = msg;
	return ;
}

void	Game::oth_part(CliIRC& cli, void *ptr, UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string msg)
{
	if (chan->_game.isPlayer(src))
		if ((chan->_game._state != 0) && (chan->_game.isPlayer(src) == true))
			chan->_game._game_unsuscribe(cli, chan, src, msg);
	ptr = ptr; dst = dst;
	return ;
}

void	Game::oth_part_rev(CliIRC& cli, void *ptr, UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string msg)
{
	Game::oth_part(cli, ptr, dst, chan, src, msg);
	return ;
}

void				Game::oth_msg(CliIRC& cli, void *ptr, UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string msg)
{
	std::map<std::string, void (Game::*)(CliIRC&, ChannelIRC *, UserIRC *, std::string& msg)>::iterator it;
	std::string		command;
	std::string		parameters;

	msg = lowCase(cli.strip(msg));
	command = gettok(msg, 1, ' ');
	while ((command.size() > 0) && (command.at(0) == '!'))
		command = command.substr(1, (command.length() - 1));
	for (it = chan->_game._cmds.begin(); (it != chan->_game._cmds.end()); it++)
		if (lowCase((*it).first) == command)
			(chan->_game.*(*it).second)(cli, chan, src, msg);
	parameters = fulltok(msg, 2, ' ');
	chan->_game._play_message(cli, src, chan, command, parameters);
	ptr = ptr; dst = dst;
	return ;
}

void		Game::oth_nick(CliIRC& cli, void *ptr, UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string msg)
{
	std::list<ChannelIRC *>::iterator	it_c;
	std::list<_player>::iterator		it_p;
	
	CliIRC	*client;
	client = &cli;
	chan = chan;
	for (it_c = cli._channels.begin(); (it_c != cli._channels.end()); it_c++)
		for (it_p = (*it_c)->_game._players.begin(); (it_p != (*it_c)->_game._players.end()); it_p++)
			if ((*it_p).user == src)
				(*it_p).nick = msg;
			ptr = ptr; dst = dst;
}

bool								Game::isPlayer(const UserIRC *user)
{
	std::list<_player>::iterator	it;

	for (it = this->_players.begin(); (it != this->_players.end()); it++)
		if (((*it).user == user) && ((*it).finish == 0))
			return (true);
	return (false);
}

std::string							Game::stringPlayers()
{
	std::list<_player>::iterator	it;
	std::string						ret;

	ret = "$b";
	for (it = this->_players.begin(); (it != this->_players.end()); it++)
	{
		if (it != this->_players.begin())
			ret += "$b, $b";
		ret += (*it).nick;
	}
	ret += "$b";
	return (ret);
}

void				Game::timersOff()
{
	std::list<int>::iterator	it;

	if (this->_timer._thread.isThread() == true)
		return ;
	for (it = this->_timers.begin(); (it != this->_timers.end()); it++)
		Game::_timer.unset(*it);
	this->_timers.clear();
	return ;
}

std::string							Game::randPlayer()
{
	std::list<_player>::iterator	it;
	int								rnd;
	int								nbr;

	rnd = (random() % this->_players.size());
	nbr = 0;
	for (it = this->_players.begin(); (it != this->_players.end()); it++)
	{
		if (nbr == rnd)
			return ((*it).nick);
		nbr++;
	}
	return (std::string(""));
}

void		Game::_game_rules(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg)
{
	if (this->_state == 0)
		return ;
	client.notice(user->getNick(), client.lang("GAME", "rules", GAME_NAME, GAME_LINK));
	chan = chan;
	msg = msg;
	return ;
}

void		Game::_game_suscribe(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg)
{
	if (client.configIsOnList("AUTOJOIN", chan->getName()) == "-1")
	{
		client.notice(user->getNick(), client.lang("GAME", "no_game", GAME_NAME, chan->getName().c_str()));
		return ;
	}
	if (this->_state == 0)
	{
		client.notice(user->getNick(), client.lang("GAME", "disabled", GAME_NAME, chan->getName().c_str()));
		return ;
	}
	if (this->_state == 2)
	{
		client.notice(user->getNick(), client.lang("GAME", "game_started", GAME_NAME, chan->getName().c_str()));
		return ;
	}
	if (this->isPlayer(user) == true)
	{
		client.notice(user->getNick(), client.lang("GAME", "suscribe_already", GAME_NAME, chan->getName().c_str()));
		return ;
	}
	if (this->_players.size() == GAME_MAX_PLAY)
	{
		client.notice(user->getNick(), client.lang("GAME", "suscribe_many", GAME_NAME, chan->getName().c_str()));
		return ;
	}
	client.say(chan->getName(), client.lang("GAME", "suscribe_ok", user->getNick().c_str(), GAME_NAME));
	Game::_player	play(user);
	play.nick = user->getNick();
	this->_players.push_back(play);
	client.voice(chan->getName(), user->getNick());
	if (this->_players.size() == 1)
		client.say(chan->getName(), client.lang("GAME", "playlist_one", user->getNick().c_str()));
	else
		client.say(chan->getName(), client.lang("GAME", "playlist_several", intToString((int)this->_players.size()).c_str(), this->stringPlayers().c_str()));
	if (this->_players.size() == GAME_MAX_PLAY)
	{
		this->timersOff();
		this->_play_new_play(client, chan);
	}
	else if (this->_players.size() == GAME_MIN_PLAY)
		this->_startCountdown(client, chan);
	msg = msg;
	return ;
}

void	Game::_game_list(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg)
{
	if (this->_state == 1)
	{
		client.notice(user->getNick(), client.lang("GAME", "cmdlist_sub_1", GAME_NAME));
		client.notice(user->getNick(), client.lang("GAME", "cmdlist_sub_2"));
		client.notice(user->getNick(), client.lang("GAME", "cmdlist_sub_3"));
	}
	if (this->_state == 2)
	{
		client.notice(user->getNick(), client.lang("MODULE", "cmdlist_cur_1", GAME_NAME));
		client.notice(user->getNick(), client.lang("MODULE", "cmdlist_cur_2"));
		client.notice(user->getNick(), client.lang("MODULE", "cmdlist_cur_3"));
	}
	chan = chan; msg = msg;
	return ;
}

void		Game::_game_time(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg)
{
	int		min;
	int		sec;

	if (this->_state == 1)
	{
		if (this->_players.size() >= GAME_MIN_PLAY)
		{
			min = ((120 - chan->_game._time) / 60);
			sec = ((120 - chan->_game._time) % 60);
			client.say(chan->getName(), client.lang("GAME", "time_start", intToString(min).c_str(), intToString(sec).c_str()));
		}
		else
			client.say(chan->getName(), client.lang("GAME", "time_notenough"));
	}
	if (this->_state == 2)
	{
		min = (this->_time / 60);
		sec = (this->_time % 60);
		client.say(chan->getName(), client.lang("GAME", "play_time", GAME_NAME, intToString(min).c_str(), intToString(sec).c_str()));
	}
	user = user; msg = msg;
	return ;
}

void								Game::_game_players(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg)
{
	std::list<_player>::iterator	it;
	std::string						ret;

	if (this->_state == 1)
	{
		if (this->_players.size() == 0)
			client.say(chan->getName(), client.lang("GAME", "no_subscription"));
		else
			client.say(chan->getName(), client.lang("GAME", "subscription_players", GAME_NAME, this->stringPlayers().c_str()));
	}
	if (this->_state == 2)
	{
		ret = "$b";
		for (it = this->_players.begin(); (it != this->_players.end()); it++)
		{
			if ((*it).finish > 0)
				continue ;
			if (ret.size() > 2)
				ret += "$b, $b";
			if (it == this->_turn)
				ret = ret + "$u" + (*it).nick + "$u";
			else
				ret += (*it).nick;
		}
		ret += "$b";
		client.say(chan->getName(), client.lang("GAME", "play_order", ret.c_str()));
	}
	user = user; msg = msg;
	return ;
}

void								Game::_game_unsuscribe(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg)
{
	std::list<_player>::iterator	it;

	if (this->isPlayer(user) == false)
	{
		client.notice(user->getNick(), client.lang("GAME", "not_a_player", GAME_NAME));
		return ;
	}
	if (this->_state == 1)
	{
		for (it = this->_players.begin(); (it != this->_players.end()); it++)
			if ((*it).user == user)
			{
				this->_players.erase(it);
				break ;
			}
		client.devoice(chan->getName(), user->getNick());
		client.say(chan->getName(), client.lang("GAME", "unsuscribe_ok", user->getNick().c_str()));
		if (this->_players.size() == 0)
			client.say(chan->getName(), client.lang("GAME", "playlist_zero"));
		else if (this->_players.size() == 1)
			client.say(chan->getName(), client.lang("GAME", "playlist_one", this->stringPlayers().c_str()));
		else
			client.say(chan->getName(), client.lang("GAME", "playlist_several", intToString((int)this->_players.size()).c_str(), this->stringPlayers().c_str()));
		if (this->_players.size() == (GAME_MIN_PLAY - 1))
		{
			this->timersOff();
			client.say(chan->getName(), client.lang("GAME", "countdown_off", user->getNick().c_str()));
		}
	}
	if (this->_state == 2)
	{
		for (it = this->_players.begin(); (it != this->_players.end()); it++)
			if ((*it).user == user)
				break ;
		(*it).finish = this->_finish;
		(*it).score = -1;
		(*it).nb_secs = this->_time;
		this->_finish++;
		client.devoice(chan->getName(), user->getNick());
		client.say(chan->getName(), client.lang("GAME", "line"));
		client.say(chan->getName(), client.lang("GAME", "abandon_1", user->getNick().c_str()));
		if (this->_play_cannot_continue(client, chan) == true)
			return ;
		client.say(chan->getName(), client.lang("GAME", "abandon_2", intToString(this->_play_nb_players()).c_str(), this->_play_list().c_str()));
		client.say(chan->getName(), client.lang("GAME", "line"));
		this->_play_cancel(client, chan);
		if (it == this->_turn)
		{
			this->_play_next_player();
			this->_play_new_turn(client, chan);
		}
	}
	msg = msg;
	return ;
}

void	Game::_game_start(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg)
{
	if (chan->level(user->getNick()) < 3)
	{
		client.notice(user->getNick(), client.lang("GENERAL", "access", upCase(gettok(msg, 1, ' ')).c_str()));
		return ;
	}
	if (this->_state == 0)
	{
		this->_state = 1;
		client.say(chan->getName(), client.lang("GAME", "game_enabled", GAME_NAME));
		return ;
	}
	if (this->_state == 1)
	{
		if (this->_players.size() < GAME_MIN_PLAY)
		{
			client.say(chan->getName(), client.lang("GAME", "game_notenough"));
			return ;
		}
		this->_play_new_play(client, chan);
		return ;
	}
	if (this->_state == 2)
		client.notice(user->getNick(), client.lang("GAME", "play_already", chan->getName().c_str()));
	return ;
}

void		Game::_game_trash(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg)
{
	UserIRC	*dst;
	int		i;

	if (this->_state == 0)
		return ;
	if (chan->level(user->getNick()) < 3)
	{
		client.notice(user->getNick(), client.lang("GENERAL", "access", upCase(gettok(msg, 1, ' ')).c_str()));
		return ;
	}
	if (numtok(msg, ' ') < 2)
	{
		client.notice(user->getNick(), client.lang("GENERAL", "few", upCase(gettok(msg, 1, ' ')).c_str()));
		return ;		
	}
	for (i = 2; (i <= numtok(msg, ' ')); i++)
	{
		dst = client._getUserByName(gettok(msg, i, ' '));
		if (dst == NULL)
			client.notice(user->getNick(), client.lang("GAME", "trash_not_suscriber", gettok(msg, i, ' ').c_str()));
		else if (this->isPlayer(dst) == false)
			client.notice(user->getNick(), client.lang("GAME", "trash_not_suscriber", gettok(msg, i, ' ').c_str()));
		else
			this->_game_unsuscribe(client, chan, dst, msg);
	}
	return ;
}

void	Game::_game_reset(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg)
{
	if (chan->level(user->getNick()) < 3)
	{
		client.notice(user->getNick(), client.lang("GENERAL", "access", upCase(gettok(msg, 1, ' ')).c_str()));
		return ;
	}
	if (this->_state == 1)
	{
		this->timersOff();
		this->_players.clear();
		client.say(chan->getName(), client.lang("GAME", "game_reset"));
	}
	if (this->_state == 2)
		this->_game_stop(client, chan, user, msg);
	return ;
}

void	Game::_game_stop(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg)
{
	if (chan->level(user->getNick()) < 3)
	{
		client.notice(user->getNick(), client.lang("GENERAL", "access", upCase(gettok(msg, 1, ' ')).c_str()));
		return ;
	}
	if (this->_state == 1)
	{
		this->timersOff();
		this->_players.clear();
		this->_state = 0;
		client.say(chan->getName(), client.lang("GAME", "game_disabled", GAME_NAME));
	}
	if (this->_state == 2)
		this->_play_finish(client, chan);
	return ;
}

void								Game::_game_score(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg)
{
	std::list<_player>::iterator	it;
	std::string						nick;
	std::string						nbr;

	if (numtok(msg, ' ') == 1)
		nick = user->getNick();
	else
		nick = gettok(msg, 2, ' ');
	if (this->_state == 1)
	{
		if (client.configIsKey("SCORE", lowCase(nick)) == false)
			client.say(chan->getName(), client.lang("GAME", "gen_score_zero", nick.c_str()));
		else
		{
			nbr = ((*client._config)["SCORE"][lowCase(nick)]);
			if (gettok(nbr, 1, ':') == "1")
				client.say(chan->getName(), client.lang("GAME", "gen_score_one", nick.c_str(), gettok(nbr, 2, ':').c_str()));
			else
				client.say(chan->getName(), client.lang("GAME", "gen_score_several", nick.c_str(), gettok(nbr, 1, ':').c_str(), gettok(nbr, 2, ':').c_str()));
		}
	}
	if (this->_state == 2)
	{
		for (it = this->_players.begin(); (it != this->_players.end()); it++)
			if (lowCase((*it).nick) == lowCase(nick))
			{
				if ((*it).score == 0)
					client.say(chan->getName(), client.lang("GAME", "score_zero", nick.c_str()));
				else if ((*it).score == 1)
					client.say(chan->getName(), client.lang("GAME", "score_one", nick.c_str()));
				else
					client.say(chan->getName(), client.lang("GAME", "score_several", nick.c_str(), intToString((*it).score).c_str()));		
				break ;
			}
		if (it == this->_players.end())
		{
			client.say(chan->getName(), client.lang("GAME", "trash_not_suscriber", nick.c_str()));
			return ;
		}
	}
	return ;
}

void	Game::_game_status(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg)
{
	if (this->_state == 0)
		client.say(chan->getName(), client.lang("GAME", "state_disabled", GAME_NAME));
	else if (this->_state == 1)
		client.say(chan->getName(), client.lang("GAME", "state_subscriptions"));
	else if (this->_state == 2)
		client.say(chan->getName(), client.lang("GAME", "state_game", GAME_NAME, this->_play_list().c_str()));
	user = user; msg = msg;
	return ;
}

void														Game::_game_top(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg)
{
	std::map<int, std::string>								play;
	std::map<std::string, std::string>::iterator			it;
	std::map<int, std::string>::reverse_iterator			its;
	std::string												rep;
	int														num;
	int														lgn;

	if (this->_state == 1)
	{
		num = 0;
		for (it = (*client._config)["SCORE"].begin(); (it != (*client._config)["SCORE"].end()); it++)
		{
			std::string& s = play[(atoi(gettok((*it).second, 1, ':').c_str()))];
			if (s.size() > 0)
				s += "$b, $b";
			s += (*it).first;
			num += 1;
		}
		if (num < 15)
			client.say(chan->getName(), client.lang("GAME", "general_top", intToString((int)num).c_str()));
		else
			client.say(chan->getName(), client.lang("GAME", "general_top", "15"));
		num = 1;
		lgn = 1;
		for (its = play.rbegin(); (its != play.rend()); its++)
		{
			if (num > 15)
				break ;
			if (rep.size() > 0)
				rep += " - ";
			rep = rep + "$u" + intToString(num) + "$u ";
			rep = rep + "$b" + (*its).second + "$b ";
			rep = rep + "(" + intToString((*its).first) + ")";
			if (((lgn % 3) == 0) && (lgn != 1))
			{
				client.say(chan->getName(), rep);
				rep.clear();
			}
			num += numtok((*its).second, ',');
			lgn += 1;
		}
		if (rep.size() > 0)
			client.say(chan->getName(), rep);
		client.say(chan->getName(), "IRC Scrabble, written in C++ by Alain Tiemblo.");
	}
	user = user; msg = msg;
	return ;
}

Game::_player&						Game::_game_get_player_by_user(UserIRC *user)
{
	std::list<_player>::iterator	it;
	
	for (it = this->_players.begin(); (it != this->_players.end()); it++)
		if ((*it).user == user)
			return (*it);
	return ((*(this->_players.begin())));
}

void			Game::_startCountdown(CliIRC& client, ChannelIRC *chan)
{
	client.say(chan->getName(), client.lang("GAME", "countdown_start"));
	this->_time = 0;
	this->_timers.push_back(this->_timer.set(120, 1, &Game::_countdown_inc, (void *)&client, (void *)chan, NULL));
	this->_timers.push_back(this->_timer.set(1, 15, &Game::_countdown_01, (void *)&client, (void *)chan, NULL));
	this->_timers.push_back(this->_timer.set(1, 30, &Game::_countdown_02, (void *)&client, (void *)chan, NULL));
	this->_timers.push_back(this->_timer.set(1, 45, &Game::_countdown_03, (void *)&client, (void *)chan, NULL));
	this->_timers.push_back(this->_timer.set(1, 60, &Game::_countdown_04, (void *)&client, (void *)chan, NULL));
	this->_timers.push_back(this->_timer.set(1, 75, &Game::_countdown_05, (void *)&client, (void *)chan, NULL));
	this->_timers.push_back(this->_timer.set(1, 90, &Game::_countdown_06, (void *)&client, (void *)chan, NULL));
	this->_timers.push_back(this->_timer.set(1, 105, &Game::_countdown_07, (void *)&client, (void *)chan, NULL));
	this->_timers.push_back(this->_timer.set(1, 115, &Game::_countdown_08, (void *)&client, (void *)chan, NULL));
	this->_timers.push_back(this->_timer.set(1, 116, &Game::_countdown_09, (void *)&client, (void *)chan, NULL));
	this->_timers.push_back(this->_timer.set(1, 117, &Game::_countdown_10, (void *)&client, (void *)chan, NULL));
	this->_timers.push_back(this->_timer.set(1, 118, &Game::_countdown_11, (void *)&client, (void *)chan, NULL));
	this->_timers.push_back(this->_timer.set(1, 119, &Game::_countdown_12, (void *)&client, (void *)chan, NULL));
	this->_timers.push_back(this->_timer.set(1, 120, &Game::_countdown_13, (void *)&client, (void *)chan, NULL));
	return ;
}

void			Game::_countdown_inc(void *cli, void *chn, void *nul)
{
	ChannelIRC	*chan;
	
	chan = (ChannelIRC *)chn;
	chan->_game._time += 1;
	cli = cli;
	nul = nul;
	return ;
}

void			Game::_countdown_01(void *cli, void *chn, void *nul)
{
	CliIRC		*client;
	ChannelIRC	*chan;
	
	client = (CliIRC *)cli;
	chan = (ChannelIRC *)chn;
	client->say(chan->getName(), client->lang("GAME", "countdown_15_i", GAME_NAME));
	client->say(chan->getName(), client->lang("GAME", "countdown_15_t"));
	nul = nul;
	return ;
}

void			Game::_countdown_02(void *cli, void *chn, void *nul)
{
	CliIRC		*client;
	ChannelIRC	*chan;
	
	client = (CliIRC *)cli;
	chan = (ChannelIRC *)chn;
	client->say(chan->getName(), client->lang("GAME", "countdown_30_i", chan->_game.stringPlayers().c_str()));
	client->say(chan->getName(), client->lang("GAME", "countdown_30_t"));
	nul = nul;
	return ;
}

void			Game::_countdown_03(void *cli, void *chn, void *nul)
{
	CliIRC		*client;
	ChannelIRC	*chan;
	
	client = (CliIRC *)cli;
	chan = (ChannelIRC *)chn;
	client->say(chan->getName(), client->lang("GAME", "countdown_45_i", GAME_NAME));
	client->say(chan->getName(), client->lang("GAME", "countdown_45_t"));
	nul = nul;
	return ;
}

void			Game::_countdown_04(void *cli, void *chn, void *nul)
{
	CliIRC		*client;
	ChannelIRC	*chan;
	
	client = (CliIRC *)cli;
	chan = (ChannelIRC *)chn;
	client->say(chan->getName(), client->lang("GAME", "countdown_60_i", chan->_game.randPlayer().c_str(), GAME_NAME));
	client->say(chan->getName(), client->lang("GAME", "countdown_60_t"));
	nul = nul;
	return ;
}

void			Game::_countdown_05(void *cli, void *chn, void *nul)
{
	CliIRC		*client;
	ChannelIRC	*chan;
	
	client = (CliIRC *)cli;
	chan = (ChannelIRC *)chn;
	client->say(chan->getName(), client->lang("GAME", "countdown_75_i", GAME_NAME, chan->_game.stringPlayers().c_str()));
	client->say(chan->getName(), client->lang("GAME", "countdown_75_t"));
	nul = nul;
	return ;
}

void			Game::_countdown_06(void *cli, void *chn, void *nul)
{
	CliIRC		*client;
	ChannelIRC	*chan;
	
	client = (CliIRC *)cli;
	chan = (ChannelIRC *)chn;
	client->say(chan->getName(), client->lang("GAME", "countdown_90_i"));
	client->say(chan->getName(), client->lang("GAME", "countdown_90_t"));
	nul = nul;
	return ;
}

void			Game::_countdown_07(void *cli, void *chn, void *nul)
{
	CliIRC		*client;
	ChannelIRC	*chan;
	
	client = (CliIRC *)cli;
	chan = (ChannelIRC *)chn;
	client->say(chan->getName(), client->lang("GAME", "countdown_105_i"));
	client->say(chan->getName(), client->lang("GAME", "countdown_105_t"));
	nul = nul;
	return ;
}

void			Game::_countdown_08(void *cli, void *chn, void *nul)
{
	CliIRC		*client;
	ChannelIRC	*chan;
	
	client = (CliIRC *)cli;
	chan = (ChannelIRC *)chn;
	client->say(chan->getName(), client->lang("GAME", "countdown_115"));
	nul = nul;
	return ;
}

void			Game::_countdown_09(void *cli, void *chn, void *nul)
{
	CliIRC		*client;
	ChannelIRC	*chan;
	
	client = (CliIRC *)cli;
	chan = (ChannelIRC *)chn;
	client->say(chan->getName(), client->lang("GAME", "countdown_116"));
	nul = nul;
	return ;
}

void			Game::_countdown_10(void *cli, void *chn, void *nul)
{
	CliIRC		*client;
	ChannelIRC	*chan;
	
	client = (CliIRC *)cli;
	chan = (ChannelIRC *)chn;
	client->say(chan->getName(), client->lang("GAME", "countdown_117"));
	nul = nul;
	return ;
}

void			Game::_countdown_11(void *cli, void *chn, void *nul)
{
	CliIRC		*client;
	ChannelIRC	*chan;
	
	client = (CliIRC *)cli;
	chan = (ChannelIRC *)chn;
	client->say(chan->getName(), client->lang("GAME", "countdown_118"));
	nul = nul;
	return ;
}

void			Game::_countdown_12(void *cli, void *chn, void *nul)
{
	CliIRC		*client;
	ChannelIRC	*chan;
	
	client = (CliIRC *)cli;
	chan = (ChannelIRC *)chn;
	client->say(chan->getName(), client->lang("GAME", "countdown_119"));
	nul = nul;
	return ;
}

void			Game::_countdown_13(void *cli, void *chn, void *nul)
{
	CliIRC		*client;
	ChannelIRC	*chan;
	
	client = (CliIRC *)cli;
	chan = (ChannelIRC *)chn;
	chan->_game._play_new_play(*client, chan);
	nul = nul;
	return ;
}

void								Game::_countdown_devoice(void *cli, void *chn, void *nul)
{
	CliIRC							*client;
	ChannelIRC						*chan;
	std::list<UserIRC *>::iterator	it;

	client = (CliIRC *)cli;
	chan = (ChannelIRC *)chn;
	for (it = chan->_voice.begin(); (it != chan->_voice.end()); it++)
		if (chan->_game.isPlayer(*it) == false)
			client->devoice(chan->getName(), (*it)->getNick());
	nul = nul;
	return ;
}

void							Game::_play_new_play(CliIRC& client, ChannelIRC *chan)
{
	std::list<Game::_player>	list;

	srandom(time(0));
	this->_state = 2;
	this->_time = 0;
	while (this->_players.size())
		list.push_back(removeRandElem<Game::_player>(this->_players));
	this->_players = list;
	this->_finish = 1;
	this->_turn = this->_players.begin();
	client.say(chan->getName(), client.lang("GAME", "play_start_1", GAME_NAME));
	client.say(chan->getName(), client.lang("GAME", "play_start_2"));
	client.say(chan->getName(), client.lang("GAME", "play_start_3", this->stringPlayers().c_str()));
	this->_play_start(client, chan);
	this->_play_new_turn(client, chan);
	return ;
}

void		Game::_play_new_turn(CliIRC& client, ChannelIRC *chan)
{
	this->timersOff();
	client.say(chan->getName(), client.lang("GAME", "line"));
	this->_play_turn(client, chan);
	client.say(chan->getName(), client.lang("MODULE", "new_turn", (*(this->_turn)).nick.c_str()));
	client.say(chan->getName(), client.lang("GAME", "line"));
	this->_timers.push_back(this->_timer.set(1000000000, 1, &Game::_countdown_inc, (void *)&client, (void *)chan, NULL));
	this->_timers.push_back(this->_timer.set(1, 60, &Game::_play_timeout_c1, (void *)&client, (void *)chan, NULL));
	this->_timers.push_back(this->_timer.set(1, 110, &Game::_play_timeout_c2, (void *)&client, (void *)chan, NULL));
	this->_timers.push_back(this->_timer.set(1, 120, &Game::_play_timeout_c3, (void *)&client, (void *)chan, NULL));
	return ;
}

void			Game::_play_timeout_c1(void *cli, void *chan, void *nul)
{
	CliIRC		*client;
	ChannelIRC	*channel;
	
	client = (CliIRC *)cli;
	channel = (ChannelIRC *)chan;
	client->say(channel->getName(), client->lang("GAME", "timeout_c1", (*(channel->_game._turn)).nick.c_str()));
	nul = nul;
	return ;
}

void		Game::_play_timeout_c2(void *cli, void *chan, void *nul)
{
	CliIRC		*client;
	ChannelIRC	*channel;
	
	client = (CliIRC *)cli;
	channel = (ChannelIRC *)chan;
	client->say(channel->getName(), client->lang("GAME", "timeout_c2", (*(channel->_game._turn)).nick.c_str()));
	nul = nul;
	return ;
}

void		Game::_play_timeout_c3(void *cli, void *chan, void *nul)
{
	CliIRC		*client;
	ChannelIRC	*channel;
	std::string	str;
	
	client = (CliIRC *)cli;
	channel = (ChannelIRC *)chan;
	client->say(channel->getName(), client->lang("GAME", "timeout_c3", (*(channel->_game._turn)).nick.c_str()));
	channel->_game._game_unsuscribe(*client, channel, (*(channel->_game._turn)).user, str);
	nul = nul;
	return ;
}

bool			Game::_play_stay_players()
{
	std::list<_player>::iterator	it;
	int								alive;

	alive = 0;
	for (it = this->_players.begin(); (it != this->_players.end()); it++)
		if ((*it).finish == 0)
			alive++;
	if (alive >= GAME_MIN_PLAY)
		return (true);
	return (false);
}

void			Game::_play_next_player()
{
	if (this->_play_stay_players() == false)
		return ;
	while (24400)
	{
		this->_turn++;
		if (this->_turn == this->_players.end())
			this->_turn = this->_players.begin();
		if ((*(this->_turn)).finish == 0)
			break ;
	}
	return ;
}

void			Game::_play_prev_player()
{
	if (this->_play_stay_players() == false)
		return ;
	while (24400)
	{
		this->_turn--;
		if ((*(this->_turn)).finish == 0)
			break ;
		if (this->_turn == this->_players.begin())
			this->_turn = (this->_players.end())--;
	}
	return ;
}

bool	Game::_play_cannot_continue(CliIRC& client, ChannelIRC *chan)
{
	if (this->_play_nb_players() < GAME_MIN_PLAY)
	{
		this->_play_finish(client, chan);
		return (true);
	}
	return (false);
}

int									Game::_play_nb_players()
{
	std::list<_player>::iterator	it;
	int								nbr;

	nbr = 0;
	for (it = this->_players.begin(); (it != this->_players.end()); it++)
		if ((*it).finish == 0)
			nbr++;
	return (nbr);
}

bool			Game::_play_state_game(CliIRC& client, UserIRC *user)
{
	if (this->_state == 2)
		return (true);
	client.notice(user->getNick(), client.lang("GAME", "game_not_started", GAME_NAME));
	return (false);
}

bool			Game::_play_state_suscrib(CliIRC& client, UserIRC *user)
{
	if (this->_state == 1)
		return (true);
	client.notice(user->getNick(), client.lang("GAME", "game_started", GAME_NAME));
	return (false);
}

bool			Game::_play_is_player(CliIRC& client, UserIRC *user)
{
	if (this->isPlayer(user) == true)
		return (true);
	client.notice(user->getNick(), client.lang("GAME", "not_a_player", GAME_NAME));
	return (false);
}

bool								Game::_play_is_turn_player(CliIRC& client, UserIRC *user)
{
	std::list<_player>::iterator	it;
	
	for (it = this->_players.begin(); (it != this->_players.end()); it++)
		if ((*it).user == user)
			break ;
	if (it == this->_players.end())
		client.notice(user->getNick(), client.lang("GAME", "not_a_player_act", GAME_NAME));
	else if (user != (*(this->_turn)).user)
		client.notice(user->getNick(), client.lang("GAME", "not_your_turn", (*(this->_turn)).nick.c_str()));
	else
		return (true);
	return (false);
}

std::string							Game::_play_list()
{
	std::list<_player>::iterator	it;
	std::string						ret;
	
	for (it = this->_players.begin(); (it != this->_players.end()); it++)
		if ((*it).finish == 0)
		{
			ret += "$b";
			if (ret.size() > 2)
				ret += "$b, $b";
			if (it == this->_turn)
				ret = ret + "$u" + (*it).nick + "$u";
			else
				ret += (*it).nick;
			ret += "$b";
		}
	return (ret);
}

void								Game::_play_winner(CliIRC& client, ChannelIRC *chan)
{
	std::list<_player>::iterator	it;

	it = this->_turn;
	(*it).nb_turn++;
	(*it).finish = this->_finish;
	(*it).nb_secs = this->_time;
	this->_finish++;
	if (this->_play_cannot_continue(client, chan) == true)
		return ;
	client.say(chan->getName(), client.lang("GAME", "line"));
	if ((*it).finish == 1)
		client.say(chan->getName(), client.lang("GAME", "finish_1", (*it).nick.c_str()));
	else
		client.say(chan->getName(), client.lang("GAME", "finish_n", (*it).nick.c_str(), intToString((*it).finish).c_str()));
	client.say(chan->getName(), client.lang("GAME", "finish_time", intToString((*it).nb_secs / 60).c_str(), intToString((*it).nb_secs % 60).c_str()));
	client.say(chan->getName(), client.lang("GAME", "abandon_2", intToString(this->_play_nb_players()).c_str(), this->_play_list().c_str()));
	this->_play_wins(client, chan);
	if (it == this->_turn)
	{
		this->_play_next_player();
		this->_play_new_turn(client, chan);
	}
	return ;
}

void								Game::_play_pass(CliIRC& client, ChannelIRC *chan)
{
	this->_play_next_player();
	this->_play_new_turn(client, chan);
	return ;
}

void								Game::_play_play(CliIRC& client, ChannelIRC *chan)
{
	(*(this->_turn)).nb_turn++;
	this->_play_next_player();
	this->_play_new_turn(client, chan);
	return ;
}

void														Game::_play_finish(CliIRC& client, ChannelIRC *chan)
{
	std::list<_player>::iterator							itp;
	std::string												str;
	int														num;
	int														nbr;

	this->timersOff();
	this->_play_end(client, chan);
	this->_state = 1;
	client.say(chan->getName(), client.lang("GAME", "line"));
	client.say(chan->getName(), client.lang("GAME", "play_finish", intToString(this->_time / 60).c_str(), intToString(this->_time % 60).c_str()));
	client.say(chan->getName(), client.lang("GAME", "line"));
	for (itp = this->_players.begin(); (itp != this->_players.end()); itp++)
		if ((*itp).finish == 0)
			(*itp).nb_secs = this->_time;
	this->_players.sort();
	this->_players.reverse();
	client.say(chan->getName(), client.lang("GAME", "play_top", intToString(this->_players.size()).c_str()));
	num = 1;
	for (itp = this->_players.begin(); (itp != this->_players.end()); itp++)
	{
		if ((*itp).score == -1)
			client.say(chan->getName(), client.lang("GAME", "play_top_abandon", intToString(num).c_str(), (*itp).nick.c_str(), intToString((*itp).nb_secs / 60).c_str(), intToString((*itp).nb_secs % 60).c_str()));
		else
			client.say(chan->getName(), client.lang("GAME", "play_top_finish", intToString(num).c_str(), (*itp).nick.c_str(), intToString((*itp).score).c_str(), intToString((*itp).nb_turn).c_str(), intToString((*itp).nb_secs / 60).c_str(), intToString((*itp).nb_secs % 60).c_str()));
		num++;
	}
	client.say(chan->getName(), client.lang("GAME", "line"));
	for (itp = this->_players.begin(); (itp != this->_players.end()); itp++)
		if ((*itp).score > 0)
		{
			if (client.configIsKey("SCORE", lowCase((*itp).nick)) == false)
			{
				num = 0;
				nbr = 0;
			}
			else
			{
				num = atoi(gettok(((*client._config)["SCORE"][lowCase((*itp).nick)]), 1, ':').c_str());
				nbr = atoi(gettok(((*client._config)["SCORE"][lowCase((*itp).nick)]), 2, ':').c_str());
			}
			num += (*itp).score;
			nbr += 1;
			(*client._config)["SCORE"][lowCase((*itp).nick)] = intToString(num) + ':' + intToString(nbr);
		}
	client.configSave();
	this->_game_top(client, chan, NULL, str);
	client.say(chan->getName(), client.lang("GAME", "line"));
	client.say(chan->getName(), client.lang("GAME", "suscribe_re", GAME_NAME));
	this->_timers.push_back(Game::_timer.set(1, 3, &Game::_countdown_devoice, (void *)&client, (void *)chan, NULL));
	this->_players.clear();
	this->_state = 1;
}

Game::_player::_player()
{
 	user = NULL;
	nb_turn = 0;
	nb_secs = 0;
	score = 0;
	finish = 0;
}

Game::_player::_player(UserIRC *player)
{
 	user = player;
	nb_turn = 0;
	nb_secs = 0;
	score = 0;
	finish = 0;
}

Game::_player::_player(const _player& player)
{
	user = player.user;
	nb_turn = player.nb_turn;
	nb_secs = player.nb_secs;
	score = player.score;
	finish = player.finish;
	nick = player.nick;
}

Game::_player& Game::_player::operator=(const _player& player)
{
	if (this != &player)
	{
		this->user = player.user;
		this->nb_turn = player.nb_turn;
		this->nb_secs = player.nb_secs;
		this->score = player.score;
		this->finish = player.finish;
		this->nick = player.nick;
	}
	return (*this);
}

bool	Game::_player::operator<(const _player& p)
{
	return (this->score < p.score);
}

bool	Game::_player::operator>(const _player& p)
{
	return (this->score > p.score);	
}

Game::_player::~_player()
{
	
}
