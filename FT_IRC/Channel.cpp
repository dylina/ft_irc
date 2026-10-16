#include "Channel.hpp"

Channel::Channel(Client *creator, std::string &topic, std::string &name) : _name(name), _nbrOfUsersConnected(1)
{
	if (topic.empty())
		this->_topic.clear();
	else
		this->_topic = topic;
	this->_users.push_back(creator);
	this->mode.setMode("o", creator, NULL, NULL);
}


void Channel::joinChannel(Client *user)
{
	this->_users.push_back(user);
	user->join(this->_name);
	this->_nbrOfUsersConnected++;
}

std::vector<int> Channel::getAllUsers()
{
	std::vector<int> usersFd;
	for (std::vector<Client*>::iterator it = this->_users.begin(); it != this->_users.end(); it++)
	{
		usersFd.push_back((*it)->getFd());
	}
	return usersFd;
}

void Channel::leaveChannel(Client *user)
{
	for (std::vector<Client*>::iterator it = this->_users.begin();
		it != this->_users.end(); ++it)
	{
		if (*it == user)
		{
			user->leave(this->_name);
			this->_users.erase(it);
			this->_nbrOfUsersConnected--;
			return;
		}
	}
}

bool Channel::isInChannel(Client *user) const
{
	for (std::vector<Client*>::const_iterator it = this->_users.begin(); it != this->_users.end(); it++)
	{
		if (user->getNickName() == (*it)->getNickName())
			return true;
	}
	return false;
}

std::string Channel::getName() const
{
	return this->_name;
}

std::string & Channel::getTopic()
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

Channel::Mode::Mode() : _i(false), _t(false), _k(false), _o(), _l(false)
{}

const char* Channel::Mode::NoModeFind::what() const throw()
{
	return "No mode match";
}

const char* Channel::Mode::ModeAlreadySet::what() const throw()
{
	return "Mode already set";
}

const char* Channel::Mode::ModeDontHaveParams::what() const throw()
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

bool Channel::Mode::setMode(std::string mode, Client *user, Client *userTarget, std::string pass)
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

		if (pass.empty())
			throw ModeDontHaveParams();

		_password = pass;
		this->_k = true;
		return _k;
	}
	else if (mode == "l")
	{
		if (_l)
			throw ModeAlreadySet();
		else
			this->_l = true;
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
