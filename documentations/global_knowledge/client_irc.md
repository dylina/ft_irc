## Clients IRC

[REVENIR AU SOMMAIRE](README.md)

Un **client IRC** est un programme permettant à un utilisateur de se connecter à un serveur IRC et de communiquer avec d'autres utilisateurs.

Le client prend en charge la communication entre :

```text
Utilisateur
     │
     ▼
Client IRC
     │
     │ TCP
     ▼
Serveur IRC
     │
     ▼
Réseau IRC
```

Le client IRC n'est généralement **pas le serveur**.

Il envoie des commandes IRC au serveur :

```text
NICK Manuel
USER manuel 0 * :Manuel
JOIN #42
PRIVMSG #42 :Bonjour !
```

Et reçoit les réponses du serveur :

```text
:serveur 001 Manuel :Bienvenue sur le réseau IRC
```

## Exemples

* HexChat
* irssi
* WeeChat
* Quassel IRC
* The Lounge

Certains clients comme Quassel ou The Lounge peuvent intégrer une architecture permettant de rester connectés au serveur même lorsque l'interface utilisateur est déconnectée.

---
