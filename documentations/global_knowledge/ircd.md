# Serveur IRC / IRCd

[REVENIR AU SOMMAIRE](README.md)

**IRCd** signifie :

```text
Internet Relay Chat daemon
```

C'est le logiciel qui implémente le protocole IRC côté serveur.

Il permet notamment de :

* accepter les connexions des clients ;
* authentifier les utilisateurs ;
* gérer les pseudonymes ;
* créer et gérer les salons ;
* transmettre les messages ;
* gérer les modes ;
* gérer les opérateurs ;
* éventuellement communiquer avec d'autres serveurs IRC.

Architecture :

```text
                Réseau IRC
                    │
        ┌───────────┴───────────┐
        │                       │
     IRCd A                  IRCd B
        │                       │
   ┌────┴────┐             ┌────┴────┐
   │         │             │         │
Client A  Client B       Client C  Client D
```

Un **IRCd** n'est pas un bot.

```text
IRCd
 │
 └── Serveur IRC

Bot IRC
 │
 └── Client automatisé
```

---
