# `bind()`

### Rôle

Associe un socket à une **IP + port local**.

### Prototype

```cpp
int bind(
    int sockfd,
    const struct sockaddr *addr,
    socklen_t addrlen
);
```

### Exemple

```cpp
struct sockaddr_in addr;

addr.sin_family = AF_INET;
addr.sin_port = htons(6667);
addr.sin_addr.s_addr = INADDR_ANY;

bind(
    serverFd,
    (struct sockaddr *)&addr,
    sizeof(addr)
);
```

### Schéma

```text
socket()
   │
   ▼
bind()
   │
   ▼
127.0.0.1:6667
```

### À retenir

```text
bind = "je réserve cette adresse/ce port pour mon socket"
```

---
