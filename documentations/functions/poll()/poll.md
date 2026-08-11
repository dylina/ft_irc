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
