## `getaddrinfo()`

[return value and schema](return-and-schema.md)

### Rôle

Traduit un nom d'hôte et un service en informations réseau utilisables.

### Prototype

```cpp
int getaddrinfo(
    const char *node,
    const char *service,
    const struct addrinfo *hints,
    struct addrinfo **result
);
```

### Exemple

```cpp
struct addrinfo hints;
struct addrinfo *result;

memset(&hints, 0, sizeof(hints));

hints.ai_family = AF_INET;
hints.ai_socktype = SOCK_STREAM;

int ret = getaddrinfo(
    "localhost",
    "6667",
    &hints,
    &result
);
```
