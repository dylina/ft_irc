# `sigdelset()`

Supprime un signal.

```cpp
int sigdelset(
    sigset_t *set,
    int signum
);
```

```cpp
sigdelset(&set, SIGINT);
```

---
