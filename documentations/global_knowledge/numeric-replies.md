# Numeric Replies

[REVENIR AU SOMMAIRE](README.md)

Le serveur IRC répond avec des **codes numériques**, c'est-à-dire des messages composés de trois chiffres.

Exemples :

```text
001
002
003
...
```

## Format

Chaque réponse numérique respecte le format suivant :

```text
:<nom_serveur> <code_numérique> <pseudo_cible> <paramètres> :<message>
```

## Catégories

### 001 à 099

Messages de bienvenue et informations initiales envoyés lors de la connexion au serveur.

### 200 à 399

Réponses de commandes réussies :

* statistiques ;
* listes de canaux ;
* informations utilisateurs.

### 400 à 599

Messages d'erreur :

* canal plein ;
* pseudonyme déjà utilisé ;
* privilèges insuffisants.

## Exemples

| Code |     Nom standard     | Description                                                     |
| :--: | :------------------: | :-------------------------------------------------------------- |
|  001 |     `RPL_WELCOME`    | Confirmation de la connexion réussie au réseau.                 |
|  322 |      `RPL_LIST`      | Réponse contenant les informations d'un canal dans une liste.   |
|  332 |      `RPL_TOPIC`     | Renvoie le sujet actuel du canal lors de l'entrée sur celui-ci. |
|  353 |    `RPL_NAMREPLY`    | Renvoie la liste des utilisateurs présents sur le canal.        |
|  401 |   `ERR_NOSUCHNICK`   | Le pseudonyme ou le canal spécifié n'existe pas.                |
|  433 |  `ERR_NICKNAMEINUSE` | Le pseudonyme choisi est déjà attribué à un autre utilisateur.  |
|  473 | `ERR_INVITEONLYCHAN` | Le canal est accessible uniquement sur invitation.              |

Pour `ft_irc`, il faut surtout connaître les **numeric replies exigées par le sujet**, plutôt que d'essayer d'implémenter toute la RFC.
