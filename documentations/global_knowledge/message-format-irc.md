# Format des messages IRC

[REVENIR AU SOMMAIRE](README.md)

Un message IRC est une seule ligne, délimitée par une paire de caractères **CR (`\r`, `0x0D`)** et **LF (`\n`, `0x0A`)**.

Lors de la lecture des messages provenant d'un flux, les données entrantes doivent être chargées dans une mémoire tampon.

Un message ne doit être traité qu'à partir du moment où le caractère de fin `\r\n` est rencontré.

Si un message vide est reçu, il doit être ignoré silencieusement.

Lors de l'envoi d'un message, une paire de caractères `\r\n` doit suivre chaque message envoyé.

> ⚠️ Un message IRC se termine toujours par `\r\n`.

## Exemples

```text
NICK Bob\r\n
```

```text
JOIN #42\r\n
```

```text
PRIVMSG #42 :Salut tout le monde\r\n
```
