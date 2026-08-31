# Protocole de communication

[REVENIR AU SOMMAIRE](README.md)

Un **protocole de communication** est un ensemble de règles permettant à plusieurs systèmes de communiquer correctement.

Il définit notamment :

```text
Format des données
Ordre des messages
Signification des messages
Gestion des erreurs
Établissement de connexion
Fin de communication
...
```

Exemple :

```text
Client
  │
  │ protocole IRC
  │
  ▼
Serveur
```

Les deux parties doivent comprendre les mêmes règles.

Sans protocole commun :

```text
Client → "ABC"
Serveur → ???
```

Avec un protocole :

```text
Client → PRIVMSG #42 :Bonjour
Serveur → comprend la commande
```

---
