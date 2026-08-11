# `socket()` — Paramètres

[REVENIR AU SOMMAIRE](../../global_knowledge/README.md)




[SOCKETS Description](#socket)
- [1. `domain` — Famille d'adresses](sockets_domain.md)
- [2. `type` — Type de socket](sockets_type.md)
- [3. `protocol` — Protocole](socket_protocol.md)
- [4. `RETURN Value`](socket_return.md)

<br>
<br>
<br>
<br>

# `socket()`

## Description (Réseau)
```
C'est un point de terminaison virtuel qui permet à deux programmes de communiquer à travers un réseau (comme Internet) ou sur la même machine.Il associe une adresse IP et un numéro de port pour acheminer les données d'un logiciel émetteur vers un logiciel récepteur (par exemple, entre votre navigateur web et un serveur distant).On l'utilise beaucoup en programmation pour le modèle client-serveur (chargement de pages web, jeux en ligne, applications de messagerie).
```

La fonction `socket()` permet de créer un socket.

```cpp
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
```

Elle prend **3 paramètres** :


> FD =
> socket(
> - [`domain` — Famille d'adresses](sockets_domain.md)
> - [`type` — Type de socket](sockets_type.md)
> - [`protocol` — Protocole](socket_protocol.md)
>
> );


---
