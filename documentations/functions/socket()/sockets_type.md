# `socket()` — `type`

Le deuxième paramètre de `socket()` indique **le type de communication du socket**.

```cpp
int socket(int domain, int type, int protocol);
                         ^^^^
```

Le paramètre `type` détermine notamment si le socket fonctionne comme un **[flux](../../global_knowledge/Flux.md)**, des **[datagrammes](../../global_knowledge/datagrammes.md)**, des **[paquets](../../global_knowledge/paquets.md)**, etc.

---

## Sommaire

* [`SOCK_STREAM` — Flux](#sock_stream--flux)
* [`SOCK_DGRAM` — Datagrammes](#sock_dgram--datagrammes)
* [`SOCK_SEQPACKET` — Paquets séquencés](#sock_seqpacket--paquets-séquencés)
* [`SOCK_RAW` — Paquets bruts](#sock_raw--paquets-bruts)
* [`SOCK_RDM` — Datagrammes fiables](#sock_rdm--datagrammes-fiables)
* [`SOCK_PACKET` — Ancien socket paquet](#sock_packet--ancien-socket-paquet)
* [Options supplémentaires du `type`](#options-supplémentaires-du-type)
* [Combinaisons courantes](#combinaisons-courantes)
* [Résumé](#résumé)
* [Pour un serveur IRC](#pour-un-serveur-irc)

---

# `SOCK_STREAM` — Flux

```cpp
socket(AF_INET, SOCK_STREAM, 0);
```

`SOCK_STREAM` crée un socket orienté **flux de données**.

Avec :

```cpp
AF_INET + SOCK_STREAM
```

le protocole utilisé est généralement **TCP**.

### Caractéristiques

```text
Connexion nécessaire
        ↓
Données fiables
        ↓
Données dans l'ordre
        ↓
Retransmission si nécessaire
        ↓
Flux continu de données
```

Contrairement à UDP, TCP ne conserve pas les limites des messages.

Par exemple, si le client envoie :

```text
"HELLO"
"WORLD"
```

le serveur peut recevoir :

```text
"HELLOWORLD"
```

ou :

```text
"HEL"
"LOWOR"
"LD"
```

C'est au programme de gérer le découpage des données.

### Exemple

```cpp
int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
```

Utilisé notamment pour :

* HTTP
* HTTPS
* SSH
* FTP
* IRC

---

# `SOCK_DGRAM` — Datagrammes

```cpp
socket(AF_INET, SOCK_DGRAM, 0);
```

`SOCK_DGRAM` crée un socket orienté **datagrammes**.

Avec :

```text
AF_INET + SOCK_DGRAM
        ↓
       UDP
```

### Caractéristiques

```text
Pas de connexion obligatoire
        ↓
Messages indépendants
        ↓
Pas de garantie de livraison
        ↓
Pas de garantie d'ordre
        ↓
Faible surcharge
```

Chaque envoi correspond à un **datagramme**.

Par exemple :

```cpp
sendto(socket, "HELLO", 5, ...);
sendto(socket, "WORLD", 5, ...);
```

Le récepteur reçoit deux datagrammes distincts :

```text
HELLO
WORLD
```

Contrairement à TCP, les frontières des messages sont conservées.

### Exemple

```cpp
int serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
```

Utilisé notamment pour :

* DNS
* DHCP
* certains jeux en réseau
* streaming
* applications temps réel

---

# `SOCK_SEQPACKET` — Paquets séquencés

```cpp
socket(AF_INET, SOCK_SEQPACKET, 0);
```

`SOCK_SEQPACKET` combine certaines caractéristiques de `SOCK_STREAM` et `SOCK_DGRAM`.

Il conserve :

* l'ordre des données
* les frontières des messages
* une certaine fiabilité selon le protocole utilisé

Exemple conceptuel :

```text
Message 1
Message 2
Message 3
```

Le récepteur conserve les limites :

```text
Message 1
Message 2
Message 3
```

Contrairement à `SOCK_STREAM`, les messages ne sont donc pas simplement considérés comme un flux continu.

⚠️ Sa disponibilité dépend du protocole et du système.

---

# `SOCK_RAW` — Paquets bruts

```cpp
socket(AF_INET, SOCK_RAW, protocol);
```

`SOCK_RAW` permet un accès beaucoup plus bas niveau au réseau.

Le programme peut travailler directement avec certains paquets réseau.

Par exemple :

```cpp
socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
```

permet de travailler avec ICMP.

### Utilisations

On retrouve les sockets RAW dans :

* outils réseau
* analyse de paquets
* implémentation de certains protocoles
* diagnostic réseau
* logiciels de sécurité réseau

⚠️ L'utilisation de `SOCK_RAW` nécessite généralement des privilèges élevés.

---

# `SOCK_RDM` — Datagrammes fiables

```cpp
socket(AF_INET, SOCK_RDM, 0);
```

`SOCK_RDM` signifie **Reliable Datagram Message**.

Il est destiné à des communications sous forme de datagrammes avec des garanties de fiabilité.

Il conserve notamment l'idée de **messages distincts**, contrairement au flux de `SOCK_STREAM`.

⚠️ Ce type est très peu utilisé et n'est pas supporté de la même manière sur tous les systèmes.

---

# `SOCK_PACKET` — Ancien socket paquet

`SOCK_PACKET` est un ancien type de socket principalement associé à Linux.

Il permettait d'accéder aux paquets réseau à un niveau très bas.

Il est aujourd'hui **obsolète**.

Il ne faut pas l'utiliser pour un nouveau projet.

Sur Linux, on utilise plutôt :

```cpp
AF_PACKET
```

avec un type approprié.

---

# Options supplémentaires du `type`

Le paramètre `type` peut également être combiné avec certaines options.

Par exemple :

```cpp
SOCK_STREAM | SOCK_NONBLOCK
```

ou :

```cpp
SOCK_STREAM | SOCK_CLOEXEC
```

---

## `SOCK_NONBLOCK`

Met le socket en mode **non bloquant** dès sa création.

```cpp
int sock = socket(
    AF_INET,
    SOCK_STREAM | SOCK_NONBLOCK,
    0
);
```

Sans `SOCK_NONBLOCK`, certaines opérations peuvent bloquer le programme.

Exemple :

```text
recv()
 ↓
attend des données
 ↓
bloque le programme
```

Avec `SOCK_NONBLOCK` :

```text
recv()
 ↓
pas de données
 ↓
retourne immédiatement
```

C'est particulièrement utile avec :

```text
poll()
select()
epoll()
```

---

## `SOCK_CLOEXEC`

Ajoute automatiquement le flag `close-on-exec` au descripteur.

```cpp
int sock = socket(
    AF_INET,
    SOCK_STREAM | SOCK_CLOEXEC,
    0
);
```

Cela permet au système de fermer automatiquement le descripteur lorsqu'un processus exécute un nouveau programme avec `exec()`.

⚠️ `SOCK_CLOEXEC` et `SOCK_NONBLOCK` sont des extensions disponibles sur les systèmes modernes, notamment Linux, et ne font pas partie des types historiques de socket.

---

# Combinaisons courantes

## TCP IPv4

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

## TCP IPv4 non bloquant

```cpp
socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
```

---

## TCP IPv4 avec `close-on-exec`

```cpp
socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
```

---

## UDP IPv4

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

## TCP IPv6

```cpp
socket(AF_INET6, SOCK_STREAM, 0);
```

---

## UDP IPv6

```cpp
socket(AF_INET6, SOCK_DGRAM, 0);
```

---

## Socket Unix en flux

```cpp
socket(AF_UNIX, SOCK_STREAM, 0);
```

Communication locale sous forme de flux.

---

## Socket Unix en datagrammes

```cpp
socket(AF_UNIX, SOCK_DGRAM, 0);
```

Communication locale sous forme de datagrammes.

---

# Résumé

| `type`           | Fonction                   | Utilisation          |
| ---------------- | -------------------------- | -------------------- |
| `SOCK_STREAM`    | Flux fiable                | TCP                  |
| `SOCK_DGRAM`     | Datagrammes                | UDP                  |
| `SOCK_SEQPACKET` | Paquets séquencés          | Protocoles supportés |
| `SOCK_RAW`       | Paquets bruts              | Bas niveau / réseau  |
| `SOCK_RDM`       | Datagrammes fiables        | Rare                 |
| `SOCK_PACKET`    | Paquets bas niveau         | Obsolète             |
| `SOCK_NONBLOCK`  | Mode non bloquant          | Option               |
| `SOCK_CLOEXEC`   | Fermeture lors de `exec()` | Option               |

---

# À retenir

Le paramètre `type` répond à la question :

> **« Comment mon socket va-t-il échanger les données ? »**

```text
SOCK_STREAM
    ↓
Flux
    ↓
TCP généralement


SOCK_DGRAM
    ↓
Datagrammes
    ↓
UDP généralement


SOCK_SEQPACKET
    ↓
Messages séquencés


SOCK_RAW
    ↓
Accès bas niveau aux paquets
```

Pour un serveur IRC classique :

```cpp
int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
```

Le `type` est donc :

```cpp
SOCK_STREAM
```

ce qui signifie que le serveur utilise un **flux TCP**.
