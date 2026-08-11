# `inet_ntop()`

### Rôle

Convertit une adresse réseau vers une représentation texte.

### Prototype

```cpp
const char *inet_ntop(
    int af,
    const void *src,
    char *dst,
    socklen_t size
);
```

### Exemple

```cpp
char ip[INET_ADDRSTRLEN];

inet_ntop(
    AF_INET,
    &addr.sin_addr,
    ip,
    sizeof(ip)
);

std::cout << ip;
```

Résultat :

```text
127.0.0.1
```

### À retenir

```text
inet_addr()
    texte → réseau

inet_ntoa()
    réseau → texte

inet_ntop()
    réseau → texte
```

---
