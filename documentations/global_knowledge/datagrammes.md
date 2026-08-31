# Datagrammes — `SOCK_DGRAM` / UDP

## Sommaire

* [1. Qu'est-ce qu'un datagramme ?](#1-quest-ce-quun-datagramme)
* [2. `SOCK_DGRAM`](#2-sock_dgram)
* [3. UDP](#3-udp)
* [4. Un datagramme est un message](#4-un-datagramme-est-un-message)
* [5. Datagramme vs flux](#5-datagramme-vs-flux)
* [6. Fonctionnement](#6-fonctionnement)
* [7. Exemple avec `sendto()`](#7-exemple-avec-sendto)
* [8. Exemple avec `recvfrom()`](#8-exemple-avec-recvfrom)
* [9. Adresse contenue dans un datagramme](#9-adresse-contenue-dans-un-datagramme)
* [10. Datagrammes IPv4](#10-datagrammes-ipv4)
* [11. Datagrammes IPv6](#11-datagrammes-ipv6)
* [12. Avantages](#12-avantages)
* [13. Inconvénients](#13-inconvénients)
* [14. Exemple concret](#14-exemple-concret)
* [15. À retenir](#15-à-retenir)

---

# 1. Qu'est-ce qu'un datagramme ?

Un **datagramme** est une unité de données envoyée indépendamment des autres.

Contrairement à un flux TCP, les données sont envoyées sous forme de **messages séparés**.

Par exemple :

```text
Datagramme 1
┌──────────────┐
│    Bonjour   │
└──────────────┘

Datagramme 2
┌──────────────┐
│     Salut    │
└──────────────┘

Datagramme 3
┌──────────────┐
│     Hello    │
└──────────────┘
```

Chaque datagramme possède sa propre frontière.

---

# 2. `SOCK_DGRAM`

Lorsque l'on crée un socket avec :

```cpp
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
```

on demande un socket de type :

```text
SOCK_DGRAM
```

`DGRAM` signifie :

```text
DATAGRAM
```

Le protocole généralement associé est :

```text
SOCK_DGRAM → UDP
```

On obtient donc :

```text
AF_INET
    │
    └── IPv4

SOCK_DGRAM
    │
    └── Datagrammes

0
    │
    └── UDP choisi automatiquement
```

Pour IPv6 :

```cpp
socket(AF_INET6, SOCK_DGRAM, 0);
```

on obtient :

```text
IPv6 + UDP
```

---

# 3. UDP

UDP signifie :

```text
User Datagram Protocol
```

UDP est un protocole **sans connexion**.

Cela signifie qu'avant d'envoyer des données, il n'est pas nécessaire d'établir une connexion avec le destinataire.

Avec TCP :

```text
CLIENT
  │
  │── SYN ───────────────►
  │◄─ SYN-ACK ────────────
  │── ACK ───────────────►
  │
  │      CONNEXION
  │
  └───────────────────────
```

Avec UDP :

```text
CLIENT
  │
  │── Datagramme ─────────► SERVEUR
  │
  │── Datagramme ─────────► SERVEUR
  │
  │── Datagramme ─────────► SERVEUR
```

Il n'y a pas de handshake TCP.

---

# 4. Un datagramme est un message

C'est la différence fondamentale avec `SOCK_STREAM`.

Si on fait :

```cpp
sendto(sockfd, "ABC", 3, ...);
sendto(sockfd, "DEF", 3, ...);
```

on envoie deux datagrammes :

```text
Datagramme 1
┌─────┐
│ ABC │
└─────┘

Datagramme 2
┌─────┐
│ DEF │
└─────┘
```

Le destinataire peut recevoir :

```cpp
recvfrom(...);
```

puis :

```text
ABC
```

et ensuite :

```text
DEF
```

Les frontières entre les deux datagrammes sont conservées.

---

# 5. Datagramme vs flux

## TCP — `SOCK_STREAM`

TCP fournit un flux continu :

```text
send("ABC")
send("DEF")

            ↓

       ABCDEF
```

Le destinataire pourrait recevoir :

```text
AB
CDEF
```

Les frontières des `send()` sont perdues.

---

## UDP — `SOCK_DGRAM`

UDP conserve les datagrammes :

```text
sendto("ABC")
sendto("DEF")

            ↓

┌─────┐
│ ABC │
└─────┘

┌─────┐
│ DEF │
└─────┘
```

Le destinataire reçoit donc deux unités distinctes.

---

## Tableau comparatif

|                         | TCP           | UDP          |
| ----------------------- | ------------- | ------------ |
| Socket                  | `SOCK_STREAM` | `SOCK_DGRAM` |
| Connexion               | Oui           | Non          |
| Données                 | Flux d'octets | Datagrammes  |
| Frontières des messages | Non           | Oui          |
| Ordre garanti           | Oui           | Non          |
| Livraison garantie      | Oui*          | Non          |
| Retransmission          | Oui           | Non          |
| Handshake               | Oui           | Non          |
| Protocole               | TCP           | UDP          |

`*` TCP garantit une livraison fiable des octets dans une connexion établie, sous réserve que la connexion puisse effectivement aboutir ; il ne garantit pas qu'un correspondant reste joignable indéfiniment.

---

# 6. Fonctionnement

Un programme peut envoyer un datagramme directement à une adresse :

```text
CLIENT
  │
  │
  │ Datagramme
  │
  ▼
┌─────────────────────────────┐
│ UDP                         │
│                             │
│ Destination : 192.168.1.20 │
│ Port        : 4242          │
│                             │
│ Données : "Hello"           │
└─────────────────────────────┘
  │
  ▼
RÉSEAU
  │
  ▼
SERVEUR
```

Contrairement à TCP, le socket UDP n'a pas besoin d'être connecté au serveur pour effectuer un `sendto()`.

---

# 7. Exemple avec `sendto()`

La fonction principale pour envoyer un datagramme est :

```cpp
ssize_t sendto(
    int sockfd,
    const void *buf,
    size_t len,
    int flags,
    const struct sockaddr *dest_addr,
    socklen_t addrlen
);
```

Exemple IPv4 :

```cpp
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

struct sockaddr_in server;

memset(&server, 0, sizeof(server));

server.sin_family = AF_INET;
server.sin_port = htons(4242);
server.sin_addr.s_addr = inet_addr("127.0.0.1");

const char *message = "Hello";

sendto(
    sockfd,
    message,
    5,
    0,
    (struct sockaddr *)&server,
    sizeof(server)
);
```

Le programme envoie :

```text
┌─────────────┐
│    Hello    │
└─────────────┘
      │
      ▼
127.0.0.1:4242
```

---

# 8. Exemple avec `recvfrom()`

Pour recevoir un datagramme, on utilise généralement :

```cpp
ssize_t recvfrom(
    int sockfd,
    void *buf,
    size_t len,
    int flags,
    struct sockaddr *src_addr,
    socklen_t *addrlen
);
```

Exemple :

```cpp
char buffer[1024];

struct sockaddr_in client;
socklen_t client_len = sizeof(client);

ssize_t received = recvfrom(
    sockfd,
    buffer,
    sizeof(buffer),
    0,
    (struct sockaddr *)&client,
    &client_len
);
```

Après réception :

```text
client
   │
   ├── IP
   │
   └── port

buffer
   │
   └── données reçues
```

Le `recvfrom()` permet donc de récupérer :

1. les données ;
2. l'adresse de l'expéditeur ;
3. le port de l'expéditeur.

---

# 9. Adresse contenue dans un datagramme

Avec UDP, le destinataire est indiqué lors de l'envoi.

Par exemple :

```cpp
sendto(
    sockfd,
    data,
    size,
    0,
    (struct sockaddr *)&server,
    sizeof(server)
);
```

La structure :

```cpp
struct sockaddr_in
```

peut contenir :

```text
┌─────────────────────────┐
│ Famille : AF_INET       │
│ IP      : 192.168.1.20  │
│ Port    : 4242          │
└─────────────────────────┘
```

Le datagramme est alors envoyé vers :

```text
192.168.1.20:4242
```

---

# 10. Datagrammes IPv4

Pour IPv4 :

```cpp
socket(AF_INET, SOCK_DGRAM, 0);
```

On utilise :

```cpp
struct sockaddr_in
```

Exemple :

```cpp
struct sockaddr_in addr;

memset(&addr, 0, sizeof(addr));

addr.sin_family = AF_INET;
addr.sin_port = htons(4242);
addr.sin_addr.s_addr = inet_addr("192.168.1.20");
```

Schéma :

```text
UDP + IPv4
│
├── AF_INET
│
├── SOCK_DGRAM
│
└── sockaddr_in
      │
      ├── sin_family
      │      └── AF_INET
      │
      ├── sin_port
      │      └── 4242
      │
      └── sin_addr
             └── 192.168.1.20
```

---

# 11. Datagrammes IPv6

Pour IPv6 :

```cpp
socket(AF_INET6, SOCK_DGRAM, 0);
```

On utilise :

```cpp
struct sockaddr_in6
```

Exemple :

```cpp
struct sockaddr_in6 addr;

memset(&addr, 0, sizeof(addr));

addr.sin6_family = AF_INET6;
addr.sin6_port = htons(4242);

inet_pton(
    AF_INET6,
    "::1",
    &addr.sin6_addr
);
```

Schéma :

```text
UDP + IPv6
│
├── AF_INET6
│
├── SOCK_DGRAM
│
└── sockaddr_in6
      │
      ├── sin6_family
      │      └── AF_INET6
      │
      ├── sin6_port
      │      └── 4242
      │
      └── sin6_addr
             └── ::1
```

---

# 12. Avantages

UDP est intéressant lorsque l'on privilégie la simplicité et la faible latence.

### Pas de connexion

Pas besoin de handshake TCP.

```text
sendto()
   │
   ▼
datagramme
   │
   ▼
réseau
```

### Faible surcharge

UDP possède moins de mécanismes que TCP.

### Conservation des messages

Chaque `sendto()` produit un datagramme distinct.

### Rapidité

UDP peut être intéressant lorsque perdre certaines données est préférable à attendre une retransmission.

On retrouve UDP notamment dans :

* DNS ;
* certaines communications temps réel ;
* jeux en réseau ;
* VoIP ;
* streaming temps réel ;
* DHCP.

---

# 13. Inconvénients

UDP ne fournit pas les garanties de TCP.

Un datagramme peut :

```text
┌──────────────┐
│   Datagramme │
└──────┬───────┘
       │
       ▼
     RÉSEAU
       │
       ├──► reçu
       │
       ├──► perdu
       │
       ├──► retardé
       │
       └──► doublé
```

UDP ne garantit donc pas :

* la livraison ;
* l'ordre ;
* l'absence de doublons ;
* la retransmission.

Si l'application a besoin de ces garanties, elle doit les implémenter elle-même ou utiliser un protocole adapté.

---

# 14. Exemple concret

Imaginons un serveur UDP :

```text
Serveur
192.168.1.10:4242
```

Deux clients envoient des datagrammes.

```text
Client A
192.168.1.20:5000
       │
       │ "Hello"
       ▼
       │
       ├──────────────────► Serveur
       │                   192.168.1.10:4242
       │
Client B                   │
192.168.1.30:5001          │
       │                   │
       │ "Salut"           │
       └──────────────────►│
```

Le serveur utilise `recvfrom()` :

```cpp
recvfrom(
    sockfd,
    buffer,
    sizeof(buffer),
    0,
    (struct sockaddr *)&client,
    &client_len
);
```

Il peut alors savoir qui a envoyé le datagramme :

```text
Datagramme 1
┌──────────────────────────┐
│ Source : 192.168.1.20    │
│ Port   : 5000             │
│ Données: "Hello"          │
└──────────────────────────┘

Datagramme 2
┌──────────────────────────┐
│ Source : 192.168.1.30    │
│ Port   : 5001             │
│ Données: "Salut"          │
└──────────────────────────┘
```

Le même socket UDP peut donc recevoir des datagrammes provenant de plusieurs clients.

---

# 15. À retenir

```text
SOCK_DGRAM
    │
    ▼
 Datagrammes
    │
    ▼
 Généralement UDP
```

Un datagramme est une **unité de données indépendante** :

```text
┌───────────────┐
│   DONNÉES     │
└───────────────┘
       │
       ├── source
       ├── destination
       ├── port
       └── données
```

Contrairement à TCP :

```text
TCP
│
└── Flux continu d'octets
```

UDP :

```text
UDP
│
└── Datagrammes indépendants
```

### Résumé

| Concept                 | TCP           | UDP                                              |
| ----------------------- | ------------- | ------------------------------------------------ |
| `socket()`              | `SOCK_STREAM` | `SOCK_DGRAM`                                     |
| Modèle                  | Flux          | Datagrammes                                      |
| Connexion               | Oui           | Non                                              |
| `send()` / `recv()`     | Oui           | Oui, mais généralement `sendto()` / `recvfrom()` |
| Frontières des messages | ❌             | ✅                                                |
| Ordre                   | ✅             | ❌                                                |
| Livraison               | ✅             | ❌                                                |
| Retransmission          | ✅             | ❌                                                |
| Adresse du destinataire | Connexion     | Chaque datagramme peut avoir une destination     |

> **Un flux est une suite continue d'octets. Un datagramme est une unité de données indépendante.**
>
> **`SOCK_STREAM` → flux → TCP**
>
> **`SOCK_DGRAM` → datagrammes → UDP**
