# `recv()`

### Rôle

Reçoit des données depuis un socket.

[`RETURN VALUE!`](return-value.md)

### Prototype

```cpp
ssize_t recv(
    int sockfd,
    void *buf,
    size_t len,
    int flags
);
```

### Exemple

```cpp
char buffer[512];

ssize_t ret = recv(
    clientFd,
    buffer,
    sizeof(buffer),
    0
);
```

