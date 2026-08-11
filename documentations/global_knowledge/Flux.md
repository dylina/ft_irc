

# 1. Qu'est-ce qu'un flux ?

Un **flux** est une suite continue d'octets échangés entre deux points.

Avec un socket de type :

```cpp
SOCK_STREAM
```

les données sont considérées comme un **flux continu d'octets**.

On peut imaginer cela comme un tuyau :

```text
CLIENT                                      SERVEUR

ABCDEF
  │
  │       Flux d'octets
  ├───────────────────────────────────────►
  │
GHIJKL
```

Le protocole généralement utilisé avec `SOCK_STREAM` est **TCP**.

---

# 2. Flux TCP

TCP ne conserve **pas les frontières des messages**.

Par exemple :

```cpp
send(sockfd, "ABC", 3, 0);
send(sockfd, "DEF", 3, 0);
```

On pourrait penser que le serveur recevra :

```text
ABC
DEF
```

Mais TCP voit simplement :

```text
ABCDEF
```

Le serveur pourrait recevoir :

```text
recv() → "ABCDEF"
```

ou :

```text
recv() → "AB"
recv() → "CDEF"
```

ou :

```text
recv() → "ABC"
recv() → "DEF"
```

ou encore :

```text
recv() → "A"
recv() → "BCDE"
recv() → "F"
```

Il n'y a donc **aucune garantie qu'un `send()` corresponde à un `recv()`**.

```text
send()                         recv()

"ABC" ────────────────┐
                      ├──────► "AB"
"DEF" ────────────────┘        "CDEF"
```

Le programme doit donc définir lui-même comment séparer les données.

Par exemple, IRC utilise des lignes terminées par :

```text
\r\n
```

---
