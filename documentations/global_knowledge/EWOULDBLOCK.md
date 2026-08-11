# EWOULDBLOCK

`EWOULDBLOCK` signifie qu'une opération bloquerait alors que le descripteur est configuré pour ne pas bloquer.

C'est particulièrement important avec :

```text
O_NONBLOCK
```

Exemple :

```text
recv()
  │
  ├── données disponibles
  │       │
  │       └── réception
  │
  └── aucune donnée
          │
          └── EAGAIN / EWOULDBLOCK
```

Sur de nombreux systèmes Unix :

```text
EAGAIN == EWOULDBLOCK
```

Mais le code portable peut tester les deux.

```cpp
if (errno == EAGAIN || errno == EWOULDBLOCK)
{
    // Pas prêt pour le moment
}
```

---
