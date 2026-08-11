# `socket()` — `protocol`

Le troisième paramètre de `socket()` indique **le protocole utilisé par le socket**.

```cpp
int socket(int domain, int type, int protocol);
                                  ^^^^^^^^
```

Le paramètre `protocol` permet de préciser le protocole réseau à utiliser.

Dans la majorité des cas, on utilise :

```cpp
0
```

pour demander au système de choisir automatiquement le protocole correspondant au `domain` et au `type`.

---

## Sommaire

* [1. `protocol` — Protocole](#1-protocol--protocole)

  * [`0` — Protocole par défaut](#0--protocole-par-défaut)
  * [`IPPROTO_TCP` — TCP](#ipproto_tcp--tcp)
  * [`IPPROTO_UDP` — UDP](#ipproto_udp--udp)
  * [`IPPROTO_ICMP` — ICMP](#ipproto_icmp--icmp)
  * [`IPPROTO_IPV6` — IPv6](#ipproto_ipv6--ipv6)
* [2. Relation entre `domain`, `type` et `protocol`](#2-relation-entre-domain-type-et-protocol)
* [3. Combinaisons courantes](#3-combinaisons-courantes)

  * [TCP IPv4](#tcp-ipv4)
  * [UDP IPv4](#udp-ipv4)
  * [TCP IPv6](#tcp-ipv6)
  * [UDP IPv6](#udp-ipv6)
  * [ICMP](#icmp)
* [4. Comment choisir `protocol` ?](#4-comment-choisir-protocol)
* [5. Pour un serveur IRC](#5-pour-un-serveur-irc)
* [6. Résumé](#6-résumé)

---

# 1. `protocol` — Protocole

Le paramètre `protocol` indique **quel protocole doit être utilisé par le socket**.

```cpp
int socket(int domain, int type, int protocol);
```

Les trois paramètres travaillent ensemble :

```text
domain
  ↓
Quelle famille d'adresses ?

type
  ↓
Quel type de communication ?

protocol
  ↓
Quel protocole ?
```

Par exemple :

```cpp
socket(AF_INET, SOCK_STREAM, 0);
```

signifie :

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

---

# `0` — Protocole par défaut

La valeur :

```cpp
0
```

est la valeur la plus couramment utilisée.

Elle signifie :

> Laisse le système choisir le protocole correspondant au `domain` et au `type`.

---

## Avec `SOCK_STREAM`

```cpp
socket(AF_INET, SOCK_STREAM, 0);
```

Le système choisit généralement :

```text
AF_INET
   +
SOCK_STREAM
   +
0
   ↓
TCP
```

Donc :

```cpp
socket(AF_INET, SOCK_STREAM, 0);
```

est équivalent, dans ce cas, à :

```cpp
socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
```

---

## Avec `SOCK_DGRAM`

```cpp
socket(AF_INET, SOCK_DGRAM, 0);
```

Le système choisit généralement :

```text
AF_INET
   +
SOCK_DGRAM
   +
0
   ↓
UDP
```

Donc :

```cpp
socket(AF_INET, SOCK_DGRAM, 0);
```

correspond généralement à :

```cpp
socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
```

---

# `IPPROTO_TCP` — TCP

```cpp
IPPROTO_TCP
```

Indique explicitement que le protocole utilisé est **TCP**.

Exemple :

```cpp
socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
```

Cela signifie :

```text
AF_INET
    ↓
IPv4

SOCK_STREAM
    ↓
Flux

IPPROTO_TCP
    ↓
TCP
```

TCP fournit notamment :

* connexion
* fiabilité
* ordre des données
* retransmission
* contrôle de flux

C'est le protocole utilisé par un serveur IRC classique.

---

## `0` ou `IPPROTO_TCP` ?

Ces deux formes sont généralement équivalentes :

```cpp
socket(AF_INET, SOCK_STREAM, 0);
```

et :

```cpp
socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
```

Pour un serveur classique, `0` est généralement préférable car le système choisit automatiquement le protocole adapté.

---

# `IPPROTO_UDP` — UDP

```cpp
IPPROTO_UDP
```

Indique explicitement que le protocole utilisé est **UDP**.

Exemple :

```cpp
socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
```

Cela signifie :

```text
AF_INET
    ↓
IPv4

SOCK_DGRAM
    ↓
Datagrammes

IPPROTO_UDP
    ↓
UDP
```

UDP ne fournit pas les mêmes garanties que TCP :

```text
Pas de garantie de livraison
Pas de garantie d'ordre
Pas de retransmission automatique
Pas de connexion TCP
```

---

## `0` ou `IPPROTO_UDP` ?

Ces deux formes sont généralement équivalentes :

```cpp
socket(AF_INET, SOCK_DGRAM, 0);
```

et :

```cpp
socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
```

---

# `IPPROTO_ICMP` — ICMP

```cpp
IPPROTO_ICMP
```

Indique le protocole **ICMP**.

ICMP est notamment utilisé pour des fonctions de diagnostic et de contrôle réseau.

Un exemple célèbre est :

```text
ping
```

qui utilise ICMP dans son fonctionnement classique.

Un socket ICMP utilise généralement :

```cpp
socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
```

Ici :

```text
AF_INET
    ↓
IPv4

SOCK_RAW
    ↓
Accès brut

IPPROTO_ICMP
    ↓
ICMP
```

⚠️ Les sockets RAW nécessitent généralement des privilèges particuliers.

---

# `IPPROTO_IPV6` — IPv6

```cpp
IPPROTO_IPV6
```

Cette constante permet de référencer le protocole IPv6 dans certains contextes.

Elle peut notamment être utilisée avec des sockets RAW ou avec des opérations liées à IPv6.

Exemple conceptuel :

```cpp
socket(AF_INET6, SOCK_RAW, IPPROTO_IPV6);
```

⚠️ Pour un socket TCP ou UDP IPv6 classique, on utilise généralement simplement :

```cpp
socket(AF_INET6, SOCK_STREAM, 0);
```

ou :

```cpp
socket(AF_INET6, SOCK_DGRAM, 0);
```

---

# 2. Relation entre `domain`, `type` et `protocol`

Les trois paramètres ne sont pas indépendants.

```text
┌──────────────┐
│    domain    │
│              │
│ IPv4 / IPv6  │
│ Unix / etc.  │
└──────┬───────┘
       │
       ▼
┌──────────────┐
│     type     │
│              │
│ STREAM       │
│ DGRAM        │
│ RAW          │
│ etc.         │
└──────┬───────┘
       │
       ▼
┌──────────────┐
│   protocol   │
│              │
│ TCP          │
│ UDP          │
│ ICMP         │
│ etc.         │
└──────────────┘
```

Le système vérifie que la combinaison demandée est cohérente.

---

# 3. Combinaisons courantes

## TCP IPv4

```cpp
socket(AF_INET, SOCK_STREAM, 0);
```

ou :

```cpp
socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
```

Résultat :

```text
IPv4 + TCP
```

---

## UDP IPv4

```cpp
socket(AF_INET, SOCK_DGRAM, 0);
```

ou :

```cpp
socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
```

Résultat :

```text
IPv4 + UDP
```

---

## TCP IPv6

```cpp
socket(AF_INET6, SOCK_STREAM, 0);
```

ou :

```cpp
socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
```

Résultat :

```text
IPv6 + TCP
```

---

## UDP IPv6

```cpp
socket(AF_INET6, SOCK_DGRAM, 0);
```

ou :

```cpp
socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
```

Résultat :

```text
IPv6 + UDP
```

---

## ICMP

```cpp
socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
```

Résultat :

```text
IPv4 + RAW + ICMP
```

---

# 4. Comment choisir `protocol` ?

Dans la majorité des programmes réseau classiques :

```cpp
0
```

suffit.

### TCP

```cpp
socket(AF_INET, SOCK_STREAM, 0);
```

### UDP

```cpp
socket(AF_INET, SOCK_DGRAM, 0);
```

Le système déduit :

```text
SOCK_STREAM → TCP
SOCK_DGRAM  → UDP
```

Tu peux utiliser explicitement :

```cpp
IPPROTO_TCP
```

ou :

```cpp
IPPROTO_UDP
```

mais ce n'est généralement pas nécessaire.

---

# 5. Pour un serveur IRC

Pour un serveur IRC classique :

```cpp
int serverSocket = socket(
    AF_INET,
    SOCK_STREAM,
    0
);
```

Les paramètres sont :

```text
domain
  ↓
AF_INET
  ↓
IPv4

type
  ↓
SOCK_STREAM
  ↓
Flux

protocol
  ↓
0
  ↓
TCP automatiquement
```

On peut aussi écrire :

```cpp
int serverSocket = socket(
    AF_INET,
    SOCK_STREAM,
    IPPROTO_TCP
);
```

Mais pour un serveur IRC classique, la première forme est parfaitement adaptée :

```cpp
socket(AF_INET, SOCK_STREAM, 0);
```

---

# 6. Résumé

| `protocol`     | Signification        | Utilisation                  |
| -------------- | -------------------- | ---------------------------- |
| `0`            | Protocole par défaut | Cas général                  |
| `IPPROTO_TCP`  | TCP                  | `SOCK_STREAM`                |
| `IPPROTO_UDP`  | UDP                  | `SOCK_DGRAM`                 |
| `IPPROTO_ICMP` | ICMP                 | Généralement avec `SOCK_RAW` |
| `IPPROTO_IPV6` | IPv6                 | Cas spécifiques IPv6         |

---

## Les trois paramètres ensemble

```cpp
socket(AF_INET, SOCK_STREAM, 0);
```

se lit :

```text
┌────────────────────────────────────┐
│             socket()               │
├────────────────────────────────────┤
│                                    │
│ AF_INET                            │
│ └── Famille : IPv4                 │
│                                    │
│ SOCK_STREAM                        │
│ └── Type : flux                    │
│                                    │
│ 0                                  │
│ └── Protocole : par défaut → TCP   │
│                                    │
└────────────────────────────────────┘
```

### À retenir

```text
domain
  ↓
IPv4 / IPv6 / Unix / ...

type
  ↓
STREAM / DGRAM / RAW / ...

protocol
  ↓
TCP / UDP / ICMP / ...
```

Pour Ft_IRC :

```cpp
socket(AF_INET, SOCK_STREAM, 0);
```

→ **IPv4 + TCP**.
