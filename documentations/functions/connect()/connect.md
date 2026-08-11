# `connect()`

### Rôle

Connecte un [socket](../socket()/SOCKET.md) à un serveur distant.

### Prototype

```cpp
int connect(
    int sockfd,
    const struct sockaddr *addr,
    socklen_t addrlen
);
```

### Exemple

```cpp
connect(
    clientFd,
    (struct sockaddr *)&server,
    sizeof(server)
);
```

### Schéma

```text
CLIENT
  │
  │ connect()
  ▼
SERVER
```

`connect()` est principalement utilisé côté **client**.

---
