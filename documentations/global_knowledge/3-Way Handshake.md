# 21. 3-Way Handshake

[REVENIR AU SOMMAIRE](README.md)

Le **TCP 3-Way Handshake** permet d'établir une connexion TCP.

Il comporte trois étapes :

```text
Client                         Serveur

  │                               │
  │──────── SYN ─────────────────►│
  │                               │
  │◄────── SYN + ACK ─────────────│
  │                               │
  │──────── ACK ─────────────────►│
  │                               │
  │       CONNEXION ÉTABLIE       │
```

### Étape 1

```text
SYN
```

Le client demande l'établissement d'une connexion.

### Étape 2

```text
SYN + ACK
```

Le serveur accepte et accuse réception du SYN.

### Étape 3

```text
ACK
```

Le client confirme.

La connexion TCP peut ensuite transporter des données.

---
