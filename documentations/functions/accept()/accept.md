# `accept()`

### Rôle

Accepte une connexion entrante.

### Prototype

```cpp
int accept(
    int sockfd,
    struct sockaddr *addr,
    socklen_t *addrlen
);
```

### ⚠️ Point essentiel

`accept()` crée un **nouveau socket**.

```text
serverFd = 3
     │
     │ accept()
     ▼
clientFd = 4
```

Le serveur conserve :

```text
fd 3 → écouter les nouveaux clients
fd 4 → communiquer avec client 1
fd 5 → communiquer avec client 2
fd 6 → communiquer avec client 3
```

---


### Exemple

```cpp
struct sockaddr_in clientAddr;
socklen_t clientLen = sizeof(clientAddr);

int clientFd = accept(
    serverFd,
    (struct sockaddr *)&clientAddr,
    &clientLen
);
```










































# 🔢 4. Conversion des adresses et ports

Les ordinateurs et les réseaux peuvent utiliser des ordres de bytes différents.

On distingue :

```text
HOST BYTE ORDER
NETWORK BYTE ORDER
```

---
