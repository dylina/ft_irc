# Robots IRC

[REVENIR AU SOMMAIRE](README.md)

Un **bot IRC** est un programme qui se connecte à un serveur IRC comme un client et effectue automatiquement certaines actions.

Architecture :

```text
          Serveur IRC
              │
       ┌──────┼──────┐
       ▼      ▼      ▼
    Client  Client   Bot
```

Un bot peut :

```text
répondre à une commande
modérer un canal
envoyer des messages
surveiller des événements
gérer des utilisateurs
```

Exemple :

```text
Utilisateur :
!weather Marseille

Bot :
Température : 25°C
```

Un bot IRC n'est donc pas un IRCd.

```text
IRCd → serveur

Bot → client automatisé
```

---
