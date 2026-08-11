# Découpage d'un message IRC

[REVENIR AU SOMMAIRE](README.md)

Un message IRC peut être découpé en plusieurs parties.

Exemple :

```text
:Bob!bob@localhost PRIVMSG #42 :Salut
```

## Prefix

```text
:Bob!bob@localhost
│
└── prefix
```

Le **prefix** identifie l'origine du message.

## Commande

```text
PRIVMSG
│
└── commande
```

La **commande** indique l'action demandée.

## Paramètre

```text
#42
│
└── paramètre
```

Le **paramètre** fournit une information utilisée par la commande.

## Trailing parameter

```text
:Salut
│
└── trailing parameter
```

Le **trailing parameter** permet notamment de transmettre un texte pouvant contenir des espaces.

## Exemple complet

```text
:Bob!bob@localhost PRIVMSG #42 :Salut
│                    │       │
│                    │       └── trailing parameter
│                    └────────── paramètre
└─────────────────────────────── prefix
                     └────────── commande
```
