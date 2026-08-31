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
