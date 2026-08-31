# DNS

[REVENIR AU SOMMAIRE](README.md)

**DNS** signifie :

```text
Domain Name System
```

Le DNS permet d'associer des noms à des informations réseau, notamment des adresses IP.

Exemple :

```text
example.com
     │
     │ DNS
     ▼
93.184.216.34
```

Sans DNS, il faudrait utiliser directement les adresses IP.

## Fonctionnement simplifié

```text
Navigateur
    │
    │ "Quelle IP pour example.com ?"
    ▼
Serveur DNS
    │
    │ "93.184.216.34"
    ▼
Navigateur
```

Le DNS ne sert pas uniquement à traduire un nom en IPv4.

Il peut notamment fournir :

```text
A       → IPv4
AAAA    → IPv6
MX      → serveur mail
CNAME   → alias
NS      → serveur DNS
TXT     → informations textuelles
```

---
