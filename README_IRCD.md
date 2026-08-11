# IRCd — IRC Daemon

## 📌 Définition

**IRCd** signifie **IRC daemon**.

C'est le **programme serveur qui implémente le protocole IRC**.

Dans le projet **ft_irc**, le programme `ircserv` que tu développes est donc un **IRCd**.

---

# 🌐 Architecture générale

```text
                    Réseau IRC

        ┌──────────────┐
        │  Client IRC  │
        │              │
        │ HexChat      │
        │ irssi        │
        │ WeeChat      │
        └──────┬───────┘
               │
               │ TCP
               │
               ▼
        ┌──────────────┐
        │     IRCd     │
        │              │
        │  ircserv     │
        │              │
        │ - Clients    │
        │ - Channels   │
        │ - Commands   │
        │ - Modes      │
        │ - Messages   │
        └──────────────┘
```

Le client se connecte au serveur en **TCP**.

Le serveur reçoit ensuite des commandes IRC, les interprète et renvoie des réponses.

---

# 🧩 IRC vs IRCd

Il est important de ne pas confondre les deux.

### IRC

**IRC (Internet Relay Chat)** est un **protocole de communication**.

Il définit notamment :

* la syntaxe des messages ;
* les commandes ;
* les réponses ;
* les codes numériques ;
* les règles de communication entre clients et serveurs.

Exemple :

```text
NICK Bob
```

ou :

```text
JOIN #42
```

---

### IRCd

L'**IRCd** est le programme qui va **implémenter ces règles**.

Par exemple, lorsqu'il reçoit :

```text
JOIN #42
```

il doit :

1. vérifier que la commande est valide ;
2. vérifier que le client est correctement enregistré ;
3. trouver ou créer le channel ;
4. ajouter le client au channel ;
5. informer les autres utilisateurs ;
6. envoyer les réponses appropriées au client.

---

# 🔌 Connexion d'un client

Un IRCd commence généralement par créer un socket serveur.

```cpp
socket()
```

Puis il associe le socket à une adresse et un port :

```cpp
bind()
```

Il passe ensuite en mode écoute :

```cpp
listen()
```

Et accepte les connexions :

```cpp
accept()
```

On obtient donc :

```text
                 IRCd

                  │
               socket()
                  │
                bind()
                  │
               listen()
                  │
                  ▼
          ┌───────────────┐
          │ Port IRC      │
          │ ex: 6667      │
          └───────┬───────┘
                  │
            nouvelles
            connexions
                  │
                  ▼
              accept()
                  │
                  ▼
               Client
```

---

# 👤 Gestion des clients

L'IRCd doit garder une représentation des clients connectés.

Par exemple :

```text
Clients

├── Bob
│   ├── nickname : Bob
│   ├── username : bob
│   └── socket : 4
│
├── Alice
│   ├── nickname : Alice
│   ├── username : alice
│   └── socket : 5
│
└── John
    ├── nickname : John
    ├── username : john
    └── socket : 6
```

Chaque connexion TCP correspond à un client.

L'IRCd doit donc savoir :

* quel socket appartient à quel client ;
* quel est son nickname ;
* s'il est enregistré ;
* quels channels il rejoint ;
* quels privilèges il possède.

---

# 🏠 Gestion des channels

L'IRCd gère également les salons IRC.

Exemple :

```text
#42

├── Bob       @
├── Alice
└── John
```

Le serveur doit savoir :

```text
Channel
├── nom
├── utilisateurs
├── modes
├── topic
└── opérateurs
```

Lorsqu'un utilisateur fait :

```text
JOIN #42
```

l'IRCd modifie son état interne.

Puis il peut envoyer aux utilisateurs concernés :

```text
:Bob!bob@localhost JOIN :#42
```

---

# 💬 Gestion des messages

Lorsqu'un utilisateur écrit :

```text
PRIVMSG #42 :Salut tout le monde !
```

l'IRCd reçoit cette commande.

Il doit déterminer :

```text
PRIVMSG
   │
   ├── destination = #42
   │
   └── message = Salut tout le monde !
```

Puis transmettre le message aux utilisateurs concernés.

```text
              Bob
               │
               │ PRIVMSG #42
               ▼
             IRCd
            /    \
           /      \
          ▼        ▼
       Alice      John
```

Bob ne parle donc pas directement à Alice.

Le serveur joue le rôle de **relay**.

---


# 🧠 Le rôle du parsing

L'IRCd reçoit simplement une suite de caractères.

Par exemple :

```text
PRIVMSG #42 :Hello World!\r\n
```

Il doit comprendre :

```text
Command   → PRIVMSG
Target    → #42
Message   → Hello World!
```

Il peut ensuite appeler le traitement correspondant :

```cpp
handlePrivmsg(client, message);
```

---

# 🔐 Authentification et enregistrement

Un client IRC n'est pas forcément considéré comme **enregistré** immédiatement après sa connexion TCP.

Dans un IRCd classique, le client peut envoyer :

```text
PASS password
NICK Bob
USER bob 0 * :Bob
```

Le serveur conserve alors l'état du client :

```text
TCP connecté
      │
      ▼
   PASS ?
      │
      ▼
   NICK ?
      │
      ▼
   USER ?
      │
      ▼
  REGISTERED
```

Une fois les informations nécessaires reçues, l'IRCd peut envoyer le message de bienvenue.

Exemple :

```text
:ircserv 001 Bob :Welcome to the IRC Network
```

---

# ⚙️ Les commandes

L'IRCd possède généralement un système permettant de router les commandes.

```text
Commande reçue
      │
      ▼
   Parser
      │
      ▼
 ┌─────────────┐
 │   Command   │
 └──────┬──────┘
        │
        ├── NICK   → handleNick()
        ├── USER   → handleUser()
        ├── JOIN   → handleJoin()
        ├── PART   → handlePart()
        ├── PRIVMSG → handlePrivmsg()
        ├── MODE   → handleMode()
        └── KICK   → handleKick()
```

Cela permet de séparer :

```text
Parsing
   ↓
Identification de la commande
   ↓
Validation
   ↓
Traitement
   ↓
Réponse IRC
```

---

# 👑 Les opérateurs

Un IRCd peut également gérer différents niveaux de privilèges.

Par exemple :

```text
Utilisateur normal
        │
        ▼
    Opérateur
        │
        ▼
 Privilèges supplémentaires
```

Dans un channel :

```text
#42

Bob       @
Alice
John
```

Le `@` indique généralement que Bob possède le statut **operator** du channel.

Il peut alors avoir le droit d'effectuer certaines actions :

```text
KICK
MODE
TOPIC
INVITE
...
```

---



# 🔄 Boucle principale de l'IRCd

Un serveur IRC doit rester actif et gérer plusieurs clients.

Une architecture simplifiée ressemble à :

```text
                IRCd
                 │
                 ▼
          ┌─────────────┐
          │   poll()    │
          └──────┬──────┘
                 │
        ┌────────┼────────┐
        │        │        │
        ▼        ▼        ▼
     Client A Client B Client C
        │        │        │
       recv()   recv()   recv()
        │        │        │
        ▼        ▼        ▼
      parse    parse    parse
        │        │        │
        └────────┼────────┘
                 ▼
             Commands
                 │
                 ▼
             Responses
                 │
                 ▼
               send()
```

C'est notamment pour cela que `poll()` est important dans **ft_irc**.

Le serveur peut surveiller plusieurs sockets sans avoir besoin d'un thread par client.

---

# 🧱 Structure logique d'un IRCd

Une architecture possible :

```text
IRCd
│
├── Server
│   ├── socket
│   ├── bind
│   ├── listen
│   └── poll
│
├── Client
│   ├── socket
│   ├── nickname
│   ├── username
│   └── state
│
├── Channel
│   ├── name
│   ├── users
│   ├── operators
│   ├── topic
│   └── modes
│
├── Parser
│   └── IRC messages
│
└── Commands
    ├── PASS
    ├── NICK
    ├── USER
    ├── JOIN
    ├── PART
    ├── PRIVMSG
    ├── MODE
    ├── KICK
    ├── INVITE
    └── TOPIC
```

---

# 🔥 Dans ft_irc

Ton projet peut être résumé ainsi :

```text
                ┌──────────────┐
                │ Client IRC   │
                └──────┬───────┘
                       │
                       │ TCP
                       ▼
                ┌──────────────┐
                │   ircserv    │
                │    (IRCd)    │
                └──────┬───────┘
                       │
          ┌────────────┼────────────┐
          ▼            ▼            ▼
       Clients       Channels     Commands
          │            │            │
          ▼            ▼            ▼
       sockets       users       parsing
                                   │
                                   ▼
                                replies
```

Ton objectif n'est donc pas simplement de **faire un programme qui reçoit des messages**.

Tu construis un véritable **serveur IRC**, c'est-à-dire un **IRCd**, qui doit :

* accepter des connexions TCP ;
* gérer plusieurs clients ;
* comprendre le protocole IRC ;
* parser les commandes ;
* maintenir l'état du réseau ;
* gérer les channels ;
* gérer les utilisateurs ;
* appliquer les permissions ;
* envoyer les réponses IRC appropriées ;
* gérer correctement les déconnexions et erreurs.

---

# 📝 À retenir

```text
IRC
│
└── Protocole
    ├── commandes
    ├── messages
    ├── réponses
    └── règles

IRCd
│
└── Programme serveur
    ├── sockets
    ├── clients
    ├── channels
    ├── parsing
    ├── commandes
    ├── modes
    └── réponses
```

### La phrase à retenir

> **Un IRCd est un serveur qui implémente le protocole IRC et qui permet à plusieurs clients de communiquer entre eux via ce serveur.**

Dans **ft_irc** :

```text
ircserv = IRCd
```
