SERVER = ../FT_IRC/ircserv
PORT   = 6667
PASS   = pass

# Lance le serveur, exécute tous les tests, détecte un crash
run:
	-python3 irc_tester.py --exe $(SERVER) --port $(PORT) --password $(PASS)

# Idem avec les échanges affichés en cas d'échec
verbose:
	python3 irc_tester.py --exe $(SERVER) --port $(PORT) --password $(PASS) -v

# Filtre par nom : make only T=kick,mode
only:
	python3 irc_tester.py --exe $(SERVER) --port $(PORT) --password $(PASS) --only $(T) -v

# Si vous avez déjà lancé ./ircserv vous-même dans un autre terminal
attach:
	python3 irc_tester.py --port $(PORT) --password $(PASS)

ping:
	./ping.sh

.PHONY: run verbose only attach
