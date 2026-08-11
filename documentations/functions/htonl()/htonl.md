# `htonl()`

### Host TO Network Long

Convertit un entier 32 bits vers l'ordre réseau.

```cpp
uint32_t htonl(uint32_t hostlong);
```

Exemple :

```cpp
addr.sin_addr.s_addr = htonl(INADDR_ANY);
```

---
