# `socket()` — `domain`

Le premier paramètre de `socket()` indique la **famille d'adresses** utilisée par le socket.

```cpp
int socket(int domain, int type, int protocol);
             ^^^^^^
```

Le `domain` détermine **comment une adresse sera représentée et quel espace d'adresses sera utilisé**.

---

# Sommaire

* [`AF_INET` — IPv4](#af_inet--ipv4)

  * [Adresse IPv4](#adresse-ipv4)
  * [Structure `sockaddr_in`](#structure-sockaddr_in)
  * [Exemple](#exemple)
* [`AF_INET6` — IPv6](#af_inet6--ipv6)

  * [Adresse IPv6](#adresse-ipv6)
  * [Structure `sockaddr_in6`](#structure-sockaddr_in6)
  * [Exemple](#exemple-1)
* [`AF_UNIX` / `AF_LOCAL` — Communication locale](#af_unix--af_local--communication-locale)

  * [Adresse Unix](#adresse-unix)
  * [Structure `sockaddr_un`](#structure-sockaddr_un)
  * [Exemple](#exemple-2)
* [`AF_UNSPEC` — Famille non spécifiée](#af_unspec--famille-non-spécifiée)
* [Autres familles](#autres-familles)

  * [`AF_PACKET`](#af_packet)
  * [`AF_NETLINK`](#af_netlink)
  * [`AF_CAN`](#af_can)
  * [`AF_BLUETOOTH`](#af_bluetooth)
* [Peut-on mettre plusieurs `domain` ?](#peut-on-mettre-plusieurs-domain)
* [Comparaison](#comparaison)
* [Combinaisons avec `type`](#combinaisons-avec-type)
* [Quel `domain` pour Ft_IRC ?](#quel-domain-pour-ft_irc)
* [À retenir](#à-retenir)

---

# `AF_INET` — IPv4

```cpp
socket(AF_INET, type, protocol);
```

`AF_INET` indique que le socket utilise **IPv4**.

IPv4 utilise des adresses de **32 bits**.

Exemples :

```text
127.0.0.1
192.168.1.10
8.8.8.8
```

---

## Adresse IPv4

Une adresse IPv4 est généralement représentée sous cette forme :

```text
192.168.1.10
```

Elle contient quatre nombres séparés par des points :

```text
192 . 168 . 1 . 10
```

Chaque nombre représente un octet.

```text
192      168      1       10
 ↓        ↓       ↓        ↓
8 bits   8 bits  8 bits   8 bits

          = 32 bits
```

---

## Structure `sockaddr_in`

Avec `AF_INET`, on utilise généralement :

```cpp
struct sockaddr_in;
```

Elle est définie notamment dans :

```cpp
#include <netinet/in.h>
```

Structure simplifiée :

```cpp
struct sockaddr_in
{
    sa_family_t    sin_family;
    in_port_t      sin_port;
    struct in_addr sin_addr;
};
```

Les éléments importants sont :

```text
sin_family
    ↓
AF_INET

sin_port
    ↓
Port

sin_addr
    ↓
Adresse IPv4
```

---

## Exemple

```cpp
int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
```

Ici :

```text
AF_INET
    ↓
IPv4

SOCK_STREAM
    ↓
Flux

0
    ↓
Protocole par défaut
    ↓
TCP
```

On obtient donc :

```text
IPv4 + TCP
```

<br>
<br>
<br>
<br>
<br>
<br>

---

# `AF_INET6` — IPv6

```cpp
socket(AF_INET6, type, protocol);
```

`AF_INET6` indique que le socket utilise **IPv6**.

IPv6 utilise des adresses de **128 bits**.

Exemples :

```text
::1
2001:db8::1
fe80::1
```

---

## Adresse IPv6

Une adresse IPv6 est beaucoup plus grande qu'une adresse IPv4.

Exemple :

```text
2001:db8:85a3:0000:0000:8a2e:0370:7334
```

Elle peut être raccourcie :

```text
2001:db8:85a3::8a2e:370:7334
```

L'adresse locale de la machine est souvent :

```text
::1
```

C'est l'équivalent IPv6 de :

```text
127.0.0.1
```

en IPv4.

---

## Structure `sockaddr_in6`

Avec `AF_INET6`, on utilise généralement :

```cpp
struct sockaddr_in6;
```

Elle est définie notamment dans :

```cpp
#include <netinet/in.h>
```

Structure simplifiée :

```cpp
struct sockaddr_in6
{
    sa_family_t     sin6_family;
    in_port_t       sin6_port;
    uint32_t        sin6_flowinfo;
    struct in6_addr sin6_addr;
    uint32_t        sin6_scope_id;
};
```

Les éléments principaux sont :

```text
sin6_family
    ↓
AF_INET6

sin6_port
    ↓
Port

sin6_addr
    ↓
Adresse IPv6
```

---

## Exemple

```cpp
int serverSocket = socket(AF_INET6, SOCK_STREAM, 0);
```

Cela signifie :

```text
AF_INET6
    ↓
IPv6

SOCK_STREAM
    ↓
Flux

0
    ↓
TCP
```

Donc :

```text
IPv6 + TCP
```
<br>
<br>
<br>
<br>
<br>
<br>
<br>
<br>

---

# `AF_UNIX` / `AF_LOCAL` — Communication locale

```cpp
socket(AF_UNIX, type, protocol);
```

`AF_UNIX` permet à plusieurs processus de communiquer **sur la même machine**.

Il n'utilise pas d'adresse IP.

Il ne sert donc pas à communiquer avec :

```text
192.168.1.20
8.8.8.8
2001:db8::1
```

---

## Adresse Unix

Une adresse Unix peut par exemple être :

```text
/tmp/my_socket
```

On parle généralement de **Unix Domain Socket**.

Schéma :

```text
Programme A
     │
     │
     ▼
 /tmp/my_socket
     │
     │
     ▼
Programme B
```

La communication reste à l'intérieur de la machine.

---

## Structure `sockaddr_un`

Avec `AF_UNIX`, on utilise :

```cpp
struct sockaddr_un;
```

Structure simplifiée :

```cpp
struct sockaddr_un
{
    sa_family_t sun_family;
    char        sun_path[...];
};
```

Les éléments importants sont :

```text
sun_family
    ↓
AF_UNIX

sun_path
    ↓
Chemin du socket
```

---

## Exemple

```cpp
int serverSocket = socket(AF_UNIX, SOCK_STREAM, 0);
```

Cela crée un socket Unix orienté flux.

Il pourrait ensuite être associé à :

```text
/tmp/my_socket
```
<br>
<br>
<br>
<br>
<br>
<br>
<br>

---

# `AF_UNSPEC` — Famille non spécifiée

```cpp
AF_UNSPEC
```

`AF_UNSPEC` signifie :

> aucune famille d'adresses spécifiée.

Ce n'est généralement **pas une famille que tu utilises directement pour créer un socket concret**.

On la rencontre notamment avec des fonctions comme :

```cpp
getaddrinfo()
```

Exemple :

```cpp
struct addrinfo hints;

hints.ai_family = AF_UNSPEC;
```

Cela signifie :

```text
IPv4 accepté
+
IPv6 accepté
```

Le système peut alors retourner des résultats IPv4 et IPv6.

⚠️ Attention :

```cpp
socket(AF_UNSPEC, SOCK_STREAM, 0);
```

n'est pas la manière normale de créer un socket IPv4/IPv6.

---

# Autres familles

Il existe beaucoup d'autres familles d'adresses.

Elles dépendent notamment du système d'exploitation.

---

## `AF_PACKET`

Principalement utilisé sous Linux pour accéder aux paquets réseau à un niveau très bas.

```cpp
socket(AF_PACKET, SOCK_RAW, ...);
```

Cela permet notamment de travailler au niveau de la couche liaison.

On peut par exemple travailler avec :

```text
Ethernet
MAC
trames réseau
```

C'est beaucoup plus bas niveau que :

```cpp
AF_INET
```

---

## `AF_NETLINK`

Famille principalement utilisée sous Linux pour permettre la communication entre :

```text
applications
      ↕
noyau Linux
```

Exemple d'utilisation :

```text
configuration réseau
interfaces réseau
routage
informations système
```

---

## `AF_CAN`

Utilisé pour le protocole :

```text
CAN
```

Controller Area Network.

On le retrouve notamment dans :

```text
automobile
systèmes embarqués
électronique industrielle
```

---

## `AF_BLUETOOTH`

Utilisé pour certaines communications Bluetooth.

La disponibilité et les protocoles associés dépendent du système.

---

# Peut-on mettre plusieurs `domain` ?

**Non.**

`socket()` prend un seul `domain`.

```cpp
socket(domain, type, protocol);
       ↑
       un seul
```

Tu ne peux pas faire :

```cpp
socket(AF_INET | AF_INET6, SOCK_STREAM, 0);
```

❌ Ce n'est pas la bonne façon de demander IPv4 + IPv6.

---

## IPv4 + IPv6

Si tu veux explicitement créer deux sockets :

```cpp
int ipv4Socket = socket(AF_INET, SOCK_STREAM, 0);

int ipv6Socket = socket(AF_INET6, SOCK_STREAM, 0);
```

Tu as alors :

```text
              Serveur
              /     \
             /       \
         IPv4         IPv6
          │             │
     AF_INET         AF_INET6
          │             │
       Socket 1       Socket 2
```

C'est une approche courante pour un serveur qui veut écouter sur les deux familles.

---

# IPv4 + IPv6 avec `AF_INET6`

Il existe également le concept de **dual-stack**.

Un socket :

```cpp
socket(AF_INET6, SOCK_STREAM, 0);
```

peut, selon la configuration du système, accepter :

```text
IPv6
+
IPv4
```

Cela dépend notamment de l'option :

```cpp
IPV6_V6ONLY
```

Avec :

```cpp
int value = 1;

setsockopt(
    socketFd,
    IPPROTO_IPV6,
    IPV6_V6ONLY,
    &value,
    sizeof(value)
);
```

le socket est limité à IPv6.

Avec une configuration permettant le dual-stack, un socket IPv6 peut également accepter des connexions IPv4 représentées sous forme d'adresses IPv4-mappées IPv6.

⚠️ Le comportement par défaut peut varier selon les systèmes et leur configuration.

Pour un serveur portable et explicite, deux sockets restent souvent plus simples à comprendre.

---

# Comparaison

| `domain`       | Adresse            | Utilisation                                      |
| -------------- | ------------------ | ------------------------------------------------ |
| `AF_INET`      | IPv4               | Réseau IPv4                                      |
| `AF_INET6`     | IPv6               | Réseau IPv6                                      |
| `AF_UNIX`      | Chemin Unix        | Communication locale                             |
| `AF_LOCAL`     | Chemin Unix        | Alias de `AF_UNIX` sur les systèmes concernés    |
| `AF_UNSPEC`    | Aucune             | Famille non spécifiée, notamment `getaddrinfo()` |
| `AF_PACKET`    | Ethernet / liaison | Bas niveau Linux                                 |
| `AF_NETLINK`   | Noyau Linux        | Communication avec le kernel                     |
| `AF_CAN`       | CAN                | Automobile / embarqué                            |
| `AF_BLUETOOTH` | Bluetooth          | Communications Bluetooth                         |

---

# Combinaisons avec `type`

Le `domain` ne détermine **pas à lui seul** si le socket utilise TCP ou UDP.

Le `type` intervient également.

## IPv4 + TCP

```cpp
socket(AF_INET, SOCK_STREAM, 0);
```

```text
AF_INET
   ↓
IPv4

SOCK_STREAM
   ↓
Flux

0
   ↓
TCP
```

---

## IPv4 + UDP

```cpp
socket(AF_INET, SOCK_DGRAM, 0);
```

```text
AF_INET
   ↓
IPv4

SOCK_DGRAM
   ↓
Datagrammes

0
   ↓
UDP
```

---

## IPv6 + TCP

```cpp
socket(AF_INET6, SOCK_STREAM, 0);
```

---

## IPv6 + UDP

```cpp
socket(AF_INET6, SOCK_DGRAM, 0);
```

---

## Unix + TCP-like stream

```cpp
socket(AF_UNIX, SOCK_STREAM, 0);
```

Ici, il ne s'agit pas de TCP : `SOCK_STREAM` décrit le **type de communication**, mais le transport est local via Unix Domain Sockets.

---

# Quel `domain` pour Ft_IRC ?

Pour un serveur IRC classique, tu vas principalement rencontrer :

```cpp
AF_INET
```

pour IPv4.

Par exemple :

```cpp
int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
```

Cela signifie :

```text
AF_INET
    ↓
IPv4

SOCK_STREAM
    ↓
Flux

0
    ↓
TCP
```

Donc :

```text
IRC
 ↓
TCP
 ↓
IPv4
 ↓
AF_INET
```

Si ton serveur IRC doit également accepter IPv6 :

```cpp
int ipv4Socket = socket(AF_INET, SOCK_STREAM, 0);
int ipv6Socket = socket(AF_INET6, SOCK_STREAM, 0);
```

---

# À retenir

Le paramètre :

```cpp
domain
```

répond à la question :

> **« Quelle famille d'adresses mon socket utilise-t-il ? »**

Les trois familles les plus importantes à connaître sont :

```text
AF_INET
   ↓
IPv4


AF_INET6
   ↓
IPv6


AF_UNIX
   ↓
Communication locale
```

Et surtout :

```cpp
socket(AF_INET, SOCK_STREAM, 0);
```

se lit :

```text
AF_INET
└── Quelle famille ?
    └── IPv4

SOCK_STREAM
└── Quel type ?
    └── Flux

0
└── Quel protocole ?
    └── Par défaut → TCP
```

**Un appel à `socket()` choisit une seule famille (`domain`).**
Pour gérer plusieurs familles, comme IPv4 et IPv6, on crée généralement plusieurs sockets ou on utilise une configuration dual-stack adaptée.
