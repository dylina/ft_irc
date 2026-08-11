
# EAGAIN

`EAGAIN` est une valeur d'`errno` indiquant qu'une opération ne peut pas être effectuée immédiatement sans bloquer.

C'est particulièrement fréquent avec un socket en mode non bloquant.

Exemple :

```cpp
recv(sockfd, buffer, sizeof(buffer), 0);
```

Si aucune donnée n'est disponible :

```text
recv()
  │
  ▼
-1
  │
  ▼
errno == EAGAIN
```

Cela signifie :

```text
"Il n'y a rien à faire maintenant.
Réessaie lorsque le socket sera prêt."
```

Ce n'est généralement **pas une erreur fatale** dans ce contexte.

On peut utiliser :

```text
select()
poll()
epoll()
kqueue()
```

pour attendre que le socket soit prêt.

---
