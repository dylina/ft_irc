# 🔄 Mode bloquant et non-bloquant

Une socket peut fonctionner en **mode bloquant** ou en **mode non-bloquant**.

La différence principale concerne le comportement du programme lorsqu'une opération réseau ne peut pas être effectuée immédiatement.

---

# Sommaire

1. [Mode bloquant](#mode-bloquant)
2. [Mode non-bloquant](#mode-non-bloquant)
3. [Comparaison](#comparaison)
4. [Avec `poll()`](#avec-poll)
5. [Résumé](#résumé)

---

# Mode bloquant

En mode bloquant, une opération d'entrée/sortie attend que l'opération puisse être effectuée.

Par exemple :

```text
recv()
  │
  ▼
attend des données
  │
  ▼
PROGRAMME BLOQUÉ
```

Si aucune donnée n'est disponible, `recv()` attend.

Lorsque les données arrivent :

```text
recv()
  │
  ▼
attend des données
  │
  ▼
données disponibles
  │
  ▼
lecture
  │
  ▼
programme continue
```

Le problème est que le programme ne peut pas continuer normalement pendant cette attente.

---

# Mode non-bloquant

En mode non-bloquant, une opération ne reste pas en attente.

```text
recv()
  │
  ├── données disponibles → lecture
  │
  └── rien → retour immédiat
```

Si aucune donnée n'est disponible, l'appel retourne immédiatement.

```text
recv()
  │
  ▼
aucune donnée
  │
  ▼
retour immédiat
  │
  ▼
programme continue
```

Le programme peut alors effectuer d'autres tâches.

---

# Comparaison

| Mode             | Données disponibles | Aucune donnée   |
| :--------------- | :------------------ | :-------------- |
| **Bloquant**     | Lecture             | Attend          |
| **Non-bloquant** | Lecture             | Retour immédiat |

### Bloquant

```text
recv()
  │
  ▼
attente
  │
  ▼
données
  │
  ▼
continue
```

### Non-bloquant

```text
recv()
  │
  ├── données → lecture
  │
  └── rien → continue
```

---

# Avec `poll()`

Le mode non-bloquant est particulièrement utile avec `poll()`.

`poll()` permet d'attendre qu'un ou plusieurs descripteurs soient prêts à effectuer une opération d'entrée/sortie.

```text
              ┌── Socket 1
              │
poll() ───────┼── Socket 2
              │
              └── Socket 3
                    │
                    ▼
             événement disponible
                    │
                    ▼
                  recv()
```

Au lieu de rester bloqué sur une seule socket, le programme peut surveiller plusieurs sockets.

C'est particulièrement adapté à un serveur qui doit gérer plusieurs connexions simultanément.

---

# Résumé

```text
MODE BLOQUANT

recv()
  │
  ▼
attend
  │
  ▼
données
  │
  ▼
continue
```

```text
MODE NON-BLOQUANT

recv()
  │
  ├── données → lecture
  │
  └── rien → continue
```

Avec `poll()`, le programme peut surveiller plusieurs file descriptors et effectuer une lecture lorsqu'une socket est prête.

**À retenir :**

> **Bloquant = attendre.**
> **Non-bloquant = ne pas attendre.**
