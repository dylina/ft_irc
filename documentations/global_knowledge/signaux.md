# 🚦 Signaux

Les **signaux** permettent au système d'exploitation de prévenir un programme qu'un événement particulier vient de se produire.

Un signal est une notification envoyée à un processus.

---

# Sommaire

1. [Principe](#principe)
2. [Fonctionnement](#fonctionnement)
3. [Exemple avec `CTRL+C`](#exemple-avec-ctrlc)
4. [Les signaux courants](#les-signaux-courants)
5. [`SIGINT`](#sigint)
6. [`SIGTERM`](#sigterm)
7. [`SIGQUIT`](#sigquit)
8. [`SIGPIPE`](#sigpipe)
9. [Gestion d'un signal](#gestion-dun-signal)
10. [Résumé](#résumé)

---

# Principe

Un signal permet à un processus de recevoir une notification lorsqu'un événement particulier se produit.

On peut représenter le fonctionnement ainsi :

```text
Événement
    │
    ▼
Système d'exploitation
    │
    │ signal
    ▼
Processus
```

Le processus peut alors réagir au signal.

---

# Fonctionnement

Un signal est identifié par un nom et un numéro.

Par exemple :

```text
SIGINT
```

correspond au signal envoyé généralement lorsqu'un utilisateur appuie sur :

```text
CTRL+C
```

Le fonctionnement peut être représenté ainsi :

```text
Utilisateur
    │
    │ CTRL+C
    ▼
Système d'exploitation
    │
    │ SIGINT
    ▼
Programme
```

Le programme peut alors effectuer une action lorsqu'il reçoit le signal.

---

# Exemple avec `CTRL+C`

Lorsqu'un programme est lancé dans un terminal :

```text
./server
```

et que l'utilisateur appuie sur :

```text
CTRL+C
```

le terminal demande au système d'envoyer `SIGINT` au processus.

```text
┌──────────────┐
│  Utilisateur │
└──────┬───────┘
       │
       │ CTRL+C
       ▼
┌────────────────────┐
│ Système d'exploitation │
└─────────┬──────────┘
          │
          │ SIGINT
          ▼
     ┌──────────┐
     │ Serveur  │
     └──────────┘
```

Par défaut, `SIGINT` provoque l'arrêt du programme.

Un programme peut cependant installer une gestion spécifique du signal.

---

# Les signaux courants

| Signal    | Signification                      | Déclenchement courant         |
| :-------- | :--------------------------------- | :---------------------------- |
| `SIGINT`  | Interruption                       | `CTRL+C`                      |
| `SIGTERM` | Demande d'arrêt                    | arrêt demandé au processus    |
| `SIGQUIT` | Quitter                            | `CTRL+\`                      |
| `SIGPIPE` | Écriture vers une connexion fermée | communication interrompue     |
| `SIGKILL` | Arrêt immédiat                     | demande forcée                |
| `SIGSTOP` | Suspension                         | arrêt temporaire du processus |

---

# `SIGINT`

`SIGINT` signifie **Signal Interrupt**.

Il est généralement envoyé lorsqu'un utilisateur appuie sur :

```text
CTRL+C
```

Exemple :

```text
./server

CTRL+C
  │
  ▼
SIGINT
  │
  ▼
serveur
```

Il est couramment utilisé pour demander à un programme de s'arrêter proprement.

---

# `SIGTERM`

`SIGTERM` signifie **Signal Terminate**.

Il indique au processus qu'il doit terminer son exécution.

Contrairement à `SIGKILL`, le programme peut gérer `SIGTERM` et effectuer certaines opérations avant de s'arrêter.

```text
SIGTERM
   │
   ▼
Programme
   │
   ├── termine son travail
   ├── libère ses ressources
   └── s'arrête
```

---

# `SIGQUIT`

`SIGQUIT` demande également l'arrêt d'un processus.

Dans un terminal, il est généralement envoyé avec :

```text
CTRL+\
```

Contrairement à `SIGINT`, son comportement par défaut peut notamment produire un **core dump**.

---

# `SIGPIPE`

`SIGPIPE` peut être généré lorsqu'un programme tente d'écrire dans une connexion dont l'autre extrémité a été fermée.

Dans un programme utilisant des sockets, cela peut arriver lorsqu'un client se déconnecte alors que le serveur tente encore de lui envoyer des données.

```text
Serveur
   │
   │ écriture
   ▼
Socket
   X
Client déconnecté
```

Le système peut alors envoyer :

```text
SIGPIPE
```

---

# `SIGKILL`

`SIGKILL` demande l'arrêt immédiat d'un processus.

Le processus **ne peut pas intercepter, ignorer ou gérer** `SIGKILL`.

```text
SIGKILL
   │
   ▼
Processus
   │
   ▼
Arrêt immédiat
```

Il est donc différent de signaux comme `SIGINT` ou `SIGTERM`, qui peuvent être gérés par le programme.

---

# `SIGSTOP`

`SIGSTOP` suspend l'exécution d'un processus.

Le processus n'est pas terminé : son exécution est simplement arrêtée.

```text
Processus
    │
    │ SIGSTOP
    ▼
Processus suspendu
```

`SIGCONT` permet ensuite de reprendre son exécution.

---

# Gestion d'un signal

Un programme peut définir la manière dont il réagit à certains signaux.

Par exemple, il peut recevoir `SIGINT` et effectuer une action particulière au lieu de simplement s'arrêter immédiatement.

```text
SIGINT
   │
   ▼
Gestionnaire du signal
   │
   ▼
Action du programme
```

Dans un serveur, cela peut notamment permettre de détecter une demande d'arrêt et de terminer proprement son exécution.

---

# Signaux et serveur IRC

Dans un serveur IRC, les signaux peuvent être utilisés pour gérer certains événements liés au processus.

Par exemple :

```text
CTRL+C
   │
   ▼
SIGINT
   │
   ▼
Serveur IRC
   │
   ▼
Arrêt du serveur
```

L'intérêt est de pouvoir réagir à une demande d'arrêt au lieu de laisser le processus être interrompu sans traitement.

---

# Résumé

| Élément    | Rôle                                |
| :--------- | :---------------------------------- |
| **Signal** | Notification envoyée à un processus |
| `SIGINT`   | Interruption, généralement `CTRL+C` |
| `SIGTERM`  | Demande d'arrêt                     |
| `SIGQUIT`  | Demande de quitter                  |
| `SIGPIPE`  | Écriture vers une connexion fermée  |
| `SIGKILL`  | Arrêt immédiat et non interceptable |
| `SIGSTOP`  | Suspension du processus             |
| `SIGCONT`  | Reprise d'un processus suspendu     |

### À retenir

```text
Événement
    │
    ▼
Système
    │
    │ signal
    ▼
Programme
    │
    ▼
Réaction
```

Un **signal est donc un mécanisme de notification entre le système d'exploitation et un processus**.

