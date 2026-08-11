### ⚠️ Valeur de retour

```text
ret > 0
    données reçues

ret == 0
    connexion fermée proprement

ret < 0
    erreur
```

Exemple :

```cpp
if (ret > 0)
{
    // traiter les données
}
else if (ret == 0)
{
    // client déconnecté
}
else
{
    // erreur
}
```

---
