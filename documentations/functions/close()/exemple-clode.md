# Exemple

```cpp
#include <unistd.h>

int main()
{
    int fd = 3;

    if (close(fd) == -1)
        return 1;

    return 0;
}
```

Dans le cas d'une socket :

```cpp
#include <sys/socket.h>
#include <unistd.h>

int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd == -1)
        return 1;

    close(fd);

    return 0;
}
```

---
