## `setsockopt()`

### Rôle

Modifie une option d'un socket.

### Prototype

```cpp
int setsockopt(
    int socket,
    int level,
    int option_name,
    const void *option_value,
    socklen_t option_len
);
```

### Exemple classique

```cpp
int opt = 1;

setsockopt(
    serverFd,
    SOL_SOCKET,
    SO_REUSEADDR,
    &opt,
    sizeof(opt)
);
```

### `SO_REUSEADDR`

Permet notamment de réutiliser rapidement une adresse/port après l'arrêt du serveur.

```text
Serveur
   │
   ├── bind(6667)
   │
   ├── CTRL+C
   │
   └── relance
          ↓
   SO_REUSEADDR
```

---
