# Modes IRC

[REVENIR AU SOMMAIRE](README.md)

Les **modes IRC** sont des options permettant de modifier le comportement d'un utilisateur ou d'un canal.

Ils sont représentés par :

```text
+
```

pour activer un mode et :

```text
-
```

pour le désactiver.

Exemple :

```text
MODE #42 +i
```

Active le mode `i` du canal.

```text
MODE #42 -i
```

Désactive le mode `i`.

Les modes peuvent concerner :

```text
Utilisateur
    │
    ├── visibilité
    ├── opérateur
    └── autres propriétés

Canal
    │
    ├── invitation
    ├── modération
    ├── limite
    ├── clé
    └── opérateurs
```

Exemples classiques :

```text
+i    canal sur invitation
+t    seuls certains utilisateurs peuvent modifier le topic
+n    interdit les messages provenant de l'extérieur du canal
+m    canal modéré
+o    opérateur
+v    voice
```

Les modes disponibles dépendent de l'implémentation IRC.

---
