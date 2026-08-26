# IRC — Commandes et syntaxe du protocole

## 1. Qu'est-ce qu'une commande IRC ?

IRC (*Internet Relay Chat*) est un protocole de communication basé sur du **texte brut**.

Un client IRC communique avec le serveur en envoyant des lignes de texte. Chaque ligne contient une commande et éventuellement plusieurs paramètres.

Par exemple, lorsque l'utilisateur tape :

```text
/join #42
```

le client ne transmet pas forcément cette chaîne telle quelle au serveur. Il la transforme en une commande IRC valide :

```text
JOIN #42\r\n
```

Le serveur reçoit donc une ligne de protocole terminée par `CRLF`.

---

## 2. Structure générale d'une commande

La syntaxe générale d'un message IRC est :

```text
[:préfixe] COMMANDE [paramètre1] [paramètre2] ... [:dernier paramètre]\r\n
```

### Exemple

```text
PRIVMSG #general :Bonjour tout le monde !\r\n
```

On peut la décomposer ainsi :

```text
PRIVMSG
   │
   ├── #general
   │      └── paramètre
   │
   └── :Bonjour tout le monde !
          └── dernier paramètre
```

---

## 3. Les différentes parties

### Préfixe

Le préfixe est optionnel et commence par `:`.

```text
:nick!user@host PRIVMSG #general :Hello
```

Il permet notamment d'identifier l'origine d'un message.

Un **client** n'a généralement pas besoin d'ajouter son préfixe lorsqu'il envoie une commande au serveur.

Le serveur peut ensuite ajouter le préfixe lorsqu'il transmet le message à d'autres clients.

---

### Commande

La commande indique l'action à effectuer.

Exemples :

```text
NICK
USER
JOIN
PART
PRIVMSG
NOTICE
MODE
KICK
INVITE
TOPIC
QUIT
```

Les commandes IRC ne sont pas sensibles à la casse :

```text
JOIN #42
join #42
JoIn #42
```

sont conceptuellement la même commande.

En pratique, on utilise généralement les commandes en majuscules.

---

### Paramètres

Les paramètres sont séparés par des espaces.

Exemple :

```text
JOIN #42
```

Ici :

```text
JOIN
```

est la commande et :

```text
#42
```

est son paramètre.

Une commande peut avoir plusieurs paramètres :

```text
MODE #42 +o bob
```

Ici :

```text
MODE
#42
+o
bob
```

sont respectivement la commande et ses paramètres.

---

## 4. Le dernier paramètre

Le dernier paramètre possède une particularité importante.

Lorsqu'il contient des espaces, il doit être précédé de `:`.

Exemple :

```text
PRIVMSG #42 :Bonjour tout le monde !
```

Sans `:`, le serveur pourrait interpréter chaque mot comme un paramètre différent :

```text
PRIVMSG #42 Bonjour tout le monde !
```

Avec `:` :

```text
PRIVMSG #42 :Bonjour tout le monde !
```

tout ce qui se trouve après le `:` est considéré comme **un seul paramètre** :

```text
Bonjour tout le monde !
```

Ce paramètre est souvent appelé **trailing parameter**.

---

## 5. Fin de ligne : CRLF

Chaque message IRC doit se terminer par :

```text
\r\n
```

Cela correspond à :

- `\r` → **CR** (*Carriage Return*) → `0x0D`
- `\n` → **LF** (*Line Feed*) → `0x0A`

Donc :

```text
JOIN #42\r\n
```

correspond aux octets :

```text
J O I N   # 4 2 \r \n
```

Le serveur utilise cette séquence pour savoir où se termine une commande et où commence la suivante.

Par exemple, un client peut envoyer :

```text
NICK bob\r\n
USER bob 0 * :Bob\r\n
JOIN #42\r\n
```

Le serveur reçoit donc trois commandes distinctes.

---

## 6. Limite de taille

Dans le protocole IRC traditionnel, un message ne doit pas dépasser **512 octets**, `CRLF` compris.

Donc la partie réellement disponible pour la commande et ses paramètres est au maximum :

```text
510 octets
```

Le serveur doit donc être capable de gérer correctement les messages trop longs.

> Attention : la limite concerne des **octets**, pas forcément le nombre de caractères. Avec un encodage multioctet comme UTF-8, un caractère peut occuper plusieurs octets.

---

# 7. Commandes principales

## NICK

Permet de définir ou modifier le pseudonyme.

### Ce que tape l'utilisateur

```text
/nick Bob
```

### Ce qui est envoyé

```text
NICK Bob\r\n
```

---

## USER

Permet de fournir les informations utilisateur lors de la connexion.

Exemple :

```text
USER bob 0 * :Bob Example\r\n
```

Les paramètres sont :

```text
USER
bob
0
*
:Bob Example
```

---

## JOIN

Permet de rejoindre un canal.

### Exemple

```text
JOIN #42\r\n
```

Le canal est :

```text
#42
```

Plusieurs canaux peuvent également être spécifiés selon la syntaxe supportée par le serveur.

---

## PART

Permet de quitter un canal.

### Exemple

```text
PART #42\r\n
```

Avec un message :

```text
PART #42 :Je dois partir\r\n
```

---

## PRIVMSG

Permet d'envoyer un message à un utilisateur ou à un canal.

### Message sur un canal

```text
PRIVMSG #42 :Bonjour tout le monde !\r\n
```

### Message privé

```text
PRIVMSG Bob :Salut !\r\n
```

Le premier paramètre indique le destinataire.

Le dernier paramètre contient le message.

---

## NOTICE

Semblable à `PRIVMSG`, mais utilisée pour envoyer une notification.

```text
NOTICE Bob :Message de notification\r\n
```

---

## TOPIC

Permet de consulter ou modifier le sujet d'un canal.

### Consulter le sujet

```text
TOPIC #42\r\n
```

### Modifier le sujet

```text
TOPIC #42 :Bienvenue sur le canal\r\n
```

---

## MODE

Permet de modifier les modes d'un utilisateur ou d'un canal.

Exemple :

```text
MODE #42 +o Bob\r\n
```

Ici :

```text
#42
```

est le canal,

```text
+o
```

indique l'ajout du mode opérateur,

et :

```text
Bob
```

est l'utilisateur concerné.

---

## KICK

Permet à un opérateur d'expulser un utilisateur d'un canal.

```text
KICK #42 Bob :Comportement incorrect\r\n
```

Les paramètres sont :

```text
KICK
#42
Bob
:Comportement incorrect
```

---

## INVITE

Permet à un opérateur d'inviter un utilisateur sur un canal.

```text
INVITE Bob #42\r\n
```

---

## QUIT

Permet de se déconnecter du serveur.

Sans message :

```text
QUIT\r\n
```

Avec un message :

```text
QUIT :Goodbye\r\n
```

---

# 8. Commandes utilisateur vs protocole

Il est important de distinguer **la commande de l'interface client** et **la commande du protocole IRC**.

Par exemple :

```text
/join #42
```

Le `/` est une convention de l'application cliente.

Le protocole IRC reçoit :

```text
JOIN #42\r\n
```

Même chose pour :

```text
/nick Bob
```

qui devient :

```text
NICK Bob\r\n
```

En revanche, lorsqu'un utilisateur écrit directement :

```text
Salut tout le monde !
```

le client peut automatiquement transformer cela en :

```text
PRIVMSG #42 :Salut tout le monde !\r\n
```

Le serveur IRC ne reçoit donc pas forcément ce que l'utilisateur a directement tapé dans son interface.

---

# 9. Exemple complet

Supposons qu'un client se connecte au serveur et effectue les opérations suivantes :

```text
/nick Alice
```

Le serveur reçoit :

```text
NICK Alice\r\n
```

Puis :

```text
/user alice 0 * :Alice
```

devient :

```text
USER alice 0 * :Alice\r\n
```

Alice rejoint ensuite :

```text
/join #42
```

Le serveur reçoit :

```text
JOIN #42\r\n
```

Elle envoie ensuite :

```text
Bonjour Bob !
```

Le client transforme cela en :

```text
PRIVMSG #42 :Bonjour Bob !\r\n
```

Et lorsqu'elle quitte :

```text
/part #42
```

le serveur reçoit :

```text
PART #42\r\n
```

---

# 10. Résumé

Une commande IRC suit donc généralement ce modèle :

```text
[:prefix] COMMAND [parameters] [:trailing]\r\n
```

Exemple :

```text
PRIVMSG #42 :Hello world!\r\n
```

| Élément | Exemple | Rôle |
|---|---|---|
| Préfixe | `:Bob!user@host` | Identifie l'origine |
| Commande | `PRIVMSG` | Action demandée |
| Paramètre | `#42` | Destinataire |
| Trailing | `:Hello world!` | Texte contenant éventuellement des espaces |
| Fin | `\r\n` | Fin du message IRC |

Le serveur IRC doit donc principalement :

1. recevoir les octets avec `recv()` ;
2. détecter les lignes terminées par `\r\n` ;
3. parser la commande ;
4. parser ses paramètres ;
5. vérifier que la commande est valide ;
6. effectuer l'action demandée ;
7. envoyer une réponse IRC au client.

## Références

- [RFC 1459 — Internet Relay Chat Protocol](https://www.rfc-editor.org/rfc/rfc1459)
- [RFC 2812 — Internet Relay Chat: Client Protocol](https://www.rfc-editor.org/rfc/rfc2812)
