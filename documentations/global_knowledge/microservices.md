# Microservices

Un **microservice** est un service logiciel spécialisé dans une fonction donnée.

Au lieu d'avoir :

```text
┌──────────────────────────┐
│      Application         │
│                          │
│ Auth + Users + Messages  │
│ + Paiement + Logs        │
└──────────────────────────┘
```

on peut avoir :

```text
        Application
             │
    ┌────────┼────────┐
    ▼        ▼        ▼
 Auth     Users    Messages
Service   Service   Service
```

Chaque service peut être développé, déployé et dimensionné indépendamment.

Les services communiquent généralement via :

```text
HTTP
gRPC
Messages
Queues
Events
...
```

L'architecture en microservices permet notamment de séparer les responsabilités.

---
