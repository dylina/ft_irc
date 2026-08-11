# 23. SYN-ACK

[REVENIR AU SOMMAIRE](README.md)

Le serveur répond au SYN avec un segment contenant :

```text
SYN = 1
ACK = 1
```

D'où :

```text
SYN + ACK
```

Schéma :

```text
Client                         Serveur

  │──────── SYN ───────────────►│
  │                             │
  │◄────── SYN + ACK ──────────│
```

Le serveur :

1. accepte la demande ;
2. accuse réception du SYN du client ;
3. initialise son propre numéro de séquence.

---
