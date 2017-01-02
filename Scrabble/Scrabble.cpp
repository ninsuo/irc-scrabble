
#include "Game.hpp"
#include "ChannelIRC.hpp"
#include "UserIRC.hpp"

#define SCRABBLE_DB "/home/ninsuo/robots/sources/Scrabble/db-fr/Scrabble.db"
#define	SCRABBLE_DB_WORDS	363734
#define	SCRABBLE_DICO_DIR "/home/ninsuo/robots/sources/Scrabble/db-fr/"

struct		scrabble
{

	scrabble(std::string _word, unsigned int _score)
	{
		word = _word;
		score = _score;
	}

	~scrabble() {}

	scrabble(const scrabble& scrabble)
	{
		word = scrabble.word;
		score = scrabble.score;
	}

	scrabble& operator=(const scrabble& scrabble)
	{
		if (this != &scrabble)
		{
			this->word = scrabble.word;
			this->score = scrabble.score;
		}
		return (*this);
	}

	bool	operator<(const scrabble& scrabble)
	{
		return (this->score < scrabble.score);
	}

	std::string		word;
	unsigned int	score;

};

std::string	Game::scrabble_colorize_letter(char c)
{
	if (c == 'a') return ("$k10A");
	if (c == 'b') return ("$k13B");
	if (c == 'c') return ("$k13C");
	if (c == 'd') return ("$k14D");
	if (c == 'e') return ("$k10E");
	if (c == 'f') return ("$k07F");
	if (c == 'g') return ("$k14G");
	if (c == 'h') return ("$k07H");
	if (c == 'i') return ("$k10I");
	if (c == 'j') return ("$k12J");
	if (c == 'k') return ("$k04K");
	if (c == 'l') return ("$k10L");
	if (c == 'm') return ("$k14M");
	if (c == 'n') return ("$k10N");
	if (c == 'o') return ("$k10O");
	if (c == 'p') return ("$k13P");
	if (c == 'q') return ("$k12Q");
	if (c == 'r') return ("$k10R");
	if (c == 's') return ("$k10S");
	if (c == 't') return ("$k10T");
	if (c == 'u') return ("$k10U");
	if (c == 'v') return ("$k07V");
	if (c == 'w') return ("$k04W");
	if (c == 'x') return ("$k04X");
	if (c == 'y') return ("$k04Y");
	if (c == 'z') return ("$k04Z");
	return ("");
}

std::string			Game::scrabble_colorize_word(std::string word)
{
	std::string		ret;
	unsigned int	i;

	for (i = 0; (i < word.size()); i++)
		ret += this->scrabble_colorize_letter(word.at(i));
	ret += "$k";
	return (ret);
}

std::string	Game::scrabble_colorize_points(int pts)
{
	if (pts < 25) { return ("$k10" + intToString(pts) + "$k"); }
	if (pts < 50) { return ("$k14" + intToString(pts) + "$k"); }
	if (pts < 75) { return ("$k13" + intToString(pts) + "$k"); }
	if (pts < 100) { return ("$k07" + intToString(pts) + "$k"); }
	if (pts < 125) { return ("$k12" + intToString(pts) + "$k"); }
	if (pts < 150) { return ("$k04" + intToString(pts) + "$k"); }
	return ("$k04" + intToString(pts) + "$k");
}

std::string	Game::scrabble_colorize_value(int value)
{
	if (value < 3) { return ("$k10" + intToString(value) + "$k"); }
	if (value < 5) { return ("$k14" + intToString(value) + "$k"); }
	if (value < 7) { return ("$k13" + intToString(value) + "$k"); }
	if (value < 9) { return ("$k07" + intToString(value) + "$k"); }
	if (value < 11) { return ("$k12" + intToString(value) + "$k"); }
	return ("$k04" + intToString(value) + "$k");
}

void								Game::scrabble_distrib_letters(CliIRC& client)
{
	std::list<_player>::iterator	it;
	int								i;

	for (it = this->_players.begin(); (it != this->_players.end()); it++)
	{
		for (i = 0; (i < 7); i++)
			(*it).scrabble_letters.push_back(this->scrabble_rand_letter());
		client.notice((*it).user->getNick(), client.lang("MODULE", "letters", this->scrabble_spacing_letters(this->_game_get_player_by_user((*it).user).scrabble_letters).c_str()));
	}
	return ;
}

void		Game::scrabble_give_one_letter()
{
	(*(this->_turn)).scrabble_letters.push_back(this->scrabble_rand_letter());
	return ;
}

char		Game::scrabble_rand_letter()
{
	if ((random() % 2) == 0)
		return (gettok("a e i o u", ((random() % 5) + 1), ' ').at(0));
	return (gettok("b c d f g h j k l m n p q r s t v w x y z", ((random() % 21) + 1), ' ').at(0));
}

std::string			Game::scrabble_rand_word(int size)
{
	std::string		ret;
	std::ifstream	ifs;
	char			line[17];
	int				number;
	bool			cnt;

	cnt = true;
	while (cnt)
	{
		cnt = false;
		number = (random() % SCRABBLE_DB_WORDS);
		ifs.open(SCRABBLE_DB);
		if (ifs.is_open() == true)
			while (ifs.good() != false)
			{
				ifs.getline(line, 17);
				if (number == 0)
					break ;
				number--;
			}
		ifs.close();
		if (strlen(line) > 0)
			(*(line + strlen(line) - 1)) = '\0';
		ret = lowCase(line);
		if ((int)ret.length() > size)
			cnt = true;
	}
	return (ret);
}

bool				Game::scrabble_is_word(std::string word)
{
	std::ifstream	ifs;
	char			rline[17];

	if ((word.size() < 4) || (word.size() > 15))
		return (false);
	word = upCase(word);
	ifs.open(SCRABBLE_DB);
	if (ifs.is_open() == true)
		while (ifs.good() != false)
		{
			ifs.getline(rline, 17);
			if (strlen(rline) > 0)
				(*(rline + strlen(rline) - 1)) = '\0';
			if (strcmp(rline, word.c_str()) == 0)
			{
				ifs.close();
				return (true);
			}
		}
	ifs.close();
	return (false);
}

std::string			Game::scrabble_get_definition(std::string word)
{
	std::ifstream	ifs;
	char			rline[1024];
	std::string		file;
	std::string		def;

	if ((word.size() < 4) || (word.size() > 15))
		return (false);
	word = lowCase(word);
	if (word.size() < 10)
		file = std::string(SCRABBLE_DICO_DIR) + std::string("definitions.0") + intToString((int)word.size()) + std::string(".db");
	else
		file = std::string(SCRABBLE_DICO_DIR) + std::string("definitions.") + intToString((int)word.size()) + std::string(".db");
	ifs.open(file.c_str());
	if (ifs.is_open() == true)
		while (ifs.good() != false)
		{
			ifs.getline(rline, 1024);
			if (strncmp(rline, word.c_str(), word.size()) == 0)
			{
				def = "(" + gettok(rline, 2, '/') + "): " + gettok(gettok(gettok(rline, 3, '/'), 2, '('), 1, ')') + " signifiant " + lowCase(gettok(rline, 4, '/'));
				ifs.close();
				return (def);
			}
		}
	ifs.close();
	return (def);
}

int			Game::scrabble_value_letter(char c)
{
	if (c == 'a') { return (1); }
	if (c == 'b') { return (3); }
	if (c == 'c') { return (3); }
	if (c == 'd') { return (2); }
	if (c == 'e') { return (1); }
	if (c == 'f') { return (4); }
	if (c == 'g') { return (2); }
	if (c == 'h') { return (4); }
	if (c == 'i') { return (1); }
	if (c == 'j') { return (8); }
	if (c == 'k') { return (10); }
	if (c == 'l') { return (1); }
	if (c == 'm') { return (2); }
	if (c == 'n') { return (1); }
	if (c == 'o') { return (1); }
	if (c == 'p') { return (3); }
	if (c == 'q') { return (8); }
	if (c == 'r') { return (1); }
	if (c == 's') { return (1); }
	if (c == 't') { return (1); }
	if (c == 'u') { return (1); }
	if (c == 'v') { return (4); }
	if (c == 'w') { return (10); }
	if (c == 'x') { return (10); }
	if (c == 'y') { return (10); }
	if (c == 'z') { return (10); }
	return (0);
}

int					Game::scrabble_value_word(std::string word)
{
	int				ret;
	unsigned int	i;

	ret = 0;
	for (i = 0; (i < word.size()); i++)
		ret += this->scrabble_value_letter(word.at(i));
	return (ret);
}

int					Game::scrabble_calc_bonus(std::string word)
{
	int				bonus;
	unsigned int	i;
	unsigned int	j;

	bonus = 0;
	if (word.length() > 4)
		bonus += word.length();
	for (i = 0; (i < this->_scrabble_word.length()); i++)
		for (j = 0; (j < word.length()); j++)
			if (this->_scrabble_word.at(i) == word.at(j))
				bonus += this->scrabble_value_letter(word.at(j));
	return (bonus);
}

void				Game::scrabble_show_bonus(CliIRC& client, ChannelIRC *chan, std::string word)
{
	std::list<char>	list;
	int				bonus;
	unsigned int	i;
	unsigned int	j;

	bonus = 0;
	for (i = 0; (i < this->_scrabble_word.length()); i++)
		for (j = 0; (j < word.length()); j++)
			if (this->_scrabble_word.at(i) == word.at(j))
			{
				list.push_back(word.at(j));
				bonus += this->scrabble_value_letter(word.at(j));
			}
	if (bonus > 0)
	{
		if ((bonus == 1) && (list.size() == 1))
			client.say(chan->getName(), client.lang("MODULE", "bonus_word_1", this->scrabble_colorize_value(bonus).c_str(), this->scrabble_spacing_letters(list).c_str()));
		if ((bonus > 1) && (list.size() == 1))
			client.say(chan->getName(), client.lang("MODULE", "bonus_word_2", this->scrabble_colorize_value(bonus).c_str(), this->scrabble_spacing_letters(list).c_str()));
		if ((bonus == 1) && (list.size() > 1))
			client.say(chan->getName(), client.lang("MODULE", "bonus_word_3", this->scrabble_colorize_value(bonus).c_str(), this->scrabble_spacing_letters(list).c_str()));
		if ((bonus > 1) && (list.size() > 1))
			client.say(chan->getName(), client.lang("MODULE", "bonus_word_4", this->scrabble_colorize_value(bonus).c_str(), this->scrabble_spacing_letters(list).c_str()));
	}
	if (word.length() > 4)
	{
		client.say(chan->getName(), client.lang("MODULE", "bonus_size", this->scrabble_colorize_value((int)word.length()).c_str(), intToString((int)word.length()).c_str()));
	}
	return ;
}

void		Game::scrabble_show_actual_word(CliIRC& client, ChannelIRC *chan)
{
	if (this->_scrabble_last == NULL)
		client.say(chan->getName(), client.lang("MODULE", "first_word", this->scrabble_colorize_word(this->_scrabble_word).c_str()));
	else
		client.say(chan->getName(), client.lang("MODULE", "actual_word", this->scrabble_colorize_word(this->_scrabble_word).c_str(), this->_scrabble_last->getNick().c_str()));
	return ;
}

void		Game::scrabble_show_points_value(CliIRC& client, ChannelIRC *chan)
{
	client.say(chan->getName(), client.lang("MODULE", "value_letters_1"));
	client.say(chan->getName(), client.lang("MODULE", "value_letters_2"));
	return ;
}

std::string						Game::scrabble_spacing_letters(std::list<char> list)
{
	std::list<char>::iterator	it;
	std::string					res;

	for (it = list.begin(); (it != list.end()); it++)
	{
		if ((*it) < 0)
			continue ;
		if (res.length() > 0)
			res += " / ";
		res += "$b" + this->scrabble_colorize_letter((*it)) + "$k$b";
	}
	return (res);
}

std::list<char>					Game::scrabble_can_play_this_word(std::string word)
{
	std::list<char>::iterator	it;
	std::list<char>				letters;
	std::list<char>				copy;
	char						bonus;
	unsigned int				i;
	unsigned int				j;

	bonus = '\0';
	copy = (*(this->_turn)).scrabble_letters;
	for (i = 0; (i < word.length()); i++)
	{
		for (it = copy.begin(); (it != copy.end()); it++)
			if ((*it) == word.at(i))
			{
				copy.erase(it);
				break ;
			}
		if (it == copy.end())
		{
			if (bonus != '\0')
				letters.push_back(word.at(i));
			else
			{
				for (j = 0; (j < this->_scrabble_word.length()); j++)
					if (this->_scrabble_word.at(j) == word.at(i))
					{
						bonus = word.at(i);
						letters.push_back((char)(-1 * word.at(i)));
						break ;
					}
				if (j == this->_scrabble_word.length())
					letters.push_back(word.at(i));
			}
		}
	}
	return (letters);
}

void							Game::scrabble_del_letter(char c)
{
	std::list<char>::iterator	it;

	for (it = (*(this->_turn)).scrabble_letters.begin(); (it != (*(this->_turn)).scrabble_letters.end()); it++)
		if ((*it) == c)
		{
			(*(this->_turn)).scrabble_letters.erase(it);
			break ;
		}
	return ;
}

void							Game::scrabble_del_word(std::list<char> word)
{
	std::list<char>::iterator	it;

	for (it = word.begin(); (it != word.end()); it++)
		if ((*it) > 0)
			this->scrabble_del_letter(*it);
	return ;
}

bool				Game::scrabble_verif(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& cmd, std::string& param)
{
	unsigned int	i;

	param = gettok(param, 1, ' ');
	if (param.length() == 0)
	{
		client.say(chan->getName(), client.lang("MODULE", "word_bad_1", user->getNick().c_str(), cmd.c_str()));
		return (false);
	}
	if (param.length() < 4)
	{
		client.say(chan->getName(), client.lang("MODULE", "word_bad_2", user->getNick().c_str()));
		return (false);
	}
	if (param.length() > 15)
	{
		client.say(chan->getName(), client.lang("MODULE", "word_bad_3", user->getNick().c_str()));
		return (false);
	}
	for (i = 0; (i < param.length()); i++)
		if ((param.at(i) < 'a') || (param.at(i) > 'z'))
		{
			client.say(chan->getName(), client.lang("MODULE", "word_bad_4", user->getNick().c_str()));
			return (false);
		}
	return (true);
}

void							Game::scrabble_get_play(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& cmd, std::string& param)
{
	std::list<char>				list;
	std::list<char>::iterator	it;
	std::string					def;
	unsigned int				i;
	int							error;

	if ((cmd != "jeu") && (cmd != "j") && (cmd != "play") && (cmd != "game") && (cmd != "g"))
		return ;
	if (this->_play_is_turn_player(client, user) == false)
		return ;
	if (this->scrabble_verif(client, chan, user, cmd, param) == false)
		return ;
	error = 0;
	list = scrabble_can_play_this_word(param);
	for (it = list.begin(); (it != list.end()); it++)
		if ((*it) > 0)
			error++;
	if (error > 0)
	{
		if (error == 1)
			client.say(chan->getName(), client.lang("MODULE", "word_bad_5_1", user->getNick().c_str(), this->scrabble_spacing_letters(list).c_str(), this->scrabble_colorize_word(param).c_str()));
		else
			client.say(chan->getName(), client.lang("MODULE", "word_bad_5_2", user->getNick().c_str(), this->scrabble_spacing_letters(list).c_str(), this->scrabble_colorize_word(param).c_str()));
		return ;
	}
	if (this->scrabble_is_word(param) == false)
	{
		client.say(chan->getName(), client.lang("MODULE", "word_bad_6", user->getNick().c_str(), this->scrabble_colorize_word(param).c_str()));
		return ;
	}
	client.say(chan->getName(), client.lang("GAME", "line"));
	client.say(chan->getName(), client.lang("MODULE", "word_good", user->getNick().c_str(), this->scrabble_colorize_word(param).c_str(), this->scrabble_colorize_value(this->scrabble_value_word(param)).c_str()));
	def = this->scrabble_get_definition(param);
	if (def.length() > 0)
		client.say(chan->getName(), client.lang("MODULE", "word_define", param.c_str(), def.c_str()));
	this->scrabble_show_bonus(client, chan, param);
	for (i = 0; (i < param.length()); i++)
		this->scrabble_del_letter(param.at(i));
	if ((*(this->_turn)).scrabble_letters.size() == 0)
		for (i = 0; (i < 15); i++)
			this->scrabble_give_one_letter();
	else
		for (i = 0; (i < (param.length() / 2)); i++)
			this->scrabble_give_one_letter();
	(*(this->_turn)).score += this->scrabble_value_word(param);
	(*(this->_turn)).score += this->scrabble_calc_bonus(param);
	this->_scrabble_word = param;
	if ((*(this->_turn)).score >= 150)
	{
		this->_play_winner(client, chan);
		return ;
	}
	client.say(chan->getName(), client.lang("MODULE", "new_score", user->getNick().c_str(), this->scrabble_colorize_points((*(this->_turn)).score).c_str()));
	this->_play_play(client, chan);
	return ;
}

void								Game::scrabble_get_pick(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& cmd, std::string& param)
{
	std::list<scrabble>				map;
	std::list<scrabble>::iterator	it;

	if ((cmd != "pioche") && (cmd != "p") && (cmd != "pick"))
		return ;
	if (this->_play_is_turn_player(client, user) == false)
		return ;
	if ((*(this->_turn)).scrabble_letters.size() >= 80)
	{
		client.say(chan->getName(), client.lang("MODULE", "cant_pick", user->getNick().c_str()));
		return ;
	}
	client.say(chan->getName(), client.lang("MODULE", "pick", user->getNick().c_str()));
	map = scrabble_ia((*this->_turn).scrabble_letters);
	if (map.size() > 0)
	{
		if (map.size() == 1)
			client.say(chan->getName(), client.lang("MODULE", "couldplay_1", this->scrabble_colorize_word((*(map.begin())).word).c_str(), this->scrabble_colorize_value((*(map.begin())).score).c_str()));
		else
			client.say(chan->getName(), client.lang("MODULE", "couldplay_n", intToString((int)map.size()).c_str(), this->scrabble_colorize_word( (*(map.begin())).word ).c_str(), this->scrabble_colorize_value( (*(map.begin())).score ).c_str()));
	}
	this->scrabble_give_one_letter();
	client.notice(user->getNick(), client.lang("MODULE", "letters", this->scrabble_spacing_letters(this->_game_get_player_by_user(user).scrabble_letters).c_str()));
	this->_play_pass(client, chan);
	param = param;
	return ;
}

void		Game::scrabble_get_letters(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& cmd, std::string& param)
{
	if ((cmd != "lettres") && (cmd != "l") && (cmd != "lettre") && (cmd != "letter") && (cmd != "letters"))
		return ;
	client.notice(user->getNick(), client.lang("MODULE", "letters", this->scrabble_spacing_letters(this->_game_get_player_by_user(user).scrabble_letters).c_str()));
	chan = chan;
	param = param;
	return ;
}

void		Game::scrabble_get_word(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& cmd, std::string& param)
{
	if ((cmd != "mot") && (cmd != "word") && (cmd != "m") && (cmd != "w"))
		return ;
	client.say(chan->getName(), client.lang("MODULE", "word", this->scrabble_colorize_word(this->_scrabble_word).c_str(), (*this->_turn).user->getNick().c_str()));
	user = user;
	param = param;
	return ;
}

void				Game::scrabble_get_value(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& cmd, std::string& param)
{
	if ((cmd != "valeur") && (cmd != "v") && (cmd != "value"))
		return ;
	if (this->scrabble_verif(client, chan, user, cmd, param) == false)
		return ;
	client.say(chan->getName(), client.lang("MODULE", "word_value", this->scrabble_colorize_word(param).c_str(), this->scrabble_colorize_value(this->scrabble_value_word(param)).c_str()));
	return ;
}

void				Game::scrabble_get_dico(CliIRC& client, ChannelIRC *chan, UserIRC *user, std::string& cmd, std::string& param)
{
	std::string		def;

	if ((cmd != "dico") && (cmd != "d") && (cmd != "exist") && (cmd != "existe"))
		return ;
	if (this->scrabble_verif(client, chan, user, cmd, param) == false)
		return ;
	if (this->scrabble_is_word(param) == false)
	{
		client.say(chan->getName(), client.lang("MODULE", "word_unknown", this->scrabble_colorize_word(param).c_str()));
		return ;
	}
	def = this->scrabble_get_definition(param);
	if (def.length() == 0)
		client.say(chan->getName(), client.lang("MODULE", "word_exists", this->scrabble_colorize_word(param).c_str(), this->scrabble_colorize_value(this->scrabble_value_word(param)).c_str()));
	else
	{
		client.say(chan->getName(), client.lang("MODULE", "word_exists", this->scrabble_colorize_word(param).c_str(), this->scrabble_colorize_value(this->scrabble_value_word(param)).c_str()));
		client.say(chan->getName(), client.lang("MODULE", "word_define", param.c_str(), def.c_str()));
	}
	return ;
}

std::list<scrabble>				Game::scrabble_ia(std::list<char> letters)
{
	std::list<scrabble>			result;
	std::list<char>::iterator	it;
	std::ifstream				ifs;
	char						ltrs[99];
	char						copy[99];
	char						word[17];
	char						bonus;
	bool						valid;
	int							points;
	int							i;
	int							j;
	int							k;

	memset(copy, 0, 99);
	for (i = 0; (i < 99); i++)
		(*(copy + i)) = '\0';
	for (i = 0, it = letters.begin(); (it != letters.end()); it++, i++)
		(*(copy + i)) = (*it);
	ifs.open(SCRABBLE_DB);
	if (ifs.is_open() == true)
	{
		while (ifs.good() != false)
		{
			valid = true;
			bonus = '\0';
			strcpy(ltrs, copy);
			ifs.getline(word, 17);
			if ((*word) != '\0')
				(*(word + strlen(word) - 1)) = '\0';
			for (i = 0; ((*(word + i)) != '\0'); i++)
			{
				(*(word + i)) = (*(word + i)) - 'A' + 'a';
				for (j = 0; ((*(ltrs + j)) != '\0'); j++)
					if ((*(ltrs + j)) == (*(word + i)))
					{
						for (k = j; ((*(ltrs + (k + 1))) != '\0'); k++)
							(*(ltrs + k)) = (*(ltrs + (k + 1)));
						(*(ltrs + k)) = '\0';
						break ;
					}
				if ((*(ltrs + j)) == '\0')
				{
					if (bonus != '\0')
						valid = false;
					else
					{
						for (j = 0; (j < (int)this->_scrabble_word.length()); j++)
							if (this->_scrabble_word.at(j) == (*(word + i)))
							{
								bonus = (*(word + i));
								break ;
							}
						if (j == (int)this->_scrabble_word.length())
							valid = false;
					}
				}
			}
			if (valid == true)
			{
				points = 0;
				for (i = 0; (*(word + i) != '\0'); i++)
					points += this->scrabble_value_letter((*(word + i)));
				if ((int)strlen(word) > 4)
					points += (int)strlen(word);
					for (i = 0; (i < (int)this->_scrabble_word.length()); i++)
						for (j = 0; (*(word + j) != '\0'); j++)
							if (this->_scrabble_word.at(i) == (*(word + j)))
								points += this->scrabble_value_letter(*(word + j));
				scrabble tmp(word, points);
				result.push_back(tmp);
			}
		}
		ifs.close();
	}
	result.sort();
	result.reverse();
	return (result);
}

void		Game::_play_init()
{

}

void								Game::_play_start(CliIRC& client, ChannelIRC *chan)
{
	CliIRC							*irc;

	this->_scrabble_word = this->scrabble_rand_word(8);
	this->scrabble_distrib_letters(client);
	client.say(chan->getName(), client.lang("GAME", "line"));
	this->scrabble_show_points_value(client, chan);
	irc = &client;
	chan = chan;
	return ;
}

void		Game::_play_turn(CliIRC& client, ChannelIRC *chan)
{
	CliIRC	*irc;
	irc = &client;
	this->scrabble_show_actual_word(client, chan);
	client.notice((*this->_turn).user->getNick(), client.lang("MODULE", "letters", this->scrabble_spacing_letters(this->_game_get_player_by_user((*this->_turn).user).scrabble_letters).c_str()));
	chan = chan;
	return ;
}

void		Game::_play_cancel(CliIRC& client, ChannelIRC *chan)
{
	CliIRC	*irc;
	irc = &client;
	chan = chan;
	return ;
}

void		Game::_play_wins(CliIRC& client, ChannelIRC *chan)
{
	CliIRC	*irc;
	irc = &client;
	chan = chan;
	return ;
}

void		Game::_play_message(CliIRC& client, UserIRC *user, ChannelIRC *chan, std::string& cmd, std::string& param)
{
	if ((this->_state != 1) && (this->_state != 2))
		return ;
	if ((cmd != "jeu") && (cmd != "j") && (cmd != "play") && (cmd != "game") && (cmd != "g") &&
		(cmd != "pioche") && (cmd != "p") && (cmd != "pick") &&
		(cmd != "lettres") && (cmd != "l") && (cmd != "lettre") && (cmd != "letter") && (cmd != "letters") &&
		(cmd != "valeur") && (cmd != "v") && (cmd != "value") &&
		(cmd != "dico") && (cmd != "d") && (cmd != "exist") && (cmd != "existe") &&
		(cmd != "mot") && (cmd != "word") && (cmd != "m") && (cmd != "w"))
		return ;
	this->scrabble_get_value(client, chan, user, cmd, param);
	this->scrabble_get_dico(client, chan, user, cmd, param);
	if (this->_state != 2)
		return ;
	if (this->_play_state_game(client, user) == false)
		return ;
	this->scrabble_get_play(client, chan, user, cmd, param);
	this->scrabble_get_pick(client, chan, user, cmd, param);
	this->scrabble_get_letters(client, chan, user, cmd, param);
	this->scrabble_get_word(client, chan, user, cmd, param);
	return ;
}

void		Game::_play_end(CliIRC& client, ChannelIRC *chan)
{
	CliIRC	*irc;
	irc = &client;
	chan->_game._scrabble_last = NULL;
	return ;
}
