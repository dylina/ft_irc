#include "Channel.hpp"
#include "Client.hpp"

Channel::Channel(Client *creator, std::string &topic, std::string name) : _name(name), _nbrOfUsersConnected(1)
{
	if (topic.empty())
		this->_topic.clear();
	else
		this->_topic = topic;
	this->_users.push_back(creator);
	this->mode.setOwner(creator);
	this->mode.setMode("o", creator, creator, creator->getNickName());
}

bool Channel::joinChannel(Client *user, std::string arg)
{
	bool hasPass = 0;

	if (user == NULL || this->isInChannel(user))
        return false;

	// check des permission et limit +i
	if (this->mode.isInviteOnly() )
	{
		if (!this->mode.isInvited(user)) //si le user a pas son ticket et que le chann est en +i ca natchav
		{
			user->sendReply(":ircserv 473 " + user->getNickName() + " " + this->getName() + MSG_INVITEONLYCHAN);
			return false;
		}
		else // si il a son ticket on le suprrime de la liste (usage unique)
			hasPass = 1;
	}
	// -----

	// check +k
	if (this->mode.getMode("k"))
	{

		if ((arg.empty() || !this->mode.checkKey(arg)))
		{
			if (!this->mode.isInvited(user)) // si l'user a ete inviter on bypass le +k
			{
				user->sendReply(":ircserv 475 " + user->getNickName() + " " + this->getName() + MSG_BADCHANNELKEY);
				return false;
			}
			else
				hasPass = 1;
		}
	}

	if (this->mode.getMode("l")) // si le serv est en +l on check si il est full ou non
	{
		if (this->mode.isFull(this->_nbrOfUsersConnected))
		{
			user->sendReply(":ircserv 471 " + user->getNickName() + " " + this->getName() + MSG_CHANNELISFULL);
			return false;
		}
	}

	this->_users.push_back(user);
	user->join(this->_name);
	this->_nbrOfUsersConnected++;
	if (hasPass)
		this->mode.removeInviteUser(user);
	return true;
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
			// removeoperator cherche dans tous les op et si le user en est un le supprime de sa db
			this->mode.removeOperator(user);

			// on check si le channel a un createur et si c'est bien lui on le supprime aussi ce qui rend le channel orphelin si il n'y a pas d'autre op
			if (this->mode.hasOwner() && this->mode.isOwner(user))
				this->mode.deleteOwner();

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

bool Channel::isInChannel(std::string &nickname) const
{
	for (std::vector<Client *>::const_iterator it = this->_users.begin(); it != this->_users.end(); it++)
	{
		if ((*it)->getNickName() == nickname)
            return true;
	}
	return false;
}

bool Channel::isOperator(Client *user) const
{
	if (this->mode.isOp(user))
		return true;
	return false;
}

bool Channel::isOwner(Client *user) const
{
	if (this->mode.isOwner(user))
		return true;
	return false;
}

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

void Channel::setTopic(std::string &topic)
{
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

int Channel::Mode::setMode(std::string mode, Client *user, Client *userTarget, std::string param, Channel *chann) // a utiliser que si +
{
	if (this->isOwner(user) && mode == "o" && user->getNickName() == param)
	{
		this->_o.push_back(user);
		return 1;
	}

	if (!this->isOp(user)) //verif si user est un operateur
		return ERR_CHANOPRIVSNEEDED;

	if (mode == "t")
	{
		this->_t = true;
		return 1;
	}
	else if (mode == "i")
	{
		this->_i = true;
		return 1;
	}
	else if (mode == "k")
	{
		_password = param;
		this->_k = true;
		return 1;
	}
	else if (mode == "l")
	{
		this->_l = true;
		this->_limitNbr = std::atoi(param.c_str());
		return _l;
	}
	else if (mode == "o")
	{
		if (!chann->isInChannel(param)) // si la target n'est pas enregsitrer dans le chann
			return ERR_USERNOTINCHANNEL;

		if (chann->isInChannel(param))
			this->_o.push_back(userTarget);
	}
	return 1;
}

int Channel::Mode::suppMode(std::string mode, Client *user, Client *userTarget, std::string param, Channel *chann) // a utiliser pour -
{
	// wip
	if (!this->isOp(user)) //verif si user est un operateur
		return ERR_CHANOPRIVSNEEDED;

	if (mode == "t")
	{
		this->_t = false;
		return 1;
	}
	else if (mode == "i")
	{

		this->_i = false;
		return 1;
	}
	else if (mode == "k")
	{
		if (this->checkKey(param))
			_password = "";
		else
			return ERR_KEYSET;

		this->_k = false;
		return 1;
	}
	else if (mode == "l")
	{
		this->_l = false;
		return 1;
	}
	else if (mode == "o")
	{
		if (!chann->isInChannel(param)) // si la target n'est pas enregsitrer dans le chann
			return ERR_USERNOTINCHANNEL;
		else
			this->removeOperator(userTarget);
	}
	return 1;

}

void Channel::Mode::removeOperator(Client *user)
{
	std::vector<Client *>::iterator it = std::find(this->_o.begin(), this->_o.end(), user);
	if (it != this->_o.end())
		this->_o.erase(it);
}

std::string Channel::Mode::getModeActive()
{
	std::string modeActiv;

	modeActiv += "+"; //on renvoie un simple + sir aucun mode n'est actif par defaut (cf:norme irc)

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

bool Channel::Mode::hasOwner() const
{
	if (this->_owner != NULL)
		return true;
	return false;
}

void Channel::Mode::setOwner(Client *creator)
{
	this->_owner = creator;
}

void Channel::Mode::deleteOwner()
{
	this->_owner = NULL;
}

bool Channel::Mode::isOwner(Client *user) const
{
	if (this->_owner == user)
		return true;
	return false;
}

bool Channel::Mode::isOp(Client* user) const
{
	for (std::vector<Client*>::const_iterator it = this->_o.begin(); it != this->_o.end(); it++)
	{
		if (user == (*it))
			return true;
	}
	return false;
}

bool Channel::Mode::addInviteUser(Client *user, Client *userTarget)
{
	if (!this->isOp(user)) //on check si le user est bien op pour inviter la target
		return false;

	this->_iList.push_back(userTarget);
	return true;
}

void Channel::Mode::removeInviteUser(Client *userTarget)
{
	for (std::vector<Client*>::iterator it = this->_iList.begin(); it != this->_iList.end(); it++)
	{
		if (userTarget == (*it))
		{
			this->_iList.erase(it);
			break;
		}
	}
}

bool Channel::Mode::isInvited(Client *user)
{
		for (std::vector<Client*>::iterator it = this->_iList.begin(); it != this->_iList.end(); it++)
	{
		if (user == (*it))
		{
			return true;
		}
	}
	return false;
}
