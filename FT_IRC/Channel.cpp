#include "Channel.hpp"

Channel::Channel(Client *creator, std::string &topic, std::string name) : _name(name), _nbrOfUsersConnected(1)
{
	if (topic.empty())
		this->_topic.clear();
	else
		this->_topic = topic;
	this->_users.push_back(creator);
	this->mode.setMode("o", creator, NULL, "");
}

void Channel::joinChannel(Client *user)
{
	if (user == NULL || this->isInChannel(user))
        return;

	this->_users.push_back(user);
	user->join(this->_name);
	this->_nbrOfUsersConnected++;
}

std::vector<int> Channel::getAllUsers()
{
	std::vector<int> usersFd;
	for (std::vector<Client *>::iterator it = this->_users.begin(); it != this->_users.end(); it++)
	{
		usersFd.push_back((*it)->getFd());
	}
	return usersFd;
}

void Channel::leaveChannel(Client *user)
{
	for (std::vector<Client *>::iterator it = this->_users.begin();
		 it != this->_users.end(); ++it)
	{
		if (*it == user)
		{
			this->mode.removeOperator(user);
			user->leave(this->_name);
			this->_users.erase(it);
			this->_nbrOfUsersConnected--;
			return;
		}
	}
}

bool Channel::isInChannel(Client *user) const
{
	if (user == NULL)
        return false;

	for (std::vector<Client *>::const_iterator it = this->_users.begin(); it != this->_users.end(); it++)
	{
		if (*it == user)
            return true;
	}
	return false;
}

// * liste des membres du channel, en LECTURE SEULE (const &) et sans copie
// ? utilisee par Server::broadcastToSharedChannels() pour annoncer un changement de pseudo
const std::vector<Client *> &Channel::getUsers() const
{
	return this->_users;
}

std::string Channel::getName() const
{
	return this->_name;
}

std::string &Channel::getTopic()
{
	return this->_topic;
}

int Channel::getNbrConnected() const
{
	return this->_nbrOfUsersConnected;
}

void Channel::sendToAll(std::string &msg)
{
	for (std::vector<Client *>::iterator it = this->_users.begin(); it != this->_users.end(); ++it)
		(*it)->sendReply(msg);
}

void Channel::sendToAllOtherClient(std::string &msg, Client *user)
{
	for (std::vector<Client *>::iterator it = this->_users.begin(); it != this->_users.end(); ++it)
	{
		if ((*it)->getNickName() != user->getNickName())
			(*it)->sendReply(msg);
	}
}

bool Channel::haveTopic()
{
	if (this->_topic.empty())
		return false;
	return true;
}

void Channel::setTopic(std::string &topic, Client *user)
{
	// verif si les modes sont bon a faire
	user->getAddr();
	// le reste de la demarche
	this->_topic = topic;
}

Channel::Mode::Mode() : _i(false), _t(false), _k(false), _o(), _l(false), _limitNbr(-1)
{
	this->_password = "";
}

const char *Channel::Mode::NoModeFind::what() const throw()
{
	return "No mode match";
}

const char *Channel::Mode::ModeAlreadySet::what() const throw()
{
	return "Mode already set";
}

const char *Channel::Mode::ModeDontHaveParams::what() const throw()
{
	return "Mode missing params";
}

bool Channel::Mode::getMode(std::string mode)
{
	if (mode == "t")
		return _t;
	else if (mode == "i")
		return _i;
	else if (mode == "k")
		return _k;
	else if (mode == "l")
		return _l;
	else if (mode == "o")
	{
		if (_o.empty())
			return false;
		else
			return true;
	}

	throw NoModeFind();
	return false;
}

bool Channel::Mode::setMode(std::string mode, Client *user, Client *userTarget, std::string param) // a utiliser que si +
{
	if (userTarget == NULL && mode == "o")
	{
		this->_o.push_back(user);
		return true;
	}

	std::vector<Client *>::iterator isOp = std::find(this->_o.begin(), this->_o.end(), user);
	if (isOp == this->_o.end())
		return false;

	if (mode == "t")
	{
		if (_t)
			throw ModeAlreadySet();
		else
			this->_t = true;
		return _t;
	}
	else if (mode == "i")
	{
		if (_i)
			throw ModeAlreadySet();
		else
			this->_i = true;
		return _i;
	}
	else if (mode == "k")
	{
		if (_k)
			throw ModeAlreadySet();

		if (param.empty())
			throw ModeDontHaveParams();

		_password = param;
		this->_k = true;
		return _k;
	}
	else if (mode == "l")
	{
		if (_l)
			throw ModeAlreadySet();

		this->_l = true;
		this->_limitNbr = std::atoi(param.c_str());
		return _l;
	}
	else if (mode == "o")
	{
		std::vector<Client *>::iterator it = std::find(this->_o.begin(), this->_o.end(), userTarget);
		if (it == this->_o.end())
			this->_o.push_back(userTarget);
		else
			throw ModeAlreadySet();
	}
	return true;
}

// void Channel::Mode::suppMode(std::string mode, Client *user, Client *userTarget, std::string param) // a utiliser pour -
// {
// 	// wip
// }

void Channel::Mode::removeOperator(Client *user)
{
	std::vector<Client *>::iterator it = std::find(this->_o.begin(), this->_o.end(), user);
	if (it != this->_o.end())
		this->_o.erase(it);
}

std::string Channel::Mode::getModeActive()
{
	std::string modeActiv;

	modeActiv += "+";

	if (this->getMode("t"))
		modeActiv += "t";
	if (this->getMode("i"))
		modeActiv += "i";
	if (this->getMode("k"))
		modeActiv += "k";
	if (this->getMode("l"))
		modeActiv += "l";

	return modeActiv;
}

bool Channel::Mode::isInviteOnly()
{
    return _i;
}

bool Channel::Mode::hasKey()
{
    return _k;
}

bool Channel::Mode::isFull(int currentUsers)
{
    return _l && currentUsers >= _limitNbr;
}

bool Channel::Mode::checkKey(const std::string &key)
{
    return !_k || key == _password;
}
