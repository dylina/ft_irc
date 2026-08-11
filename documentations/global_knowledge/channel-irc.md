# Channel IRC

[REVENIR AU SOMMAIRE](README.md)

L'élément de base pour communiquer sur un réseau IRC est le **canal** (`channel` dans la spécification).

Un canal est défini par une liste d'utilisateurs connectés sur celui-ci.

En français, un canal peut être comparé au terme **salon**.

## Types de canaux

Les canaux peuvent être visibles sur tout le réseau.

Leur nom est alors préfixé par le caractère `#` :

```text
#42
```

Ils peuvent également être des canaux locaux, portés par un serveur uniquement.

Leur nom est alors préfixé par le caractère `&` :

```text
&42
```

## Communication

Les utilisateurs présents dans un même canal peuvent communiquer entre eux.

Exemple :

```text
Bob
 │
 ├── JOIN #42
 │
 ▼
#42
 │
 ├── Alice
 ├── Bob
 └── Charlie
```
