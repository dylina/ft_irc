## `gethostbyname()`

### Rôle

Traduit un nom d'hôte en adresse IPv4.

### Prototype

```cpp
struct hostent *gethostbyname(const char *name);
```

### Exemple

```cpp
struct hostent *host = gethostbyname("localhost");
```

Conceptuellement :

```text
"localhost"
     ↓
gethostbyname()
     ↓
127.0.0.1
```

> API historique. Pour du nouveau code, préférer `getaddrinfo()`.

---
