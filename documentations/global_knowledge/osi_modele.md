# 🖧 Modèle OSI

[REVENIR AU SOMMAIRE](README.md)

1. [Vue d'ensemble](#1-vue-densemble)
2. [Les 7 couches](#2-les-7-couches)
3. [Couche 1 — Physique](#3-couche-1--physique)
4. [Couche 2 — Liaison de données](#4-couche-2--liaison-de-données)
5. [Couche 3 — Réseau](#5-couche-3--réseau)
6. [Couche 4 — Transport](#6-couche-4--transport)
7. [Couche 5 — Session](#7-couche-5--session)
8. [Couche 6 — Présentation](#8-couche-6--présentation)
9. [Couche 7 — Application](#9-couche-7--application)
10. [Encapsulation](#10-encapsulation)
11. [Désencapsulation](#11-désencapsulation)
12. [Exemple complet](#12-exemple-complet)
13. [OSI et TCP/IP](#13-osi-et-tcpip)
14. [Tableau récapitulatif](#14-tableau-récapitulatif)

---

# 1. Vue d'ensemble

Le **modèle OSI** (*Open Systems Interconnection*) est un modèle conceptuel permettant de comprendre comment les données circulent entre deux machines sur un réseau.

Il découpe les communications réseau en **7 couches**, chacune ayant un rôle précis.

L'intérêt principal du modèle OSI est de pouvoir **séparer les responsabilités** :

* la couche physique s'occupe des signaux ;
* la couche liaison s'occupe des trames ;
* la couche réseau s'occupe de l'adressage et du routage ;
* la couche transport s'occupe de la communication entre machines ;
* les couches supérieures s'occupent notamment des sessions, de la représentation des données et des applications.

---

Le modèle OSI contient **7 couches** :

```text
┌──────────────────────────────┐
│  7. APPLICATION              │
├──────────────────────────────┤
│  6. PRÉSENTATION             │
├──────────────────────────────┤
│  5. SESSION                  │
├──────────────────────────────┤
│  4. TRANSPORT                │
├──────────────────────────────┤
│  3. RÉSEAU                   │
├──────────────────────────────┤
│  2. LIAISON DE DONNÉES       │
├──────────────────────────────┤
│  1. PHYSIQUE                 │
└──────────────────────────────┘
```

On peut retenir :

```text
7 → Application
6 → Présentation
5 → Session
4 → Transport
3 → Réseau
2 → Liaison
1 → Physique
```

Une autre manière de les voir :

```text
Application
    ↓
Présentation
    ↓
Session
    ↓
Transport
    ↓
Réseau
    ↓
Liaison
    ↓
Physique
```

Les données descendent les couches lors de l'envoi et remontent les couches lors de la réception.

---

# 2. Les 7 couches

| Couche | Nom          | Rôle principal                               | Exemples                  |
| -----: | ------------ | -------------------------------------------- | ------------------------- |
|      7 | Application  | Services réseau accessibles aux applications | HTTP, FTP, SMTP, IRC, DNS |
|      6 | Présentation | Format, encodage, chiffrement, compression   | TLS, UTF-8, JPEG          |
|      5 | Session      | Gestion des sessions                         | RPC, sessions             |
|      4 | Transport    | Communication de bout en bout                | TCP, UDP                  |
|      3 | Réseau       | Adressage et routage                         | IP, ICMP                  |
|      2 | Liaison      | Communication sur le réseau local            | Ethernet, Wi-Fi, ARP      |
|      1 | Physique     | Transmission des bits                        | câble, fibre, ondes       |

---

# 3. Couche 1 — Physique

La **couche physique** est la couche la plus basse du modèle OSI.

Elle s'occupe de transmettre des **bits** sous forme de signaux.

```text
0100101101010010
      ↓
┌───────────────┐
│ Signal        │
│ électrique    │
│ optique       │
│ radio         │
└───────────────┘
```

Elle ne comprend pas ce que représentent les données.

Elle sait uniquement transmettre des :

```text
0
1
```

## Exemples

* câble Ethernet ;
* fibre optique ;
* câble coaxial ;
* ondes radio ;
* Wi-Fi ;
* connecteurs ;
* fréquences ;
* niveaux électriques ;
* modulation.

## Exemple

Avec un câble Ethernet, les bits sont représentés par des signaux électriques.

Avec une fibre optique, ils sont représentés par des impulsions lumineuses.

---

# 4. Couche 2 — Liaison de données

La couche liaison permet de communiquer entre des machines présentes sur un même réseau local.

Elle travaille notamment avec les **adresses MAC** et les **trames**.

```text
┌──────────────────────────────┐
│ MAC destination              │
├──────────────────────────────┤
│ MAC source                   │
├──────────────────────────────┤
│ Données                      │
├──────────────────────────────┤
│ FCS / contrôle d'erreur      │
└──────────────────────────────┘
```

L'unité de données de cette couche est généralement appelée :

```text
TRAME
```

## Adresse MAC

Une adresse MAC identifie une interface réseau au niveau de la couche liaison.

Exemple :

```text
00:1A:2B:3C:4D:5E
```

## Exemples de technologies

* Ethernet ;
* Wi-Fi ;
* VLAN ;
* PPP.

## Switch

Un **switch** travaille principalement à la couche 2.

Il utilise notamment les adresses MAC pour déterminer vers quel port envoyer une trame.

```text
PC A
 │
 │ Ethernet
 ▼
┌─────────┐
│ SWITCH  │
└─────────┘
 │       │
 ▼       ▼
PC B    PC C
```

---

# 5. Couche 3 — Réseau

La couche réseau permet de faire communiquer des machines qui ne sont pas nécessairement sur le même réseau local.

Elle s'occupe principalement de :

* l'adressage logique ;
* la sélection du chemin ;
* le routage ;
* le transfert des paquets.

L'exemple principal est :

```text
IP
```

L'unité de données est généralement appelée :

```text
PAQUET
```

## Adresse IP

Exemple IPv4 :

```text
192.168.1.25
```

Exemple IPv6 :

```text
2001:db8::1
```

Contrairement à une adresse MAC, une adresse IP permet notamment de déterminer à quel réseau appartient une machine.

---

## Routeur

Un routeur travaille principalement à la couche 3.

```text
Réseau A
192.168.1.0/24
      │
      ▼
 ┌─────────┐
 │ ROUTEUR │
 └─────────┘
      │
      ▼
Réseau B
10.0.0.0/24
```

Le routeur examine notamment l'adresse IP de destination afin de déterminer où envoyer le paquet.

## Protocoles

Exemples :

* IPv4 ;
* IPv6 ;
* ICMP.

---

# 6. Couche 4 — Transport

La couche transport permet une communication **de bout en bout** entre deux applications.

Elle s'occupe notamment de :

* la communication entre processus ;
* les ports ;
* la fiabilité ;
* le contrôle du flux ;
* la segmentation ;
* la retransmission selon le protocole utilisé.

Les deux protocoles les plus connus sont :

```text
TCP
UDP
```

---


# 7. Couche 5 — Session

La couche session s'occupe de la **gestion des sessions de communication** entre applications.

Elle peut notamment gérer :

* l'ouverture d'une session ;
* le maintien d'une session ;
* la fermeture d'une session ;
* la synchronisation ;
* la reprise d'une communication.

L'idée générale est :

```text
Application A
      │
      ▼
   SESSION
      │
      ▼
Application B
```

Cette couche est surtout utile comme modèle conceptuel.

Dans les architectures modernes, ses fonctions sont souvent intégrées dans d'autres couches ou protocoles.

---

# 8. Couche 6 — Présentation

La couche présentation s'occupe de la **représentation des données**.

Elle permet notamment de gérer :

* l'encodage ;
* la conversion de format ;
* la sérialisation ;
* la compression ;
* le chiffrement.

L'objectif est que deux systèmes utilisant des représentations différentes puissent comprendre les données échangées.

---

## Encodage

Exemple :

```text
"é"
```

peut être représenté en UTF-8 par plusieurs octets.

La couche présentation s'intéresse donc à la manière dont les données sont représentées.

---

## Chiffrement

Le chiffrement peut également être associé à cette couche dans le modèle OSI.

Par exemple :

```text
Données
   ↓
Chiffrement
   ↓
Données chiffrées
```

Dans la pratique, **TLS** est souvent présenté comme se situant entre les couches application et transport, même si son positionnement exact ne correspond pas parfaitement à une seule couche OSI.

---

## Compression

Une donnée peut également être compressée avant transmission :

```text
Données originales
       ↓
   Compression
       ↓
Données compressées
```

---

# 9. Couche 7 — Application

La couche application est la couche la plus proche des logiciels utilisés par l'utilisateur.

Elle fournit les services réseau utilisés directement ou indirectement par les applications.

Exemples :

```text
HTTP
HTTPS
FTP
SMTP
IMAP
DNS
SSH
IRC
```

---

## HTTP

Lorsqu'un navigateur demande une page :

```text
Navigateur
    ↓
   HTTP
    ↓
   TCP
    ↓
    IP
    ↓
 Ethernet
    ↓
Physique
```

HTTP permet de définir les requêtes et les réponses utilisées pour communiquer avec un serveur web.

---

## IRC

Dans ton projet **FT_IRC**, IRC est un protocole de couche application.

Exemple :

```text
Client IRC
    │
    │ NICK bob
    │
    ▼
   TCP
    │
    ▼
    IP
    │
    ▼
Ethernet
```

Une commande IRC comme :

```text
NICK bob\r\n
```

est une donnée de la couche application.

---

# 10. Encapsulation

Lorsqu'une application envoie des données, chaque couche peut ajouter ses propres informations.

C'est ce qu'on appelle **l'encapsulation**.

Exemple :

```text
Application
    │
    │ Données
    ▼
Transport
    │
    │ Segment TCP
    ▼
Réseau
    │
    │ Paquet IP
    ▼
Liaison
    │
    │ Trame Ethernet
    ▼
Physique
    │
    │ Bits
    ▼
   Réseau
```

On peut représenter cela ainsi :

```text
┌─────────────────────────────────────────┐
│ Ethernet                                │
│ ┌─────────────────────────────────────┐ │
│ │ IP                                  │ │
│ │ ┌─────────────────────────────────┐ │ │
│ │ │ TCP                             │ │ │
│ │ │ ┌─────────────────────────────┐ │ │ │
│ │ │ │ Données application         │ │ │ │
│ │ │ └─────────────────────────────┘ │ │ │
│ │ └─────────────────────────────────┘ │ │
│ └─────────────────────────────────────┘ │
└─────────────────────────────────────────┘
```

Chaque couche ajoute généralement un **en-tête**.

---

# 11. Désencapsulation

À la réception, le processus est inversé.

```text
        Réception
           │
           ▼
      Couche physique
           │
           ▼
       Couche liaison
           │
           ▼
        Couche réseau
           │
           ▼
      Couche transport
           │
           ▼
        Couche session
           │
           ▼
    Couche présentation
           │
           ▼
     Couche application
```

Chaque couche traite les informations qui lui sont destinées puis transmet les données à la couche supérieure.

---

# 12. Exemple complet

Imaginons :

```text
Client IRC
    │
    │ NICK bob
    ▼
Serveur IRC
```

Le client veut envoyer :

```text
NICK bob\r\n
```

## Couche 7 — Application

IRC construit les données :

```text
NICK bob\r\n
```

---

## Couche 4 — Transport

TCP ajoute ses informations :

```text
┌───────────────┐
│ Header TCP    │
├───────────────┤
│ NICK bob\r\n   │
└───────────────┘
```

---

## Couche 3 — Réseau

IP ajoute son en-tête :

```text
┌───────────────┐
│ Header IP     │
├───────────────┤
│ Header TCP    │
├───────────────┤
│ NICK bob\r\n   │
└───────────────┘
```

---

## Couche 2 — Liaison

Ethernet ajoute une trame :

```text
┌───────────────┐
│ Ethernet      │
├───────────────┤
│ IP            │
├───────────────┤
│ TCP           │
├───────────────┤
│ NICK bob\r\n   │
├───────────────┤
│ FCS           │
└───────────────┘
```

---

## Couche 1 — Physique

La trame est transformée en signaux :

```text
010101101001011010...
```

Elle est ensuite transmise sur le réseau.

---

# 13. OSI et TCP/IP

Le modèle OSI est un **modèle conceptuel**.

Internet utilise principalement la suite de protocoles **TCP/IP**.

Les deux modèles peuvent être rapprochés :

```text
          OSI                    TCP/IP

┌──────────────────┐
│ 7 Application    │
├──────────────────┤
│ 6 Présentation   │
├──────────────────┤
│ 5 Session        │ ───────► Application
├──────────────────┤
│ 4 Transport      │ ───────► Transport
├──────────────────┤
│ 3 Réseau         │ ───────► Internet
├──────────────────┤
│ 2 Liaison        │
├──────────────────┤
│ 1 Physique       │ ───────► Accès réseau
└──────────────────┘
```

Le modèle TCP/IP est généralement présenté avec **4 couches** :

```text
Application
Transport
Internet
Accès réseau
```

Le modèle OSI possède davantage de couches car il sépare notamment :

```text
Session
Présentation
Application
```

alors que TCP/IP regroupe généralement ces fonctions dans la couche application.

---

# 14. Tableau récapitulatif

| OSI | Couche       | Fonction                           | Unité                | Exemples            |
| --: | ------------ | ---------------------------------- | -------------------- | ------------------- |
|   7 | Application  | Services réseau                    | Données              | HTTP, IRC, DNS, FTP |
|   6 | Présentation | Format / chiffrement / compression | Données              | UTF-8, TLS          |
|   5 | Session      | Gestion des sessions               | Données              | RPC                 |
|   4 | Transport    | Communication de bout en bout      | Segment / datagramme | TCP, UDP            |
|   3 | Réseau       | Routage / adressage IP             | Paquet               | IPv4, IPv6, ICMP    |
|   2 | Liaison      | Réseau local / MAC                 | Trame                | Ethernet, Wi-Fi     |
|   1 | Physique     | Transmission des bits              | Bits                 | Câble, fibre, radio |

---

# 🧠 À retenir

Le modèle OSI permet surtout de comprendre **où se situe chaque problème réseau**.

```text
7  APPLICATION
   "Que veut faire mon application ?"

6  PRÉSENTATION
   "Comment représenter les données ?"

5  SESSION
   "Comment gérer la communication ?"

4  TRANSPORT
   "Comment transporter les données entre applications ?"

3  RÉSEAU
   "Vers quelle machine/réseau envoyer le paquet ?"

2  LIAISON
   "Comment communiquer sur le réseau local ?"

1  PHYSIQUE
   "Comment transmettre les bits ?"
```

Pour ton projet **FT_IRC**, le chemin à retenir est surtout :

```text
IRC
 │
 │ Couche 7
 ▼
TCP
 │
 │ Couche 4
 ▼
IP
 │
 │ Couche 3
 ▼
Ethernet / Wi-Fi
 │
 │ Couche 2
 ▼
Signal électrique / radio
 │
 │ Couche 1
 ▼
Réseau
```

Et côté programmation C/C++ :

```text
socket()
   ↓
TCP
   ↓
IP
   ↓
Ethernet
   ↓
Physique
```

Les fonctions comme :

```cpp
socket()
bind()
listen()
accept()
connect()
send()
recv()
```

permettent à ton programme de communiquer avec la pile réseau du système. Ton code ne manipule donc généralement **pas directement les couches 1 et 2** : le système d'exploitation et le matériel réseau s'en chargent.
