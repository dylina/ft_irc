# `socket()` — Valeur de retour


## Sommaire

* [`1. Valeur de retour`](#1-valeur-de-retour)
* [`2. En cas de succès`](#2-en-cas-de-succès)
* [`3. En cas d'erreur`](#3-en-cas-derreur)
* [`4. Exemple`](#4-exemple)
* [`5. Que représente le descripteur ?`](#5-que-représente-le-descripteur)
* [`6. Cycle de vie`](#6-cycle-de-vie)

---

# 1. Valeur de retour

```cpp
int fd = socket(domain, type, protocol);
```

`socket()` retourne un **`int`**.

Il existe deux possibilités :

| Retour | Signification                  |
| -----: | ------------------------------ |
| `>= 0` | Succès : descripteur du socket |
|   `-1` | Erreur                         |

Le nombre retourné n'est donc **pas le socket lui-même**, mais un **descripteur** permettant au programme de manipuler ce socket.

---

# 2. En cas de succès

Exemple :

```cpp
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
```

Si la création réussit :

```text
sockfd = 3
```

Le `3` est un **file descriptor (FD)**.

Le système utilise ce nombre pour identifier le socket.

Par exemple :

```text
0 → stdin
1 → stdout
2 → stderr
3 → socket
```

Le numéro exact n'est pas garanti.

Il peut être différent selon les fichiers déjà ouverts par le programme.

---

# 3. En cas d'erreur

Si `socket()` échoue :

```cpp
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
```

la fonction retourne :

```text
-1
```

Il faut alors vérifier `errno` pour connaître la raison de l'erreur.

Exemple :

```cpp
#include <sys/socket.h>
#include <cerrno>
#include <cstdio>

int sockfd = socket(AF_INET, SOCK_STREAM, 0);

if (sockfd == -1)
{
    perror("socket");
}
```

`perror()` affiche une description correspondant à `errno`.

---

# 4. Exemple

```cpp
int sockfd = socket(AF_INET, SOCK_STREAM, 0);

if (sockfd == -1)
{
    perror("socket");
    return 1;
}

printf("Socket créé : %d\n", sockfd);
```

Une sortie possible :

```text
Socket créé : 3
```

On peut ensuite utiliser `sockfd` avec d'autres fonctions :

```cpp
bind(sockfd, ...);
listen(sockfd, ...);
accept(sockfd, ...);
send(sockfd, ...);
recv(sockfd, ...);
close(sockfd);
```

---

# 5. Que représente le descripteur ?

Le descripteur est une **référence vers une ressource gérée par le système d'exploitation**.

```text
Programme
    │
    │ socket()
    ▼
┌───────────────┐
│   Socket      │
│   du système  │
└───────┬───────┘
        │
        │
       FD
        │
        ▼
       3
```

Ton programme ne manipule donc pas directement la structure interne du socket.

Il utilise son descripteur :

```cpp
int sockfd = 3;
```

pour demander au système d'effectuer des opérations sur celui-ci.

---

# 6. Cycle de vie

Le descripteur retourné par `socket()` est ensuite utilisé pendant toute la durée de vie du socket.

```text
socket()
   │
   │ retourne FD
   ▼
 sockfd
   │
   ├── bind()
   │
   ├── listen()
   │
   ├── accept()
   │
   ├── send()
   │
   └── recv()
   │
   ▼
 close(sockfd)
```

Exemple serveur TCP :

```cpp
int sockfd = socket(AF_INET, SOCK_STREAM, 0);

if (sockfd == -1)
{
    perror("socket");
    return 1;
}

bind(sockfd, ...);
listen(sockfd, 5);

int client = accept(sockfd, ...);

recv(client, ...);
send(client, ...);

close(client);
close(sockfd);
```

À retenir :

```text
socket()
   │
   ├── succès → FD >= 0
   │
   └── erreur → -1
```

Le **FD retourné par `socket()` doit être conservé**, car c'est lui qui sera passé aux autres fonctions réseau.
