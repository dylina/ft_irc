# 11. Bouncer / BNC

[REVENIR AU SOMMAIRE](README.md)

**BNC** signifie généralement :

```text
Bouncer
```

Dans le contexte IRC, un BNC est un intermédiaire qui maintient une connexion IRC pour l'utilisateur.

Architecture :

```text
Utilisateur
     │
     │ connexion
     ▼
    BNC
     │
     │ connexion persistante
     ▼
Serveur IRC
```

L'intérêt principal est que le BNC peut rester connecté même si le client de l'utilisateur se déconnecte.

```text
Client
   X
   │
   │ déconnexion
   ▼
BNC ───────────► IRC
```

Lorsque le client revient :

```text
Client
   │
   ▼
BNC
   │
   ▼
IRC
```

Le BNC peut alors éventuellement fournir :

* historique ;
* logs ;
* gestion de la connexion ;
* notifications ;
* cloaking ou modification de certaines informations visibles.

Un BNC est donc différent d'un proxy générique, même s'il joue également un rôle d'intermédiaire.

---
