#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <iterator>

#include "Client.hpp"

class Channel{

	public :

		Channel(Client *creator, std::string &topic, std::string &name);

		std::vector<int> getAllUsers();
		std::string getName() const;
		std::string &getTopic();
		int getNbrConnected() const;

		void joinChannel(Client *user);
		void leaveChannel(Client *user);
		bool isInChannel(Client *user) const;
		void sendToAll(std::string &msg);

	private:

		std::string &_name;
		std::string &_topic;
		std::vector<Client *> _users;
		std::vector<std::string> _historique;
		int _nbrOfUsersConnected;
};
