# Handshake

[REVENIR AU SOMMAIRE](README.md)

Un **handshake** est une procédure permettant à deux systèmes de négocier ou d'établir les paramètres nécessaires avant ou pendant une communication.

Le mot signifie littéralement :

```text
poignée de main
```

Dans TCP :

```text
TCP Handshake
     │
     ▼
Établissement de la connexion
```

Avec TLS, il existe également un handshake :

```text
TCP Handshake
     │
     ▼
Connexion TCP
     │
     ▼
TLS Handshake
     │
     ▼
Connexion TLS sécurisée
     │
     ▼
Application
```

Donc :

```text
TCP handshake → établir TCP

TLS handshake → négocier la sécurité TLS
```

---
