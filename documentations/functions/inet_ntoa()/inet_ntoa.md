# `inet_ntoa()`

### Rôle

Convertit une adresse IPv4 en texte.

```cpp
char *inet_ntoa(struct in_addr in);
```

### Exemple

```cpp
std::cout << inet_ntoa(addr.sin_addr);
```

Résultat :

```text
127.0.0.1
```

> Fonction historique utilisant un buffer statique.

---
