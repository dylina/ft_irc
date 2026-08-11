# `signal()`

### Rôle

Associe un signal à une fonction.

### Prototype

```cpp
void (*signal(int signum, void (*handler)(int)))(int);
```

### Exemple

```cpp
void handler(int sig)
{
    (void)sig;
    // arrêt du serveur
}

signal(SIGINT, handler);
```

---
