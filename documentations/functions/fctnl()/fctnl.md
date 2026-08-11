# `fcntl()`

### Rôle

Permet de manipuler un file descriptor.

### Prototype

```cpp
int fcntl(
    int fd,
    int cmd,
    ...
);
```

Il possède énormément d'utilisations.

Pour un serveur réseau, la plus importante est :

```cpp
fcntl(
    fd,
    F_SETFL,
    O_NONBLOCK
);
```

Cela met le socket en [**non-bloquant**](../../global_knowledge/bloquant-non-bloquant.md).

