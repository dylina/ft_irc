# Valeur de retour

`close()` retourne :

```text
0
```

si la fermeture réussit.

En cas d'erreur :

```text
-1
```

Exemple :

```cpp
if (close(fd) == -1)
{
    // erreur
}
```

---
