#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <iterator>

#include "Client.hpp"

class Channel{

	public :

		Channel(Client *creator, std::string &topic, std::string &name);

		// getters

		std::vector<int> getAllUsers();
		std::string getName() const;
		std::string &getTopic();
		int getNbrConnected() const;
		bool haveTopic();
		bool isInChannel(Client *user) const;

		// setter
		void setTopic(std::string &topic, Client *user);


		// other

		void joinChannel(Client *user);
		void leaveChannel(Client *user);
		void sendToAll(std::string &msg);


	private:

		std::string &_name;
		std::vector<Client *> _users;
		std::vector<std::string> _historique;
		int _nbrOfUsersConnected;
		std::string _topic;
};
