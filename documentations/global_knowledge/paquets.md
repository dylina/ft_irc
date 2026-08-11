# Paquets réseau

## Sommaire

* [1. Qu'est-ce qu'un paquet ?](#1-quest-ce-quun-paquet)
* [2. À quoi sert un paquet ?](#2-à-quoi-sert-un-paquet)
* [3. Structure générale](#3-structure-générale)
* [4. En-tête et données](#4-en-tête-et-données)
* [5. Paquet IPv4](#5-paquet-ipv4)
* [6. Paquet IPv6](#6-paquet-ipv6)
* [7. TCP et les paquets](#7-tcp-et-les-paquets)
* [8. UDP et les paquets](#8-udp-et-les-paquets)
* [9. Paquet, segment et datagramme](#9-paquet-segment-et-datagramme)
* [10. Exemple TCP](#10-exemple-tcp)
* [11. Exemple UDP](#11-exemple-udp)
* [12. Chemin d'un paquet](#12-chemin-dun-paquet)
* [13. Fragmentation](#13-fragmentation)
* [14. Paquet vs flux](#14-paquet-vs-flux)
* [15. À retenir](#15-à-retenir)

---

# 1. Qu'est-ce qu'un paquet ?

Un **paquet réseau** est une unité de données transportée sur un réseau.

Lorsqu'un programme veut envoyer des données, celles-ci sont encapsulées avec différentes informations permettant au réseau de les transporter.

On peut simplifier un paquet comme ceci :

```text
┌──────────────────────────────────────┐
│              EN-TÊTE                 │
├──────────────────────────────────────┤
│               DONNÉES                │
└──────────────────────────────────────┘
```

L'en-tête contient des informations permettant notamment de savoir :

* d'où vient le paquet ;
* où il doit aller ;
* quel protocole est utilisé ;
* quelle taille il possède ;
* comment il doit être traité.

---

# 2. À quoi sert un paquet ?

Un réseau ne transporte pas directement une énorme quantité de données comme un seul bloc.

Par exemple, si une application veut envoyer :

```text
Bonjour, comment allez-vous ?
```

les données vont être encapsulées dans différents protocoles.

On peut simplifier :

```text
Application
     │
     │ données
     ▼
   TCP / UDP
     │
     ▼
     IP
     │
     ▼
Ethernet / Wi-Fi
     │
     ▼
   Réseau
```

Chaque couche ajoute ses propres informations.

---

# 3. Structure générale

Un paquet peut être représenté ainsi :

```text
┌─────────────────────────────────────┐
│       En-tête couche réseau         │
├─────────────────────────────────────┤
│       En-tête couche transport      │
├─────────────────────────────────────┤
│             Données                 │
└─────────────────────────────────────┘
```

Dans un paquet TCP sur IPv4, par exemple :

```text
┌──────────────────────────────┐
│        En-tête IPv4          │
├──────────────────────────────┤
│        En-tête TCP           │
├──────────────────────────────┤
│          Données             │
└──────────────────────────────┘
```

Avec UDP :

```text
┌──────────────────────────────┐
│        En-tête IPv4          │
├──────────────────────────────┤
│        En-tête UDP           │
├──────────────────────────────┤
│          Données             │
└──────────────────────────────┘
```

---

# 4. En-tête et données

Un paquet possède généralement :

```text
┌─────────────────────┐
│      HEADER         │
│                     │
│ Informations        │
│ de contrôle         │
├─────────────────────┤
│      PAYLOAD        │
│                     │
│ Données utiles      │
└─────────────────────┘
```

### Header

L'**en-tête** contient les informations nécessaires au protocole.

Par exemple :

```text
IP source
IP destination
TTL
Protocole
Taille
...
```

### Payload

Le **payload** correspond aux données transportées.

Par exemple :

```text
Hello
```

---

# 5. Paquet IPv4

IPv4 ajoute un en-tête devant les données.

Une représentation simplifiée :

```text
┌─────────────────────────────────────┐
│              IPv4                   │
│                                     │
│ Source IP                           │
│ Destination IP                      │
│ TTL                                 │
│ Protocol                            │
│ Total Length                        │
│ Identification                     │
│ Flags                               │
│ Fragment Offset                     │
│ ...                                 │
├─────────────────────────────────────┤
│             Payload                 │
└─────────────────────────────────────┘
```

Exemple :

```text
Source      : 192.168.1.10
Destination : 192.168.1.20
Protocol    : TCP
TTL         : 64
```

Le champ :

```text
Protocol
```

permet notamment d'indiquer ce qui se trouve au-dessus d'IP.

Par exemple :

```text
6  → TCP
17 → UDP
```

---

# 6. Paquet IPv6

IPv6 possède également un en-tête.

```text
┌─────────────────────────────────────┐
│              IPv6                   │
│                                     │
│ Source IPv6                         │
│ Destination IPv6                    │
│ Traffic Class                       │
│ Flow Label                          │
│ Payload Length                      │
│ Next Header                         │
│ Hop Limit                           │
├─────────────────────────────────────┤
│             Payload                 │
└─────────────────────────────────────┘
```

Exemple :

```text
Source      : 2001:db8::1
Destination : 2001:db8::2
Next Header : TCP
Hop Limit   : 64
```

IPv6 utilise notamment :

```text
Hop Limit
```

qui joue un rôle similaire au :

```text
TTL
```

d'IPv4.

---

# 7. TCP et les paquets

TCP fonctionne comme un **flux** au niveau de l'application.

Mais sur le réseau, ce flux est transporté dans des unités TCP.

On parle généralement de **segments TCP**.

Par exemple :

```text
Application
     │
     │ "Bonjour"
     ▼
    TCP
     │
     ▼
┌───────────────────────┐
│ TCP Header            │
├───────────────────────┤
│ "Bonjour"             │
└───────────────────────┘
     │
     ▼
     IP
```

Un segment TCP possède notamment :

```text
Port source
Port destination
Sequence Number
Acknowledgment Number
Flags
Window
Checksum
...
```

---

# 8. UDP et les paquets

UDP fonctionne avec des **datagrammes**.

Un datagramme UDP possède un en-tête relativement simple :

```text
┌─────────────────────────────┐
│        UDP Header           │
├─────────────────────────────┤
│         Données             │
└─────────────────────────────┘
```

L'en-tête UDP contient notamment :

```text
Source Port
Destination Port
Length
Checksum
```

Exemple :

```text
Source Port      : 5000
Destination Port : 4242
Données          : "Hello"
```

---

# 9. Paquet, segment et datagramme

C'est une distinction importante.

## Flux

Le flux est ce que TCP fournit à l'application :

```text
ABCDEF123456...
```

Il n'a pas de frontières de messages.

---

## Segment TCP

Sur le réseau, TCP découpe ce flux en unités appelées :

```text
Segment TCP
```

```text
┌──────────────┐
│ TCP Header   │
├──────────────┤
│ Data         │
└──────────────┘
```

---

## Datagramme UDP

UDP travaille directement avec des unités indépendantes :

```text
┌──────────────┐
│ UDP Header   │
├──────────────┤
│ Data         │
└──────────────┘
```

---

## Paquet IP

IP encapsule le segment TCP ou le datagramme UDP :

```text
        PAQUET IP
┌──────────────────────┐
│      IP Header       │
├──────────────────────┤
│ TCP Header           │
├──────────────────────┤
│ TCP Data             │
└──────────────────────┘
```

ou :

```text
        PAQUET IP
┌──────────────────────┐
│      IP Header       │
├──────────────────────┤
│ UDP Header           │
├──────────────────────┤
│ UDP Data             │
└──────────────────────┘
```

---

# 10. Exemple TCP

Supposons qu'une application fasse :

```cpp
send(sockfd, "Hello", 5, 0);
```

On peut simplifier le chemin ainsi :

```text
Application
     │
     │ "Hello"
     ▼
    TCP
     │
     │ Segment TCP
     ▼
    IP
     │
     │ Paquet IPv4
     ▼
 Ethernet / Wi-Fi
     │
     ▼
   Réseau
```

Le paquet final ressemble conceptuellement à :

```text
┌───────────────────────────────┐
│ IPv4 Header                   │
│                               │
│ Source IP                     │
│ Destination IP                │
│ Protocol = TCP                │
├───────────────────────────────┤
│ TCP Header                    │
│                               │
│ Source Port                   │
│ Destination Port              │
│ Sequence Number               │
│ ACK                           │
│ Flags                         │
├───────────────────────────────┤
│ "Hello"                       │
└───────────────────────────────┘
```

---

# 11. Exemple UDP

Avec :

```cpp
sendto(
    sockfd,
    "Hello",
    5,
    0,
    ...
);
```

le chemin est :

```text
Application
     │
     │ "Hello"
     ▼
    UDP
     │
     │ Datagramme UDP
     ▼
    IP
     │
     │ Paquet IP
     ▼
 Ethernet / Wi-Fi
     │
     ▼
   Réseau
```

Le paquet ressemble à :

```text
┌───────────────────────────────┐
│ IPv4 Header                   │
│                               │
│ Source IP                     │
│ Destination IP                │
│ Protocol = UDP                │
├───────────────────────────────┤
│ UDP Header                    │
│                               │
│ Source Port                   │
│ Destination Port              │
│ Length                        │
│ Checksum                      │
├───────────────────────────────┤
│ "Hello"                       │
└───────────────────────────────┘
```

---

# 12. Chemin d'un paquet

Lorsqu'un paquet quitte ton ordinateur, plusieurs couches interviennent.

```text
┌─────────────────────────────┐
│       APPLICATION           │
│                             │
│        "Hello"              │
└──────────────┬──────────────┘
               │
               ▼
┌─────────────────────────────┐
│           TCP / UDP         │
│                             │
│     Ajout de l'en-tête      │
└──────────────┬──────────────┘
               │
               ▼
┌─────────────────────────────┐
│             IP              │
│                             │
│ Source / Destination        │
└──────────────┬──────────────┘
               │
               ▼
┌─────────────────────────────┐
│       Ethernet / Wi-Fi      │
│                             │
│ MAC source / destination    │
└──────────────┬──────────────┘
               │
               ▼
             RÉSEAU
```

On appelle cela **l'encapsulation**.

---

# 13. Fragmentation

Un paquet ne peut pas forcément être envoyé d'un seul bloc sur un réseau.

Chaque réseau possède une taille maximale appelée :

```text
MTU
```

Par exemple :

```text
MTU Ethernet classique = 1500 octets
```

Si les données dépassent la taille disponible, elles peuvent nécessiter une fragmentation ou une adaptation du transport.

Conceptuellement :

```text
Gros paquet
┌─────────────────────────────┐
│                             │
│          DONNÉES            │
│                             │
└─────────────────────────────┘
              │
              ▼
        Découpage
        ┌─────┐ ┌─────┐ ┌─────┐
        │  1  │ │  2  │ │  3  │
        └─────┘ └─────┘ └─────┘
```

IPv4 et IPv6 ne gèrent pas la fragmentation de la même manière.

En IPv6, les routeurs ne fragmentent pas les paquets en transit. La fragmentation est effectuée par l'émetteur lorsqu'elle est nécessaire.

---

# 14. Paquet vs flux

Il faut retenir cette différence :

```text
                    APPLICATION
                         │
                         │
                    Flux TCP
                         │
                         ▼
                  ┌─────────────┐
                  │ TCP segment │
                  └──────┬──────┘
                         │
                         ▼
                  ┌─────────────┐
                  │  IP packet  │
                  └──────┬──────┘
                         │
                         ▼
                  ┌─────────────┐
                  │ Frame réseau│
                  └─────────────┘
                         │
                         ▼
                       RÉSEAU
```

Le **flux** est une abstraction utilisée par TCP pour l'application.

Le **segment** est l'unité de données de TCP.

Le **paquet** désigne couramment l'unité de données transportée par IP.

La **trame (frame)** est l'unité utilisée par une technologie de liaison comme Ethernet ou Wi-Fi.

---

# 15. À retenir

### Flux

```text
TCP
 │
 └── Suite continue d'octets
```

### Datagramme

```text
UDP
 │
 └── Message indépendant
```

### Segment

```text
TCP
 │
 └── Unité de données TCP
```

### Paquet

```text
IP
 │
 └── Unité de données IP
```

### Trame

```text
Ethernet / Wi-Fi
 │
 └── Unité de données de la couche liaison
```

---

## Vue globale

```text
APPLICATION
     │
     │ données
     ▼
┌─────────────────┐
│     TCP/UDP     │
│                 │
│ TCP → Segment   │
│ UDP → Datagramme│
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│       IP        │
│                 │
│   Paquet IP     │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Ethernet / Wi-Fi│
│                 │
│     Trame       │
└────────┬────────┘
         │
         ▼
       RÉSEAU
```

> **Un paquet n'est donc pas simplement « les données envoyées ».**
>
> C'est une unité de données **encapsulée avec des informations de contrôle**, notamment pour permettre au réseau de savoir où l'envoyer et comment le traiter.

### Relation avec les sockets

```text
socket(AF_INET, SOCK_STREAM, 0)
        │
        ▼
       TCP
        │
        ▼
    flux d'octets
        │
        ▼
  segments TCP
        │
        ▼
    paquets IP
        │
        ▼
      réseau
```

```text
socket(AF_INET, SOCK_DGRAM, 0)
        │
        ▼
       UDP
        │
        ▼
   datagrammes
        │
        ▼
    paquets IP
        │
        ▼
      réseau
```
