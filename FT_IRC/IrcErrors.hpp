#pragma once
#include "IrcErrorsMsg.hpp"

// ============================================
// ERREURS GENERALES
// ============================================

#define ERR_NOSUCHNICK          401 // Pseudo inexistant
#define ERR_NOSUCHSERVER        402 // Serveur inexistant
#define ERR_NOSUCHCHANNEL       403 // Channel inexistant
#define ERR_CANNOTSENDTOCHAN    404 // Message interdit dans le channel
#define ERR_TOOMANYCHANNELS     405 // Trop de channels rejoints
#define ERR_TOOMANYTARGETS      407 // Trop de destinataires
#define ERR_NOORIGIN            409 // Origine manquante
#define ERR_NORECIPIENT         411 // Destinataire manquant
#define ERR_NOTEXTTOSEND        412 // Texte du message manquant
#define ERR_UNKNOWNCOMMAND      421 // Commande inconnue
#define ERR_NOMOTD              422 // Message MOTD indisponible
#define ERR_NOTOPIC				331 // Elle indique que le canal auquel vous tentez d'accéder ou d'interroger n'a aucun sujet (topic) défini pour le moment.

// ============================================
// NICK
// ============================================

#define ERR_NONICKNAMEGIVEN     431 // Aucun pseudo fourni
#define ERR_ERRONEUSNICKNAME    432 // Pseudo invalide
#define ERR_NICKNAMEINUSE       433 // Pseudo déjà utilisé

// ============================================
// CHANNEL / MEMBRES
// ============================================

#define ERR_USERNOTINCHANNEL    441 // Cible absente du channel
#define ERR_NOTONCHANNEL        442 // Expéditeur absent du channel
#define ERR_USERONCHANNEL       443 // Cible déjà dans le channel
#define ERR_NEEDMOREPARAMS      461 // Paramètres insuffisants
#define ERR_ALREADYREGISTERED   462 // Client déjà enregistré
#define ERR_PASSWDMISMATCH      464 // Mot de passe incorrect

// ============================================
// MODES DES CHANNELS
// ============================================

#define ERR_CHANNELISFULL       471 // Channel plein (+l)
#define ERR_UNKNOWNMODE         472 // Mode inconnu
#define ERR_INVITEONLYCHAN      473 // Channel sur invitation (+i)
#define ERR_BANNEDFROMCHAN      474 // Utilisateur banni (+b)
#define ERR_BADCHANNELKEY       475 // Clé incorrecte (+k)
#define ERR_BADCHANMASK         476 // Masque de channel invalide
#define ERR_CHANOPRIVSNEEDED    482 // Opérateur requis
#define ERR_INVALIDMODEPARAM	696 // mauvaise limit pour le mode l

// ============================================
// PRIVILEGES / COMMANDES
// ============================================

#define ERR_NOPRIVILEGES        481 // Privilèges insuffisants
#define ERR_RESTRICTED          484 // Connexion restreinte
#define ERR_NOOPERHOST          491 // Hôte non autorisé pour OPER
#define ERR_UMODEUNKNOWNFLAG    501 // Mode utilisateur inconnu
#define ERR_USERSDONTMATCH      502 // Impossible d'agir sur un autre utilisateur

// ============================================
// ENREGISTREMENT / CONNEXION
// ============================================

#define ERR_NOTREGISTERED       451 // Client non enregistré
