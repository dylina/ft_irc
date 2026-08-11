# 🔀 Multiplexage

Le **multiplexage I/O** permet à un programme de surveiller **plusieurs file descriptors en même temps** sans rester bloqué sur un seul.

C'est particulièrement utile pour les serveurs réseau comme un serveur IRC.

---

## 📌 Principe

Sans multiplexage :

```text
socket 1
   │
 recv()
   │
   ▼
attente...
   │
   ▼
socket 2
   │
 recv()
   │
   ▼
attente...
```

Le programme doit gérer les sockets une par une.

Avec le multiplexage :

```text
          ┌── socket 1
          │
          ├── socket 2
poll() ───┼── socket 3
          │
          ├── socket 4
          │
          └── socket 5
```

Le programme demande au système :

> « Dis-moi quels sockets sont prêts à être utilisés. »

---

## 🎯 Pourquoi utiliser le multiplexage ?

Un serveur IRC peut avoir plusieurs clients connectés simultanément.

```text
              Serveur IRC
                  │
       ┌──────────┼──────────┐
       │          │          │
     Bob         Alice      Charlie
       │          │          │
    socket       socket     socket
       │          │          │
       └──────────┼──────────┘
                  │
                poll()
```

Le serveur n'a pas besoin d'attendre Bob avant de pouvoir traiter Alice ou Charlie.

---

## ⚙️ Fonctionnement

Le programme possède plusieurs file descriptors :

```text
fd 3 → socket serveur
fd 4 → client Bob
fd 5 → client Alice
fd 6 → client Charlie
```

Il les donne à un mécanisme de multiplexage :

```text
        ┌───────────────┐
fd 3 ──►│               │
fd 4 ──►│     poll()    │
fd 5 ──►│               │
fd 6 ──►│               │
        └───────┬───────┘
                │
                ▼
       sockets prêtes
```

Le système indique ensuite quels file descriptors peuvent effectuer une opération sans bloquer.

---

## 📡 Exemple avec un serveur IRC

Le serveur possède une socket d'écoute :

```text
fd 3
│
└── socket serveur
```

Puis plusieurs clients :

```text
fd 4 → Bob
fd 5 → Alice
fd 6 → Charlie
```

Le multiplexage permet de surveiller tout le monde :

```text
                 poll()
                   │
        ┌──────────┼──────────┐
        ▼          ▼          ▼
      fd 4       fd 5       fd 6
      Bob        Alice     Charlie
```

Si Alice envoie un message :

```text
Alice
  │
  ▼
socket fd 5
  │
  ▼
poll()
  │
  ▼
fd 5 prêt en lecture
  │
  ▼
recv()
```

Le serveur peut alors traiter Alice sans attendre les autres clients.

---

## 🔄 Boucle d'événements

Un serveur utilisant le multiplexage fonctionne généralement avec une boucle :

```text
        ┌──────────────────────┐
        │                      │
        │       poll()         │
        │                      │
        └──────────┬───────────┘
                   │
                   ▼
            Descripteurs prêts
                   │
          ┌────────┼────────┐
          ▼        ▼        ▼
        accept()  recv()   send()
          │        │        │
          └────────┼────────┘
                   │
                   ▼
                poll()...
```

Le serveur reste continuellement capable de surveiller ses connexions.

---

## 🧩 Principaux mécanismes

Plusieurs mécanismes permettent de faire du multiplexage I/O :

| Fonction   | Système      | Principe                                          |
| ---------- | ------------ | ------------------------------------------------- |
| `select()` | Unix / Linux | Surveille plusieurs file descriptors              |
| `poll()`   | Unix / Linux | Surveille plusieurs file descriptors              |
| `epoll()`  | Linux        | Multiplexage efficace pour beaucoup de connexions |
| `kqueue()` | BSD / macOS  | Multiplexage basé sur les événements              |

Pour un projet **C++98 / 42**, `poll()` est particulièrement courant.

---

## 📌 `poll()`

`poll()` permet de surveiller plusieurs file descriptors.

Son principe :

```text
poll()
  │
  ▼
attend un événement
  │
  ├── lecture disponible
  ├── écriture possible
  ├── erreur
  └── fermeture
```

Une fois qu'un événement est détecté, `poll()` retourne et le programme traite le file descriptor concerné.

---

## 📥 Lecture

On peut surveiller si des données sont disponibles :

```text
socket
  │
  ▼
poll()
  │
  ▼
POLLIN
  │
  ▼
recv()
```

`POLLIN` indique qu'une lecture est possible sans bloquer.

---

## 📤 Écriture

On peut également surveiller si une socket est prête à recevoir des données :

```text
socket
  │
  ▼
poll()
  │
  ▼
POLLOUT
  │
  ▼
send()
```

`POLLOUT` indique qu'une écriture peut être effectuée sans bloquer.

---

## ⚠️ Erreurs et déconnexions

Le multiplexage permet également de détecter certains événements :

```text
POLLIN
   │
   └── données disponibles

POLLOUT
   │
   └── écriture possible

POLLERR
   │
   └── erreur

POLLHUP
   │
   └── connexion fermée

POLLNVAL
   │
   └── file descriptor invalide
```

---

## 🔗 Multiplexage et sockets non bloquantes

Le multiplexage est souvent utilisé avec des sockets configurées en **non-bloquant**.

```text
        Sockets
           │
           ▼
      non-bloquantes
           │
           ▼
         poll()
           │
           ▼
    socket prête ?
       │       │
      oui     non
       │       │
       ▼       ▼
     recv()   retour
```

Cela permet d'éviter qu'une opération sur une socket bloque tout le serveur.

---

## 🆚 Sans multiplexage / avec multiplexage

### Sans multiplexage

```text
Client A
   │
 recv()
   │
 attente
   │
   ▼
Client B
   │
 recv()
   │
 attente
```

Un client peut empêcher le serveur de traiter correctement les autres.

### Avec multiplexage

```text
             poll()
                │
       ┌────────┼────────┐
       ▼        ▼        ▼
     Client A Client B Client C
       │        │        │
       ▼        ▼        ▼
     prêt ?   prêt ?   prêt ?
```

Le serveur traite uniquement les sockets qui nécessitent une intervention.

---

## 🧠 À retenir

```text
Multiplexage
     │
     ▼
Surveiller plusieurs file descriptors
     │
     ▼
Attendre un événement
     │
     ▼
Identifier les sockets prêtes
     │
     ▼
Traiter uniquement celles-ci
```

> **Le multiplexage permet à un seul programme de gérer plusieurs communications simultanément sans rester bloqué sur une seule socket.**
