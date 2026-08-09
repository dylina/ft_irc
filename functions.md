# 📚 C++98 — Référence des fonctions système & réseau

> Guide de révision pour les fonctions utilisées dans un serveur TCP/IRC en **C++98**.

---

# 📑 Sommaire

- [🌐 1. Création et gestion des sockets](#-1-création-et-gestion-des-sockets)
  - [`socket()`](#socket)
  - [`close()`](#close)
  - [`setsockopt()`](#setsockopt)
  - [`getsockname()`](#getsockname)
- [🔎 2. Résolution réseau](#-2-résolution-réseau)
  - [`getprotobyname()`](#getprotobyname)
  - [`gethostbyname()`](#gethostbyname)
  - [`getaddrinfo()`](#getaddrinfo)
  - [`freeaddrinfo()`](#freeaddrinfo)
- [🔌 3. Connexion TCP](#-3-connexion-tcp)
  - [`bind()`](#bind)
  - [`connect()`](#connect)
  - [`listen()`](#listen)
  - [`accept()`](#accept)
- [🔢 4. Conversion des adresses et ports](#-4-conversion-des-adresses-et-ports)
  - [`htons()`](#htons)
  - [`htonl()`](#htonl)
  - [`ntohs()`](#ntohs)
  - [`ntohl()`](#ntohl)
  - [`inet_addr()`](#inet_addr)
  - [`inet_ntoa()`](#inet_ntoa)
  - [`inet_ntop()`](#inet_ntop)
- [📨 5. Communication](#-5-communication)
  - [`send()`](#send)
  - [`recv()`](#recv)
- [🚦 6. Signaux](#-6-signaux)
  - [`signal()`](#signal)
  - [`sigaction()`](#sigaction)
  - [`sigemptyset()`](#sigemptyset)
  - [`sigfillset()`](#sigfillset)
  - [`sigaddset()`](#sigaddset)
  - [`sigdelset()`](#sigdelset)
  - [`sigismember()`](#sigismember)
- [📁 7. Fichiers et file descriptors](#-7-fichiers-et-file-descriptors)
  - [`lseek()`](#lseek)
  - [`fstat()`](#fstat)
  - [`fcntl()`](#fcntl)
- [⏳ 8. Multiplexage avec poll](#-8-multiplexage-avec-poll)
  - [`poll()`](#poll)
  - [`struct pollfd`](#struct-pollfd)
- [🚀 9. Architecture d'un serveur IRC](#-9-architecture-dun-serveur-irc)
- [🧠 10. Tableau de révision](#-10-tableau-de-révision)

---

# 🌐 1. Création et gestion des sockets

## `socket()`

### Rôle

Crée un socket.

Un socket est un **file descriptor permettant de communiquer via le réseau**.

### Prototype

```cpp
int socket(int domain, int type, int protocol);
```

### Exemple

```cpp
int serverFd = socket(AF_INET, SOCK_STREAM, 0);
```

### Paramètres

| Paramètre | Exemple | Signification |
|---|---|---|
| `domain` | `AF_INET` | IPv4 |
| `type` | `SOCK_STREAM` | TCP |
| `protocol` | `0` | Choix automatique |

### Retour

```text
>= 0 → file descriptor du socket
-1   → erreur
```

### Schéma

```text
socket()
   │
   ▼
┌─────────────┐
│ Socket fd=3 │
└─────────────┘
```

---

## `close()`

### Rôle

Ferme un file descriptor.

### Prototype

```cpp
int close(int fd);
```

### Exemple

```cpp
close(clientFd);
```

### Retour

```text
0  → succès
-1 → erreur
```

Utilisé pour fermer :

- sockets
- fichiers
- autres file descriptors

---

## `setsockopt()`

### Rôle

Modifie une option d'un socket.

### Prototype

```cpp
int setsockopt(
    int socket,
    int level,
    int option_name,
    const void *option_value,
    socklen_t option_len
);
```

### Exemple classique

```cpp
int opt = 1;

setsockopt(
    serverFd,
    SOL_SOCKET,
    SO_REUSEADDR,
    &opt,
    sizeof(opt)
);
```

### `SO_REUSEADDR`

Permet notamment de réutiliser rapidement une adresse/port après l'arrêt du serveur.

```text
Serveur
   │
   ├── bind(6667)
   │
   ├── CTRL+C
   │
   └── relance
          ↓
   SO_REUSEADDR
```

---

## `getsockname()`

### Rôle

Récupère l'adresse **locale** associée à un socket.

### Prototype

```cpp
int getsockname(
    int sockfd,
    struct sockaddr *addr,
    socklen_t *addrlen
);
```

### Exemple

```cpp
struct sockaddr_in addr;
socklen_t len = sizeof(addr);

getsockname(
    serverFd,
    (struct sockaddr *)&addr,
    &len
);
```

Permet notamment de récupérer :

```text
IP
PORT
```

### À retenir

```text
getsockname()
    ↓
"Quelle est mon adresse ?"
```

---

# 🔎 2. Résolution réseau

## `getprotobyname()`

### Rôle

Recherche les informations d'un protocole à partir de son nom.

### Prototype

```cpp
struct protoent *getprotobyname(const char *name);
```

### Exemple

```cpp
struct protoent *proto = getprotobyname("tcp");
```

Retourne une structure contenant notamment le numéro du protocole.

### À retenir

```text
"tcp"
  ↓
getprotobyname()
  ↓
informations sur TCP
```

> Fonction ancienne. Elle est rarement nécessaire dans un serveur moderne utilisant `socket(..., SOCK_STREAM, 0)`.

---

## `gethostbyname()`

### Rôle

Traduit un nom d'hôte en adresse IPv4.

### Prototype

```cpp
struct hostent *gethostbyname(const char *name);
```

### Exemple

```cpp
struct hostent *host = gethostbyname("localhost");
```

Conceptuellement :

```text
"localhost"
     ↓
gethostbyname()
     ↓
127.0.0.1
```

> API historique. Pour du nouveau code, préférer `getaddrinfo()`.

---

## `getaddrinfo()`

### Rôle

Traduit un nom d'hôte et un service en informations réseau utilisables.

### Prototype

```cpp
int getaddrinfo(
    const char *node,
    const char *service,
    const struct addrinfo *hints,
    struct addrinfo **result
);
```

### Exemple

```cpp
struct addrinfo hints;
struct addrinfo *result;

memset(&hints, 0, sizeof(hints));

hints.ai_family = AF_INET;
hints.ai_socktype = SOCK_STREAM;

int ret = getaddrinfo(
    "localhost",
    "6667",
    &hints,
    &result
);
```

### Résultat

`result` contient notamment :

```cpp
result->ai_family
result->ai_socktype
result->ai_protocol
result->ai_addr
result->ai_addrlen
```

### Schéma

```text
"localhost" + "6667"
        │
        ▼
  getaddrinfo()
        │
        ▼
┌─────────────────────┐
│ IP                  │
│ Port                │
│ Protocol            │
│ Socket type         │
│ sockaddr             │
└─────────────────────┘
```

---

## `freeaddrinfo()`

### Rôle

Libère les données créées par `getaddrinfo()`.

### Prototype

```cpp
void freeaddrinfo(struct addrinfo *res);
```

### Exemple

```cpp
freeaddrinfo(result);
```

Toujours faire :

```cpp
getaddrinfo(...)
    ↓
utilisation
    ↓
freeaddrinfo()
```

---

# 🔌 3. Connexion TCP

# `bind()`

### Rôle

Associe un socket à une **IP + port local**.

### Prototype

```cpp
int bind(
    int sockfd,
    const struct sockaddr *addr,
    socklen_t addrlen
);
```

### Exemple

```cpp
struct sockaddr_in addr;

addr.sin_family = AF_INET;
addr.sin_port = htons(6667);
addr.sin_addr.s_addr = INADDR_ANY;

bind(
    serverFd,
    (struct sockaddr *)&addr,
    sizeof(addr)
);
```

### Schéma

```text
socket()
   │
   ▼
bind()
   │
   ▼
127.0.0.1:6667
```

### À retenir

```text
bind = "je réserve cette adresse/ce port pour mon socket"
```

---

# `connect()`

### Rôle

Connecte un socket à un serveur distant.

### Prototype

```cpp
int connect(
    int sockfd,
    const struct sockaddr *addr,
    socklen_t addrlen
);
```

### Exemple

```cpp
connect(
    clientFd,
    (struct sockaddr *)&server,
    sizeof(server)
);
```

### Schéma

```text
CLIENT
  │
  │ connect()
  ▼
SERVER
```

`connect()` est principalement utilisé côté **client**.

---

# `listen()`

### Rôle

Met un socket serveur en mode écoute.

### Prototype

```cpp
int listen(int sockfd, int backlog);
```

### Exemple

```cpp
listen(serverFd, 10);
```

### Schéma

```text
socket()
   ↓
bind()
   ↓
listen()
   ↓
"J'attends des connexions"
```

### `backlog`

Nombre de connexions pouvant être placées dans la file d'attente en attente d'acceptation.

---

# `accept()`

### Rôle

Accepte une connexion entrante.

### Prototype

```cpp
int accept(
    int sockfd,
    struct sockaddr *addr,
    socklen_t *addrlen
);
```

### Exemple

```cpp
struct sockaddr_in clientAddr;
socklen_t clientLen = sizeof(clientAddr);

int clientFd = accept(
    serverFd,
    (struct sockaddr *)&clientAddr,
    &clientLen
);
```

### ⚠️ Point essentiel

`accept()` crée un **nouveau socket**.

```text
serverFd = 3
     │
     │ accept()
     ▼
clientFd = 4
```

Le serveur conserve :

```text
fd 3 → écouter les nouveaux clients
fd 4 → communiquer avec client 1
fd 5 → communiquer avec client 2
fd 6 → communiquer avec client 3
```

---

# 🔢 4. Conversion des adresses et ports

Les ordinateurs et les réseaux peuvent utiliser des ordres de bytes différents.

On distingue :

```text
HOST BYTE ORDER
NETWORK BYTE ORDER
```

---

# `htons()`

### Host TO Network Short

Convertit un entier 16 bits vers l'ordre réseau.

```cpp
uint16_t htons(uint16_t hostshort);
```

### Principal usage

Les ports.

```cpp
addr.sin_port = htons(6667);
```

```text
6667
 │
 ▼
htons()
 │
 ▼
Network Byte Order
```

---

# `htonl()`

### Host TO Network Long

Convertit un entier 32 bits vers l'ordre réseau.

```cpp
uint32_t htonl(uint32_t hostlong);
```

Exemple :

```cpp
addr.sin_addr.s_addr = htonl(INADDR_ANY);
```

---

# `ntohs()`

### Network TO Host Short

Inverse de `htons()`.

```cpp
uint16_t ntohs(uint16_t netshort);
```

Exemple :

```cpp
int port = ntohs(addr.sin_port);
```

```text
Network
   ↓
ntohs()
   ↓
Host
```

---

# `ntohl()`

### Network TO Host Long

Inverse de `htonl()`.

```cpp
uint32_t ntohl(uint32_t netlong);
```

---

# `inet_addr()`

### Rôle

Convertit une IPv4 sous forme texte vers une valeur réseau.

```cpp
in_addr_t inet_addr(const char *cp);
```

### Exemple

```cpp
addr.sin_addr.s_addr = inet_addr("127.0.0.1");
```

```text
"127.0.0.1"
     ↓
inet_addr()
     ↓
IPv4 binaire
```

> API historique. Préférer `inet_pton()` dans du nouveau code.

---

# `inet_ntoa()`

### Rôle

Convertit une adresse IPv4 en texte.

```cpp
char *inet_ntoa(struct in_addr in);
```

### Exemple

```cpp
std::cout << inet_ntoa(addr.sin_addr);
```

Résultat :

```text
127.0.0.1
```

> Fonction historique utilisant un buffer statique.

---

# `inet_ntop()`

### Rôle

Convertit une adresse réseau vers une représentation texte.

### Prototype

```cpp
const char *inet_ntop(
    int af,
    const void *src,
    char *dst,
    socklen_t size
);
```

### Exemple

```cpp
char ip[INET_ADDRSTRLEN];

inet_ntop(
    AF_INET,
    &addr.sin_addr,
    ip,
    sizeof(ip)
);

std::cout << ip;
```

Résultat :

```text
127.0.0.1
```

### À retenir

```text
inet_addr()
    texte → réseau

inet_ntoa()
    réseau → texte

inet_ntop()
    réseau → texte
```

---

# 📨 5. Communication

# `send()`

### Rôle

Envoie des données via un socket.

### Prototype

```cpp
ssize_t send(
    int sockfd,
    const void *buf,
    size_t len,
    int flags
);
```

### Exemple

```cpp
std::string msg = "PING :hello\r\n";

send(
    clientFd,
    msg.c_str(),
    msg.size(),
    0
);
```

### Schéma

```text
SERVEUR
   │
   │ send()
   ▼
CLIENT
```

### ⚠️ Attention

`send()` peut envoyer **moins de données que demandé**.

```text
Tu demandes : 1000 octets

send()
   ↓
600 octets envoyés
```

Un serveur robuste doit donc gérer les envois partiels.

---

# `recv()`

### Rôle

Reçoit des données depuis un socket.

### Prototype

```cpp
ssize_t recv(
    int sockfd,
    void *buf,
    size_t len,
    int flags
);
```

### Exemple

```cpp
char buffer[512];

ssize_t ret = recv(
    clientFd,
    buffer,
    sizeof(buffer),
    0
);
```

### ⚠️ Valeur de retour

```text
ret > 0
    données reçues

ret == 0
    connexion fermée proprement

ret < 0
    erreur
```

Exemple :

```cpp
if (ret > 0)
{
    // traiter les données
}
else if (ret == 0)
{
    // client déconnecté
}
else
{
    // erreur
}
```

---

# 🚦 6. Signaux

Les signaux permettent au système de prévenir ton programme qu'un événement particulier est arrivé.

Exemple :

```text
CTRL+C
   ↓
SIGINT
   ↓
serveur
```

---

# `signal()`

### Rôle

Associe un signal à une fonction.

### Prototype

```cpp
void (*signal(int signum, void (*handler)(int)))(int);
```

### Exemple

```cpp
void handler(int sig)
{
    (void)sig;
    // arrêt du serveur
}

signal(SIGINT, handler);
```

---

# `sigaction()`

### Rôle

Permet de configurer plus précisément le comportement d'un signal.

### Prototype

```cpp
int sigaction(
    int signum,
    const struct sigaction *act,
    struct sigaction *oldact
);
```

### Exemple

```cpp
struct sigaction sa;

memset(&sa, 0, sizeof(sa));

sa.sa_handler = handler;

sigemptyset(&sa.sa_mask);

sigaction(SIGINT, &sa, NULL);
```

### Pourquoi préférer `sigaction()` ?

Parce qu'elle offre davantage de contrôle que `signal()`.

---

# `sigemptyset()`

Initialise un ensemble de signaux vide.

```cpp
int sigemptyset(sigset_t *set);
```

```cpp
sigset_t set;

sigemptyset(&set);
```

Résultat :

```text
{}
```

---

# `sigfillset()`

Ajoute tous les signaux à un ensemble.

```cpp
int sigfillset(sigset_t *set);
```

Conceptuellement :

```text
{
    SIGINT,
    SIGTERM,
    ...
}
```

---

# `sigaddset()`

Ajoute un signal.

```cpp
int sigaddset(
    sigset_t *set,
    int signum
);
```

Exemple :

```cpp
sigemptyset(&set);
sigaddset(&set, SIGINT);
```

Résultat :

```text
{ SIGINT }
```

---

# `sigdelset()`

Supprime un signal.

```cpp
int sigdelset(
    sigset_t *set,
    int signum
);
```

```cpp
sigdelset(&set, SIGINT);
```

---

# `sigismember()`

Teste si un signal est présent.

```cpp
int sigismember(
    const sigset_t *set,
    int signum
);
```

Exemple :

```cpp
if (sigismember(&set, SIGINT))
{
    // SIGINT présent
}
```

Retour :

```text
1  → présent
0  → absent
-1 → erreur
```

---

# 📁 7. Fichiers et file descriptors

Les sockets sont eux-mêmes des **file descriptors**.

C'est pourquoi des fonctions de gestion de fichiers peuvent aussi fonctionner sur eux.

---

# `lseek()`

### Rôle

Déplace la position de lecture/écriture d'un fichier.

### Prototype

```cpp
off_t lseek(
    int fd,
    off_t offset,
    int whence
);
```

### `whence`

```text
SEEK_SET
    depuis le début

SEEK_CUR
    depuis la position actuelle

SEEK_END
    depuis la fin
```

### Exemple

```cpp
lseek(fd, 0, SEEK_SET);
```

Retourne au début.

---

# `fstat()`

### Rôle

Récupère les informations d'un file descriptor.

### Prototype

```cpp
int fstat(
    int fd,
    struct stat *statbuf
);
```

### Exemple

```cpp
struct stat st;

fstat(fd, &st);

std::cout << st.st_size;
```

Informations disponibles notamment :

```text
st_size
st_mode
st_uid
st_gid
st_mtime
```

---

# `fcntl()`

### Rôle

Permet de manipuler un file descriptor.

### Prototype

```cpp
int fcntl(
    int fd,
    int cmd,
    ...
);
```

Il possède énormément d'utilisations.

Pour un serveur réseau, la plus importante est :

```cpp
fcntl(
    fd,
    F_SETFL,
    O_NONBLOCK
);
```

Cela met le socket en **non-bloquant**.

### Bloquant

```text
recv()
  │
  ▼
attend des données
  │
  ▼
PROGRAMME BLOQUÉ
```

### Non-bloquant

```text
recv()
  │
  ├── données disponibles → lecture
  │
  └── rien → retour immédiat
```

Avec `poll()`, c'est particulièrement utile.

---

# ⏳ 8. Multiplexage avec `poll`

# `poll()`

### Rôle

Surveille plusieurs file descriptors simultanément.

### Prototype

```cpp
int poll(
    struct pollfd *fds,
    nfds_t nfds,
    int timeout
);
```

### Exemple

```cpp
struct pollfd fds[10];

fds[0].fd = serverFd;
fds[0].events = POLLIN;

poll(fds, 1, -1);
```

### Pourquoi ?

Sans `poll()` :

```text
recv(client1)
     ↓
BLOQUÉ
     ↓
client2 ignoré
     ↓
client3 ignoré
```

Avec `poll()` :

```text
                 ┌── client 1
                 │
poll() ──────────┼── client 2 → données !
                 │
                 └── client 3

                         ↓

                    recv(client2)
```

Un seul thread peut donc surveiller plusieurs clients.

---

# `struct pollfd`

Structure utilisée avec `poll()` :

```cpp
struct pollfd
{
    int   fd;
    short events;
    short revents;
};
```

### `fd`

Le file descriptor à surveiller.

### `events`

Ce que tu souhaites surveiller.

Exemple :

```cpp
fds[i].events = POLLIN;
```

### `revents`

Ce qui s'est réellement produit.

Exemple :

```cpp
if (fds[i].revents & POLLIN)
{
    // données disponibles
}
```

---

# 🚀 9. Architecture d'un serveur IRC

Voici comment les fonctions s'assemblent réellement.

```text
                    ┌──────────────┐
                    │   socket()   │
                    └──────┬───────┘
                           │
                           ▼
                    ┌──────────────┐
                    │ setsockopt() │
                    └──────┬───────┘
                           │
                           ▼
                    ┌──────────────┐
                    │    bind()    │
                    └──────┬───────┘
                           │
                           ▼
                    ┌──────────────┐
                    │   listen()   │
                    └──────┬───────┘
                           │
                           ▼
                    ┌──────────────┐
                    │    poll()    │
                    └──────┬───────┘
                           │
             ┌─────────────┴─────────────┐
             │                           │
             ▼                           ▼
    nouvelle connexion             client parle
             │                           │
             ▼                           ▼
         accept()                      recv()
             │                           │
             ▼                           ▼
       nouveau fd                  parser IRC
                                         │
                                         ▼
                                       send()
```

---

# 🔄 Cycle complet

## Démarrage du serveur

```cpp
int serverFd = socket(AF_INET, SOCK_STREAM, 0);

setsockopt(serverFd, ...);

bind(serverFd, ...);

listen(serverFd, SOMAXCONN);
```

Puis :

```cpp
poll(...);
```

---

## Nouveau client

```text
poll()
  │
  │ POLLIN sur serverFd
  ▼
accept()
  │
  ▼
nouveau clientFd
  │
  ▼
ajout au tableau/vector de pollfd
```

---

## Message client

```text
poll()
   │
   │ POLLIN sur clientFd
   ▼
recv()
   │
   ▼
buffer
   │
   ▼
parser IRC
   │
   ▼
traitement de la commande
   │
   ▼
send()
```

---

## Déconnexion

```text
poll()
   │
   ▼
recv()
   │
   ▼
ret == 0
   │
   ▼
close(clientFd)
   │
   ▼
retirer le client de poll()
```

---

# 🧠 10. Tableau de révision

| Fonction | À quoi ça sert | Importance IRC |
|---|---|---:|
| `socket()` | Créer un socket | 🔴🔴🔴 |
| `close()` | Fermer un fd | 🔴🔴🔴 |
| `setsockopt()` | Configurer un socket | 🔴🔴 |
| `getsockname()` | Récupérer son adresse | 🟡 |
| `getprotobyname()` | Infos sur un protocole | 🟢 |
| `gethostbyname()` | Nom → IPv4 | 🟡 |
| `getaddrinfo()` | Résolution réseau | 🟠 |
| `freeaddrinfo()` | Libérer `getaddrinfo()` | 🟠 |
| `bind()` | Associer IP + port | 🔴🔴🔴 |
| `connect()` | Se connecter | 🔴🔴 |
| `listen()` | Écouter | 🔴🔴🔴 |
| `accept()` | Accepter un client | 🔴🔴🔴 |
| `htons()` | Host → Network 16 bits | 🔴🔴 |
| `htonl()` | Host → Network 32 bits | 🟠 |
| `ntohs()` | Network → Host 16 bits | 🟠 |
| `ntohl()` | Network → Host 32 bits | 🟢 |
| `inet_addr()` | Texte → IPv4 | 🟡 |
| `inet_ntoa()` | IPv4 → texte | 🟡 |
| `inet_ntop()` | Adresse → texte | 🟠 |
| `send()` | Envoyer | 🔴🔴🔴 |
| `recv()` | Recevoir | 🔴🔴🔴 |
| `signal()` | Gérer un signal | 🟠 |
| `sigaction()` | Gérer un signal précisément | 🟠 |
| `sigemptyset()` | Set de signaux vide | 🟡 |
| `sigfillset()` | Tous les signaux | 🟢 |
| `sigaddset()` | Ajouter un signal | 🟡 |
| `sigdelset()` | Retirer un signal | 🟡 |
| `sigismember()` | Tester un signal | 🟡 |
| `lseek()` | Déplacer position fichier | 🟢 |
| `fstat()` | Infos fichier/fd | 🟢 |
| `fcntl()` | Manipuler fd | 🔴🔴 |
| `poll()` | Surveiller plusieurs fd | 🔴🔴🔴 |

---

# 🧩 La chaîne à connaître par cœur

Pour un **serveur TCP** :

```text
socket()
   ↓
setsockopt()
   ↓
bind()
   ↓
listen()
   ↓
poll()
   ↓
accept()
   ↓
poll()
   ↓
recv()
   ↓
traitement
   ↓
send()
```

Pour un **client TCP** :

```text
socket()
   ↓
connect()
   ↓
send() / recv()
   ↓
close()
```

Pour les **adresses** :

```text
getaddrinfo()
      ↓
sockaddr
      ↓
htons()
```

Pour afficher une IP :

```text
sockaddr_in
     ↓
inet_ntop()
     ↓
"192.168.1.42"
```

Pour gérer l'arrêt du serveur :

```text
CTRL+C
   ↓
SIGINT
   ↓
sigaction()
   ↓
flag d'arrêt
   ↓
fermeture des sockets
```

---

# 🧠 La phrase qui résume tout

> **`socket()` crée le canal, `bind()` lui donne une adresse, `listen()` attend les clients, `accept()` crée leur socket, `poll()` surveille tout le monde, `recv()` reçoit, `send()` répond et `close()` ferme.**

