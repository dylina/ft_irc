#!/bin/bash

DIRECTORY="../FT_IRC/"
EXECUTABLE="./ircserv"

echo "DIRECTORY = $DIRECTORY"
echo "EXECUTABLE = $EXECUTABLE"

cd "$DIRECTORY" || exit 1

# Compilation
if ! make re; then
    echo "Compilation failed"
    exit 1
fi

sleep 1

# Recherche d'un port libre
if ! ss -ltn | grep -q ':1051 '; then
    PORT=1051
elif ! ss -ltn | grep -q ':1052 '; then
    PORT=1052
elif ! ss -ltn | grep -q ':1053 '; then
    PORT=1053
elif ! ss -ltn | grep -q ':1054 '; then
    PORT=1054
else
    echo "Aucun port libre entre 1051 et 1054"
    exit 1
fi

echo "PORT = $PORT"


# Lance le serveur
valgrind --leak-check=full -s "$EXECUTABLE" "$PORT" 10 \
    > server.log 2> valgrind.log &

PID=$!

echo "SERVER PID = $PID"

sleep 1

# Vérifie que le serveur tourne encore
if ! kill -0 "$PID" 2>/dev/null; then
    echo "Le serveur n'a pas démarré"
    cat server.log
    cat valgrind.log
    exit 1
fi

# Création des FIFO
mkfifo aled.in
mkfifo oskour.in
mkfifo jipep.in
mkfifo jmenoie.in

# Lance les clients.
# Le nc attendra qu'on ouvre le FIFO en écriture.
nc localhost "$PORT" < aled.in > aled.log &
CLIENT_1=$!

nc localhost "$PORT" < oskour.in > oskour.log &
CLIENT_2=$!

nc localhost "$PORT" < jipep.in > jipep.log &
CLIENT_3=$!

nc localhost "$PORT" < jmenoie.in > jmenoie.log &
CLIENT_4=$!

sleep 1

echo
echo "===== SIGUSR1 : 0 CLIENT ====="
kill -SIGUSR1 "$PID"
sleep 1
cat server.log

# Client 1
echo
echo "===== CONNEXION CLIENT 1 ====="

exec 3>aled.in

sleep 1

echo "===== SIGUSR1 : 1 CLIENT ====="
echo
kill -SIGUSR1 "$PID"
sleep 1
cat server.log

# Client 2
echo
echo "===== CONNEXION CLIENT 2 ====="

exec 4>oskour.in

sleep 1

echo "===== SIGUSR1 : 2 CLIENTS ====="
echo
kill -SIGUSR1 "$PID"
sleep 1
cat server.log

# Client 3
echo
echo "===== CONNEXION CLIENT 3 ====="

exec 5>jipep.in

sleep 1

echo "===== SIGUSR1 : 3 CLIENTS ====="
echo
kill -SIGUSR1 "$PID"
sleep 1
cat server.log

# Client 4
echo
echo "===== CONNEXION CLIENT 4 ====="

exec 6>jmenoie.in

sleep 1

echo "===== SIGUSR1 : 4 CLIENTS ====="
echo
kill -SIGUSR1 "$PID"
sleep 1
cat server.log

# Ferme les clients
echo
echo "===== FERMETURE DES CLIENTS ====="

kill "$CLIENT_1" "$CLIENT_2" "$CLIENT_3" "$CLIENT_4" 2>/dev/null

# Ferme les FIFO
exec 3>&-
exec 4>&-
exec 5>&-
exec 6>&-

rm -f aled.in oskour.in jipep.in jmenoie.in

sleep 1

# Sauvegarde les logs clients
mkdir -p ../tester

mv aled.log ../tester/client1.log
mv oskour.log ../tester/client2.log
mv jipep.log ../tester/client3.log
mv jmenoie.log ../tester/client4.log
mv server.log ../tester/server.log
mv valgrind.log ../tester/valgrind.log

# Arrêt du serveur
echo
echo "===== ARRÊT DU SERVEUR ====="

kill -INT "$PID" 2>/dev/null

wait "$PID"

echo
echo "===== FIN DU TEST ====="
echo
echo "Server log : $DIRECTORY/server.log"
echo "Valgrind   : $DIRECTORY/valgrind.log"
echo "Clients    : ../tester/client*.log"

make fclean

