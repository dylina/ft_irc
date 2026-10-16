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
		void sendToAllOtherClient(std::string &msg, Client *user);

		class Mode{
			public:
				Mode();

				class NoModeFind : public std::exception
				{
					public:
						const char* what() const throw();
				};
				class ModeAlreadySet : public std::exception
				{
					public:
						const char* what() const throw();
				};
				class ModeDontHaveParams : public std::exception
				{
					public:
						const char* what() const throw();
				};

				// setters

				bool setMode(std::string mode, Client *user, Client *userTarget, std::string pass);

				//getters

				bool getMode(std::string mode);
			private:
				int _i; //invitation mode
				int _t; // operators reserved
				int _k; // set a pass
				std::vector<Client *> _o; // set operator mode to a user
				int _l;	//limit nbr of members
				std::string _password;
		};
		Mode mode;

	private:

		std::string &_name;
		std::vector<Client *> _users;
		std::vector<std::string> _historique;
		int _nbrOfUsersConnected;
		std::string _topic;

};
