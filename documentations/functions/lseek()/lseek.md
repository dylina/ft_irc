# `lseek()`

### Rôle

Déplace la position de lecture/écriture d'un fichier.

### Prototype

```cpp
off_t lseek(
    int fd,
    off_t offset,
    int whence
);
```

### `whence`

```text
SEEK_SET
    depuis le début

SEEK_CUR
    depuis la position actuelle

SEEK_END
    depuis la fin
```

### Exemple

```cpp
lseek(fd, 0, SEEK_SET);
```

Retourne au début.

---
