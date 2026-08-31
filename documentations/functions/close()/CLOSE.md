# `close()` — Fermeture d'un descripteur

# Sommaire

1. [`close()`](#close)
2. [Prototype](#prototype)
3. [Paramètre](#paramètre)
4. [Valeur de retour](close-retrun.md)
5. [Exemple](exemple-clode.md)
6. [Résumé](#résumé)

---

# `close()`

	`close()` permet de **fermer un descripteur de fichier**.

	Dans le contexte des sockets, elle permet de **fermer une socket** et de libérer les ressources associées.

---

Une socket est représentée par un **descripteur de fichier**.

Par exemple :

```cpp
int fd = socket(AF_INET, SOCK_STREAM, 0);
```

Si `socket()` retourne `3`, alors :

```text
fd = 3
```

Lorsque la socket n'est plus utilisée, le descripteur doit être fermé avec `close()` :

```cpp
close(fd);
```

On peut représenter le fonctionnement ainsi :

```text
Socket créée
     │
     ▼
   fd = 3
     │
     │
     │  close(fd)
     ▼
Socket fermée
```

---

# Prototype

```cpp
#include <unistd.h>

int close(int fd);
```

---

# Paramètre

## `fd` — Descripteur de fichier

`fd` correspond au **descripteur que l'on souhaite fermer**.

Exemple :

```cpp
int fd = socket(AF_INET, SOCK_STREAM, 0);

close(fd);
```

Ici :

```text
fd
│
└── descripteur de la socket
```

`close()` peut être utilisé avec différents descripteurs de fichiers, pas uniquement avec des sockets.

---
# Fermeture d'une socket

Lorsqu'une socket est fermée :

```cpp
close(fd);
```

le descripteur n'est plus utilisable pour cette socket.

```text
Avant :

fd = 3
│
▼
┌──────────┐
│  Socket  │
└──────────┘


close(fd)


Après :

fd = 3
│
X
│
└── Socket fermée
```

Le système peut ensuite réutiliser ce numéro de descripteur pour une autre ressource.

---

# Résumé

| Élément   | Rôle                 |
| :-------- | :------------------- |
| `close()` | Ferme un descripteur |
| `fd`      | Descripteur à fermer |
| `0`       | Fermeture réussie    |
| `-1`      | Erreur               |

### Syntaxe

```cpp
close(fd);
```

### À retenir

`socket()` crée une socket et retourne un descripteur.

```text
socket()
   │
   ▼
  fd
   │
   ▼
close(fd)
   │
   ▼
socket fermée
```

`close()` permet donc de **fermer le descripteur et libérer les ressources associées**.
