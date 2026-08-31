# `htons()`

### Host TO Network Short

Convertit un entier 16 bits vers l'ordre réseau.

```cpp
uint16_t htons(uint16_t hostshort);
```

### Principal usage

Les ports.

```cpp
addr.sin_port = htons(6667);
```

```text
6667
 │
 ▼
htons()
 │
 ▼
Network Byte Order
```

---
