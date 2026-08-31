# Grades IRC

[REVENIR AU SOMMAIRE](README.md)

En IRC, il existe plusieurs grades principaux.

## Utilisateurs normaux

Les utilisateurs normaux ne sont marqués avec aucun signe à gauche de leur pseudonyme dans la liste des personnes du salon.

Ils n'ont aucun droit particulier sur le salon, sauf celui de parler lorsque le salon n'est pas en mode `+m`.

Lorsque le salon est en mode `-t`, ils peuvent modifier ou mettre le sujet du salon normalement.

## Voices

Les voices sont des utilisateurs ayant reçu le mode `+v` sur un canal de discussion.

Ils ne peuvent pas éjecter ou bannir et n'ont pas de pouvoir particulier sur le salon.

Lorsque le salon est en `+m`, les voices peuvent parler, contrairement aux utilisateurs normaux.

## Half-Ops

Les Half-Ops sont des utilisateurs ayant reçu le mode `+h` sur un canal de discussion.

Ils peuvent :

* changer le sujet du salon, même lorsque le salon est en `+t` ;
* éjecter (`kick`) des utilisateurs ;
* bannir (`ban`) des utilisateurs ;
* appliquer certains modes sur le salon ;
* donner le mode `+v` à des utilisateurs ;
* appliquer l'option `+e`.

Pour bannir quelqu'un :

```text
MODE #NOM_DU_SALON +b XXX
```

Pour donner la voix :

```text
MODE #NOM_DU_SALON +v XXX
```

Certains réseaux IRC ne disposent pas de ce mode.

## Ops

Les Ops sont des utilisateurs ayant reçu le mode `+o` sur un canal de discussion.

Ils disposent des mêmes pouvoirs que les Half-Ops, mais peuvent également appliquer certains modes supplémentaires sur le salon.

Ils peuvent notamment gérer :

* les salons privés ;
* la limite d'utilisateurs (`+l`) ;
* la protection contre le flood ;
* le blocage de certains messages.

## Admins

Les Admins sont des utilisateurs ayant reçu le mode `+a` sur un canal de discussion.

Sous UnrealIRCd, ce mode peut être attribué à un utilisateur ayant un accès d'au moins dix sur un canal.

Ils peuvent notamment :

* modérer les opérateurs ;
* modifier une partie de la configuration d'un canal ;
* retirer ou ajouter des accès inférieurs à leur niveau.

## Owner / Founder

L'Owner ou Founder est un utilisateur ayant reçu le mode `+q` sur un canal de discussion.

L'Owner peut modifier complètement la configuration du canal et déléguer son statut d'Owner à un autre utilisateur.
