# `struct pollfd`

Structure utilisée avec `poll()` :

```cpp
struct pollfd
{
    int   fd;
    short events;
    short revents;
};
```

### `fd`

Le file descriptor à surveiller.

### `events`

Ce que tu souhaites surveiller.

Exemple :

```cpp
fds[i].events = POLLIN;
```

### `revents`

Ce qui s'est réellement produit.

Exemple :

```cpp
if (fds[i].revents & POLLIN)
{
    // données disponibles
}
```

---
[struct pollfd](struct%20pollfd.md)
