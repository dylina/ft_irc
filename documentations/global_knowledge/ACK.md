# ACK

[REVENIR AU SOMMAIRE](README.md)

**ACK** signifie :

```text
Acknowledgment
```

C'est un flag TCP utilisé pour indiquer qu'une donnée ou un numéro de séquence a été reçu.

Pendant le handshake :

```text
Client
  │
  │ SYN
  ▼
Serveur
  │
  │ SYN + ACK
  ▼
Client
  │
  │ ACK
  ▼
Serveur
```

La connexion est alors établie.

ACK est également utilisé pendant toute la communication TCP pour les accusés de réception.

---
