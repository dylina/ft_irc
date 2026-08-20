# Structure `sockaddr_in` en C/C++

La structure `sockaddr_in` sert à stocker une **adresse IPv4** et un **numéro de port** pour les sockets Internet.

## Composition

| Champ        | Rôle                                                                                               |
| ------------ | -------------------------------------------------------------------------------------------------- |
| `sin_family` | Famille d'adresses. Pour IPv4, vaut `AF_INET`...                                                     |
| `sin_port`   | Numéro de port réseau, généralement converti avec `htons()`.                                       |
| `sin_addr`   | Adresse IPv4, stockée dans une structure `in_addr`.                                                |
| `sin_zero`   | 8 octets de remplissage, généralement mis à zéro, afin d'assurer la compatibilité avec `sockaddr`. |

### Exemple

```cpp
struct sockaddr_in myaddr;

myaddr.sin_family = AF_INET;
myaddr.sin_port = htons(1050);
myaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
```

## Utilisation avec les fonctions système

Les fonctions réseau comme `bind()` ou `connect()` attendent un pointeur vers une structure générique :

```cpp
struct sockaddr *
```

On passe donc un pointeur vers `sockaddr_in` en effectuant un cast :

```cpp
(struct sockaddr *)&myaddr
```

Par exemple avec `bind()` :

```cpp
bind(
    socket_fd,
    (struct sockaddr *)&myaddr,
    sizeof(myaddr)
);
```

`struct sockaddr` sert ainsi d'interface générique, tandis que `sockaddr_in` contient les informations spécifiques à une adresse **IPv4**.

## À retenir

* `sockaddr_in` → adresse **IPv4** + port.
* `sin_family` → `AF_INET`.
* `sin_port` → port réseau avec `htons()`.
* `sin_addr` → adresse IPv4.
* `bind()` / `connect()` → attendent un `struct sockaddr *`.
* Il faut donc convertir `sockaddr_in *` en `sockaddr *`.

> Pour les détails techniques sous Linux : `man 7 ip` et `man 2 bind`.
