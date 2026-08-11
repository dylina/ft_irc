# 🚚 Couche Transport

La **couche transport** est la **couche 4 du modèle OSI**.

Elle se situe entre la couche **réseau** et les couches supérieures :

```text
┌──────────────────────────────┐
│ 7. Application              │
├──────────────────────────────┤
│ 6. Présentation             │
├──────────────────────────────┤
│ 5. Session                  │
├──────────────────────────────┤
│ 4. Transport      ◄─────────┤
├──────────────────────────────┤
│ 3. Réseau                   │
├──────────────────────────────┤
│ 2. Liaison                  │
├──────────────────────────────┤
│ 1. Physique                 │
└──────────────────────────────┘
```

Son rôle général est de permettre une **communication entre applications situées sur des machines différentes**.

Les deux principaux protocoles de cette couche sont :

```text
TCP
UDP
```

---

# 📑 Sommaire

1. [Rôle de la couche transport](#1-rôle-de-la-couche-transport)
2. [Pourquoi une couche transport ?](#2-pourquoi-une-couche-transport)
3. [Communication de bout en bout](#3-communication-de-bout-en-bout)
4. [Les ports](#4-les-ports)
5. [TCP](#5-tcp)
6. [UDP](#6-udp)
7. [TCP vs UDP](#7-tcp-vs-udp)
8. [Segmentation](#8-segmentation)
9. [Fiabilité](#9-fiabilité)
10. [Contrôle de flux](#10-contrôle-de-flux)
11. [Contrôle de congestion](#11-contrôle-de-congestion)
12. [Numéros de séquence](#12-numéros-de-séquence)
13. [ACK](#13-ack)
14. [Retransmission](#14-retransmission)
15. [Connexion TCP](#15-connexion-tcp)
16. [Three-Way Handshake](#16-three-way-handshake)
17. [Fermeture TCP](#17-fermeture-tcp)
18. [Socket et couche transport](#18-socket-et-couche-transport)
19. [Exemple avec FT_IRC](#19-exemple-avec-ft_irc)
20. [Encapsulation](#20-encapsulation)
21. [Résumé](#21-résumé)

---

# 1. Rôle de la couche transport

La couche transport permet à deux **applications** de communiquer à travers un réseau.

Elle s'occupe notamment de :

* identifier les applications avec les **ports** ;
* transporter les données ;
* découper les données en morceaux ;
* remettre les données dans le bon ordre ;
* détecter certaines pertes ;
* retransmettre certaines données ;
* contrôler le débit ;
* gérer une connexion selon le protocole utilisé.

On peut simplifier ainsi :

```text
Application A
     │
     ▼
Couche transport
     │
     ▼
Réseau
     │
     ▼
Couche transport
     │
     ▼
Application B
```

La couche transport fournit donc une communication **de bout en bout** entre les applications.

---

# 2. Pourquoi une couche transport ?

La couche réseau, notamment IP, permet principalement de faire parvenir des paquets d'une machine à une autre.

Mais IP ne s'occupe pas directement de savoir :

```text
Quelle application doit recevoir les données ?
Les données sont-elles arrivées ?
Sont-elles dans le bon ordre ?
Faut-il les retransmettre ?
À quel rythme envoyer les données ?
```

C'est notamment le rôle de la couche transport.

Exemple :

```text
Client
192.168.1.10
    │
    │ IP
    ▼
Serveur
192.168.1.20
```

IP sait atteindre :

```text
192.168.1.20
```

Mais cette machine peut avoir plusieurs applications :

```text
Serveur
│
├── Serveur Web
├── Serveur SSH
├── Serveur IRC
└── Serveur DNS
```

Il faut donc identifier **quelle application** doit recevoir les données.

C'est là qu'interviennent les **ports**.

---

# 3. Communication de bout en bout

La couche transport permet une communication entre deux processus.

Exemple :

```text
Client
192.168.1.10:4242
       │
       │
       │ TCP
       │
       ▼
Serveur
192.168.1.20:6667
```

On peut identifier une communication avec :

```text
IP source
+
Port source
+
IP destination
+
Port destination
+
Protocole
```

Par exemple :

```text
192.168.1.10:4242
        │
        │ TCP
        ▼
192.168.1.20:6667
```

Ici :

```text
192.168.1.10 → machine source
4242         → port source

192.168.1.20 → machine destination
6667         → port destination

TCP          → protocole de transport
```

---

# 4. Les ports

Un **port** permet d'identifier un service ou une application sur une machine.

Un port est un nombre compris entre :

```text
0 → 65535
```

Exemples courants :

|  Port | Service |
| ----: | ------- |
| 20/21 | FTP     |
|    22 | SSH     |
|    25 | SMTP    |
|    53 | DNS     |
|    80 | HTTP    |
|   443 | HTTPS   |
|  6667 | IRC     |

Un serveur peut écouter sur un port :

```text
IP : 192.168.1.20
Port : 6667
```

On obtient :

```text
192.168.1.20:6667
```

---

## Port source et port destination

Lorsqu'un client communique avec un serveur, il y a généralement deux ports :

```text
Client                          Serveur

192.168.1.10:50000 ──────────► 192.168.1.20:6667
       │                              │
   port source                  port destination
```

Le client utilise souvent un port temporaire, appelé **port éphémère**.

Le serveur écoute généralement sur un port connu.

---


# 7. TCP vs UDP

| Caractéristique        | TCP            | UDP         |
| ---------------------- | -------------- | ----------- |
| Connexion              | Oui            | Non         |
| Fiabilité              | Oui            | Non         |
| Ordre garanti          | Oui            | Non         |
| Retransmission         | Oui            | Non         |
| ACK                    | Oui            | Non         |
| Contrôle de flux       | Oui            | Non         |
| Contrôle de congestion | Oui            | Non         |
| Flux d'octets          | Oui            | Non         |
| Datagrammes            | Non            | Oui         |
| Overhead               | Plus important | Plus faible |

---


# 8. Segmentation

Une application peut produire beaucoup de données.

Par exemple :

```text
1 MB de données
```

TCP peut découper ce flux en plusieurs unités plus petites.

```text
Données application

┌─────────────────────────────────────┐
│              1 MB                   │
└─────────────────────────────────────┘
                 │
                 ▼
              TCP
                 │
       ┌─────────┼─────────┐
       ▼         ▼         ▼
   Segment 1  Segment 2  Segment 3
```

Chaque segment TCP possède notamment :

* un numéro de séquence ;
* un numéro d'acquittement ;
* des flags ;
* une fenêtre ;
* des ports source/destination.

---

# 9. Fiabilité

La fiabilité de TCP repose sur plusieurs mécanismes.

Par exemple :

```text
Client                         Serveur

DATA ─────────────────────────►

      ◄────────────────────── ACK

DATA ─────────────────────────►

      ◄────────────────────── ACK
```

Le serveur indique qu'il a reçu les données avec un **ACK**.

Si TCP détecte qu'une donnée n'a probablement pas été reçue, il peut la retransmettre.

```text
Client                         Serveur

DATA ─────────────────────────►

           X perdu

DATA ─────────────────────────►

      ◄────────────────────── ACK
```

---

# 10. Contrôle de flux

Le contrôle de flux évite qu'un émetteur envoie plus de données que le récepteur ne peut temporairement traiter.

Le récepteur indique notamment une **fenêtre de réception**.

Conceptuellement :

```text
Client                         Serveur

        "Je peux recevoir
         X octets"

        ◄────────────────

DATA ────────────────────────►
DATA ────────────────────────►
```

Si le récepteur a moins de capacité disponible :

```text
Fenêtre grande
     ↓
Envoi possible important

Fenêtre petite
     ↓
Envoi limité
```

Cela permet d'éviter de saturer le buffer du récepteur.

---

# 11. Contrôle de congestion

Le **contrôle de congestion** est différent du contrôle de flux.

### Contrôle de flux

Concerne principalement :

```text
Émetteur → Récepteur
```

Il évite de surcharger le récepteur.

### Contrôle de congestion

Concerne :

```text
Émetteur → Réseau
```

Il cherche à éviter de surcharger le réseau.

On peut résumer :

```text
Contrôle de flux
    ↓
Ne pas surcharger le récepteur

Contrôle de congestion
    ↓
Ne pas surcharger le réseau
```

TCP adapte son débit en fonction des conditions observées sur le réseau.

---

# 12. Numéros de séquence

TCP numérote les données afin de pouvoir notamment :

* suivre leur progression ;
* détecter les pertes ;
* remettre les données dans le bon ordre.

Exemple :

```text
Client                         Serveur

SEQ 1000 ─────────────────────►
SEQ 1100 ─────────────────────►
SEQ 1200 ─────────────────────►
```

Si les données arrivent dans le désordre :

```text
1200
1000
1100
```

TCP peut utiliser les numéros de séquence pour reconstruire le flux correctement.

---


# 14. Retransmission

Si un segment TCP est perdu, TCP peut détecter la situation et retransmettre les données.

```text
Client                         Serveur

SEGMENT 1 ────────────────────►

SEGMENT 2 ─────────── X

SEGMENT 3 ────────────────────►

        ◄──────────────────── ACK

        ↓

Retransmission du segment 2

SEGMENT 2 ────────────────────►
```

C'est l'un des mécanismes qui permettent à TCP d'offrir une communication fiable.


---

# 18. Socket et couche transport

Dans un programme C/C++, une socket permet à l'application d'utiliser les fonctionnalités réseau du système.

Exemple :

```cpp
int fd = socket(AF_INET, SOCK_STREAM, 0);
```

Ici :

```text
AF_INET
    ↓
IPv4

SOCK_STREAM
    ↓
Flux

0
    ↓
Protocole par défaut correspondant
```

Pour TCP, on utilise généralement :

```cpp
socket(AF_INET, SOCK_STREAM, 0);
```

Pour UDP :

```cpp
socket(AF_INET, SOCK_DGRAM, 0);
```

---

## TCP

```cpp
int fd = socket(AF_INET, SOCK_STREAM, 0);
```

Puis côté serveur :

```text
socket()
    ↓
bind()
    ↓
listen()
    ↓
accept()
    ↓
recv() / send()
```

---

## UDP

```cpp
int fd = socket(AF_INET, SOCK_DGRAM, 0);
```

On utilise notamment :

```text
socket()
    ↓
bind()
    ↓
recvfrom()
    ↓
sendto()
```

---

# 19. Exemple avec FT_IRC

Dans **FT_IRC**, ton serveur utilise TCP.

Un client IRC se connecte par exemple à :

```text
serveur:6667
```

On peut représenter la communication ainsi :

```text
┌──────────────────────────┐
│       Client IRC         │
│                          │
│ NICK bob                 │
│ USER bob ...             │
└────────────┬─────────────┘
             │
             │ IRC
             ▼
┌──────────────────────────┐
│          TCP             │
│                          │
│ Port source              │
│ Port destination : 6667  │
│ Séquence                 │
│ ACK                      │
└────────────┬─────────────┘
             │
             ▼
┌──────────────────────────┐
│           IP             │
│                          │
│ IP source                │
│ IP destination           │
└────────────┬─────────────┘
             │
             ▼
┌──────────────────────────┐
│      Ethernet / Wi-Fi    │
└──────────────────────────┘
```

Lorsque ton code fait :

```cpp
send(clientFd, buffer, len, 0);
```

ton programme transmet des données à travers la pile réseau du système.

Le chemin conceptuel est :

```text
Ton code
   ↓
socket
   ↓
TCP
   ↓
IP
   ↓
Ethernet / Wi-Fi
   ↓
Réseau
```

À la réception :

```text
Réseau
   ↓
Ethernet / Wi-Fi
   ↓
IP
   ↓
TCP
   ↓
socket
   ↓
recv()
   ↓
Ton code
```

---

# 20. Encapsulation

Les données traversent plusieurs couches.

Exemple avec IRC sur TCP :

```text
Données IRC
"NICK bob\r\n"
      │
      ▼
┌─────────────────────────┐
│ TCP                     │
│ Header + données IRC    │
└─────────────────────────┘
      │
      ▼
┌─────────────────────────┐
│ IP                      │
│ Header IP + TCP         │
└─────────────────────────┘
      │
      ▼
┌─────────────────────────┐
│ Ethernet                │
│ Header + IP + TCP       │
└─────────────────────────┘
      │
      ▼
   Transmission
```

Chaque couche ajoute ses propres informations.

On obtient donc conceptuellement :

```text
Ethernet
└── IP
    └── TCP
        └── IRC
            └── Données
```

---

# 21. Résumé

La couche transport est la **couche 4 du modèle OSI**.

Son objectif principal est de fournir une communication entre applications.

```text
Application
     │
     ▼
TRANSPORT
     │
     ▼
RÉSEAU
```

Les deux protocoles principaux sont :

```text
TCP
UDP
```

```

### Les ports

Les ports permettent d'identifier les applications :

```text
IP + PORT
```

Exemple :

```text
192.168.1.20:6667
```

### Pour FT_IRC

```text
IRC
 ↓
TCP
 ↓
IP
 ↓
Ethernet / Wi-Fi
 ↓
Réseau
```

Et côté C++ :

```cpp
socket(AF_INET, SOCK_STREAM, 0);
```

permet de créer une socket TCP IPv4.

Le serveur utilise ensuite principalement :

```text
socket()
   ↓
bind()
   ↓
listen()
   ↓
accept()
   ↓
recv() / send()
```

La chose essentielle à retenir est donc :

> **IP permet d'acheminer les paquets vers une machine, tandis que TCP ou UDP permettent de transporter les données vers une application identifiée par un port.**
