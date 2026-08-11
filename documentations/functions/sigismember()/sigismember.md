# `sigismember()`

Teste si un signal est présent.

```cpp
int sigismember(
    const sigset_t *set,
    int signum
);
```

Exemple :

```cpp
if (sigismember(&set, SIGINT))
{
    // SIGINT présent
}
```

Retour :

```text
1  → présent
0  → absent
-1 → erreur
```

---
