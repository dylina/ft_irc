# En-tête TCP

[REVENIR AU SOMMAIRE](README.md)

Un segment TCP possède un en-tête contenant les informations nécessaires au fonctionnement de TCP.

Structure simplifiée :

```text
 0                   15 16                  31
┌─────────────────────┬─────────────────────┐
│    Port Source      │  Port Destination   │
├─────────────────────┴─────────────────────┤
│             Numéro de séquence            │
├────────────────────────────────────────────┤
│          Numéro d'accusé de réception      │
├──────┬──────┬──────────────────────────────┤
│Offset│Réserv│          Flags               │
├──────┴──────┴──────────────────────────────┤
│              Fenêtre                       │
├────────────────────────────────────────────┤
│              Checksum                      │
├────────────────────────────────────────────┤
│          Pointeur urgent                   │
├────────────────────────────────────────────┤
│              Options                       │
├────────────────────────────────────────────┤
│                DONNÉES                     │
└────────────────────────────────────────────┘
```

## Port source

```text
16 bits
```

Identifie le port de l'expéditeur.

## Port destination

```text
16 bits
```

Identifie le port du destinataire.

## Numéro de séquence

```text
32 bits
```

Permet notamment d'identifier la position des données dans le flux TCP et de gérer leur remise en ordre.

## Numéro d'accusé de réception

```text
32 bits
```

Indique le prochain numéro de séquence attendu.

## Data Offset

```text
4 bits
```

Indique la longueur de l'en-tête TCP.

Cela permet de savoir où commencent les données.

## Flags

Les flags TCP modernes comprennent notamment :

```text
SYN
ACK
FIN
RST
PSH
URG
ECE
CWR
```

Les principaux à connaître pour `ft_irc` sont :

```text
SYN → établissement
ACK → accusé de réception
FIN → fermeture normale
RST → réinitialisation
```

## Window

Indique la quantité de données que le récepteur est actuellement capable d'accepter.

Ce mécanisme participe au **contrôle de flux TCP**.

## Checksum

Permet de détecter certaines erreurs de transmission.

## Urgent Pointer

Utilisé avec le flag `URG` pour le mécanisme de données urgentes TCP.

Il est rarement utilisé dans les applications modernes.

## Options

Les options permettent d'ajouter des fonctionnalités à TCP.

Exemples :

```text
MSS
Window Scale
SACK
Timestamps
```

La taille minimale de l'en-tête TCP est :

```text
20 octets
```

et peut être plus grande lorsque des options sont présentes.

---

# 🧠 Vue globale

Tous ces concepts sont liés :

```text
                         INTERNET
                            │
                            ▼
                    ┌──────────────┐
                    │      IP      │
                    └──────┬───────┘
                           │
                 ┌─────────┴─────────┐
                 ▼                   ▼
               TCP                  UDP
                 │                   │
          Flux d'octets         Datagrammes
                 │                   │
                 └─────────┬─────────┘
                           │
                         Socket
                           │
                           ▼
                     Application
                           │
              ┌────────────┴────────────┐
              ▼                         ▼
            IRC                       HTTP
              │                         │
              ▼                         ▼
            IRCd                   Serveur Web
```

Et autour de ces protocoles :

```text
DNS
 │
 └── résolution des noms

DHCP
 │
 └── configuration réseau

TLS
 │
 └── sécurisation

Proxy / BNC
 │
 └── intermédiaire

Buffer
 │
 └── stockage temporaire

Handshake
 │
 └── établissement / négociation

RFC
 │
 └── spécifications et standards
```

---
