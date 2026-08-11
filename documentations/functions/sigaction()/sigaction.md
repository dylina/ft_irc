# `sigaction()`

### Rôle

Permet de configurer plus précisément le comportement d'un signal.

### Prototype

```cpp
int sigaction(
    int signum,
    const struct sigaction *act,
    struct sigaction *oldact
);
```

### Exemple

```cpp
struct sigaction sa;

memset(&sa, 0, sizeof(sa));

sa.sa_handler = handler;

sigemptyset(&sa.sa_mask);

sigaction(SIGINT, &sa, NULL);
```

### Pourquoi préférer `sigaction()` ?

Parce qu'elle offre davantage de contrôle que [`signal()`](../signal()/signal.md).

---
