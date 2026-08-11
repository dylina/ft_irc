# Proxy

Un **proxy** est un intermédiaire entre un client et une destination.

```text
Client
   │
   ▼
Proxy
   │
   ▼
Serveur
```

Au lieu de communiquer directement :

```text
Client ─────────► Serveur
```

le client communique avec :

```text
Client ─────────► Proxy ─────────► Serveur
```

Le proxy peut notamment être utilisé pour :

* filtrage ;
* cache ;
* journalisation ;
* contrôle d'accès ;
* authentification ;
* routage ;
* répartition de charge.

Un proxy HTTP fonctionne au niveau applicatif.

Il ne faut pas le confondre avec un routeur ou un NAT.

---
