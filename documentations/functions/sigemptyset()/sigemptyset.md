# `sigemptyset()`

Initialise un ensemble de signaux vide.

```cpp
int sigemptyset(sigset_t *set);
```

```cpp
sigset_t set;

sigemptyset(&set);
```

Résultat :

```text
{}
```

---
