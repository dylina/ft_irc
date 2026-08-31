# `poll()` en C/C++

## 1. À quoi sert `poll()` ?

`poll()` permet de surveiller plusieurs **file descriptors (FD)** en même temps et de savoir lesquels sont prêts à effectuer une opération sans bloquer.

C'est particulièrement utile pour un serveur réseau comme un serveur IRC :

```text
                    ┌──────────────┐
client A ──────────>│              │
client B ──────────>│    serveur   │
client C ──────────>│              │
                    └──────┬───────┘
                           │
                         poll()
                           │
               ┌───────────┴───────────┐
               │                       │
          nouveau client          données client
               │                       │
            accept()                 recv()
```

L'idée fondamentale :

> Tu donnes à `poll()` une liste de FD et tu lui demandes lesquels sont prêts.

`poll()` **ne fait pas** `accept()`, `recv()` ou `send()` à ta place. Il indique seulement qu'une opération est prête.

---

# 2. La structure `pollfd`

La structure est définie ainsi :

```cpp
struct pollfd
{
    int   fd;
    short events;
    short revents;
};
```

Elle contient trois informations.

## `fd`

Le file descriptor que tu veux surveiller.

Exemple :

```cpp
struct pollfd pfd;

pfd.fd = server_fd;
```

---

## `events`

Les événements que **toi tu demandes à `poll()` de surveiller**.

Exemple :

```cpp
pfd.events = POLLIN;
```

Cela signifie :

> « Dis-moi quand ce FD peut être lu sans bloquer. »

---

## `revents`

Les événements que **`poll()` a détectés**.

Tu remplis `events`, mais normalement pas `revents`.

Avant `poll()` :

```text
fd       events       revents
3        POLLIN       0
```

Après `poll()` :

```text
fd       events       revents
3        POLLIN       POLLIN
```

Cela signifie que le FD 3 est maintenant prêt à être lu.

---

# 3. Les principaux flags

| Flag | Signification |
|---|---|
| `POLLIN` | Des données peuvent être lues |
| `POLLOUT` | On peut écrire sans bloquer |
| `POLLERR` | Une erreur est survenue sur le FD |
| `POLLHUP` | Le FD a été fermé/déconnecté |
| `POLLNVAL` | Le FD est invalide |

Pour un serveur IRC, `POLLIN` est le flag principal.

---

# 4. `POLLIN`

```cpp
pfd.events = POLLIN;
```

Cela signifie :

> Je veux savoir quand ce FD est prêt à être lu.

Pour un client TCP :

```text
client ───────────────> serveur
          données
```

Le client envoie par exemple :

```text
NICK toto\r\n
```

Le socket devient lisible.

`poll()` peut alors indiquer :

```cpp
if (pfd.revents & POLLIN)
{
    recv(...);
}
```

---

# 5. Pourquoi utiliser `&` avec les flags ?

Les flags de `poll()` sont des **bits**. Plusieurs événements peuvent donc être présents en même temps.

Par exemple :

```cpp
revents = POLLIN | POLLERR;
```

Il ne faut donc généralement pas faire :

```cpp
if (pfd.revents == POLLIN)
```

car `revents` pourrait contenir plusieurs flags.

Il faut faire :

```cpp
if (pfd.revents & POLLIN)
```

Cela signifie :

> Est-ce que le flag `POLLIN` est présent ?

Même principe :

```cpp
if (pfd.revents & POLLERR)
{
    // erreur
}

if (pfd.revents & POLLHUP)
{
    // déconnexion
}

if (pfd.revents & POLLNVAL)
{
    // FD invalide
}
```

---

# 6. La fonction `poll()`

La fonction est déclarée comme ceci :

```cpp
int poll(
    struct pollfd *fds,
    nfds_t nfds,
    int timeout
);
```

Exemple :

```cpp
poll(fds, nfds, -1);
```

Les trois paramètres sont :

## `fds`

Le tableau de structures `pollfd` à surveiller.

```cpp
struct pollfd fds[10];
```

---

## `nfds`

Le nombre de structures présentes dans le tableau.

```cpp
poll(fds, 10, -1);
```

Ici, `poll()` surveille les 10 entrées.

---

## `timeout`

Le temps maximum d'attente.

| Valeur | Comportement |
|---|---|
| `-1` | Attend indéfiniment |
| `0` | Ne bloque pas |
| `1000` | Attend maximum 1000 ms |

Pour un serveur IRC classique :

```cpp
poll(fds, nfds, -1);
```

est très courant.

---

# 7. Exemple minimal

Imaginons que l'on ait un socket serveur et un client :

```cpp
struct pollfd fds[2];

fds[0].fd = server_fd;
fds[0].events = POLLIN;

fds[1].fd = client_fd;
fds[1].events = POLLIN;
```

On appelle :

```cpp
poll(fds, 2, -1);
```

Le programme attend.

Si le client envoie des données, on peut obtenir :

```text
fd       events       revents
server   POLLIN       0
client   POLLIN       POLLIN
```

On peut alors faire :

```cpp
if (fds[1].revents & POLLIN)
{
    // recv(client_fd, ...)
}
```

---

# 8. Le socket serveur et `POLLIN`

C'est particulièrement important dans `ft_irc`.

Après :

```cpp
listen(server_fd, ...);
```

on ajoute le socket serveur à `poll()` :

```cpp
fds[0].fd = server_fd;
fds[0].events = POLLIN;
```

Pourquoi `POLLIN` sur le socket serveur ?

Parce que lorsqu'un nouveau client attend d'être accepté, le socket d'écoute devient "lisible" au sens de `poll()`.

On peut donc faire :

```cpp
if (fds[0].revents & POLLIN)
{
    int client_fd = accept(server_fd, ...);
}
```

Attention :

> `POLLIN` sur un socket d'écoute signifie essentiellement qu'un `accept()` peut être effectué.

---

# 9. Plusieurs clients

C'est là que `poll()` devient intéressant pour un serveur IRC.

On peut avoir :

```text
fds[0] = serveur
fds[1] = client A
fds[2] = client B
fds[3] = client C
fds[4] = client D
```

Et :

```cpp
fds[0].events = POLLIN;
fds[1].events = POLLIN;
fds[2].events = POLLIN;
fds[3].events = POLLIN;
fds[4].events = POLLIN;
```

Puis :

```cpp
poll(fds, 5, -1);
```

Si le client B envoie :

```text
PRIVMSG toto :salut
```

on peut avoir :

```text
fds[0]  revents = 0
fds[1]  revents = 0
fds[2]  revents = POLLIN
fds[3]  revents = 0
fds[4]  revents = 0
```

On parcourt alors le tableau :

```cpp
for (int i = 0; i < 5; i++)
{
    if (fds[i].revents & POLLIN)
    {
        // traiter fds[i]
    }
}
```

---

# 10. Structure classique d'un serveur IRC

Le principe général est :

```cpp
while (true)
{
    poll(fds, nfds, -1);

    for (int i = 0; i < nfds; i++)
    {
        if (fds[i].revents & POLLIN)
        {
            if (fds[i].fd == server_fd)
            {
                // nouveau client
                accept(...);
            }
            else
            {
                // client existant
                recv(...);
            }
        }
    }
}
```

Le fonctionnement est donc :

```text
                    poll()
                      │
                      ▼
          Quels FD sont prêts ?
                      │
          ┌───────────┴───────────┐
          ▼                       ▼
   socket serveur            socket client
       POLLIN                    POLLIN
          │                       │
          ▼                       ▼
       accept()                 recv()
```

---

# 11. `POLLHUP`

`POLLHUP` indique généralement que le côté distant a fermé la connexion ou qu'une fermeture est détectée sur le FD.

Exemple :

```cpp
if (fds[i].revents & POLLHUP)
{
    close(fds[i].fd);
    // supprimer le client de la liste
}
```

Cependant, dans un serveur TCP, il est également important de vérifier le résultat de `recv()`.

---

# 12. `recv() == 0`

Quand un client TCP ferme proprement sa connexion :

```cpp
int ret = recv(fd, buffer, sizeof(buffer), 0);
```

Si :

```cpp
ret == 0
```

cela signifie :

> L'autre côté a fermé proprement la connexion.

Exemple :

```cpp
int ret = recv(fd, buffer, sizeof(buffer), 0);

if (ret == 0)
{
    close(fd);
    // supprimer le client
}
```

---

# 13. `POLLERR`

```cpp
if (fds[i].revents & POLLERR)
{
    // erreur
}
```

Cela indique qu'une erreur est associée au file descriptor.

---

# 14. `POLLNVAL`

```cpp
if (fds[i].revents & POLLNVAL)
{
    // FD invalide
}
```

Cela peut arriver si le FD n'est plus valide.

Par exemple, une mauvaise gestion pourrait produire :

```cpp
close(fd);
```

alors que le FD est toujours présent dans ton tableau de `pollfd`.

Il faut donc penser à retirer correctement les FD fermés de la liste surveillée.

---

# 15. `POLLOUT`

`POLLOUT` indique que le FD peut généralement être écrit sans bloquer.

Exemple :

```cpp
pfd.events = POLLOUT;
```

Puis :

```cpp
if (pfd.revents & POLLOUT)
{
    send(...);
}
```

On peut également surveiller plusieurs événements :

```cpp
pfd.events = POLLIN | POLLOUT;
```

Puis :

```cpp
if (pfd.revents & POLLIN)
{
    recv(...);
}

if (pfd.revents & POLLOUT)
{
    send(...);
}
```

---

# 16. Pourquoi ne pas toujours utiliser `POLLOUT` ?

Pour un socket TCP, il est généralement possible d'écrire immédiatement dans le buffer d'envoi du système.

Donc si tu mets :

```cpp
pfd.events = POLLIN | POLLOUT;
```

en permanence, le socket risque d'être presque toujours signalé comme `POLLOUT`.

Ton serveur pourrait alors passer son temps à traiter :

```text
POLLOUT
POLLOUT
POLLOUT
POLLOUT
POLLOUT
...
```

sans avoir réellement quelque chose à envoyer.

Une approche classique est donc :

```text
Pas de données à envoyer
        │
        ▼
events = POLLIN

Données en attente dans le buffer d'envoi
        │
        ▼
events = POLLIN | POLLOUT

Données envoyées
        │
        ▼
events = POLLIN
```

---

# 17. `poll()` ne réalise pas les opérations

C'est un point essentiel.

`poll()` ne fait pas :

```cpp
accept();
recv();
send();
```

Il dit simplement :

> "Ce FD est prêt pour telle opération."

Par exemple :

```text
                 poll()
                   │
                   ▼
          "FD 3 est lisible"
                   │
                   ▼
              recv(fd 3)
```

Ou :

```text
                 poll()
                   │
                   ▼
       "Un client peut être accepté"
                   │
                   ▼
             accept()
```

Ou :

```text
                 poll()
                   │
                   ▼
       "FD 5 peut être écrit"
                   │
                   ▼
              send(fd 5)
```

---

# 18. Exemple avec `std::vector<pollfd>`

Pour un projet comme `ft_irc`, on peut stocker les `pollfd` dans un `std::vector`.

Initialisation du socket serveur :

```cpp
std::vector<struct pollfd> fds;

struct pollfd server;

server.fd = server_fd;
server.events = POLLIN;
server.revents = 0;

fds.push_back(server);
```

Lorsqu'un client est accepté :

```cpp
struct pollfd client;

client.fd = client_fd;
client.events = POLLIN;
client.revents = 0;

fds.push_back(client);
```

Puis :

```cpp
poll(&fds[0], fds.size(), -1);
```

Et on parcourt :

```cpp
for (size_t i = 0; i < fds.size(); i++)
{
    if (fds[i].revents & POLLIN)
    {
        if (fds[i].fd == server_fd)
        {
            // accept()
        }
        else
        {
            // recv()
        }
    }
}
```

---

# 19. Vue globale d'un serveur `ft_irc`

```text
┌──────────────────────────────────────────┐
│              SERVER SOCKET               │
│                                          │
│ socket()                                 │
│ bind()                                   │
│ listen()                                 │
└──────────────────┬───────────────────────┘
                   │
                   ▼
              ┌─────────┐
              │  poll() │
              └────┬────┘
                   │
          ┌────────┴─────────┐
          │                  │
          ▼                  ▼
    serveur POLLIN       client POLLIN
          │                  │
          ▼                  ▼
      accept()             recv()
          │                  │
          ▼                  ▼
    nouveau client       commande IRC
          │                  │
          └────────┬─────────┘
                   │
                   ▼
                 poll()
                   │
                   ▼
                  ...
```

---

# 20. Résumé à retenir

La structure :

```cpp
struct pollfd
{
    int   fd;        // QUI ?
    short events;    // QUOI surveiller ?
    short revents;   // QU'EST-CE QUI S'EST PASSÉ ?
};
```

La fonction :

```cpp
poll(fds, nfds, timeout);
```

Les flags principaux :

```cpp
POLLIN     // prêt à lire
POLLOUT    // prêt à écrire
POLLERR    // erreur
POLLHUP    // déconnexion/fermeture
POLLNVAL   // FD invalide
```

Tester un flag :

```cpp
if (fds[i].revents & POLLIN)
{
    ...
}
```

Plusieurs flags :

```cpp
fds[i].events = POLLIN | POLLOUT;
```

Et le principe général :

```text
1. Ajouter les FD à surveiller
          ↓
2. Définir events
          ↓
3. Appeler poll()
          ↓
4. Lire revents
          ↓
5. Effectuer accept(), recv(), send(), etc.
          ↓
6. Ajouter/supprimer des FD si nécessaire
          ↓
7. Recommencer
```

Pour `ft_irc`, le cœur du serveur peut donc être résumé par :

```cpp
while (running)
{
    poll(fds, fds.size(), -1);

    for (size_t i = 0; i < fds.size(); ++i)
    {
        if (fds[i].revents & POLLIN)
        {
            if (fds[i].fd == server_fd)
                acceptNewClient();
            else
                receiveFromClient(i);
        }

        if (fds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
        {
            removeClient(i);
        }
    }
}
```

> **Attention :** dans du vrai code, la suppression d'un élément d'un `vector` pendant qu'on le parcourt demande une gestion soigneuse des indices/itérateurs. Il faut aussi gérer les retours de `accept()`, `recv()` et `send()`, notamment `EAGAIN`/`EWOULDBLOCK` si les sockets sont non bloquants.
