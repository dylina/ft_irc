# Événements / Events

Un **événement** représente quelque chose qui s'est produit dans un système.

Exemples :

```text
Utilisateur connecté
Message reçu
Commande créée
Paiement effectué
Fichier téléchargé
Client déconnecté
```

Un événement peut être représenté ainsi :

```text
Événement
    │
    ▼
┌─────────────────────┐
│ type : USER_LOGIN   │
│ user : 42           │
│ time : ...          │
└─────────────────────┘
```

## Producteur

Le producteur crée l'événement :

```text
Producteur
     │
     ▼
  EVENT
```

## Bus d'événements

Un bus peut transporter les événements :

```text
Producteur
     │
     ▼
Event Bus
     │
 ┌───┼────┐
 ▼   ▼    ▼
 A   B    C
```

## Consommateur

Un consommateur reçoit l'événement et agit en conséquence.

---
