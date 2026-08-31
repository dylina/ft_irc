# `fstat()`

### Rôle

Récupère les informations d'un file descriptor.

### Prototype

```cpp
int fstat(
    int fd,
    struct stat *statbuf
);
```

### Exemple

```cpp
struct stat st;

fstat(fd, &st);

std::cout << st.st_size;
```

Informations disponibles notamment :

```text
st_size
st_mode
st_uid
st_gid
st_mtime
```

---
