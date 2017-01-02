
#ifndef __GAME_HPP__
#define __GAME_HPP__

#define	GAME_NAME		"Scrabble"
#define	GAME_MIN_PLAY	2
#define	GAME_MAX_PLAY	20
#define	GAME_LINK		"http://www.ircbots.fr/robot.php?p=info&id=4"

#include <string>
#include <map>
#include <list>

#include "CliIRC.hpp"
#include "MyTimer.hpp"

class CliIRC;
class UserIRC;
class ChannelIRC;

struct scrabble;

class Game
{

	friend class CliIRC;
	friend class ChannelIRC;

public:

	Game();
	Game(const Game& game);
	Game&	operator=(const Game& game);
	~Game();
	
	static void	init(CliIRC& c);
	bool		isPlayer(const UserIRC *user);
	std::string	stringPlayers();
	void		timersOff();
	std::string	randPlayer();

protected:

	static void		bot_join(CliIRC& cli, void *ptr, UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string msg);
	static void		oth_join(CliIRC& cli, void *ptr, UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string msg);
	static void		oth_part(CliIRC& cli, void *ptr, UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string msg);
	static void		oth_part_rev(CliIRC& cli, void *ptr, UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string msg);
	static void		bot_part(CliIRC& cli, void *ptr, UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string msg);
	static void		oth_msg(CliIRC& cli, void *ptr, UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string msg);
	static void		oth_nick(CliIRC& cli, void *ptr, UserIRC *src, ChannelIRC *chan, UserIRC *dst, std::string msg);

	void			_game_rules(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg);
	void			_game_suscribe(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg);
	void			_game_list(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg);
	void			_game_time(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg);
	void			_game_players(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg);
	void			_game_unsuscribe(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg);
	void			_game_start(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg);
	void			_game_trash(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg);
	void			_game_reset(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg);
	void			_game_stop(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg);
	void			_game_score(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg);
	void			_game_status(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg);
	void			_game_top(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& msg);

	void			_play_new_play(CliIRC& client, ChannelIRC *chan);
	void			_play_new_turn(CliIRC& client, ChannelIRC *chan);
	bool			_play_stay_players();
	void			_play_next_player();
	void			_play_prev_player();
	bool			_play_cannot_continue(CliIRC& client, ChannelIRC *chan);
	int				_play_nb_players();
	bool			_play_state_game(CliIRC& client, UserIRC *user);
	bool			_play_state_suscrib(CliIRC& client, UserIRC *user);
	bool			_play_is_player(CliIRC& client, UserIRC *user);
	bool			_play_is_turn_player(CliIRC& client, UserIRC *user);
	std::string		_play_list();
	static void		_play_timeout_c1(void *cli, void *chan, void *nul);
	static void		_play_timeout_c2(void *cli, void *chan, void *nul);
	static void		_play_timeout_c3(void *cli, void *chan, void *nul);
	void			_play_winner(CliIRC& client, ChannelIRC *chan);
	void			_play_pass(CliIRC& client, ChannelIRC *chan);
	void			_play_play(CliIRC& client, ChannelIRC *chan);
	void			_play_finish(CliIRC& client, ChannelIRC *chan);

	void			_play_init();
	void			_play_start(CliIRC& client, ChannelIRC *chan);
	void			_play_turn(CliIRC& client, ChannelIRC *chan);
	void			_play_cancel(CliIRC& client, ChannelIRC *chan);
	void			_play_wins(CliIRC& client, ChannelIRC *chan);
	void			_play_message(CliIRC& client, UserIRC *user, ChannelIRC *chan, std::string& cmd, std::string& param);
	void			_play_end(CliIRC& client, ChannelIRC *chan);

	void			_startCountdown(CliIRC& client, ChannelIRC *chan);
	static void		_countdown_inc(void *cli, void *chan, void *nul);
	static void		_countdown_01(void *cli, void *chan, void *nul);
	static void		_countdown_02(void *cli, void *chan, void *nul);
	static void		_countdown_03(void *cli, void *chan, void *nul);
	static void		_countdown_04(void *cli, void *chan, void *nul);
	static void		_countdown_05(void *cli, void *chan, void *nul);
	static void		_countdown_06(void *cli, void *chan, void *nul);
	static void		_countdown_07(void *cli, void *chan, void *nul);
	static void		_countdown_08(void *cli, void *chan, void *nul);
	static void		_countdown_09(void *cli, void *chan, void *nul);
	static void		_countdown_10(void *cli, void *chan, void *nul);
	static void		_countdown_11(void *cli, void *chan, void *nul);
	static void		_countdown_12(void *cli, void *chan, void *nul);
	static void		_countdown_13(void *cli, void *chan, void *nul);
	static void		_countdown_devoice(void *cli, void *chan, void *nul);

public:

	std::string					scrabble_colorize_letter(char c);
	std::string					scrabble_colorize_word(std::string word);
	std::string					scrabble_colorize_points(int pts);
	std::string					scrabble_colorize_value(int value);
	char						scrabble_rand_letter();
	void						scrabble_distrib_letters(CliIRC& client);
	void						scrabble_give_one_letter();
	std::string					scrabble_rand_word(int size);
	bool						scrabble_is_word(std::string word);
	std::string					scrabble_get_definition(std::string word);
	int							scrabble_value_letter(char c);
	int							scrabble_value_word(std::string word);
	int							scrabble_calc_bonus(std::string word);
	void						scrabble_show_bonus(CliIRC& client, ChannelIRC *chan, std::string word);
	void						scrabble_show_actual_word(CliIRC& client, ChannelIRC *chan);
	void						scrabble_show_points_value(CliIRC& client, ChannelIRC *chan);
	std::string					scrabble_spacing_letters(std::list<char> list);
	std::list<char>				scrabble_can_play_this_word(std::string word);
	void						scrabble_del_letter(char c);
	void						scrabble_del_word(std::list<char> word);
	bool						scrabble_verif(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& cmd, std::string& param);
	void						scrabble_get_play(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& cmd, std::string& param);
	void						scrabble_get_pick(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& cmd, std::string& param);
	void						scrabble_get_word(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& cmd, std::string& param);
	void						scrabble_get_letters(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& cmd, std::string& param);
	void						scrabble_get_value(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& cmd, std::string& param);
	void						scrabble_get_dico(CliIRC& client, ChannelIRC *chan, UserIRC *use, std::string& cmd, std::string& paramr);
	std::list<scrabble>			scrabble_ia(std::list<char> letters);

public:

	class _player
	{

	public:

		_player();
		_player(UserIRC *user);
		_player(const _player& player);
		_player&		operator=(const _player& player);
		bool			operator<(const _player& p);
		bool			operator>(const _player& p);
		~_player();

		std::string		nick;
		UserIRC			*user;
		unsigned int	nb_turn;
		unsigned int	nb_secs;
		int				score;
		int				finish;
		
		std::list<char>	scrabble_letters;
	
	};

private:

	_player&		_game_get_player_by_user(UserIRC *user);

private:

	std::map<std::string, void (Game::*)(CliIRC& client, ChannelIRC *, UserIRC *, std::string& msg)>	_cmds;
	unsigned int					_state;
	std::list<_player>				_players;
	std::list<_player>::iterator	_turn;
	unsigned int					_time;
	static MyTimer					_timer;
	std::list<int>					_timers;
	int								_finish;

	std::string						_scrabble_word;
	UserIRC							*_scrabble_last;
};

#endif // __GAME_HPP__
