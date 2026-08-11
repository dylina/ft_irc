# `listen()`

### Rôle

Met un [socket](../socket()/SOCKET.md) serveur en mode écoute.

### Prototype

```cpp
int listen(int sockfd, int backlog);
```

`sockfd` = OpenFd

[`backlog`](#backlog)

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
