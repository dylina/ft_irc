# 📁 Fichiers et File Descriptors

Les **file descriptors** (`FD`) sont des identifiants utilisés par le système pour représenter des ressources ouvertes par un processus.

Une socket est elle-même un **file descriptor**.

C'est pourquoi certaines fonctions utilisées pour les fichiers peuvent également être utilisées avec des sockets.

---

# Sommaire

1. [File Descriptor](#file-descriptor)
2. [Fichiers](#fichiers)
3. [Sockets](#sockets)
4. [Même principe](#même-principe)
5. [Résumé](#résumé)

---

# File Descriptor

Un **file descriptor** est un nombre entier qui identifie une ressource ouverte par un processus.

Par exemple :

```text
0 → entrée standard
1 → sortie standard
2 → erreur standard
3 → fichier ou socket
4 → fichier ou socket
```

Le système attribue un numéro au processus pour chaque ressource ouverte.

---

# Fichiers

Lorsqu'un programme ouvre un fichier, le système lui retourne un file descriptor.

```text
Programme
    │
    │ ouverture d'un fichier
    ▼
Système
    │
    ▼
FD = 3
```

Le programme utilise ensuite ce descripteur pour accéder au fichier.

---

# Sockets

Une socket fonctionne selon le même principe.

Lorsqu'une socket est créée, le système retourne également un file descriptor.

```text
socket()
   │
   ▼
FD = 3
   │
   ▼
Socket
```

Le `3` représente alors la socket auprès du système.

---

# Même principe

Le système utilise donc des file descriptors pour représenter différentes ressources :

```text
                 Processus
                    │
          ┌─────────┼─────────┐
          │         │         │
          ▼         ▼         ▼
        FD 0      FD 1      FD 3
          │         │         │
       stdin      stdout    Socket
```

C'est pour cette raison que certaines fonctions de gestion des fichiers peuvent également fonctionner avec des sockets.

Le système ne manipule pas directement le fichier ou la socket avec le programme : il utilise le **file descriptor** comme identifiant.

---

# Résumé

| Élément             | Rôle                                |
| :------------------ | :---------------------------------- |
| **File Descriptor** | Identifiant d'une ressource ouverte |
| **Fichier**         | Peut être représenté par un FD      |
| **Socket**          | Peut être représentée par un FD     |
| **FD**              | Toujours un entier côté programme   |

### À retenir

```text
Fichier ──┐
          ├──► File Descriptor
Socket ───┘
```

Une **socket est un file descriptor**.

C'est cette caractéristique qui permet au système de gérer les sockets comme des ressources d'entrée/sortie similaires aux fichiers.
