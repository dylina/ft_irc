## `getprotobyname()`

### Rôle

Recherche les informations d'un protocole à partir de son nom.

### Prototype

```cpp
struct protoent *getprotobyname(const char *name);
```

### Exemple

```cpp
struct protoent *proto = getprotobyname("tcp");
```

Retourne une structure contenant notamment le numéro du protocole.

### À retenir

```text
"tcp"
  ↓
getprotobyname()
  ↓
informations sur TCP
```

> Fonction ancienne. Elle est rarement nécessaire dans un serveur moderne utilisant `socket(..., SOCK_STREAM, 0)`.

---
