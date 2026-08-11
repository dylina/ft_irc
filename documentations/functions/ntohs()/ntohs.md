# `ntohs()`

### Network TO Host Short

Inverse de [`htons()`](../hton()/hton.md).

```cpp
uint16_t ntohs(uint16_t netshort);
```

Exemple :

```cpp
int port = ntohs(addr.sin_port);
```

```text
Network
   ↓
ntohs()
   ↓
Host
```

---
