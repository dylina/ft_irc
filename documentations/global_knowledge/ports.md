# Conversion des adresses et ports

Les ordinateurs et les réseaux peuvent utiliser des **ordres de bytes différents**.

On distingue principalement :

```text
HOST BYTE ORDER
NETWORK BYTE ORDER
```

---

# Sommaire

1. [Host Byte Order](#host-byte-order)
2. [Network Byte Order](#network-byte-order)
3. [Pourquoi convertir](#pourquoi-convertir)
4. [Résumé](#résumé)

---

# Host Byte Order

Le **Host Byte Order** est l'ordre dans lequel les données sont stockées par l'ordinateur.

Cet ordre dépend de l'architecture du système.

Les deux principaux ordres sont :

```text
Big Endian
Little Endian
```

Un même nombre peut donc être représenté différemment en mémoire selon l'architecture utilisée.

---

# Network Byte Order

Le **Network Byte Order** est l'ordre utilisé pour représenter les données sur le réseau.

Les protocoles réseau utilisent le **Big Endian** comme ordre standard.

```text
Network Byte Order
        │
        ▼
    Big Endian
```

Cela permet aux différentes machines de communiquer correctement, même si elles utilisent des architectures différentes.

---

# Pourquoi convertir

Lorsqu'un programme manipule une valeur provenant du réseau, il peut être nécessaire de convertir cette valeur entre :

```text
HOST BYTE ORDER
        ↕
NETWORK BYTE ORDER
```

C'est particulièrement important pour les **ports** et certaines valeurs présentes dans les structures réseau.

Exemple :

```text
Programme
   │
   │ Host Byte Order
   ▼
Conversion
   │
   │ Network Byte Order
   ▼
Réseau
```

Lorsqu'une donnée revient du réseau, la conversion inverse peut être nécessaire :

```text
Réseau
   │
   │ Network Byte Order
   ▼
Conversion
   │
   │ Host Byte Order
   ▼
Programme
```

---

# Résumé

| Ordre                  | Utilisation                                            |
| :--------------------- | :----------------------------------------------------- |
| **Host Byte Order**    | Ordre utilisé par la machine                           |
| **Network Byte Order** | Ordre standard utilisé sur le réseau                   |
| **Big Endian**         | Ordre utilisé par le Network Byte Order                |
| **Little Endian**      | Ordre utilisé par de nombreuses architectures modernes |

### À retenir

```text
HOST BYTE ORDER
       │
       │ conversion
       ▼
NETWORK BYTE ORDER
       │
       │ réseau
       ▼
NETWORK BYTE ORDER
       │
       │ conversion
       ▼
HOST BYTE ORDER
```

Le **Host Byte Order** dépend de la machine, tandis que le **Network Byte Order** fournit un ordre commun permettant aux machines de communiquer correctement.
