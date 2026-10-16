#pragma once

// ============================================
// ERREURS GENERALES
// ============================================

#define MSG_NOSUCHNICK          ":No such nick/channel"
#define MSG_NOSUCHSERVER        ":No such server"
#define MSG_NOSUCHCHANNEL       ":No such channel"
#define MSG_CANNOTSENDTOCHAN    ":Cannot send to channel"
#define MSG_TOOMANYCHANNELS     ":You have joined too many channels"
#define MSG_TOOMANYTARGETS      ":Duplicate recipients. No message delivered"
#define MSG_NOORIGIN            ":No origin specified"
#define MSG_NORECIPIENT         ":No recipient given"
#define MSG_NOTEXTTOSEND        ":No text to send"
#define MSG_UNKNOWNCOMMAND      ":Unknown command"
#define MSG_NOMOTD              ":MOTD File is missing"

// ============================================
// NICK
// ============================================

#define MSG_NONICKNAMEGIVEN     ":No nickname given"
#define MSG_ERRONEUSNICKNAME    ":Erroneous nickname"
#define MSG_NICKNAMEINUSE       ":Nickname is already in use"

// ============================================
// CHANNEL / MEMBRES
// ============================================

#define MSG_USERNOTINCHANNEL    ":They aren't on that channel"
#define MSG_NOTONCHANNEL        ":You're not on that channel"
#define MSG_USERONCHANNEL       ":is already on channel"
#define MSG_NEEDMOREPARAMS      ":Not enough parameters"
#define MSG_ALREADYREGISTERED   ":You may not reregister"
#define MSG_PASSWDMISMATCH      ":Password incorrect"

// ============================================
// MODES DES CHANNELS
// ============================================

#define MSG_CHANNELISFULL       ":Cannot join channel (+l)"
#define MSG_UNKNOWNMODE         ":is unknown mode char to me"
#define MSG_INVITEONLYCHAN      ":Cannot join channel (+i)"
#define MSG_BANNEDFROMCHAN      ":Cannot join channel (+b)"
#define MSG_BADCHANNELKEY       ":Cannot join channel (+k)"
#define MSG_BADCHANMASK         ":Bad Channel Mask"
#define MSG_CHANOPRIVSNEEDED    ":You're not channel operator"

// ============================================
// PRIVILEGES / COMMANDES
// ============================================

#define MSG_NOPRIVILEGES        ":Permission Denied- You're not an IRC operator"
#define MSG_RESTRICTED          ":Your connection is restricted"
#define MSG_NOOPERHOST          ":No O-lines for your host"
#define MSG_UMODEUNKNOWNFLAG    ":Unknown MODE flag"
#define MSG_USERSDONTMATCH      ":Cannot change mode for other users"

// ============================================
// ENREGISTREMENT / CONNEXION
// ============================================

#define MSG_NOTREGISTERED       ":You have not registered"
