# Commandes IRC

[REVENIR AU SOMMAIRE](README.md)

Pour tous les exemples :

```text
{CHANNEL} = #42
{USER} = Bob
{MESSAGE} = Hello World !
```

## Connexion

### PASS

Permet de fournir le mot de passe nécessaire à l'enregistrement.

```text
PASS
```

### NICK

Le client choisit son pseudonyme.

```text
NICK {USER}
```

### USER

Fournit les informations utilisateur nécessaires à l'enregistrement.

```text
USER
```

### CAP

Permet la gestion des capacités du client.

```text
CAP
```

## Canaux

### JOIN

Permet de rejoindre un canal.

```text
JOIN {CHANNEL}
```

### PART

Permet de quitter un canal sans se déconnecter du serveur.

```text
PART {CHANNEL}
```

### KICK

Un opérateur expulse un utilisateur du canal.

```text
KICK {CHANNEL} {USER} :{MESSAGE}
```

### INVITE

Permet à un opérateur d'inviter un utilisateur dans un canal.

```text
INVITE {USER} {CHANNEL}
```

### TOPIC

Permet de lire le sujet du canal.

```text
TOPIC {CHANNEL}
```

Pour modifier le sujet :

```text
TOPIC {CHANNEL} :{MESSAGE}
```

Avec le mode `+t`, seuls certains utilisateurs peuvent le modifier.

### MODE

Permet de modifier les modes d'un utilisateur ou d'un canal.

```text
MODE
```

## Communication

### PRIVMSG

Envoie un message à un utilisateur :

```text
PRIVMSG {USER} :{MESSAGE}
```

Le serveur transmet le message à `{USER}`.

Pour envoyer un message à un canal :

```text
PRIVMSG {CHANNEL} :{MESSAGE}
```

Le serveur doit envoyer le message aux autres membres du canal.

### NOTICE

Permet d'envoyer une notification IRC.

```text
NOTICE
```

## Déconnexion

### QUIT

Permet de quitter le serveur.

```text
QUIT :{MESSAGE}
```

La connexion TCP est alors fermée.
