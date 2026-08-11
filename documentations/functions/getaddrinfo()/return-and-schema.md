### Résultat

`result` contient notamment :

```cpp
result->ai_family
result->ai_socktype
result->ai_protocol
result->ai_addr
result->ai_addrlen
```

### Schéma

```text
"localhost" + "6667"
        │
        ▼
  getaddrinfo()
        │
        ▼
┌─────────────────────┐
│ IP                  │
│ Port                │
│ Protocol            │
│ Socket type         │
│ sockaddr             │
└─────────────────────┘
```

---
