# `sigaddset()`

Ajoute un signal.

```cpp
int sigaddset(
    sigset_t *set,
    int signum
);
```

Exemple :

```cpp
sigemptyset(&set);
sigaddset(&set, SIGINT);
```

Résultat :

```text
{ SIGINT }
```

---
