# 6. DHCP

[REVENIR AU SOMMAIRE](README.md)

**DHCP** signifie :

```text
Dynamic Host Configuration Protocol
```

DHCP permet à une machine d'obtenir automatiquement sa configuration réseau.

Il peut notamment fournir :

```text
Adresse IP
Masque réseau
Passerelle
Serveur DNS
Durée du bail
```

## DORA

La procédure classique utilise quatre étapes :

```text
DISCOVER
    │
    ▼
OFFER
    │
    ▼
REQUEST
    │
    ▼
ACKNOWLEDGE
```

### 1. Discover

Le client cherche un serveur DHCP.

### 2. Offer

Le serveur propose une configuration.

### 3. Request

Le client demande cette configuration.

### 4. Acknowledge

Le serveur confirme.

L'adresse IP est généralement attribuée pour une durée appelée **bail DHCP**.

---
