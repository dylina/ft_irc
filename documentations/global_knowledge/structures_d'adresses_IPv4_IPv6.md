# Flux (`SOCK_STREAM`) et structures d'adresses IPv4 / IPv6

## Sommaire

* [1. `SOCK_STREAM`](#2-sock_stream)
* [2. `SOCK_STREAM` vs `SOCK_DGRAM`](#4-sock_stream-vs-sock_dgram)
* [3. IPv4 — `AF_INET`](#5-ipv4--af_inet)
* [4. `struct sockaddr_in`](#6-struct-sockaddr_in)

  * [`sin_family`](#sin_family)
  * [`sin_port`](#sin_port)
  * [`sin_addr`](#sin_addr)
  * [`sin_zero`](#sin_zero)
* [5. `struct in_addr`](#7-struct-in_addr)
* [6. IPv6 — `AF_INET6`](#8-ipv6--af_inet6)
* [7. `struct sockaddr_in6`](#9-struct-sockaddr_in6)

  * [`sin6_family`](#sin6_family)
  * [`sin6_port`](#sin6_port)
  * [`sin6_flowinfo`](#sin6_flowinfo)
  * [`sin6_addr`](#sin6_addr)
  * [`sin6_scope_id`](#sin6_scope_id)
* [8. `struct in6_addr`](#10-struct-in6_addr)
* [9. `struct sockaddr`](#11-struct-sockaddr)
* [10. IPv4 vs IPv6](#12-ipv4-vs-ipv6)
* [11. Exemple IPv4](#13-exemple-ipv4)
* [12. Exemple IPv6](#14-exemple-ipv6)
* [13. À retenir](#15-à-retenir)

---

# 1. Qu'est-ce qu'un flux ?

Un **flux** est une suite continue d'octets échangés entre deux points.

Avec un socket de type :

```cpp
SOCK_STREAM
```

les données sont considérées comme un **flux continu d'octets**.

On peut imaginer cela comme un tuyau :

```text
CLIENT                                      SERVEUR

ABCDEF
  │
  │       Flux d'octets
  ├───────────────────────────────────────►
  │
GHIJKL
```

Le protocole généralement utilisé avec `SOCK_STREAM` est **TCP**.

---

# 2. `SOCK_STREAM`

Lorsque l'on écrit :

```cpp
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
```

les paramètres signifient :

```text
AF_INET
    │
    └── Famille d'adresses IPv4

SOCK_STREAM
    │
    └── Socket orienté flux

0
    │
    └── Protocole choisi automatiquement
```

`SOCK_STREAM` indique donc que le socket doit fournir une communication :

* orientée connexion ;
* fiable ;
* ordonnée ;
* sous forme de flux d'octets.

Le protocole typique associé est :

```text
SOCK_STREAM → TCP
```

---

# 4. `SOCK_STREAM` vs `SOCK_DGRAM`

Les deux types les plus courants sont :

| Type          | Protocole typique | Données       |
| ------------- | ----------------- | ------------- |
| `SOCK_STREAM` | TCP               | Flux d'octets |
| `SOCK_DGRAM`  | UDP               | Datagrammes   |

### `SOCK_STREAM`

```text
┌─────────────────────────────────────┐
│ A B C D E F G H I J K L M N O ... │
└─────────────────────────────────────┘
              Flux continu
```

### `SOCK_DGRAM`

```text
┌───────────┐
│ A B C D   │
└───────────┘

┌───────────┐
│ E F G H   │
└───────────┘

┌───────────┐
│ I J K L   │
└───────────┘
```

Avec UDP, chaque datagramme possède sa propre frontière.

Avec TCP, les frontières des appels `send()` ne sont pas conservées.

---

# 5. IPv4 — `AF_INET`

`AF_INET` indique que le socket utilise la famille d'adresses **IPv4**.

Exemple :

```cpp
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
```

Cela signifie :

```text
IPv4 + TCP
```

Une adresse IPv4 ressemble à :

```text
127.0.0.1
```

ou :

```text
192.168.1.100
```

Une adresse IPv4 contient :

```text
32 bits = 4 octets
```

---

# 6. `struct sockaddr_in`

Pour représenter une adresse IPv4, on utilise :

```cpp
struct sockaddr_in
```

Sa structure contient principalement :

```text
sockaddr_in
│
├── sin_family
├── sin_port
├── sin_addr
└── sin_zero
```

Conceptuellement :

```cpp
struct sockaddr_in
{
    sa_family_t    sin_family;
    in_port_t      sin_port;
    struct in_addr sin_addr;
    unsigned char  sin_zero[8];
};
```

> La définition exacte peut légèrement varier selon le système.

---

## `sin_family`

Indique la famille d'adresses.

Pour IPv4 :

```cpp
addr.sin_family = AF_INET;
```

On obtient :

```text
sin_family
     │
     ▼
  AF_INET
     │
     ▼
   IPv4
```

---

## `sin_port`

Contient le numéro de port.

Exemple :

```cpp
addr.sin_port = htons(6667);
```

Le port est stocké en **network byte order**.

C'est pourquoi on utilise :

```cpp
htons()
```

qui signifie :

```text
Host TO Network Short
```

Exemple :

```cpp
htons(6667);
```

---

## `sin_addr`

Contient l'adresse IPv4.

```cpp
addr.sin_addr
```

est lui-même une structure :

```cpp
struct in_addr
```

On peut par exemple écrire :

```cpp
addr.sin_addr.s_addr = inet_addr("127.0.0.1");
```

---

## `sin_zero`

```cpp
unsigned char sin_zero[8];
```

Ce champ sert principalement au **padding / alignement** de la structure afin de conserver une compatibilité avec `struct sockaddr`.

Dans la programmation socket classique, on ne l'utilise généralement pas directement.

---

# 7. `struct in_addr`

`sin_addr` contient une structure :

```cpp
struct in_addr
{
    in_addr_t s_addr;
};
```

Schéma :

```text
sockaddr_in
    │
    └── sin_addr
          │
          ▼
       in_addr
          │
          └── s_addr
                │
                └── adresse IPv4
```

L'adresse IPv4 est représentée sur :

```text
32 bits
  │
  └── 4 octets
```

Exemple :

```text
127.0.0.1
```

---

# 8. IPv6 — `AF_INET6`

`AF_INET6` indique que le socket utilise la famille d'adresses **IPv6**.

Exemple :

```cpp
int sockfd = socket(AF_INET6, SOCK_STREAM, 0);
```

Cela signifie :

```text
IPv6 + TCP
```

Une adresse IPv6 peut ressembler à :

```text
2001:db8::1
```

IPv6 utilise :

```text
128 bits = 16 octets
```

contre :

```text
IPv4 → 32 bits = 4 octets
```

---

# 9. `struct sockaddr_in6`

Pour représenter une adresse IPv6, on utilise :

```cpp
struct sockaddr_in6
```

Conceptuellement :

```cpp
struct sockaddr_in6
{
    uint8_t         sin6_len;
    sa_family_t     sin6_family;
    in_port_t       sin6_port;
    uint32_t        sin6_flowinfo;
    struct in6_addr sin6_addr;
    uint32_t        sin6_scope_id;
};
```

> La définition exacte dépend du système. Par exemple, certains systèmes n'ont pas `sin6_len`.

Schéma :

```text
sockaddr_in6
│
├── sin6_family
├── sin6_port
├── sin6_flowinfo
├── sin6_addr
└── sin6_scope_id
```

---

## `sin6_family`

Indique la famille d'adresses :

```cpp
addr.sin6_family = AF_INET6;
```

Donc :

```text
AF_INET6
   │
   ▼
 IPv6
```

---

## `sin6_port`

Contient le port :

```cpp
addr.sin6_port = htons(6667);
```

Comme pour IPv4, le port doit être placé en **network byte order**.

---

## `sin6_flowinfo`

```cpp
sin6_flowinfo
```

Contient des informations relatives au **flux IPv6**.

Il est rarement manipulé directement dans les applications réseau classiques.

On le laisse généralement à :

```cpp
0
```

---

## `sin6_addr`

Contient l'adresse IPv6.

Il s'agit d'une structure :

```cpp
struct in6_addr
```

Une adresse IPv6 fait :

```text
128 bits
   │
   ▼
16 octets
```

Exemple :

```text
2001:db8::1
```

---

## `sin6_scope_id`

Permet notamment d'identifier une interface réseau pour certaines adresses IPv6, en particulier les adresses **link-local**.

Exemple :

```text
fe80::1234
```

Une adresse `fe80::/10` peut être présente sur plusieurs interfaces.

Le `scope_id` permet alors de préciser l'interface concernée.

---

# 10. `struct in6_addr`

La structure utilisée pour stocker une adresse IPv6 contient généralement :

```cpp
struct in6_addr
{
    unsigned char s6_addr[16];
};
```

Schéma :

```text
in6_addr
│
└── s6_addr[16]
      │
      ├── octet 0
      ├── octet 1
      ├── octet 2
      ├── ...
      └── octet 15
```

Soit :

```text
16 octets
×
8 bits
=
128 bits
```

---

# 11. `struct sockaddr`

Les fonctions réseau utilisent souvent une structure générique :

```cpp
struct sockaddr
```

Par exemple :

```cpp
bind(
    sockfd,
    (struct sockaddr *)&addr,
    sizeof(addr)
);
```

Mais `addr` peut être :

```cpp
struct sockaddr_in
```

ou :

```cpp
struct sockaddr_in6
```

On peut représenter cela comme :

```text
                    sockaddr
                       ▲
                       │
             ┌─────────┴─────────┐
             │                   │
      sockaddr_in          sockaddr_in6
             │                   │
           IPv4                IPv6
```

Le cast :

```cpp
(struct sockaddr *)&addr
```

permet de transmettre la structure spécialisée à une fonction qui attend un `struct sockaddr *`.

---

# 12. IPv4 vs IPv6

| Élément        | IPv4          | IPv6           |
| -------------- | ------------- | -------------- |
| Famille        | `AF_INET`     | `AF_INET6`     |
| Structure      | `sockaddr_in` | `sockaddr_in6` |
| Adresse        | `in_addr`     | `in6_addr`     |
| Taille adresse | 32 bits       | 128 bits       |
| Taille         | 4 octets      | 16 octets      |
| Port           | `sin_port`    | `sin6_port`    |
| Adresse        | `sin_addr`    | `sin6_addr`    |

Schéma :

```text
IPv4
│
├── AF_INET
│
└── sockaddr_in
      │
      ├── sin_family
      ├── sin_port
      └── sin_addr
            └── 32 bits
```

```text
IPv6
│
├── AF_INET6
│
└── sockaddr_in6
      │
      ├── sin6_family
      ├── sin6_port
      ├── sin6_flowinfo
      ├── sin6_addr
      │      └── 128 bits
      └── sin6_scope_id
```

---

# 13. Exemple IPv4

```cpp
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

int sockfd = socket(AF_INET, SOCK_STREAM, 0);

struct sockaddr_in addr;

std::memset(&addr, 0, sizeof(addr));

addr.sin_family = AF_INET;
addr.sin_port = htons(6667);
addr.sin_addr.s_addr = inet_addr("127.0.0.1");

bind(
    sockfd,
    (struct sockaddr *)&addr,
    sizeof(addr)
);
```

On peut visualiser la structure ainsi :

```text
sockfd
  │
  ▼
socket
  │
  ▼
bind()
  │
  ▼
sockaddr_in
  │
  ├── AF_INET
  ├── port 6667
  └── 127.0.0.1
```

---

# 14. Exemple IPv6

```cpp
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

int sockfd = socket(AF_INET6, SOCK_STREAM, 0);

struct sockaddr_in6 addr;

std::memset(&addr, 0, sizeof(addr));

addr.sin6_family = AF_INET6;
addr.sin6_port = htons(6667);

inet_pton(
    AF_INET6,
    "::1",
    &addr.sin6_addr
);

bind(
    sockfd,
    (struct sockaddr *)&addr,
    sizeof(addr)
);
```

Ici :

```text
::1
```

est l'équivalent IPv6 de :

```text
127.0.0.1
```

---

# 15. À retenir

Il faut séparer **la famille d'adresses**, **le type de socket** et **le protocole**.

```text
socket(domain, type, protocol)
       │       │       │
       │       │       └── Quel protocole ?
       │       │
       │       └────────── Comment communiquer ?
       │
       └────────────────── Quelle famille d'adresses ?
```

### Famille

```cpp
AF_INET
```

→ IPv4

```cpp
AF_INET6
```

→ IPv6

### Type

```cpp
SOCK_STREAM
```

→ flux d'octets, généralement TCP

```cpp
SOCK_DGRAM
```

→ datagrammes, généralement UDP

### Structures

```text
AF_INET
   │
   └── sockaddr_in
         └── in_addr
```

```text
AF_INET6
   │
   └── sockaddr_in6
         └── in6_addr
```

### Les quatre combinaisons classiques

```cpp
socket(AF_INET,  SOCK_STREAM, 0);  // IPv4 + TCP
socket(AF_INET6, SOCK_STREAM, 0);  // IPv6 + TCP

socket(AF_INET,  SOCK_DGRAM,  0);  // IPv4 + UDP
socket(AF_INET6, SOCK_DGRAM,  0);  // IPv6 + UDP
```

> **`AF_INET` / `AF_INET6` répondent à « quelle adresse ? »**
> **`SOCK_STREAM` / `SOCK_DGRAM` répondent à « comment transporter les données ? »**
> **`sockaddr_in` / `sockaddr_in6` permettent ensuite de décrire concrètement l'adresse et le port.**
