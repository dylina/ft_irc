## `freeaddrinfo()`

### Rôle

Libère les données créées par [`getaddrinfo()`](../getaddrinfo()/getaddrinfo.md).

### Prototype

```cpp
void freeaddrinfo(struct addrinfo *res);
```

### Exemple

```cpp
freeaddrinfo(result);
```

Toujours faire :

```cpp
getaddrinfo(...)
    ↓
utilisation
    ↓
freeaddrinfo()
```

---
