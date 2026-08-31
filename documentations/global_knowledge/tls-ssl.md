# TLS / SSL

[REVENIR AU SOMMAIRE](README.md)

**SSL** et **TLS** sont des protocoles permettant de sécuriser une communication.

SSL est l'ancien protocole.

TLS l'a remplacé.

```text
SSL
 │
 └── ancien

TLS
 │
 └── version moderne
```

Aujourd'hui, lorsqu'on parle de « SSL » dans le contexte moderne, il s'agit généralement de TLS.

TLS fournit notamment :

### Confidentialité

Les données sont chiffrées :

```text
"Bonjour"
    │
    ▼
Chiffrement
    │
    ▼
"8fA$...?"
```

### Intégrité

TLS permet de détecter une modification des données.

### Authentification

TLS permet notamment au client de vérifier l'identité du serveur grâce aux certificats.

Architecture :

```text
Application
     │
     ▼
    TLS
     │
     ▼
    TCP
     │
     ▼
    IP
```

Pour IRC :

```text
IRC
 │
 ▼
TLS
 │
 ▼
TCP
```

---
