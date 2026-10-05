**„Generat automat de GPT, sper că e bombă, ești un tigru, sora mea de pe zonă, wola.”**

# Documentația testelor

Aceste scripturi permit testarea serverului IRC cu un client interactiv și obținerea stării clienților conectați folosind `SIGUSR1`.

## 1. Pornirea serverului

Din directorul serverului:

./ircserv 1052 10


Serverul trebuie să fie pornit înainte de utilizarea scripturilor de testare.

---

## 2. Testarea cu un client IRC

Scriptul `testerClient.sh` permite lansarea unui client IRC Python conectat la server.

### Utilizare

./testerClient.sh 1052


`1052` corespunde portului pe care `ircserv` ascultă.

Scriptul se conectează la:

localhost:1052


Odată conectat, poți introduce direct comenzi IRC, de exemplu:

NICK test USER test 0 * :Test User


Răspunsurile serverului sunt afișate direct în terminal.

### Mai mulți clienți

Pentru a testa mai multe conexiuni simultane, deschide mai multe terminale și rulează:

./testerClient.sh 1052


în fiecare dintre ele.

Poți avea, de exemplu:

Terminal 1 → client 1 Terminal 2 → client 2 Terminal 3 → client 3


---

## 3. Testarea dump-ului clienților

Scriptul `ping` permite solicitarea informațiilor despre toți clienții conectați în prezent la `ircserv`.

### Utilizare

Într-un alt terminal:

./ping


Scriptul:

1. Caută PID-ul procesului `ircserv`.
2. Verifică dacă serverul este pornit.
3. Creează FIFO-ul `/tmp/ircserv_dump` dacă este necesar.
4. Trimite semnalul `SIGUSR1` serverului.
5. Recuperează informațiile trimise de `ircserv`.
6. Salvează rezultatul în `result.log`.
7. Afișează rezultatul în terminal.

Fișierul generat se află în același director cu `ping`:

result.log


---

## 4. Exemplu de test complet

### Terminalul 1 — server

./ircserv 1052 10


### Terminalul 2 — primul client

./testerClient.sh 1052


Conectează clientul cu:

NICK alice USER alice 0 * :Alice


### Terminalul 3 — al doilea client

./testerClient.sh 1052


Apoi:

NICK bob USER bob 0 * :Bob


### Terminalul 4 — dump

./ping


Rezultatul conține informațiile despre clienții conectați în prezent și este, de asemenea, salvat în:

result.log


---

## 5. Testarea comportamentului fără server

Dacă `ircserv` nu este pornit:

./ping


Scriptul trebuie să afișeze:

ircserv n'est pas lancé


și să se închidă corect, fără să rămână blocat.

---

## 6. Curățarea

FIFO-ul creat de `ping` este:

/tmp/ircserv_dump


Acesta poate fi șters manual cu:

rm -f /tmp/ircserv_dump


Fișierul cu rezultatul poate fi șters cu:

rm -f result.log


`Makefile` șterge `result.log` în timpul operației `fclean` și FIFO-ul în timpul operației `clean`.
