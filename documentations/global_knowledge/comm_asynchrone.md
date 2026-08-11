# 14. Communication asynchrone

Une communication **asynchrone** permet à une opération de continuer sans attendre immédiatement sa réponse.

Exemple :

```text
Client
  │
  │ demande
  ▼
Serveur
  │
  │ traitement
  │
  │
Client peut continuer
  │
  ▼
Réponse plus tard
```

Dans un programme réseau, cela permet notamment d'éviter de bloquer tout le programme pendant qu'un client ou une opération attend.

Attention :

> **Communication asynchrone** et **protocole asynchrone** ne désignent pas exactement la même chose.

Dans la programmation réseau moderne, on parle souvent d'I/O asynchrones ou événementielles.

Exemple :

```text
poll()
   │
   ├── socket A prêt
   ├── socket B prêt
   └── socket C prêt
```

---
