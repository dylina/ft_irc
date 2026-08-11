# Buffer

Un **buffer**, ou tampon, est une zone de mémoire temporaire permettant de stocker des données avant leur traitement.

Exemple avec un socket :

```text
Réseau
   │
   ▼
┌──────────────┐
│    BUFFER    │
├──────────────┤
│ A B C D ...  │
└──────┬───────┘
       │
       ▼
   Programme
```

Dans un serveur IRC, un buffer peut contenir des données reçues mais pas encore entièrement traitées.

Exemple :

```text
Réception :

"PRIVMSG #42 :Hello\r\n"
```

Le serveur peut stocker les données dans un buffer puis chercher :

```text
\r\n
```

pour déterminer où se termine la commande IRC.

Avec TCP, c'est particulièrement important car TCP fournit un **flux**, pas des messages séparés.

---
