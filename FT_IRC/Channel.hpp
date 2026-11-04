#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <iterator>

#include "IrcErrors.hpp"

class Client;

class Channel
{

public:
	Channel(Client *creator, std::string &topic, std::string name);

	// getters

	std::vector<int> getAllUsers();
	std::string getName() const;
	std::string &getTopic();
	int getNbrConnected() const;
	bool haveTopic();
	bool isInChannel(Client *user) const;
	bool isInChannel(std::string &nickname) const;
	bool isOperator(Client *user) const;
	bool isOwner(Client *user) const;

	// setter

	void setTopic(std::string &topic);

	// other

	void joinChannel(Client *user);
	void leaveChannel(Client *user);
	void sendToAll(std::string &msg);
	void sendToAllOtherClient(std::string &msg, Client *user);

	class Mode
	{
	public:
		Mode();

		class NoModeFind : public std::exception
		{
		public:
			const char *what() const throw();
		};
		class ModeAlreadySet : public std::exception
		{
		public:
			const char *what() const throw();
		};
		class ModeDontHaveParams : public std::exception
		{
		public:
			const char *what() const throw();
		};

		// setters

		int setMode(std::string mode, Client *user, Client *userTarget, std::string param, Channel* chann = NULL);
		// void suppMode(std::string mode, Client *user, Client *userTarget, std::string param); wip
		void setOwner(Client *creator);
		void deleteOwner();

		void removeOperator(Client *user);
		// getters

		bool getMode(std::string mode);
		std::string getModeActive();

		bool isOwner(Client *user) const;
		bool isOp(Client* user) const;
		bool isInviteOnly();
		bool hasKey();
		bool isFull(int currentUsers);
		bool checkKey(const std::string &key);

		bool hasOwner() const;

	private:
		bool _i;				  // invitation mode
		bool _t;				  //  (Topic protection) operators reserved
		bool _k;				  // set a pass
		std::vector<Client *> _o; // set operator mode to a user
		bool _l;				  // limit nbr of members
		std::string _password;
		int _limitNbr;
		Client* _owner;
	};
	Mode mode;

private:
	std::string _name;
	std::vector<Client *> _users;
	std::vector<std::string> _historique;
	int _nbrOfUsersConnected;
	std::string _topic;
};
