#include "Channel.hpp"

Channel::Channel(Client *creator, std::string &topic, std::string &name) : _name(name), _topic(topic), _nbrOfUsersConnected(1)
{
	this->_users.push_back(creator);
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
