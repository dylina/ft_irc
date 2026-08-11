# `inet_addr()`

### Rôle

Convertit une IPv4 sous forme texte vers une valeur réseau.

```cpp
in_addr_t inet_addr(const char *cp);
```

### Exemple

```cpp
addr.sin_addr.s_addr = inet_addr("127.0.0.1");
```

```text
"127.0.0.1"
     ↓
inet_addr()
     ↓
IPv4 binaire
```

> API historique. Préférer `inet_pton()` dans du nouveau code.

---
