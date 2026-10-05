# Documentation des tests

Ces scripts permettent de tester le serveur IRC avec un client interactif et de récupérer l'état des clients connectés avec `SIGUSR1`.

## 1. Démarrer le serveur

Depuis le dossier du serveur :

./ircserv 1052 10


Le serveur doit être lancé avant d'utiliser les scripts de test.

---

## 2. Tester avec un client IRC

Le script `testerClient.sh` permet de lancer un client IRC Python connecté au serveur.

### Utilisation

./testerClient.sh 1052


Le `1052` correspond au port sur lequel `ircserv` écoute.

Le script se connecte à :

localhost:1052


Une fois connecté, tu peux entrer directement des commandes IRC, par exemple :

NICK test USER test 0 * :Test User


Les réponses du serveur sont affichées directement dans le terminal.

### Plusieurs clients

Pour tester plusieurs connexions simultanées, ouvre plusieurs terminaux et lance :

./testerClient.sh 1052


dans chacun.

Tu peux ainsi avoir par exemple :

Terminal 1 → client 1 Terminal 2 → client 2 Terminal 3 → client 3


---

## 3. Tester le dump des clients

Le script `ping` permet de demander à `ircserv` les informations de tous les clients actuellement connectés.

### Utilisation

Dans un autre terminal :

./ping


Le script :

1. Cherche le PID de `ircserv`.
2. Vérifie que le serveur est lancé.
3. Crée la FIFO `/tmp/ircserv_dump` si nécessaire.
4. Envoie `SIGUSR1` au serveur.
5. Récupère les informations envoyées par `ircserv`.
6. Enregistre le résultat dans `result.log`.
7. Affiche le résultat dans le terminal.

Le fichier généré est situé dans le même dossier que `ping` :

result.log


---

## 4. Exemple de test complet

### Terminal 1 — serveur

./ircserv 1052 10


### Terminal 2 — premier client

./testerClient.sh 1052


Connecte le client avec :

NICK alice USER alice 0 * :Alice


### Terminal 3 — deuxième client

./testerClient.sh 1052


Puis :

NICK bob USER bob 0 * :Bob


### Terminal 4 — dump

./ping


Le résultat contient les informations des clients actuellement connectés et est également enregistré dans :

result.log


---

## 5. Tester le comportement sans serveur

Si `ircserv` n'est pas lancé :

./ping


Le script doit afficher :

ircserv n'est pas lancé


et quitter proprement sans rester bloqué.

---

## 6. Nettoyage

La FIFO créée par `ping` est :

/tmp/ircserv_dump


Elle peut être supprimée manuellement avec :

rm -f /tmp/ircserv_dump


Le fichier de résultat peut être supprimé avec :

rm -f result.log


le `Makefile` supprime `result.log` lors du `fclean` et la `FIFO` lors du `clean`.
