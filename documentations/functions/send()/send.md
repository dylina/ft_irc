# `send()`

### Rôle

Envoie des données via un socket.

### ⚠️ Attention

`send()` peut envoyer **moins de données que demandé**.

```text
Tu demandes : 1000 octets

send()
   ↓
600 octets envoyés
```

Un serveur robuste doit donc gérer les envois partiels.

---

### Prototype

```cpp
ssize_t send(
    int sockfd,
    const void *buf,
    size_t len,
    int flags
);
```

### Exemple

```cpp
std::string msg = "PING :hello\r\n";

send(
    clientFd,
    msg.c_str(),
    msg.size(),
    0
);
```

### Schéma

```text
SERVEUR
   │
   │ send()
   ▼
CLIENT
```


