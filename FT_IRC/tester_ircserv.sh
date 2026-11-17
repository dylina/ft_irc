#!/usr/bin/env bash
# =============================================================================
#  test_ircserv.sh : batterie de tests pour ft_irc
#
#  Usage :
#     ./test_ircserv.sh                  # tout lancer
#     ./test_ircserv.sh auth mode        # seulement certaines sections
#     VALGRIND=1 ./test_ircserv.sh       # serveur lance sous valgrind
#
#  Sections : build readme static args net compat format partial edge kill flood
#             auth privmsg relay channel perms opx kick invite mode robust cpu
#             lowmem fdlimit syscalls signals
#
#  Variables d'environnement :
#     BIN=./ircserv   PORT=16667   PASSW=pass   SRC_DIR=.
#     VALGRIND=0|1    WAIT=2 (timeout d'attente d'une reponse, en secondes)
#     NEG=0.6 (delai avant de conclure "rien recu")   FLOOD_N=20000
#     EXCLUDE_DIR=<motif> (dossier a ignorer pour l'analyse statique)
#
#  Les clients sont de simples connexions bash (/dev/tcp), pas besoin de nc.
#  Necessite bash >= 4 (tableaux associatifs). Lancer depuis la racine du projet.
# =============================================================================

if [ "${BASH_VERSINFO[0]}" -lt 4 ]; then
	echo "bash >= 4 requis (macOS : brew install bash)"; exit 2
fi

BIN=${BIN:-./ircserv}
PORT=${PORT:-16667}
PASSW=${PASSW:-pass}
SRC_DIR=${SRC_DIR:-.}
VALGRIND=${VALGRIND:-0}
WAIT=${WAIT:-2}
NEG=${NEG:-0.6}
FLOOD_N=${FLOOD_N:-20000}
EXCLUDE_DIR=${EXCLUDE_DIR:-__aucun_dossier__}
HOST=127.0.0.1
TMP=$(mktemp -d)
RUN=0
SRV_PID=""
SRV_RC=0
VLOG=""

trap '' PIPE   # ecrire sur un socket ferme ne doit pas tuer le script

if [ -t 1 ]; then
	RED=$'\033[31m'; GRN=$'\033[32m'; YEL=$'\033[33m'; BLU=$'\033[36m'; RST=$'\033[0m'
else
	RED=""; GRN=""; YEL=""; BLU=""; RST=""
fi

PASS_N=0; FAIL_N=0; WARN_N=0; SKIP_N=0
FAILED=()
WARNED=()
CUR_SECTION=""

# ----------------------------------------------------------------------------
#  Affichage
# ----------------------------------------------------------------------------
section() { CUR_SECTION=$1; printf '\n%s=== %s ===%s\n' "$BLU" "$1" "$RST"; }
ok()   { PASS_N=$((PASS_N+1)); printf '  %s[ OK ]%s %s\n' "$GRN" "$RST" "$1"; }
ko()   { FAIL_N=$((FAIL_N+1)); FAILED+=("[$CUR_SECTION] $1")
         printf '  %s[FAIL]%s %s\n' "$RED" "$RST" "$1"
         [ -n "${2:-}" ] && printf '         %s\n' "$2"; }
warn() { WARN_N=$((WARN_N+1)); WARNED+=("[$CUR_SECTION] $1"); printf '  %s[WARN]%s %s\n' "$YEL" "$RST" "$1"; }
skip() { SKIP_N=$((SKIP_N+1)); printf '  %s[SKIP]%s %s\n' "$YEL" "$RST" "$1"; }
info() { printf '  %s[INFO]%s %s\n' "$BLU" "$RST" "$1"; }

# ----------------------------------------------------------------------------
#  Gestion du serveur
# ----------------------------------------------------------------------------
wait_port() { # PID : attend que le port accepte les connexions
	local pid=$1 i
	for i in $(seq 1 100); do
		kill -0 "$pid" 2>/dev/null || return 1
		if (exec 9<>/dev/tcp/$HOST/$PORT) 2>/dev/null; then return 0; fi
		sleep 0.2
	done
	return 1
}

start_server() {
	RUN=$((RUN+1)); VLOG="$TMP/valgrind.$RUN.log"
	: > "$TMP/server.$RUN.log"
	if [ "$VALGRIND" = 1 ]; then
		valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes \
			--log-file="$VLOG" "$BIN" "$PORT" "$PASSW" > "$TMP/server.$RUN.log" 2>&1 &
	else
		"$BIN" "$PORT" "$PASSW" > "$TMP/server.$RUN.log" 2>&1 &
	fi
	SRV_PID=$!
	wait_port "$SRV_PID"
}

stop_server() { # [SIGNAL] -> SRV_RC = code de sortie (137 si du kill -9 a ete necessaire)
	local sig=${1:-INT} i
	[ -n "$SRV_PID" ] || return 0
	kill -"$sig" "$SRV_PID" 2>/dev/null
	for i in $(seq 1 200); do
		kill -0 "$SRV_PID" 2>/dev/null || break
		sleep 0.1
	done
	if kill -0 "$SRV_PID" 2>/dev/null; then
		kill -9 "$SRV_PID" 2>/dev/null; { wait "$SRV_PID"; } 2>/dev/null
		SRV_RC=137; SRV_PID=""; return 1
	fi
	{ wait "$SRV_PID"; } 2>/dev/null; SRV_RC=$?
	SRV_PID=""
	return 0
}

server_alive() { [ -n "$SRV_PID" ] && kill -0 "$SRV_PID" 2>/dev/null; }

check_valgrind() { # label
	[ "$VALGRIND" = 1 ] || return 0
	[ -s "$VLOG" ] || { warn "valgrind: pas de log ($1)"; return; }
	if grep -Eq 'definitely lost: [1-9]|indirectly lost: [1-9]|possibly lost: [1-9]|Invalid (read|write|free)|Conditional jump|ERROR SUMMARY: [1-9]' "$VLOG"; then
		ko "valgrind: fuite ou erreur memoire ($1)" "voir $VLOG (copie dans ./valgrind_fail.log)"
		cp "$VLOG" ./valgrind_fail.log 2>/dev/null
	else
		ok "valgrind: 0 fuite / 0 erreur ($1)"
	fi
	grep -Eq 'still reachable: [1-9]' "$VLOG" && warn "valgrind: still reachable non nul ($1)"
	local leaked
	leaked=$(( $(grep -c 'Open file descriptor' "$VLOG") - $(grep -c 'inherited from parent' "$VLOG") ))
	if [ "$leaked" -gt 0 ]; then ko "valgrind: $leaked fd ouvert(s) a la sortie ($1)"
	else ok "valgrind: aucun fd ouvert a la sortie ($1)"; fi
}

# ----------------------------------------------------------------------------
#  Clients IRC (connexions /dev/tcp + lecteur en arriere-plan vers un fichier)
# ----------------------------------------------------------------------------
declare -A FD PID LOG MARK
NEXTFD=20   # >= 20 : bash utilise lui-meme le fd 10 pour ses redirections temporaires ; jamais 255

new() { # NAME : ouvre une connexion
	local n=$1 fd=$NEXTFD
	NEXTFD=$((NEXTFD+1))
	if ! eval "exec $fd<>/dev/tcp/$HOST/$PORT" 2>/dev/null; then return 1; fi
	FD[$n]=$fd; LOG[$n]="$TMP/c_$n.log"; MARK[$n]=0; : > "${LOG[$n]}"
	(
		# le lecteur ne doit garder QUE son propre socket, sinon fermer un
		# client n'enverrait pas de FIN tant que d'autres lecteurs le tiennent
		for f in "${FD[@]}"; do
			[ "$f" != "$fd" ] && eval "exec $f>&-"
		done
		exec cat <&"$fd" >> "${LOG[$n]}"
	) &
	PID[$n]=$!
	return 0
}

closec() { # NAME : fermeture brutale (equivalent d'un kill -9 de nc)
	local n=$1
	[ -n "${FD[$n]:-}" ] || return 0
	kill -9 "${PID[$n]}" 2>/dev/null; { wait "${PID[$n]}"; } 2>/dev/null
	eval "exec ${FD[$n]}>&-"
	unset "FD[$n]"
}
closeall() { local n; for n in "${!FD[@]}"; do closec "$n"; done; }

tx()    { local n=$1; shift; { printf '%s\r\n' "$*" >&"${FD[$n]}"; } 2>/dev/null; }   # ligne + CRLF
txlf()  { local n=$1; shift; { printf '%s\n' "$*" >&"${FD[$n]}"; } 2>/dev/null; }     # ligne + LF seul
txraw() { { printf '%b' "$2" >&"${FD[$1]}"; } 2>/dev/null; }                           # octets bruts
mark()  { MARK[$1]=$(wc -l < "${LOG[$1]}" | tr -d ' '); }
markall() { local n; for n in "${!FD[@]}"; do mark "$n"; done; }
cmd()   { mark "$1"; tx "$@"; }                       # mark puis envoi
new_out() { tail -n +$(( ${MARK[$1]} + 1 )) "${LOG[$1]}" | tr -d '\r'; }
nm()    { printf '(^| )%s( |$)' "$1"; }               # regex d'un code numerique
show_tail() { new_out "$1" | tail -n 4 | sed 's/^/           | /'; }

wait_for() { # NAME REGEX [timeout]
	local n=$1 re=$2 t=${3:-$WAIT} i=0 max
	max=$(awk -v t="$t" 'BEGIN{print int(t*10)}')
	while :; do
		new_out "$n" | grep -Eq -- "$re" && return 0
		[ "$i" -ge "$max" ] && return 1
		sleep 0.1; i=$((i+1))
	done
}
rx() { # NAME REGEX DESC : doit recevoir
	if wait_for "$1" "$2"; then ok "$3"
	else ko "$3" "attendu /$2/ pour $1"; show_tail "$1"; fi
}
nrx() { # NAME REGEX DESC : ne doit PAS recevoir
	sleep "$NEG"
	if new_out "$1" | grep -Eq -- "$2"; then ko "$3" "recu /$2/ pour $1 (inattendu)"; show_tail "$1"
	else ok "$3"; fi
}
alive() { # NAME DESC : le serveur repond encore a ce client
	cmd "$1" "PING alivechk"; rx "$1" "PONG.*alivechk" "$2"
}
peer_closed() { # NAME : le serveur a ferme la connexion ?
	local i
	for i in $(seq 1 20); do
		kill -0 "${PID[$1]}" 2>/dev/null || return 0
		sleep 0.1
	done
	return 1
}

reg() { # NAME NICK : connexion + enregistrement complet
	if ! new "$1"; then ko "setup: connexion de $1 impossible"; return 1; fi
	tx "$1" "PASS $PASSW"; tx "$1" "NICK $2"; tx "$1" "USER $2 0 * :$2 test"
	if ! wait_for "$1" "$(nm 001)" 3; then
		ko "setup: $1 ($2) n'a pas recu 001"; show_tail "$1"; return 1
	fi
	sleep 0.2; mark "$1"; return 0
}
jn() { # NAME #chan [key] : JOIN et attend la fin de la liste de noms
	mark "$1"; tx "$1" "JOIN $2${3:+ $3}"
	wait_for "$1" "$(nm 366)|$(nm 47[135])|$(nm 403)" 1.5
	sleep 0.15; mark "$1"
}
modes_of() { # NAME #chan : lettres de modes dans le dernier 324
	new_out "$1" | grep -E "$(nm 324)" | grep -Eo "$2 \+[a-z]+" | tail -n 1 | sed 's/.*+//'
}
count_fds() {
	if [ -d "/proc/$SRV_PID/fd" ]; then ls "/proc/$SRV_PID/fd" | wc -l
	else lsof -p "$SRV_PID" 2>/dev/null | wc -l; fi
}
flood_bg() { # NAME CHAN N : envoie N PRIVMSG en arriere-plan
	( yes "PRIVMSG $2 :flood"$'\r' | head -n "$3" >&"${FD[$1]}" ) 2>/dev/null &
	FLOOD_PID=$!
}

# ----------------------------------------------------------------------------
#  1. Build / Makefile
# ----------------------------------------------------------------------------
mtime() { stat -c %Y "$1" 2>/dev/null || stat -f %m "$1" 2>/dev/null; }

# Retire commentaires (// et /* */) et contenu des chaines/caracteres, pour que
# "poll() failed" ou un commentaire ne soient pas comptes comme du code.
# Sortie : "fichier:ligne: code"
strip_code() { # FILE
	awk -v f="$1" '
	BEGIN { inblock = 0 }
	{
		line = $0; out = ""; i = 1; n = length(line); state = inblock ? "B" : "C"
		while (i <= n) {
			c = substr(line, i, 1); d = substr(line, i, 2)
			if (state == "B") {
				if (d == "*/") { state = "C"; i += 2 } else i++
			} else if (state == "S") {
				if (c == "\\") i += 2
				else if (c == "\"") { out = out "\""; state = "C"; i++ }
				else i++
			} else if (state == "H") {
				if (c == "\\") i += 2
				else if (c == "\047") { out = out "\047"; state = "C"; i++ }
				else i++
			} else {
				if (d == "//") break
				else if (d == "/*") { state = "B"; i += 2 }
				else if (c == "\"") { out = out "\""; state = "S"; i++ }
				else if (c == "\047") { out = out "\047"; state = "H"; i++ }
				else { out = out c; i++ }
			}
		}
		inblock = (state == "B")
		printf "%s:%d: %s\n", f, NR, out
	}' "$1"
}

t_build() {
	section "1. Compilation / Makefile / fonctions interdites"
	[ -f Makefile ] || { ko "Makefile absent"; return; }
	grep -Eq '^\$\(NAME\)[[:space:]]*:' Makefile && ok "regle \$(NAME)" || ko "regle \$(NAME) absente"
	for r in all clean fclean re; do
		grep -Eq "^$r[[:space:]]*:" Makefile && ok "regle $r" || ko "regle $r absente"
	done
	grep -q -- '-Wall' Makefile && grep -q -- '-Wextra' Makefile && grep -q -- '-Werror' Makefile \
		&& ok "flags -Wall -Wextra -Werror" || ko "flags -Wall -Wextra -Werror manquants"
	grep -q -- '-std=c++98' Makefile && ok "-std=c++98" || warn "-std=c++98 absent du Makefile (doit compiler avec)"

	make fclean > /dev/null 2>&1
	if make > "$TMP/make.log" 2>&1 && [ -x "$BIN" ]; then ok "make compile, binaire $BIN present"
	else ko "make echoue ou binaire $BIN absent"; tail -n 8 "$TMP/make.log"; return; fi
	grep -qi 'warning' "$TMP/make.log" && ko "warnings a la compilation" || ok "aucun warning"

	local t1 t2
	t1=$(mtime "$BIN"); sleep 1.1
	make 2>&1 | grep -Eqi 'nothing to be done|up to date|rien' ; t2=$(mtime "$BIN")
	[ "$t1" = "$t2" ] && ok "2e make : pas de relink" || ko "2e make : le binaire a ete relink"

	if grep -Eq '^bonus' Makefile; then
		make bonus > /dev/null 2>&1 && ok "make bonus compile" || ko "make bonus echoue"
		t1=$(mtime "$BIN"); sleep 1.1; make bonus > /dev/null 2>&1; t2=$(mtime "$BIN")
		[ "$t1" = "$t2" ] && ok "2e make bonus : pas de relink" || ko "2e make bonus : relink"
	else skip "pas de regle bonus"; fi

	make clean > /dev/null 2>&1
	[ "$(find . -name '*.o' -not -path './.git/*' | wc -l)" -eq 0 ] && ok "make clean supprime les .o" || ko "make clean laisse des .o"
	make fclean > /dev/null 2>&1
	[ ! -e "$BIN" ] && ok "make fclean supprime $BIN" || ko "make fclean laisse $BIN"
	make re > /dev/null 2>&1 && [ -x "$BIN" ] && ok "make re recompile" || ko "make re echoue"

	# --- analyse statique du code ------------------------------------------------
	local files f bad
	files=$(find "$SRC_DIR" \( -name '*.cpp' -o -name '*.hpp' -o -name '*.tpp' -o -name '*.ipp' -o -name '*.h' \) -not -path '*/.git/*' -not -path '*/test*')
	[ -n "$files" ] || { warn "aucun source trouve dans $SRC_DIR"; return; }

	# commentaires et chaines retires avant analyse (pas de faux positifs)
	local ncode="$TMP/code_nocomment.txt"; : > "$ncode"
	for f in $files; do strip_code "$f" >> "$ncode"; done

	bad=$(grep -E 'fcntl[[:space:]]*\(' "$ncode" | grep -vE 'fcntl\([^,]+,[[:space:]]*F_SETFL,[[:space:]]*O_NONBLOCK\)')
	if [ -z "$bad" ]; then ok "fcntl uniquement sous la forme fcntl(fd, F_SETFL, O_NONBLOCK)"
	else ko "fcntl utilise autrement"; echo "$bad"; fi

	bad=$(grep -En '(^|[^A-Za-z0-9_.:])(fork|execve|exit|write|read)[[:space:]]*\(' "$ncode")
	[ -z "$bad" ] && ok "pas de fork/execve/exit/read/write" \
		|| { warn "appels suspects (a verifier, certains sont peut-etre des methodes) :"; echo "$bad" | head -n 8; }

	grep -Eq 'boost|<boost' "$ncode" && ko "Boost utilise" || ok "pas de Boost"

	info "usages d'errno (seul EINTR apres poll est tolere) :"
	grep -n 'errno' "$ncode" | sed 's/^/           /'
	info "appels recv/send (a verifier : precedes d'un POLLIN / POLLOUT) :"
	grep -En '(^|[^A-Za-z0-9_])(recv|send)[[:space:]]*\(' "$ncode" | sed 's/^/           /'
}

t_readme() {
	section "17. README"
	local r=""
	for f in README.md Readme.md readme.md; do [ -f "$f" ] && r=$f; done
	[ -n "$r" ] || { ko "README.md absent"; return; }
	head -n 1 "$r" | grep -Eq '^[*_].*42 curriculum by .+[*_]$' && ok "1re ligne en italique avec les logins" \
		|| ko "1re ligne attendue : *This project has been created as part of the 42 curriculum by <login>.*"
	grep -Eiq '^#+.*description' "$r" && ok "section Description" || ko "section Description"
	grep -Eiq '^#+.*instructions' "$r" && ok "section Instructions" || ko "section Instructions"
	grep -Eiq '^#+.*resources' "$r" && ok "section Resources" || ko "section Resources"
	grep -Eiq '\b(AI|IA|Claude|ChatGPT|Copilot)\b' "$r" && ok "usage de l'IA mentionne" || warn "usage de l'IA non mentionne dans Resources"
}

# ----------------------------------------------------------------------------
#  3. Arguments
# ----------------------------------------------------------------------------
run_args() { # DESC args... : le serveur doit refuser de demarrer proprement
	local desc=$1 p i rc; shift
	"$BIN" "$@" > "$TMP/args.out" 2>&1 &
	p=$!
	for i in $(seq 1 10); do kill -0 "$p" 2>/dev/null || break; sleep 0.15; done
	if kill -0 "$p" 2>/dev/null; then
		kill -9 "$p" 2>/dev/null; { wait "$p"; } 2>/dev/null
		ko "$desc" "le serveur a demarre alors qu'il devait refuser"; return
	fi
	{ wait "$p"; } 2>/dev/null; rc=$?
	if   [ "$rc" -ge 128 ]; then ko "$desc" "CRASH (signal $((rc-128)))"
	elif [ "$rc" -eq 0 ];   then ko "$desc" "exit 0 alors qu'une erreur etait attendue"
	elif [ ! -s "$TMP/args.out" ]; then ko "$desc" "aucun message d'erreur / usage"
	else ok "$desc"; fi
}

t_args() {
	section "3. Arguments du programme"
	[ -x "$BIN" ] || { ko "$BIN introuvable (make d'abord)"; return; }
	run_args "sans argument -> usage"
	run_args "un seul argument (port) -> usage" "$PORT"
	run_args "port non numerique (abc)" abc "$PASSW"
	run_args "port melange (12ab)" 12ab "$PASSW"
	run_args "port negatif (-1)" -1 "$PASSW"
	run_args "port > 65535 (70000)" 70000 "$PASSW"
	if [ "$(id -u)" -ne 0 ]; then run_args "port privilegie 80 sans root" 80 "$PASSW"
	else skip "port 80 (tu es root)"; fi
	run_args "mot de passe vide" "$PORT" ""
}

# ----------------------------------------------------------------------------
#  4. Reseau
# ----------------------------------------------------------------------------
t_net() {
	section "4. Connexions reseau"
	if command -v ss > /dev/null 2>&1; then
		ss -ltn 2>/dev/null | awk '{print $4}' | grep -Eq "^(0\.0\.0\.0|\*|\[::\]):$PORT$" \
			&& ok "le serveur ecoute sur toutes les interfaces (0.0.0.0)" \
			|| warn "le serveur n'ecoute pas sur 0.0.0.0:$PORT"
	else skip "ss absent : ecoute sur toutes les interfaces non verifiee"; fi

	if command -v nc > /dev/null 2>&1; then
		{ printf 'PASS %s\r\nNICK ncuser\r\nUSER ncuser 0 * :nc\r\nPING ncok\r\n' "$PASSW"; sleep 1; } \
			| nc -w 2 $HOST "$PORT" > "$TMP/nc.out" 2>&1
		grep -q ' 001' "$TMP/nc.out" && grep -q 'PONG' "$TMP/nc.out" && ok "nc : enregistrement + PING/PONG" \
			|| { ko "nc : pas de 001/PONG"; head -n 5 "$TMP/nc.out"; }
	else skip "nc non installe"; fi

	reg N1 net1; reg N2 net2; reg N3 net3; reg N4 net4; reg N5 net5
	local n ok_all=1
	for n in N1 N2 N3 N4 N5; do cmd $n "PING multi_$n"; done
	for n in N1 N2 N3 N4 N5; do wait_for $n "PONG.*multi_$n" || ok_all=0; done
	[ $ok_all = 1 ] && ok "5 clients simultanees recoivent leurs reponses" || ko "un des 5 clients simultanes n'a pas recu de reponse"

	jn N1 "#net"; jn N2 "#net"; jn N3 "#net"; markall
	cmd N1 "PRIVMSG #net :hello channel"
	rx N2 "PRIVMSG #net :?hello channel" "message de channel recu par le membre 2"
	rx N3 "PRIVMSG #net :?hello channel" "message de channel recu par le membre 3"
	nrx N1 "hello channel" "l'emetteur ne recoit pas son propre message"
	nrx N4 "hello channel" "un non-membre ne recoit rien"
	closeall
}

# ----------------------------------------------------------------------------
#  5. Commandes partielles
# ----------------------------------------------------------------------------
t_partial() {
	section "5. Commandes partielles / flux TCP"
	reg P1 part1; reg P2 part2; reg P3 part3

	# exemple du sujet : com^Dman^Dd
	mark P1; txraw P1 'com'; sleep 0.3; txraw P1 'man'; sleep 0.3; txraw P1 'd\r\n'
	srx P1 "$(nm 421).*command" "com ^D man ^D d -> 421 Unknown command 'command'"

	jn P2 "#part"; jn P3 "#part"; markall
	txraw P2 'PRIV'; sleep 0.3; txraw P2 'MSG #part :sal'; sleep 0.3; txraw P2 'ut\r\n'
	rx P3 "PRIVMSG #part :?salut" "PRIVMSG decoupe en 3 morceaux -> 'salut' arrive entier"

	# commande jamais terminee sur un client : les autres fonctionnent
	txraw P2 'PRIVMSG #part :jamais fini'
	alive P1 "commande a moitie envoyee sur P2 : P1 repond toujours"
	nrx P3 "jamais fini" "la commande incomplete n'est pas executee"

	# deux clients entrelaces
	markall
	txraw P1 'PIN'; txraw P3 'PIN'; sleep 0.2; txraw P1 'G alp'; txraw P3 'G be'; sleep 0.2
	txraw P1 'ha\r\n'; txraw P3 'ta\r\n'
	rx P1 "PONG.*alpha" "entrelacement : P1 recoit alpha"
	rx P3 "PONG.*beta"  "entrelacement : P3 recoit beta"
	nrx P1 "PONG.*beta" "entrelacement : les buffers ne se melangent pas"

	# octet par octet
	local s="PING slowbytes" i; mark P1
	for ((i=0; i<${#s}; i++)); do txraw P1 "${s:i:1}"; sleep 0.03; done
	txraw P1 '\r\n'
	rx P1 "PONG.*slowbytes" "envoi octet par octet reconstruit correctement"

	# plusieurs commandes dans un paquet + LF seul
	new P4
	{ printf 'PASS %s\r\nNICK part4\r\nUSER part4 0 * :x\r\nPING batch\r\n' "$PASSW" >&"${FD[P4]}"; } 2>/dev/null
	rx P4 "$(nm 001)" "3 commandes dans un seul paquet -> 001"
	rx P4 "PONG.*batch" "4e commande du meme paquet traitee"
	new P5
	txlf P5 "PASS $PASSW"; txlf P5 "NICK part5"; txlf P5 "USER part5 0 * :x"
	rx P5 "$(nm 001)" "lignes terminees par \\n seul acceptees"

	# lignes vides
	mark P1; txraw P1 '\r\n'; txraw P1 '   \r\n'; txraw P1 '\n'
	nrx P1 "$(nm 421)" "lignes vides / espaces ignorees"
	alive P1 "serveur OK apres lignes vides"

	# ligne > 512 sans \n
	new P6
	{ head -c 2000 /dev/zero | tr '\0' 'A' >&"${FD[P6]}"; } 2>/dev/null
	peer_closed P6 && ok "ligne > 512 octets sans \\n -> client deconnecte" || warn "ligne > 512 octets : client NON deconnecte"
	alive P1 "serveur OK apres la ligne trop longue"

	# donnees aleatoires
	new P7
	{ head -c 1000 /dev/urandom >&"${FD[P7]}"; } 2>/dev/null
	sleep 0.5
	server_alive && ok "donnees aleatoires : pas de crash" || ko "le serveur est mort apres des donnees aleatoires"
	alive P1 "serveur toujours reactif apres /dev/urandom"
	closeall
}

# ----------------------------------------------------------------------------
#  6. Kill brutal
# ----------------------------------------------------------------------------
t_kill() {
	section "6. Kill brutal de clients"
	reg K1 kill1; reg K2 kill2
	jn K1 "#kill"; jn K2 "#kill"; markall
	closec K1
	srx K2 "QUIT" "les autres recoivent un QUIT apres le kill"
	server_alive && ok "le serveur survit au kill" || ko "le serveur est mort"
	reg K3 kill3 && ok "un nouveau client se connecte normalement"
	reg K4 kill1 && ok "le nick du client tue est de nouveau disponible" || ko "nick 'kill1' toujours bloque"

	jn K3 "#kill"; markall
	txraw K4 'PRIVMSG #kill :ghost'; closec K4
	nrx K2 "ghost" "kill avec commande a moitie envoyee : pas de commande fantome"
	alive K2 "serveur OK apres kill avec commande partielle"

	new K5; tx K5 "PASS $PASSW"; closec K5
	new K6; tx K6 "PASS $PASSW"; tx K6 "NICK halfreg"; closec K6
	alive K2 "serveur OK apres kill pendant PASS/NICK/USER"
	closeall
}

# ----------------------------------------------------------------------------
#  7. Ctrl+Z et flood
# ----------------------------------------------------------------------------
t_flood() {
	section "7. Ctrl+Z (client bloque) et flood ($FLOOD_N messages)"
	reg FA flooda; reg FB floodb; reg FC floodc
	jn FA "#flood"; jn FB "#flood"; jn FC "#flood"; markall

	kill -STOP "${PID[FA]}"       # FA ne lit plus : equivalent d'un Ctrl+Z
	info "FA est gele (SIGSTOP sur son lecteur), flood par FB..."
	flood_bg FB "#flood" "$FLOOD_N"
	local fpid=$FLOOD_PID i good=0
	for i in 1 2 3 4 5; do
		cmd FC "PING during$i"
		wait_for FC "PONG.*during$i" 4 && good=$((good+1))
		sleep 0.2
	done
	[ $good -eq 5 ] && ok "pendant le flood, FC recoit ses 5/5 reponses (serveur non bloque)" \
		|| ko "pendant le flood FC n'a recu que $good/5 reponses"

	for i in $(seq 1 120); do kill -0 "$fpid" 2>/dev/null || break; sleep 0.5; done
	if kill -0 "$fpid" 2>/dev/null; then kill -9 "$fpid" 2>/dev/null; ko "le flood n'a pas fini en 60 s (serveur bloque ?)"
	else ok "le flood est entierement consomme par le serveur"; fi
	server_alive && info "memoire serveur : $(ps -o rss= -p "$SRV_PID" 2>/dev/null | awk '{printf "%.1f Mo", $1/1024}')"

	kill -CONT "${PID[FA]}"       # fg
	local cnt=0
	for i in $(seq 1 60); do
		cnt=$(grep -c 'PRIVMSG #flood :flood' "${LOG[FA]}")
		[ "$cnt" -ge "$FLOOD_N" ] && break; sleep 0.5
	done
	[ "$cnt" -ge "$FLOOD_N" ] && ok "FA (fg) recoit les $FLOOD_N messages en attente" \
		|| ko "FA n'a recu que $cnt/$FLOOD_N messages apres le fg"
	alive FC "serveur OK apres le test"
	closeall
}

# ----------------------------------------------------------------------------
#  9. Authentification
# ----------------------------------------------------------------------------
t_auth() {
	section "9. Authentification"
	new A1; tx A1 "PASS $PASSW"; tx A1 "NICK auth1"; tx A1 "USER auth1 0 * :Au One"
	rx A1 "$(nm 001)" "PASS/NICK/USER -> 001"
	new A2; tx A2 "PASS $PASSW"; tx A2 "USER auth2 0 * :x"; tx A2 "NICK auth2"
	rx A2 "$(nm 001)" "ordre USER puis NICK -> 001"

	new A3; tx A3 "JOIN #a"; rx A3 "$(nm 451)" "commande avant inscription -> 451"
	new A4; tx A4 "PASS"; rx A4 "$(nm 461)" "PASS sans argument -> 461"
	new A5; tx A5 "PASS wrongpass"; tx A5 "NICK auth5"; tx A5 "USER auth5 0 * :x"
	rx A5 "$(nm 464)" "mauvais PASS -> 464"
	nrx A5 "$(nm 001)" "mauvais PASS : pas de bienvenue"
	cmd A1 "PASS $PASSW"; srx A1 "$(nm 462)" "PASS apres inscription -> 462"

	new A7; tx A7 "NICK auth7"; tx A7 "USER auth7 0 * :x"
	nrx A7 "$(nm 001)" "NICK/USER sans PASS : pas inscrit"

	cmd A1 "NICK"; srx A1 "$(nm 431)" "NICK sans argument -> 431"
	local bad
	for bad in 1abc '#a' 'a:b' 'abcdefghijklmnopqrstuvwxyz0123456789'; do
		cmd A1 "NICK $bad"; srx A1 "$(nm 432)" "nick invalide '$bad' -> 432"
	done
	reg A8 auth8
	cmd A8 "NICK auth1"; rx A8 "$(nm 433)" "nick deja pris -> 433"
	cmd A8 "NICK AUTH1"; srx A8 "$(nm 433)" "nick deja pris avec autre casse -> 433"

	new U1; tx U1 "PASS $PASSW"; tx U1 "NICK auth9"; cmd U1 "USER auth9"
	rx U1 "$(nm 461)" "USER sans assez de parametres -> 461"
	cmd A2 "USER again 0 * :x"; srx A2 "$(nm 462)" "USER deux fois -> 462"

	reg L1 lowcase
	cmd L1 "join #low"; rx L1 "JOIN :?#low" "commande en minuscules (join) acceptee"
	cmd L1 "nick lowernick"; rx L1 "NICK :?lowernick" "commande en minuscules (nick) acceptee"
	cmd A2 "FOO"; srx A2 "$(nm 421)" "commande inconnue -> 421"
	cmd A2 "PING hello"; srx A2 "PONG.*hello" "PING x -> PONG x"
	cmd A2 "PING"; srx A2 "$(nm 409)" "PING sans argument -> 409"

	reg C1 nick1; reg C2 nick2
	jn C1 "#nk"; jn C2 "#nk"; markall
	cmd C1 "NICK nickchg"
	rx C1 "NICK :?nickchg" "changement de nick : confirme a l'interesse"
	rx C2 "NICK :?nickchg" "changement de nick : diffuse aux membres du channel"

	reg Q1 quit1; reg Q2 quit2
	jn Q1 "#qt"; jn Q2 "#qt"; markall
	cmd Q1 "QUIT :bye"
	srx Q2 "QUIT" "QUIT diffuse aux membres"
	peer_closed Q1 && ok "QUIT : socket ferme" || warn "QUIT : socket toujours ouvert"
	closeall
}

# ----------------------------------------------------------------------------
#  10. PRIVMSG
# ----------------------------------------------------------------------------
t_privmsg() {
	section "10. PRIVMSG"
	reg P1 pm1; reg P2 pm2; reg P3 pm3; reg P4 pm4
	cmd P1 "PRIVMSG pm2 :salut toi"
	rx P2 "PRIVMSG pm2 :?salut toi" "PRIVMSG user avec ':' -> message complet"
	cmd P1 "PRIVMSG pm2 salut"
	rx P2 "PRIVMSG pm2 :?salut$" "PRIVMSG user sans ':' -> 'salut'"
	jn P1 "#pm"; jn P2 "#pm"; jn P3 "#pm"; markall
	cmd P1 "PRIVMSG #pm :hello chan"
	rx P2 "hello chan" "PRIVMSG channel recu par le membre 2"
	rx P3 "hello chan" "PRIVMSG channel recu par le membre 3"
	nrx P1 "hello chan" "l'emetteur ne recoit pas son message"
	cmd P4 "PRIVMSG #pm :intrus"; srx P4 "$(nm 404)" "PRIVMSG vers channel dont on n'est pas membre -> 404"
	cmd P1 "PRIVMSG ghostnick :x"; rx P1 "$(nm 401)" "PRIVMSG nick inexistant -> 401"
	cmd P1 "PRIVMSG #nochan :x"; rx P1 "$(nm 40[13])" "PRIVMSG channel inexistant -> 403 (ou 401)"
	cmd P1 "PRIVMSG"; rx P1 "$(nm 411)" "PRIVMSG seul -> 411"
	cmd P1 "PRIVMSG pm2"; rx P1 "$(nm 412)" "PRIVMSG sans texte -> 412"
	cmd P1 "PRIVMSG pm2,pm3 :multi"
	if wait_for P2 "multi" && wait_for P3 "multi"; then ok "plusieurs cibles geree (bonus)"
	else info "plusieurs cibles non geree (optionnel selon le sujet)"; fi
	cmd P1 "PRIVMSG pm1 :moi-meme"; alive P1 "message a soi-meme : pas de crash"
	closeall
}

# ----------------------------------------------------------------------------
#  11. Channels
# ----------------------------------------------------------------------------
t_channel() {
	section "11. Channels (JOIN / PART / TOPIC / WHO)"
	reg C1 ch1; reg C2 ch2; reg C3 ch3
	cmd C1 "JOIN #chan"; rx C1 "$(nm 353).*@ch1" "premier JOIN : channel cree, on est op (@ dans 353)"
	cmd C2 "JOIN nochan"; rx C2 "$(nm 40[3]|47[6])" "JOIN sans # -> 403/476"
	cmd C2 "JOIN"; rx C2 "$(nm 461)" "JOIN sans argument -> 461"
	jn C1 "#cha"; markall
	cmd C2 "JOIN #cha,#chb"
	rx C2 "JOIN :?#cha" "JOIN #cha,#chb : rejoint #cha"
	rx C2 "JOIN :?#chb" "JOIN #cha,#chb : rejoint #chb"
	mark C1; cmd C2 "JOIN #cha"
	snrx C1 "JOIN" "JOIN d'un channel deja rejoint : ignore (pas de 2e JOIN diffuse)"

	jn C1 "#pt"; jn C2 "#pt"; markall
	cmd C2 "PART #pt :bye"
	rx C1 "PART #pt" "PART diffuse aux membres"
	rx C2 "PART #pt" "PART visible par celui qui part"
	cmd C1 "PART #pt"; sleep 0.2
	cmd C3 "JOIN #pt"; rx C3 "$(nm 353).*@ch3" "dernier membre parti -> channel supprime (le suivant est op)"
	cmd C3 "PART #cha"; srx C3 "$(nm 442)" "PART d'un channel dont on n'est pas membre -> 442"
	cmd C3 "PART #nochan"; srx C3 "$(nm 403)" "PART d'un channel inexistant -> 403"

	jn C1 "#tp"; jn C2 "#tp"; markall
	cmd C1 "TOPIC #tp"; rx C1 "$(nm 331)" "TOPIC sans topic -> 331"
	mark C2; cmd C1 "TOPIC #tp :nouveau topic"
	rx C2 "TOPIC #tp :?nouveau topic" "TOPIC diffuse aux membres"
	cmd C2 "TOPIC #tp"; rx C2 "$(nm 332)" "TOPIC avec topic -> 332"

	cmd C1 "WHO #tp"; wait_for C1 "$(nm 315)" > /dev/null
	local n; n=$(new_out C1 | grep -Ec "$(nm 352)")
	[ "$n" -eq 2 ] && ok "WHO #tp : 352 pour chaque membre (2) puis 315" || warn "WHO #tp : $n lignes 352 au lieu de 2"

	cmd C1 "PART #tp"; sleep 0.2
	cmd C2 "PRIVMSG #tp :toujours la"; alive C2 "l'op est parti : le serveur et le channel restent coherents"
	cmd C2 "WHO #tp"; wait_for C2 "$(nm 315)" > /dev/null
	new_out C2 | grep -E "$(nm 352)" | grep -q '@' && info "un nouvel op a ete designe automatiquement" \
		|| info "le channel n'a plus d'op (comportement a pouvoir justifier en soutenance)"
	closeall
}

# ----------------------------------------------------------------------------
#  12. Droits d'operateur (user non-op)
# ----------------------------------------------------------------------------
t_perms() {
	section "12. Un user non-op ne peut pas utiliser les commandes d'op"
	reg O1 opr1; reg O2 usr2; reg O3 usr3; reg O4 usr4
	jn O1 "#perm"; jn O2 "#perm"; jn O3 "#perm"
	cmd O1 "MODE #perm +it"; sleep 0.3; markall
	cmd O2 "KICK #perm opr1 :x";      rx O2 "$(nm 482)" "KICK par non-op -> 482"
	cmd O2 "INVITE usr4 #perm";       rx O2 "$(nm 482)" "INVITE sur channel +i par non-op -> 482"
	cmd O2 "TOPIC #perm :nope";       rx O2 "$(nm 482)" "TOPIC sur channel +t par non-op -> 482"
	local m
	for m in "+i" "+t" "+k key" "+l 5" "+o usr3"; do
		cmd O2 "MODE #perm $m"; rx O2 "$(nm 482)" "MODE #perm $m par non-op -> 482"
	done
	closeall
}

# ----------------------------------------------------------------------------
#  13. KICK
# ----------------------------------------------------------------------------
t_kick() {
	section "13. KICK"
	reg K1 kop; reg K2 kb; reg K3 kc; reg K4 kd
	jn K1 "#kick"; jn K2 "#kick"; jn K3 "#kick"; markall
	cmd K1 "KICK #kick kb :raison"
	rx K1 "KICK #kick kb" "KICK visible par l'op"
	rx K2 "KICK #kick kb" "KICK visible par la cible"
	rx K3 "KICK #kick kb" "KICK visible par les autres membres"
	cmd K2 "PRIVMSG #kick :encore la"; srx K2 "$(nm 404)" "la cible est bien retiree du channel (404)"
	cmd K1 "KICK #kick kd";        srx K1 "$(nm 441)" "KICK cible hors channel -> 441"
	cmd K1 "KICK #kick nobodyxx";  rx K1 "$(nm 401)" "KICK cible inexistante -> 401"
	cmd K1 "KICK #nochan kc";      rx K1 "$(nm 403)" "KICK channel inexistant -> 403"
	cmd K1 "KICK #kick";           rx K1 "$(nm 461)" "KICK sans assez de parametres -> 461"
	cmd K1 "KICK #kick kop :moi";  alive K3 "se kicker soi-meme : pas de crash"
	closeall
}

# ----------------------------------------------------------------------------
#  14. INVITE
# ----------------------------------------------------------------------------
t_invite() {
	section "14. INVITE"
	reg I1 iop; reg I2 iby; reg I3 imem; reg I4 iout
	jn I1 "#inv"; jn I3 "#inv"; markall
	cmd I1 "INVITE iby #inv"
	rx I1 "$(nm 341)" "INVITE -> 341 pour l'op"
	rx I2 "INVITE iby :?#inv" "l'invite recoit l'INVITE"
	cmd I1 "INVITE imem #inv";    srx I1 "$(nm 443)" "inviter un membre -> 443"
	cmd I1 "INVITE ghostnx #inv"; rx I1 "$(nm 401)" "inviter un nick inconnu -> 401"
	cmd I4 "INVITE iby #inv";     srx I4 "$(nm 442)" "inviter sans etre dans le channel -> 442"
	closeall
}

# ----------------------------------------------------------------------------
#  15. MODE
# ----------------------------------------------------------------------------
t_mode() {
	section "15. MODE (i t k l o)"
	reg M1 mop; reg M2 mu2; reg M3 mu3; reg M4 mu4

	# +i
	jn M1 "#mi"; cmd M1 "MODE #mi +i"; rx M1 "MODE #mi :?\+i" "MODE +i confirme"
	cmd M2 "JOIN #mi"; rx M2 "$(nm 473)" "+i : JOIN sans invitation -> 473"
	cmd M1 "INVITE mu2 #mi"; sleep 0.3
	cmd M2 "JOIN #mi"; rx M2 "JOIN :?#mi" "+i : l'invite peut rejoindre"
	cmd M1 "MODE #mi -i"; sleep 0.3
	cmd M3 "JOIN #mi"; rx M3 "JOIN :?#mi" "-i : tout le monde peut rejoindre"

	# +t
	jn M1 "#mt"; jn M2 "#mt"; cmd M1 "MODE #mt +t"; sleep 0.3
	cmd M2 "TOPIC #mt :x"; rx M2 "$(nm 482)" "+t : un non-op ne peut pas changer le topic"
	cmd M1 "MODE #mt -t"; sleep 0.3
	cmd M2 "TOPIC #mt :ok"; rx M2 "TOPIC #mt :?ok" "-t : tout le monde peut changer le topic"

	# +k
	jn M1 "#mk"
	cmd M1 "MODE #mk +k"; rx M1 "$(nm 461)" "+k sans parametre -> 461"
	cmd M1 "MODE #mk +k secret"; sleep 0.3
	cmd M2 "JOIN #mk";         rx M2 "$(nm 475)" "+k : JOIN sans cle -> 475"
	cmd M2 "JOIN #mk wrong";   rx M2 "$(nm 475)" "+k : mauvaise cle -> 475"
	cmd M2 "JOIN #mk secret";  rx M2 "JOIN :?#mk" "+k : bonne cle -> JOIN OK"
	cmd M1 "MODE #mk -k"; sleep 0.3
	cmd M3 "JOIN #mk";         rx M3 "JOIN :?#mk" "-k : plus de cle requise"

	# +l
	jn M1 "#ml"; cmd M1 "MODE #ml +l 2"; sleep 0.3
	cmd M2 "JOIN #ml"; rx M2 "JOIN :?#ml" "+l 2 : 2e membre accepte"
	cmd M3 "JOIN #ml"; rx M3 "$(nm 471)" "+l 2 : 3e membre refuse -> 471"
	cmd M1 "MODE #ml -l"; sleep 0.3
	cmd M3 "JOIN #ml"; rx M3 "JOIN :?#ml" "-l : plus de limite"
	jn M1 "#ml2"
	local v
	for v in 0 -5 abc; do cmd M1 "MODE #ml2 +l $v"; sleep 0.2; done
	cmd M1 "MODE #ml2"; wait_for M1 "$(nm 324)" > /dev/null
	modes_of M1 "#ml2" | grep -q 'l' && warn "+l 0 / -5 / abc ont ete acceptes" || ok "+l 0, +l -5, +l abc refuses"

	# +o
	jn M1 "#mo"; jn M2 "#mo"; markall
	cmd M1 "MODE #mo +o mu2"; rx M2 "MODE #mo :?\+o mu2" "+o : diffuse aux membres"
	cmd M2 "MODE #mo +i"; nrx M2 "$(nm 482)" "+o : le nouvel op peut utiliser MODE"
	cmd M1 "MODE #mo -o mu2"; sleep 0.3
	cmd M2 "MODE #mo -i"; rx M2 "$(nm 482)" "-o : l'ancien op est refuse (482)"
	cmd M1 "MODE #mo +o mu4"; srx M1 "$(nm 441)" "+o sur un nick absent du channel -> 441"
	cmd M1 "MODE #mo -o mop"; alive M1 "-o sur soi-meme : pas de crash"

	# combinaisons et affichage
	jn M1 "#mc"; cmd M1 "MODE #mc +itk key"; sleep 0.3
	cmd M1 "MODE #mc"; wait_for M1 "$(nm 324)" > /dev/null
	local mm; mm=$(modes_of M1 "#mc")
	[[ $mm == *i* && $mm == *t* && $mm == *k* ]] && ok "+itk key -> 324 affiche '$mm'" || ko "+itk key : modes affiches '$mm'"
	cmd M1 "MODE #mc +i-t"; sleep 0.3; cmd M1 "MODE #mc"; wait_for M1 "$(nm 324)" > /dev/null
	mm=$(modes_of M1 "#mc")
	[[ $mm == *i* && $mm != *t* ]] && ok "+i-t -> '$mm'" || ko "+i-t : modes affiches '$mm'"
	jn M1 "#mc2"; cmd M1 "MODE #mc2 +kl key 5"; sleep 0.3; cmd M1 "MODE #mc2"; wait_for M1 "$(nm 324)" > /dev/null
	mm=$(modes_of M1 "#mc2")
	[[ $mm == *k* && $mm == *l* ]] && ok "+kl key 5 -> '$mm'" || ko "+kl key 5 : modes affiches '$mm'"
	cmd M1 "MODE #mc +x"; srx M1 "$(nm 472)" "mode inconnu +x -> 472"
	cmd M1 "MODE #nochan +i"; rx M1 "$(nm 403)" "MODE sur channel inexistant -> 403"
	cmd M1 "MODE mop +i"; alive M1 "mode utilisateur (MODE nick +i) : pas de crash"
	closeall
}

# ----------------------------------------------------------------------------
#  8. Robustesse
# ----------------------------------------------------------------------------
t_robust() {
	section "8. Robustesse / fd / charge"
	run_args "deux serveurs sur le meme port -> erreur bind sans crash" "$PORT" "$PASSW"

	local before after i fd ok_n=0
	sleep 0.5; before=$(count_fds)
	for i in $(seq 1 50); do
		if eval "exec 300<>/dev/tcp/$HOST/$PORT" 2>/dev/null; then
			{ printf 'PASS %s\r\nNICK fdt%d\r\nUSER f 0 * :f\r\n' "$PASSW" "$i" >&300; } 2>/dev/null
			eval "exec 300>&-"
		fi
	done
	sleep 1.5; after=$(count_fds)
	[ "$before" = "$after" ] && ok "meme nombre de fd avant/apres 50 connexions ($before)" \
		|| ko "fd avant=$before apres=$after : fuite de descripteurs"

	for i in $(seq 1 200); do
		fd=$((300+i))
		if eval "exec $fd<>/dev/tcp/$HOST/$PORT" 2>/dev/null; then
			ok_n=$((ok_n+1))
			{ printf 'PASS %s\r\nNICK mass%d\r\nUSER m 0 * :m\r\n' "$PASSW" "$i" >&"$fd"; } 2>/dev/null
		fi
	done
	info "$ok_n/200 connexions ouvertes"
	sleep 2
	server_alive && ok "200 connexions simultanees : pas de crash" || ko "le serveur est mort pendant les 200 connexions"
	reg R1 robust1 && alive R1 "le serveur repond pendant que 200 clients sont connectes"
	for i in $(seq 1 200); do eval "exec $((300+i))>&-" 2>/dev/null; done
	sleep 1
	alive R1 "le serveur repond apres la fermeture des 200 clients"
	closeall
}

t_lowmem() {
	section "8b. Memoire limitee (ulimit -v 50000)"
	local oldport=$PORT lp
	PORT=$((PORT+1))
	( ulimit -v 50000 2>/dev/null || exit 3; exec "$BIN" "$PORT" "$PASSW" ) > "$TMP/low.log" 2>&1 &
	lp=$!
	if ! wait_port "$lp"; then
		skip "le serveur ne demarre pas avec ulimit -v 50000 (ou ulimit non supporte)"
		kill -9 "$lp" 2>/dev/null; PORT=$oldport; return
	fi
	reg LA lowa; reg LB lowb
	jn LA "#low"; jn LB "#low"; markall
	flood_bg LA "#low" 5000
	local fp=$FLOOD_PID; wait "$fp" 2>/dev/null; sleep 1
	if kill -0 "$lp" 2>/dev/null; then
		ok "pas de crash/abort sous ulimit -v 50000 + flood"
		alive LB "le serveur repond toujours"
	else warn "le serveur est mort sous contrainte memoire"; fi
	closeall
	kill -INT "$lp" 2>/dev/null; { wait "$lp"; } 2>/dev/null
	PORT=$oldport
}

# ----------------------------------------------------------------------------
#  3b. Signaux
# ----------------------------------------------------------------------------
t_signals() {
	section "3b. Arret propre sur signaux"
	local sig
	for sig in INT QUIT TERM HUP; do
		if ! start_server; then ko "demarrage du serveur impossible (SIG$sig)"; stop_server KILL; continue; fi
		reg SG "sg$sig" > /dev/null
		stop_server "$sig"
		sigfn=warn; [ "$sig" = INT ] && sigfn=ko   # seul Ctrl+C (SIGINT) est exige par le sujet
		if [ "$SRV_RC" -eq 137 ]; then $sigfn "SIG$sig : le serveur ne s'arrete pas (kill -9 necessaire)"
		elif [ "$SRV_RC" -ge 128 ]; then $sigfn "SIG$sig : le serveur est mort par signal (code $SRV_RC), pas d'arret propre"
		else ok "SIG$sig : arret propre (exit $SRV_RC)"; fi
		closeall
		check_valgrind "SIG$sig"
	done
}

# ----------------------------------------------------------------------------
#  Helpers supplementaires
# ----------------------------------------------------------------------------
SOH=$'\001'

srx() { # soft : OK ou WARN (comportement souhaitable mais pas exige par le sujet)
	if wait_for "$1" "$2"; then ok "$3"; else warn "$3 (non satisfait)"; fi
}
snrx() { # soft : ne doit pas recevoir, sinon WARN
	sleep "$NEG"
	if new_out "$1" | grep -Eq -- "$2"; then warn "$3"; else ok "$3"; fi
}
irx() { # info : constate sans juger
	if wait_for "$1" "$2" 1; then info "$3 : oui"; else info "$3 : non"; fi
}
cpu_ticks() { awk '{print $14+$15}' "/proc/$1/stat" 2>/dev/null; }
rss_kb() { ps -o rss= -p "$1" 2>/dev/null | tr -d ' '; }

flood_seq_bg() { # NAME CHAN N [PAD] : N PRIVMSG numerotes "flood <i>" (+ PAD octets de remplissage)
	( awk -v n="$3" -v c="$2" -v p="${4:-0}" 'BEGIN{
		pad = sprintf("%*s", p, ""); gsub(/ /, "x", pad)
		for (i = 1; i <= n; i++) printf "PRIVMSG %s :flood %d%s\r\n", c, i, (p > 0 ? " " pad : "") }' \
		>&"${FD[$1]}" ) 2>/dev/null &
	FLOOD_PID=$!
}
check_seq() { # LOGFILE N : "flood 1".."flood N" recus dans l'ordre, sans trou ni doublon
	grep -oE 'PRIVMSG #[A-Za-z0-9]+ :flood [0-9]+' "$1" | awk '{print $4}' \
		| awk -v n="$2" 'BEGIN{bad=""} { if ($1 != NR && bad == "") bad = "rang " NR " : recu " $1 }
			END { if (bad == "" && NR == n) print "OK"; else print "BAD recus=" NR "/" n " " bad }'
}
listen_addrs() { # affiche les adresses locales en ecoute sur $PORT (ss, lsof ou /proc/net/tcp)
	if command -v ss > /dev/null 2>&1; then
		ss -ltn 2>/dev/null | awk '{print $4}' | grep -E ":$PORT$"
	elif command -v lsof > /dev/null 2>&1; then
		lsof -nP -iTCP:"$PORT" -sTCP:LISTEN 2>/dev/null | awk 'NR>1{print $9}'
	elif [ -r /proc/net/tcp ]; then
		local hp; hp=$(printf '%04X' "$PORT")
		awk -v hp="$hp" 'NR>1 && $4=="0A" { split($2, a, ":"); if (a[2]==hp) print (a[1]=="00000000" ? "0.0.0.0:" : "autre:") hp }' /proc/net/tcp
	fi
}

# ----------------------------------------------------------------------------
#  2. Analyse statique (sujet : C++98, un seul poll, errno, fcntl, ...)
# ----------------------------------------------------------------------------
prep_static() {
	SFILES=$(find "$SRC_DIR" \( -name '*.cpp' -o -name '*.hpp' -o -name '*.tpp' -o -name '*.ipp' -o -name '*.h' \) \
		-not -path '*/.git/*' -not -path '*/test*' -not -ipath "*/*${EXCLUDE_DIR}*/*")
	NCODE="$TMP/static_code.txt"; : > "$NCODE"
	local f; for f in $SFILES; do strip_code "$f" >> "$NCODE"; done
}

t_static() {
	section "2. Analyse statique supplementaire (sujet + grille)"
	prep_static
	[ -n "$SFILES" ] || { warn "aucun source trouve dans $SRC_DIR"; return; }
	local f d inc="" fails=0 bad

	# --- Makefile -------------------------------------------------------------------
	if [ -f Makefile ]; then
		grep -Eq '^NAME[[:space:]]*[:?]?=[[:space:]]*ircserv([[:space:]]|$)' Makefile \
			&& ok "Makefile : NAME = ircserv" || warn "Makefile : NAME = ircserv non trouve (le binaire doit s'appeler ircserv)"
		grep -Eq '(^|[^A-Za-z0-9_+])c\+\+([^A-Za-z0-9_+]|$)' Makefile \
			&& ok "Makefile : compilateur c++" || warn "Makefile : le sujet demande de compiler avec c++"
		grep -Eq '(^|[^A-Za-z0-9_])(g\+\+|clang\+\+)' Makefile && warn "Makefile : g++/clang++ en dur (le sujet demande c++)"
		grep -Eq '^\.PHONY' Makefile && ok "Makefile : .PHONY present" || warn "Makefile : pas de .PHONY"
		grep -Eq -- '-l(boost|pthread)' Makefile && ko "Makefile : bibliotheque externe / pthread" || ok "Makefile : aucune lib externe"
	fi
	if [ -d .git ] && command -v git > /dev/null 2>&1; then
		bad=$(git ls-files 2>/dev/null | grep -E '(^|/)ircserv$|\.o$|\.dSYM')
		[ -z "$bad" ] && ok "git : ni .o ni binaire versionnes" || warn "git : fichiers compiles versionnes : $(echo "$bad" | head -n 3 | tr '\n' ' ')"
	fi

	# --- compilation C++98 stricte de chaque .cpp -----------------------------------
	for d in $(echo "$SFILES" | grep -E '\.(hpp|h|tpp|ipp)$' | while read -r h; do dirname "$h"; done | sort -u); do inc="$inc -I$d"; done
	for f in $(echo "$SFILES" | grep -E '\.cpp$'); do
		if ! c++ -std=c++98 -Wall -Wextra -Werror $inc -I. -fsyntax-only "$f" > "$TMP/syn.out" 2>&1; then
			fails=$((fails+1)); ko "compilation -std=c++98 -Wall -Wextra -Werror : $f"
			head -n 4 "$TMP/syn.out" | sed 's/^/           /'
		fi
	done
	[ $fails -eq 0 ] && ok "chaque .cpp compile en -std=c++98 -Wall -Wextra -Werror"

	# --- interdits du sujet ---------------------------------------------------------
	bad=$(grep -En '(^|[^A-Za-z0-9_.:])(fork|vfork|execve|system)[[:space:]]*\(|pthread_|std::thread|<thread>|<pthread\.h>|<mutex>' "$NCODE")
	[ -z "$bad" ] && ok "pas de fork / threads / system()" || { ko "fork, threads ou system() utilises (interdit)"; echo "$bad" | head -n 5 | sed 's/^/           /'; }

	bad=$(grep -En '(^|[^A-Za-z0-9_.:])(sleep|usleep|nanosleep)[[:space:]]*\(' "$NCODE")
	[ -z "$bad" ] && ok "aucun sleep() dans le serveur (bloquant)" || { warn "sleep/usleep : le serveur doit rester non bloquant"; echo "$bad" | head -n 5 | sed 's/^/           /'; }

	bad=$(grep -En '\b(EAGAIN|EWOULDBLOCK)\b' "$NCODE")
	[ -z "$bad" ] && ok "ni EAGAIN ni EWOULDBLOCK dans le code" \
		|| { ko "EAGAIN/EWOULDBLOCK : interdit de se fier a errno apres recv/send (note 0)"; echo "$bad" | head -n 5 | sed 's/^/           /'; }

	bad=$(grep -E '\berrno\b' "$NCODE" | grep -vE '#[[:space:]]*include' | grep -vE '\bEINTR\b|strerror|perror')
	[ -z "$bad" ] && ok "errno : uniquement EINTR (ou affichage d'erreur)" \
		|| { ko "errno utilise pour autre chose que EINTR"; echo "$bad" | head -n 5 | sed 's/^/           /'; }
	info "usages d'errno (EINTR doit suivre poll(), jamais recv/send) :"
	grep -E '\berrno\b' "$NCODE" | grep -vE '#[[:space:]]*include' | sed 's/^/           /'

	grep -Eq '\bO_NONBLOCK\b' "$NCODE" && ok "O_NONBLOCK utilise" || ko "O_NONBLOCK absent : les sockets doivent etre non bloquants"
	grep -Eq '\b(SIGPIPE|MSG_NOSIGNAL)\b' "$NCODE" && ok "SIGPIPE gere (ignore ou MSG_NOSIGNAL)" \
		|| warn "ni SIGPIPE ni MSG_NOSIGNAL : un send() sur socket ferme tue le serveur"
	grep -Eq '\b(INADDR_ANY|AI_PASSIVE)\b' "$NCODE" && ok "ecoute sur toutes les interfaces (INADDR_ANY / AI_PASSIVE)" \
		|| warn "ni INADDR_ANY ni AI_PASSIVE : l'ecoute sur toutes les interfaces n'est pas evidente"
	grep -Eq '\bSO_REUSEADDR\b' "$NCODE" && ok "SO_REUSEADDR present" || warn "SO_REUSEADDR absent : bind() echouera apres un redemarrage rapide"
	grep -Eq '\bMSG_DONTWAIT\b' "$NCODE" && info "MSG_DONTWAIT utilise (inutile : les fds sont deja non bloquants)"
	grep -Eq '(^|[^A-Za-z0-9_])(signal|sigaction)[[:space:]]*\(' "$NCODE" && ok "gestion des signaux (arret propre)" || warn "aucun handler de signal : Ctrl+C ne fera pas de nettoyage"

	local incs
	incs=$(grep -hoE '#[[:space:]]*include[[:space:]]*<[^>]+>' "$NCODE" | sed -E 's/#[[:space:]]*include[[:space:]]*<([^>]+)>/\1/' | sort -u \
		| grep -vE '^(c[a-z0-9]+|string|vector|list|map|set|deque|queue|stack|iostream|sstream|fstream|iomanip|algorithm|iterator|stdexcept|exception|new|utility|functional|memory|limits|numeric|bitset|typeinfo|locale|ostream|istream|complex|valarray|sys/[a-z/]+\.h|netinet/[a-z]+\.h|arpa/[a-z]+\.h|unistd\.h|fcntl\.h|poll\.h|signal\.h|netdb\.h|errno\.h|string\.h|stdlib\.h|stdio\.h|stdint\.h|time\.h|ctype\.h|stdarg\.h|assert\.h|limits\.h|math\.h)$')
	[ -z "$incs" ] && ok "includes : uniquement la bibliotheque standard / POSIX" || warn "includes a verifier (lib externe interdite) : $(echo $incs)"

	info "poll/recv/send/accept (voir aussi la section strace) :"
	grep -En '(^|[^A-Za-z0-9_])(poll|accept|recv|send)[[:space:]]*\(' "$NCODE" | sed 's/^/           /'
}

# ----------------------------------------------------------------------------
#  2b. Verification dynamique : strace
#      "poll() est appele avant CHAQUE accept / recv / send" + fcntl exact
# ----------------------------------------------------------------------------
analyze_strace() { # LOG -> lignes "cle valeur" + lignes V:/E:/F:
	awk '
	function cls(n) { if (n ~ /^(recv|recvfrom|recvmsg|read)$/) return "R"
	                  if (n ~ /^(send|sendto|sendmsg|write)$/) return "W"
	                  if (n ~ /^accept4?$/) return "A"; return "" }
	function ispoll(n) { return n ~ /^(poll|ppoll|select|pselect6|epoll_wait|epoll_pwait|epoll_pwait2)$/ }
	{
		line = $0
		sub(/^(\[pid +[0-9]+\] +|[0-9]+ +)/, "", line)
		if (line ~ /^(---|\+\+\+)/ || line ~ /unfinished|resumed/) next
		if (!match(line, /^[a-z_0-9]+\(/)) next
		name = substr(line, 1, RLENGTH - 1)
		np = split(line, parts, " = "); rest = parts[np]; ret = rest; sub(/ .*/, "", ret)
		args = line; sub(/^[a-z_0-9]+\(/, "", args)
		fd = args; sub(/[^0-9].*$/, "", fd)

		if (ispoll(name)) {
			split("", ready); split("", seen); polls++
			kind = (name ~ /^p?poll$/) ? "poll" : "autre"
			trunc = (line ~ /\.\.\./)
			s = rest
			while (match(s, /fd=[0-9]+, revents=[A-Z_|]+/)) {
				piece = substr(s, RSTART, RLENGTH); s = substr(s, RSTART + RLENGTH)
				f = piece; sub(/^fd=/, "", f); sub(/,.*/, "", f)
				fl = piece; sub(/.*revents=/, "", fl); ready[f] = fl
			}
			next
		}
		if (name ~ /^socket$/) { if (line ~ /SOCK_NONBLOCK/) nb[ret] = 1; next }
		if (name ~ /^fcntl(64)?$/) {
			nf++
			if (line ~ /^fcntl(64)?\([0-9]+, F_SETFL, (O_RDONLY\|)?O_NONBLOCK\)/) nb[fd] = 1
			else { badf++; if (badf <= 5) print "F: " substr(line, 1, 100) }
			next
		}
		c = cls(name); if (c == "") next
		if ((name == "read" || name == "write") && !(fd in sock)) next
		if (name ~ /^accept4?$/) {
			sock[fd] = 1; listenfd[fd] = 1
			if (ret ~ /^[0-9]+$/) { sock[ret] = 1; acc[ret] = 1; if (line ~ /SOCK_NONBLOCK/) nb[ret] = 1 }
		}
		cnt[c]++
		key = c SUBSEP fd
		if (key in seen) { cons++; if (cons <= 5) print "V: deux " c " consecutifs sur fd " fd " sans poll entre : " substr(line, 1, 80) }
		seen[key] = 1
		if (kind == "poll" && !trunc) {
			need = (c == "W") ? "POLLOUT" : "POLLIN"
			fl = ready[fd]
			good = (fl ~ need) || (c != "W" && fl ~ /POLLHUP|POLLERR/)
			if (!(fd in ready) || !good) { notr++; if (notr <= 5) print "V: " c " sur fd " fd " sans " need " signale par poll : " substr(line, 1, 80) }
		}
		if (rest ~ /EAGAIN|EWOULDBLOCK/) { eag++; if (eag <= 5) print "E: " substr(line, 1, 90) }
	}
	END {
		for (f in acc) if (!(f in nb)) miss++
		for (f in listenfd) if (!(f in nb)) miss++
		print "polls " polls + 0; print "R " cnt["R"] + 0; print "W " cnt["W"] + 0; print "A " cnt["A"] + 0
		print "cons " cons + 0; print "notr " notr + 0; print "eagain " eag + 0
		print "fcntl " nf + 0; print "badfcntl " badf + 0; print "nbmiss " miss + 0
		print "kind " (kind == "" ? "none" : kind)
	}' "$1"
}

t_syscalls() {
	section "2b. Appels systeme reels (strace) : poll avant accept/recv/send, fcntl"
	command -v strace > /dev/null 2>&1 || { skip "strace non installe (sudo apt install strace) : verification dynamique impossible"; return; }
	[ "$VALGRIND" = 1 ] && { skip "strace + valgrind incompatibles"; return; }
	[ -x "$BIN" ] || { ko "$BIN introuvable"; return; }
	local log="$TMP/strace.log" spid child saved="$SRV_PID" out v
	: > "$log"
	strace -f -qq -o "$log" -e trace=socket,accept,accept4,recv,recvfrom,recvmsg,read,send,sendto,sendmsg,write,poll,ppoll,select,pselect6,epoll_wait,epoll_pwait,epoll_pwait2,fcntl,fcntl64 \
		"$BIN" "$PORT" "$PASSW" > "$TMP/server.strace.out" 2>&1 &
	spid=$!
	if ! wait_port "$spid"; then skip "strace/ptrace inutilisable dans cet environnement"; kill -9 "$spid" 2>/dev/null; return; fi
	child=$(pgrep -P "$spid" | head -n 1); [ -n "$child" ] || child=$spid
	SRV_PID=$child

	reg SY1 sys1; reg SY2 sys2; reg SY3 sys3
	jn SY1 "#sys"; jn SY2 "#sys"; jn SY3 "#sys"; markall
	cmd SY1 "PRIVMSG #sys :hello"; wait_for SY2 "hello" > /dev/null
	txraw SY2 'PRI'; sleep 0.2; txraw SY2 'VMSG #sys :morceaux\r\n'; sleep 0.2
	cmd SY1 "MODE #sys +itk secret"; cmd SY1 "KICK #sys sys3 :bye"; sleep 0.3
	kill -STOP "${PID[SY2]}"
	flood_seq_bg SY1 "#sys" 400 200; wait "$FLOOD_PID" 2>/dev/null; sleep 1
	kill -CONT "${PID[SY2]}"; sleep 1.5
	closec SY3; reg SY4 sys4 > /dev/null; closec SY4
	closeall; sleep 0.5
	kill -INT "$child" 2>/dev/null
	for v in $(seq 1 60); do kill -0 "$spid" 2>/dev/null || break; sleep 0.2; done
	kill -0 "$spid" 2>/dev/null && { kill -9 "$child" "$spid" 2>/dev/null; }
	{ wait "$spid"; } 2>/dev/null
	SRV_PID="$saved"

	out=$(analyze_strace "$log")
	gv() { echo "$out" | awk -v k="$1" '$1 == k {print $2}'; }
	local polls=$(gv polls) R=$(gv R) W=$(gv W) A=$(gv A) kind=$(gv kind)
	if [ "${polls:-0}" -gt 0 ] && [ $(( ${R:-0} + ${W:-0} + ${A:-0} )) -gt 0 ]; then
		info "trace : $polls poll/select/epoll, $A accept, $R lectures, $W ecritures sur sockets"
		ok "le serveur utilise poll/select/epoll"
	else
		ko "aucun appel poll/recv/send observe dans la trace" "voir $log"; return
	fi
	if [ "$(gv cons)" -eq 0 ]; then ok "jamais deux recv / send / accept sur le meme fd sans poll entre les deux"
	else ko "recv/send/accept repetes sans poll entre (lecture en boucle ?)"; echo "$out" | grep '^V: deux' | sed 's/^/           /'; fi
	if [ "$kind" = poll ]; then
		if [ "$(gv notr)" -eq 0 ]; then ok "chaque recv/accept suit un POLLIN et chaque send suit un POLLOUT"
		else ko "recv/send/accept appeles sans que poll ait signale POLLIN/POLLOUT sur ce fd"; echo "$out" | grep '^V: [RWA] sur' | sed 's/^/           /'; fi
	else skip "readiness POLLIN/POLLOUT non verifiable (select/epoll)"; fi
	if [ "$(gv eagain)" -eq 0 ]; then ok "aucun recv/send/accept n'a retourne EAGAIN"
	else ko "$(gv eagain) appel(s) recv/send/accept ont retourne EAGAIN (appel sans etre prets ? a justifier)"; echo "$out" | grep '^E:' | sed 's/^/           /'; fi
	if [ "$(gv badfcntl)" -eq 0 ]; then ok "tous les fcntl() sont de la forme fcntl(fd, F_SETFL, O_NONBLOCK) ($(gv fcntl) appel(s))"
	else ko "fcntl() autre que fcntl(fd, F_SETFL, O_NONBLOCK)"; echo "$out" | grep '^F:' | sed 's/^/           /'; fi
	[ "$(gv nbmiss)" -eq 0 ] && ok "le socket d'ecoute et chaque client accepte sont passes en non bloquant" \
		|| ko "$(gv nbmiss) socket(s) jamais passes en O_NONBLOCK"
}

# ----------------------------------------------------------------------------
#  4b. Compatibilite client de reference (HexChat)
# ----------------------------------------------------------------------------
t_compat() {
	section "4b. Compatibilite client de reference (sequences reelles de HexChat)"
	local c unhandled=""
	new HX
	tx HX "CAP LS 302"; tx HX "PASS $PASSW"; tx HX "NICK hexchat1"; tx HX "USER hexchat1 0 * :Hex Chat ✓"
	tx HX "CAP REQ :multi-prefix away-notify"; tx HX "CAP END"
	rx HX "$(nm 001)" "handshake HexChat (CAP LS 302, PASS, NICK, USER UTF-8, CAP REQ, CAP END) -> 001"
	irx HX "$(nm 376)|$(nm 422)" "fin de MOTD (376/422) envoyee"
	new HX2
	tx HX2 "PASS $PASSW"; tx HX2 "NICK hexchat2"; tx HX2 "USER hexchat2 0 * :Hex Chat"
	rx HX2 "$(nm 001)" "handshake HexChat sans negociation CAP (PASS, NICK, USER) -> 001"
	new HX3
	tx HX3 "CAP LS 302"; tx HX3 "PASS :$PASSW"; tx HX3 "NICK hexchat3"; tx HX3 "USER hexchat3 hostname servername :Hex Chat"
	srx HX3 "$(nm 001)" "variante : PASS :mdp (avec ':') et USER nick host serveur :realname -> 001"
	new WC
	tx WC "PASS $PASSW"; tx WC "NICK realname1"; tx WC "USER realname1 0 * real"
	srx WC "$(nm 001)" "USER avec realname sans ':' (un seul mot) -> 001"

	reg CM1 compat1; reg CM2 compat2
	cmd CM1 "PING :ircserv"; srx CM1 "PONG.*ircserv" "PING :token (avec ':') -> PONG"
	cmd CM1 "PONG :ircserv"; snrx CM1 "$(nm 421)" "PONG envoye par le client : pas de 'Unknown command'"
	for c in "MODE compat1" "MODE compat1 +i" "WHOIS compat1" "USERHOST compat1" "ISON compat1" "AWAY :brb" "AWAY" "LIST" "NAMES" \
		"WHO compat1" "WHOWAS compat1" "VERSION" "TIME" "LUSERS" "MOTD" "INFO" "CAP LS" "CAP REQ :multi-prefix" "CAP END"; do
		cmd CM1 "$c"; sleep 0.2
		new_out CM1 | grep -Eq "$(nm 421)" && unhandled="$unhandled ${c%% *}"
	done
	alive CM1 "commandes courantes d'un vrai client (WHOIS, AWAY, CAP, LIST...) : le serveur reste stable"
	[ -n "$unhandled" ] && info "commandes non gerees (421, acceptable car hors sujet) :$unhandled"

	cmd CM1 "JOIN #compat"; wait_for CM1 "$(nm 366)" > /dev/null; sleep 0.2
	cmd CM1 "MODE #compat"; rx CM1 "$(nm 324)" "HexChat envoie MODE #chan apres JOIN -> 324"
	cmd CM1 "WHO #compat"; srx CM1 "$(nm 315)" "HexChat envoie WHO #chan apres JOIN -> 315"

	cmd CM1 "NAMES #compat"; sleep 0.3
	cmd CM1 "PART #compat :Leaving"; sleep 0.3
	cmd CM1 "JOIN #compat,#compat2"; rx CM1 "JOIN :?#compat2" "JOIN #a,#b : les deux channels rejoints"
	cmd CM1 "QUIT :Leaving"; peer_closed CM1 && ok "QUIT :Leaving ferme la connexion" || warn "QUIT :Leaving : connexion toujours ouverte"
	closeall
}

# ----------------------------------------------------------------------------
#  4c. Format des reponses
# ----------------------------------------------------------------------------
t_format() {
	section "4c. Format des reponses IRC (ce que les vrais clients attendent)"
	local a b bad
	reg F1 fmt1
	new F3; tx F3 "PASS $PASSW"; tx F3 "NICK fmt3"; tx F3 "USER fmt3 0 * :Fmt Three"
	rx F3 "^:[^ ]+ 001 fmt3 " "001 : ':serveur 001 <nick> :texte' (le nick est le 1er parametre)"
	srx F3 "^:[^ ]+ 002 fmt3 " "002 envoye apres 001"
	srx F3 "^:[^ ]+ 003 fmt3 " "003 envoye"
	srx F3 "^:[^ ]+ 004 fmt3 " "004 envoye"
	sleep 0.3; mark F3
	jn F1 "#fmt"; reg F2 fmt2; markall
	tx F2 "JOIN #fmt"; sleep 0.6
	rx F1 "^:fmt2![^ ]+@[^ ]+ JOIN :?#fmt" "JOIN diffuse : ':nick!user@host JOIN :#chan'"
	rx F2 "^:fmt2![^ ]+@[^ ]+ JOIN :?#fmt" "le nouvel arrivant recoit aussi son propre JOIN"
	a=$(new_out F2 | grep -nE "$(nm 353)" | head -n 1 | cut -d: -f1)
	b=$(new_out F2 | grep -nE "$(nm 366)" | head -n 1 | cut -d: -f1)
	if [ -n "$a" ] && [ -n "$b" ] && [ "$a" -lt "$b" ]; then ok "JOIN : 353 (liste des noms) puis 366 (fin)"
	else ko "JOIN : 353 doit preceder 366" "ligne 353='$a' ligne 366='$b'"; fi
	rx F2 "353 fmt2 [=*@] #fmt :.*@fmt1" "353 : '353 <nick> = #chan :@op reguliers' (op prefixe de @)"
	new_out F2 | grep -E "$(nm 353)" | grep -Eq '[: ]@?fmt2( |$)' && ok "353 contient le nouvel arrivant" || ko "353 ne contient pas le nouvel arrivant"
	new_out F2 | grep -E "$(nm 353)" | grep -q '@fmt2' && ko "le 2e arrivant ne doit pas etre op (@fmt2 dans 353)" || ok "le 2e arrivant n'est pas op"
	rx F2 "366 fmt2 #fmt :" "366 : '366 <nick> #chan :End of /NAMES list'"

	markall
	cmd F1 "PRIVMSG #fmt :bonjour"
	rx F2 "^:fmt1![^ ]+@[^ ]+ PRIVMSG #fmt :bonjour" "PRIVMSG channel relaye avec le prefixe ':nick!user@host'"
	cmd F1 "PRIVMSG fmt2 :prive"
	rx F2 "^:fmt1![^ ]+@[^ ]+ PRIVMSG fmt2 :prive" "PRIVMSG prive relaye (cible = destinataire)"
	cmd F1 "TOPIC #fmt :le topic"
	rx F2 "^:fmt1![^ ]+@[^ ]+ TOPIC #fmt :le topic" "TOPIC diffuse avec prefixe"
	cmd F2 "TOPIC #fmt"; rx F2 "332 fmt2 #fmt :le topic" "332 : '332 <nick> #chan :topic'"
	cmd F1 "MODE #fmt +o fmt2"; rx F2 "^:fmt1![^ ]+@[^ ]+ MODE #fmt \+o :?fmt2" "MODE +o diffuse : ':op!u@h MODE #chan +o nick'"
	cmd F1 "MODE #fmt +l 9"; rx F2 "MODE #fmt \+l :?9" "MODE +l diffuse avec son parametre"
	cmd F1 "MODE #fmt +k pw"; rx F2 "MODE #fmt \+k :?pw" "MODE +k diffuse avec son parametre"
	cmd F2 "MODE #fmt"; rx F2 "324 fmt2 #fmt \+" "324 : '324 <nick> #chan +modes [params]'"
	cmd F1 "INVITE fmt3 #fmt"
	rx F3 "^:fmt1![^ ]+@[^ ]+ INVITE fmt3 :?#fmt" "INVITE recu : ':op!u@h INVITE cible #chan'"
	rx F1 "341 fmt1 fmt3 #fmt" "341 : '341 <nick> <cible> #chan'"
	cmd F1 "WHO #fmt"
	rx F1 "352 fmt1 #fmt [^ ]+ [^ ]+ [^ ]+ fmt1 " "352 : '352 <moi> #chan user host serveur nick flags :...'"
	rx F1 "315 fmt1 #fmt" "315 apres les 352"
	cmd F1 "KICK #fmt fmt2 :bye"
	rx F2 "^:fmt1![^ ]+@[^ ]+ KICK #fmt fmt2 :bye" "KICK diffuse : ':op!u@h KICK #chan cible :raison'"
	jn F2 "#fmt" "pw"; markall
	cmd F2 "NICK fmt2b"; rx F1 "^:fmt2![^ ]+@[^ ]+ NICK :?fmt2b" "NICK diffuse : ':ancien!u@h NICK nouveau'"
	cmd F2 "PART #fmt :salut"; srx F1 "^:fmt2b![^ ]+@[^ ]+ PART #fmt" "PART diffuse avec prefixe"
	jn F2 "#fmt" "pw"; markall
	cmd F2 "QUIT :ciao"; srx F1 "^:fmt2b![^ ]+@[^ ]+ QUIT" "QUIT diffuse avec prefixe"

	cmd F1 "PRIVMSG ghostx :x"; rx F1 "401 fmt1 ghostx :" "401 : '401 <nick> <cible> :No such nick'"
	cmd F1 "PRIVMSG #nochx :x"; rx F1 "40[13] fmt1 #nochx :" "403/401 : '<code> <nick> <channel> :...'"
	cmd F3 "KICK #fmt fmt1"; rx F3 "(442|482) fmt3 #fmt :" "442/482 : '<code> <nick> #chan :...'"
	new F4; tx F4 "PASS $PASSW"; tx F4 "NICK fmt1"
	rx F4 "433 [^ ]+ fmt1 :" "433 : '433 <nick|*> <nick> :Nickname is already in use'"
	cmd F3 "FOO"; srx F3 "421 fmt3 FOO :" "421 : '421 <nick> <commande> :Unknown command'"

	bad=$(grep -vc $'\r$' "${LOG[F1]}")
	[ "$bad" -eq 0 ] && ok "toutes les lignes envoyees se terminent par \\r\\n" || warn "$bad ligne(s) sans \\r\\n (le protocole IRC exige CRLF)"
	bad=$(awk 'length($0) > 513 {c++} END {print c+0}' "${LOG[F1]}")
	[ "$bad" -eq 0 ] && ok "aucune ligne envoyee ne depasse 512 octets" || warn "$bad ligne(s) envoyee(s) depassent 512 octets"
	closeall
}

# ----------------------------------------------------------------------------
#  5b. Cas limites du protocole / du flux
# ----------------------------------------------------------------------------
t_edge() {
	section "5b. Cas limites du protocole et du flux TCP"
	local i n cnt ord got nk okn c
	reg E1 edg1; reg E2 edg2
	mark E1; txraw E1 'PING crlf\r'; sleep 0.4; txraw E1 '\n'
	rx E1 "PONG.*crlf" "CRLF coupe entre \\r et \\n -> commande traitee"
	sleep 0.3; n=$(new_out E1 | grep -c 'PONG.*crlf'); [ "$n" -eq 1 ] && ok "...exactement une fois" || ko "...traitee $n fois"
	mark E1; txraw E1 'PING nul\000x\r\n'; sleep 0.4; alive E1 "octet NUL dans une commande : pas de crash"
	mark E1; txraw E1 'PING trail   \r\n'; srx E1 "PONG.*trail" "espaces en fin de ligne toleres"
	mark E1; txraw E1 'PiNg MiXeD\r\n'; srx E1 "PONG.*MiXeD" "commande en casse mixte (PiNg)"
	markall; cmd E1 "PRIVMSG    edg2    :espaces multiples"; srx E2 "espaces multiples" "espaces multiples entre parametres acceptes"

	mark E1
	{ awk 'BEGIN{for (i = 1; i <= 1000; i++) printf "PING p%d\r\n", i}' >&"${FD[E1]}"; } 2>/dev/null
	wait_for E1 "PONG.*p1000$" 20 > /dev/null
	cnt=$(new_out E1 | grep -c 'PONG'); ord=$(new_out E1 | grep PONG | grep -oE 'p[0-9]+$' | tr -d p | awk 'NR != $1 {b = 1} END {print b ? "BAD" : "OK"}')
	[ "$cnt" -eq 1000 ] && [ "$ord" = OK ] && ok "1000 commandes dans un seul envoi : 1000 reponses, dans l'ordre" \
		|| ko "1000 commandes pipelinees : $cnt reponses, ordre $ord"

	for i in $(seq 1 20); do new "SM$i"; done
	for i in $(seq 1 20); do tx "SM$i" "PASS $PASSW"; done
	for i in $(seq 1 20); do tx "SM$i" "NICK sim$i"; done
	for i in $(seq 1 20); do tx "SM$i" "USER sim$i 0 * :s"; done
	okn=0; for i in $(seq 1 20); do wait_for "SM$i" "$(nm 001)" 5 && okn=$((okn+1)); done
	[ "$okn" -eq 20 ] && ok "20 inscriptions simultanees : 20/20" || ko "20 inscriptions simultanees : $okn/20"
	for i in $(seq 1 20); do closec "SM$i"; done

	new RA; new RB
	tx RA "PASS $PASSW"; tx RB "PASS $PASSW"; tx RA "NICK racer"; tx RB "NICK racer"; tx RA "USER ra 0 * :a"; tx RB "USER rb 0 * :b"
	sleep 1
	got=$(( $(grep -c ' 001 ' "${LOG[RA]}") + $(grep -c ' 001 ' "${LOG[RB]}") ))
	[ "$got" -eq 1 ] && ok "deux clients avec le meme nick en meme temps : un seul est enregistre" || ko "meme nick simultane : $got client(s) enregistre(s)"
	closec RA; closec RB

	new E4; tx E4 "QUIT"; peer_closed E4 && ok "QUIT avant inscription ferme la connexion" || warn "QUIT avant inscription : connexion toujours ouverte"
	new E5; tx E5 "PASS $PASSW"; tx E5 "NICK edg5"
	nrx E5 "$(nm 001)" "PASS + NICK sans USER : pas encore inscrit"
	tx E5 "USER edg5 0 * :x"; rx E5 "$(nm 001)" "USER arrive en dernier -> 001"
	new E6
	for c in "PRIVMSG x :y" "PART #a" "KICK #a b" "MODE #a +i" "TOPIC #a" "INVITE a #b" "WHO #a"; do
		cmd E6 "$c"; sleep 0.15
		if new_out E6 | grep -Eq "$(nm 451)"; then ok "'$c' avant inscription -> 451"; else warn "'$c' avant inscription : pas de 451"; fi
	done

	for nk in "a_b" "x-y" "[x]" "a|b" "a^b" "a{b}" 'a\b' 'a`b'; do
		cmd E2 "NICK $nk"; sleep 0.25
		if new_out E2 | grep -qF "NICK :$nk" || new_out E2 | grep -qF "NICK $nk"; then ok "nick '$nk' (caractere valide RFC) accepte"
		else warn "nick '$nk' refuse (valide selon la RFC 2812)"; fi
	done
	info "attente de 6 s : un client inactif ne doit jamais etre deconnecte"
	sleep 6; alive E1 "client inactif pendant 6 s : toujours connecte"
	closeall
}

# ----------------------------------------------------------------------------
#  10b. Fidelite du relais
# ----------------------------------------------------------------------------
t_relay() {
	section "10b. Fidelite du relais des messages"
	local s400 s496 s700 i okc=0 ord
	s400=$(printf 'x%.0s' $(seq 1 400)); s496=$(printf 'y%.0s' $(seq 1 496)); s700=$(printf 'z%.0s' $(seq 1 700))
	reg R1 rel1; reg R2 rel2; reg R3 rel3; reg R4 rel4
	jn R1 "#rel"; jn R2 "#rel"; jn R3 "#rel"; jn R4 "#rel2"; markall
	cmd R1 "PRIVMSG #rel :héllo wörld ☃ 日本語"; rx R2 "héllo wörld ☃ 日本語" "UTF-8 relaye octet pour octet"
	cmd R1 "PRIVMSG #rel :a:b:c :d"; rx R2 "PRIVMSG #rel :a:b:c :d" "':' dans le texte preserves"
	cmd R1 "PRIVMSG #rel :  deux espaces"; rx R2 "PRIVMSG #rel :  deux espaces" "espaces de debut de texte preserves"
	cmd R1 "PRIVMSG #rel :$s400"; rx R2 "x{400}" "message de 400 caracteres relaye en entier"
	cmd R1 "PRIVMSG #rel :$s496"; rx R2 "y{496}" "ligne de 512 octets (CRLF compris) acceptee et relayee"
	cmd R1 "PRIVMSG #rel :"; rx R1 "$(nm 412)" "PRIVMSG avec texte vide -> 412"

	markall; flood_seq_bg R1 "#rel" 150; wait "$FLOOD_PID" 2>/dev/null
	wait_for R2 "flood 150( |$)" 15 > /dev/null; wait_for R3 "flood 150( |$)" 15 > /dev/null
	ord=$(check_seq "${LOG[R2]}" 150); [ "$ord" = OK ] && ok "150 messages rapides : recus dans l'ordre, sans perte (membre 2)" || ko "ordre/perte membre 2 : $ord"
	ord=$(check_seq "${LOG[R3]}" 150); [ "$ord" = OK ] && ok "150 messages rapides : recus dans l'ordre, sans perte (membre 3)" || ko "ordre/perte membre 3 : $ord"
	nrx R4 "flood|héllo|deux espaces" "un membre d'un autre channel ne recoit rien"

	markall; cmd R1 "PRIVMSG rel2 :secret"; rx R2 "secret" "PRIVMSG prive recu par son destinataire"
	nrx R3 "secret" "un PRIVMSG prive n'est visible que de son destinataire"

	markall; tx R1 "PRIVMSG rel2 :${SOH}ACTION waves${SOH}"
	if wait_for R2 "ACTION waves" && new_out R2 | grep -q "${SOH}ACTION waves${SOH}"; then ok "CTCP ACTION (/me) relaye avec ses \\001"; else warn "CTCP ACTION (/me) corrompu ou perdu"; fi
	tx R1 "PRIVMSG #rel :${SOH}ACTION dances${SOH}"
	if wait_for R3 "ACTION dances" && new_out R3 | grep -q "${SOH}ACTION dances${SOH}"; then ok "CTCP ACTION dans un channel relaye intact"; else warn "CTCP ACTION de channel corrompu"; fi
	tx R1 "PRIVMSG rel2 :${SOH}DCC SEND fichier.txt 2130706433 5000 1234${SOH}"
	if wait_for R2 "DCC SEND" && new_out R2 | grep -q "${SOH}DCC SEND fichier.txt 2130706433 5000 1234${SOH}"; then ok "offre DCC SEND relayee intacte (base du transfert de fichier)"; else warn "offre DCC SEND corrompue : le transfert de fichier ne marchera pas"; fi

	markall;
	tx R1 "PRIVMSG REL2 :casse1"; irx R2 "casse1" "nick insensible a la casse (PRIVMSG REL2)"
	tx R1 "PRIVMSG #REL :casse2"; irx R3 "casse2" "channel insensible a la casse (#REL = #rel)"

	for i in $(seq 1 14); do reg "MB$i" "mem$i" && jn "MB$i" "#fan"; done
	jn R1 "#fan"; markall; cmd R1 "PRIVMSG #fan :fanout"
	for i in $(seq 1 14); do wait_for "MB$i" "PRIVMSG #fan :fanout" && okc=$((okc+1)); done
	[ "$okc" -eq 14 ] && ok "diffusion dans un channel de 15 membres : 14/14 destinataires" || ko "diffusion a 15 membres : $okc/14"
	for i in $(seq 1 14); do closec "MB$i"; done

	cmd R2 "PART #rel"; sleep 0.3; markall; cmd R1 "PRIVMSG #rel :apres part"
	nrx R2 "apres part" "apres PART un membre ne recoit plus rien"
	closec R3; sleep 0.4; cmd R1 "PRIVMSG #rel :apres quit"; alive R1 "diffusion apres la deconnexion d'un membre : pas de crash"
	reg RL rel9; jn RL "#rel"; cmd RL "PRIVMSG #rel :$s700"; sleep 0.5
	alive R1 "ligne de 700 octets (CRLF compris > 512) : le serveur survit"
	closeall
}

# ----------------------------------------------------------------------------
#  8d. CPU et memoire
# ----------------------------------------------------------------------------
t_cpu() {
	section "8d. CPU au repos et croissance memoire"
	[ "$VALGRIND" = 1 ] && { skip "mesure impossible sous valgrind"; return; }
	[ -r "/proc/$SRV_PID/stat" ] || { skip "/proc indisponible (Linux requis)"; return; }
	local hz t0 t1 d r0 r1 i fd
	hz=$(getconf CLK_TCK 2>/dev/null || echo 100)
	sleep 1; t0=$(cpu_ticks "$SRV_PID"); sleep 3; t1=$(cpu_ticks "$SRV_PID"); d=$((t1-t0))
	[ "$d" -le $((hz/2)) ] && ok "serveur au repos : $d ticks CPU en 3 s" || ko "serveur au repos : $d ticks CPU en 3 s (boucle active ?)"

	reg CP1 cpu1; reg CP2 cpu2; reg CP3 cpu3
	jn CP1 "#cpu"; jn CP2 "#cpu"; jn CP3 "#cpu"
	t0=$(cpu_ticks "$SRV_PID"); sleep 3; t1=$(cpu_ticks "$SRV_PID"); d=$((t1-t0))
	[ "$d" -le $((hz/2)) ] && ok "3 clients inscrits et inactifs : $d ticks CPU en 3 s" || ko "3 clients inactifs : $d ticks CPU en 3 s (POLLOUT demande en permanence ?)"

	kill -STOP "${PID[CP2]}"
	flood_seq_bg CP1 "#cpu" 3000 300; wait "$FLOOD_PID" 2>/dev/null; sleep 2
	t0=$(cpu_ticks "$SRV_PID"); sleep 3; t1=$(cpu_ticks "$SRV_PID"); d=$((t1-t0))
	[ "$d" -le $((hz/2)) ] && ok "client gele avec sortie en attente : $d ticks CPU en 3 s" || ko "client gele : $d ticks CPU en 3 s (le serveur tourne en boucle)"
	kill -CONT "${PID[CP2]}"; sleep 1
	closeall; sleep 1

	r0=$(rss_kb "$SRV_PID")
	for i in $(seq 1 300); do
		fd=300
		if eval "exec $fd<>/dev/tcp/$HOST/$PORT" 2>/dev/null; then
			{ printf 'PASS %s\r\nNICK cyc%d\r\nUSER c 0 * :c\r\nJOIN #cyc%d\r\n' "$PASSW" "$i" "$((i%5))" >&"$fd"; } 2>/dev/null
			eval "exec $fd>&-"
		fi
	done
	sleep 2; r1=$(rss_kb "$SRV_PID")
	d=$(( (${r1:-0} - ${r0:-0}) / 1024 ))
	info "memoire serveur : ${r0:-?} Ko -> ${r1:-?} Ko apres 300 connexions/deconnexions (+${d} Mo)"
	if [ "$d" -gt 20 ]; then warn "croissance memoire de ${d} Mo apres 300 connexions : fuite probable"
	elif [ "$d" -gt 5 ]; then warn "croissance memoire de ${d} Mo apres 300 connexions"
	else ok "pas de croissance memoire notable apres 300 connexions/deconnexions"; fi
	reg CP4 cpu4 && alive CP4 "serveur toujours sain apres 300 connexions"
	closeall
}

# ----------------------------------------------------------------------------
#  8c. Epuisement des descripteurs de fichiers
# ----------------------------------------------------------------------------
t_fdlimit() {
	section "8c. Limite de descripteurs (ulimit -n 40) : le serveur ne doit pas planter"
	[ "$VALGRIND" = 1 ] && { skip "non execute sous valgrind"; return; }
	local oldport=$PORT lp i fd opened=0 hz t0 t1 d
	PORT=$((PORT+2)); hz=$(getconf CLK_TCK 2>/dev/null || echo 100)
	( ulimit -n 40 2>/dev/null || exit 3; exec "$BIN" "$PORT" "$PASSW" ) > "$TMP/fdl.log" 2>&1 &
	lp=$!
	if ! wait_port "$lp"; then skip "le serveur ne demarre pas avec ulimit -n 40"; kill -9 "$lp" 2>/dev/null; PORT=$oldport; return; fi
	for i in $(seq 1 100); do
		fd=$((300+i))
		if eval "exec $fd<>/dev/tcp/$HOST/$PORT" 2>/dev/null; then
			opened=$((opened+1)); { printf 'PASS %s\r\nNICK lim%d\r\nUSER l 0 * :l\r\n' "$PASSW" "$i" >&"$fd"; } 2>/dev/null
		fi
	done
	info "$opened/100 connexions ouvertes cote client (le serveur ne peut en accepter qu'environ 35)"
	sleep 1
	kill -0 "$lp" 2>/dev/null && ok "fds epuises : le serveur survit" || warn "le serveur est mort quand il n'a plus de descripteurs"
	if [ -r "/proc/$lp/stat" ]; then
		t0=$(cpu_ticks "$lp"); sleep 2; t1=$(cpu_ticks "$lp"); d=$((t1-t0))
		[ "$d" -le $((hz*3/2)) ] && ok "fds epuises : pas de boucle active ($d ticks en 2 s)" || warn "fds epuises : $d ticks CPU en 2 s (accept() echoue en boucle ?)"
	fi
	for i in $(seq 1 100); do eval "exec $((300+i))>&-" 2>/dev/null; done
	sleep 1
	reg LM1 limok && alive LM1 "apres la liberation des fds, le serveur accepte de nouveau des clients"
	closeall
	kill -INT "$lp" 2>/dev/null; { wait "$lp"; } 2>/dev/null
	PORT=$oldport
}

# ----------------------------------------------------------------------------
#  12b. Operateurs : cas avances
# ----------------------------------------------------------------------------
t_opx() {
	section "12b. Operateurs : cas avances (MODE / KICK / TOPIC / INVITE / nick)"
	local i mm long300 long200
	long300=$(printf 't%.0s' $(seq 1 300)); long200=$(printf 'a%.0s' $(seq 1 200))
	reg X1 opx1; reg X2 opx2; reg X3 opx3; reg X4 opx4

	# --- limite d'utilisateurs ------------------------------------------------------
	jn X1 "#ol"; jn X2 "#ol"; cmd X1 "MODE #ol +l 2"; sleep 0.3
	cmd X3 "JOIN #ol"; rx X3 "$(nm 471)" "+l 2 avec 2 membres : le 3e est refuse (471)"
	cmd X1 "KICK #ol opx2"; sleep 0.3
	cmd X3 "JOIN #ol"; rx X3 "JOIN :?#ol" "un KICK libere une place sous +l"
	jn X1 "#ol2"; jn X2 "#ol2"; markall
	cmd X1 "MODE #ol2 +l 1"; sleep 0.3; cmd X2 "PRIVMSG #ol2 :toujours la"
	rx X1 "toujours la" "+l 1 avec 2 membres deja presents : personne n'est expulse"
	cmd X4 "JOIN #ol2"; rx X4 "$(nm 471)" "+l 1 : nouvel arrivant refuse (471)"
	cmd X1 "MODE #ol2 +l 99999999999999999999"; alive X1 "+l avec un nombre gigantesque : pas de crash"
	cmd X1 "MODE #ol2 +o"; rx X1 "$(nm 461)" "MODE +o sans parametre -> 461"
	cmd X1 "MODE #ol2 +l"; rx X1 "$(nm 461)" "MODE +l sans parametre -> 461"

	# --- cle -------------------------------------------------------------------------
	jn X1 "#ok"; cmd X1 "MODE #ok +k one"; sleep 0.3; cmd X1 "MODE #ok +k two"; sleep 0.3
	cmd X3 "JOIN #ok two"
	if wait_for X3 "JOIN :?#ok"; then info "+k deux apres +k un : la cle a ete remplacee"; closec X3; reg X3 opx3
	else
		cmd X3 "JOIN #ok one"
		if wait_for X3 "JOIN :?#ok"; then info "+k deux apres +k un : refuse (cle inchangee, type 467)"
		else ko "ni l'ancienne ni la nouvelle cle ne permettent de rejoindre #ok"; fi
	fi
	cmd X1 "MODE #ok -k whatever"; sleep 0.3; cmd X4 "JOIN #ok"
	if wait_for X4 "JOIN :?#ok"; then ok "-k avec un argument retire bien la cle"; else warn "-k <cle> n'a pas retire la cle (certains clients envoient -k cle)"; fi

	# --- ordre des parametres + 324 avec parametres ----------------------------------
	jn X1 "#op"; jn X2 "#op"; markall
	cmd X1 "MODE #op +ol opx2 5"; sleep 0.4
	cmd X2 "MODE #op +t"; nrx X2 "$(nm 482)" "+ol opx2 5 : le 1er parametre va a o (opx2 devient op)"
	cmd X1 "MODE #op"; rx X1 "324 .*#op \+[a-z]*l[a-z]* .*5" "+ol opx2 5 : le 2e parametre va a l (limite 5)"
	jn X1 "#ob"; cmd X1 "MODE #ob +itkl 22 2"; sleep 0.3; cmd X1 "MODE #ob"
	rx X1 "324 [^ ]+ #ob \+[a-z]+ 22 2( |$)" "MODE #chan -> '324 ... #chan +itkl 22 2' (modes avec leurs parametres)"
	cmd X1 "MODE #ob +iii"; sleep 0.3; cmd X1 "MODE #ob"; wait_for X1 "$(nm 324)" > /dev/null
	mm=$(modes_of X1 "#ob"); case "$mm" in *ii*) warn "+iii : le mode i apparait plusieurs fois ('$mm')";; *) ok "+iii : mode i unique ('$mm')";; esac
	cmd X1 "MODE #ob +"; cmd X1 "MODE #ob -"; alive X1 "MODE #chan + / MODE #chan - (vides) : pas de crash"
	cmd X1 "MODE #ob -o opx2"; alive X1 "-o sur quelqu'un qui n'est pas op : pas de crash"
	cmd X1 "MODE #ob +o opx1"; alive X1 "+o sur soi-meme (deja op) : pas de crash"

	# --- TOPIC ----------------------------------------------------------------------
	jn X1 "#ot"; jn X2 "#ot"; markall
	cmd X1 "TOPIC #ot :topic: avec deux-points et espaces"; sleep 0.3
	cmd X2 "TOPIC #ot"; rx X2 "$(nm 332).*topic: avec deux-points et espaces" "topic avec ':' et espaces conserve"
	cmd X1 "TOPIC #ot :$long300"; alive X1 "topic de 300 caracteres : pas de crash"
	cmd X1 "TOPIC"; rx X1 "$(nm 461)" "TOPIC sans parametre -> 461"
	cmd X1 "TOPIC #ot :propre"; sleep 0.2
	cmd X4 "TOPIC #ot :hack"; srx X4 "$(nm 442)" "un non-membre ne peut pas changer le topic (442)"
	cmd X2 "TOPIC #ot"; rx X2 "332.*propre" "le topic n'a pas ete modifie par le non-membre"
	cmd X1 "PART #ot"; sleep 0.3; cmd X2 "TOPIC #ot"; rx X2 "332.*propre" "le topic persiste quand l'op quitte le channel"
	cmd X1 "TOPIC #nochan :x"; rx X1 "$(nm 403)" "TOPIC sur channel inexistant -> 403"

	# --- actions d'un non-membre ------------------------------------------------------
	jn X1 "#oo"; jn X2 "#oo"; markall
	cmd X4 "MODE #oo +i"; rx X4 "(442|482)( |$)" "MODE par un non-membre refuse (442/482)"
	cmd X1 "MODE #oo"; wait_for X1 "$(nm 324)" > /dev/null; modes_of X1 "#oo" | grep -q i && ko "un non-membre a reussi a poser +i" || ok "le channel n'a pas ete modifie par le non-membre"
	cmd X4 "KICK #oo opx2"; rx X4 "(442|482)( |$)" "KICK par un non-membre refuse (442/482)"
	cmd X1 "PRIVMSG #oo :verif"; rx X2 "verif" "la cible du KICK refuse est toujours dans le channel"
	cmd X4 "INVITE opx3 #oo"; srx X4 "$(nm 442)" "INVITE par un non-membre -> 442"

	# --- invitations ------------------------------------------------------------------
	jn X1 "#oi"; cmd X1 "MODE #oi +i"; cmd X1 "INVITE opx2 #oi"; sleep 0.3
	cmd X2 "JOIN #oi"; rx X2 "JOIN :?#oi" "invite : JOIN accepte"
	cmd X2 "PART #oi"; sleep 0.3; cmd X2 "JOIN #oi"
	if wait_for X2 "$(nm 473)"; then ok "l'invitation est consommee : rejoindre apres PART -> 473"; else info "l'invitation reste valable apres PART"; closec X2; reg X2 opx2; fi
	cmd X1 "MODE #oi +k kk"; cmd X1 "INVITE opx4 #oi"; sleep 0.3; cmd X4 "JOIN #oi"
	rx X4 "$(nm 475)" "+i +k : un invite sans la cle est refuse (475)"
	cmd X1 "INVITE opx3 #nochan"; alive X1 "INVITE vers un channel inexistant : pas de crash"
	cmd X1 "INVITE opx1 #oi"; alive X1 "INVITE de soi-meme : pas de crash"
	jn X1 "#oi2"; jn X2 "#oi2"; markall
	cmd X2 "INVITE opx3 #oi2"; sleep 0.4
	if new_out X2 | grep -Eq "$(nm 341)"; then info "channel sans +i : un membre non-op peut inviter (341)"; else info "channel sans +i : un non-op ne peut pas inviter"; fi

	# --- KICK variantes ---------------------------------------------------------------
	jn X1 "#ok2"; jn X2 "#ok2"; jn X3 "#ok2"; markall
	cmd X1 "KICK #ok2 opx2"; rx X2 "KICK #ok2 opx2" "KICK sans raison : diffuse quand meme"
	cmd X2 "JOIN #ok2"; sleep 0.3; markall
	cmd X1 "KICK #ok2 opx2,opx3 :duo"; sleep 0.5
	cmd X2 "PRIVMSG #ok2 :x"; cmd X3 "PRIVMSG #ok2 :x"; sleep 0.4
	if new_out X2 | grep -Eq "$(nm 404)" && new_out X3 | grep -Eq "$(nm 404)"; then info "KICK a plusieurs cibles (a,b) : gere"; else info "KICK a plusieurs cibles (a,b) : non gere (optionnel)"; fi

	# --- nombreux channels / noms bizarres -------------------------------------------
	for i in $(seq 1 40); do tx X4 "JOIN #many$i"; done; sleep 1
	for i in $(seq 1 40); do tx X4 "PART #many$i"; done; sleep 0.5
	alive X4 "40 JOIN puis 40 PART d'affilee : pas de crash"
	cmd X4 "JOIN #"; sleep 0.2; cmd X4 "JOIN #,"; sleep 0.2; cmd X4 "JOIN #$long200"; sleep 0.3
	alive X4 "JOIN # / JOIN #, / nom de channel de 200 caracteres : pas de crash"
	cmd X4 "JOIN #a,#b,#c,#d,#e,#f,#g,#h,#i,#j"; alive X4 "JOIN de 10 channels en une commande : pas de crash"

	# --- un op qui quitte puis revient n'est plus op ----------------------------------
	jn X1 "#oq"; jn X2 "#oq"; cmd X1 "PART #oq"; sleep 0.3
	cmd X1 "JOIN #oq"; wait_for X1 "$(nm 366)" > /dev/null
	if new_out X1 | grep -E "$(nm 353)" | grep -q '@opx1'; then warn "un op qui quitte puis revient retrouve son statut d'op (privilege conserve)"
	else ok "un op qui quitte puis revient n'est plus op"; fi
	closeall

	# --- changements de nick : les droits suivent ------------------------------------
	reg Y1 nick1y; reg Y2 nick2y; reg Y3 nick3y
	jn Y1 "#on"; jn Y2 "#on"; jn Y3 "#on"; markall
	cmd Y1 "NICK nickop"; sleep 0.4
	cmd Y1 "MODE #on +i"; nrx Y1 "$(nm 482)" "l'op garde ses droits apres changement de nick"
	cmd Y1 "KICK #on nick3y :test"; rx Y3 "KICK #on nick3y" "KICK par un op ayant change de nick"
	cmd Y3 "JOIN #on"; sleep 0.3; cmd Y1 "MODE #on -i"; sleep 0.3
	cmd Y3 "JOIN #on"; rx Y3 "JOIN :?#on" "un user kicke peut rejoindre (pas de bannissement)"
	cmd Y2 "NICK reguly"; sleep 0.4; markall
	cmd Y1 "KICK #on reguly :nn"; rx Y2 "KICK #on reguly" "KICK d'un user via son nouveau nick"
	cmd Y1 "MODE #on +o nick3y"; sleep 0.3; cmd Y1 "NICK nickop2"; sleep 0.3
	cmd Y3 "MODE #on +t"; nrx Y3 "$(nm 482)" "un op nomme par +o reste op apres le changement de nick de l'autre op"
	closeall
}

# ----------------------------------------------------------------------------
#  Rappel des tests manuels
# ----------------------------------------------------------------------------
manual_list() {
	printf '\n%s=== A verifier a la main ===%s\n' "$BLU" "$RST"
	cat <<'EOF'
  - HexChat / client de reference : connexion sans aucune erreur
        /server 127.0.0.1 6667 <pass>   (ou Reseaux > Ajouter, mot de passe du serveur)
  - 3 nc + 2 HexChat en parallele, echange de messages
  - /msg, /query, /kick, /invite, /topic, /mode #test +o bob ; liste des nicks et @ mis a jour
  - Suspendre HexChat (kill -STOP $(pidof hexchat)) puis flood (yes "PRIVMSG #test :flood" | head -100000 | nc ...) puis kill -CONT
  - Fermeture brutale de HexChat (kill -9 $(pidof hexchat))
  - Ctrl+C / Ctrl+\ sur le serveur lance a la main : arret propre, 0 leak
  - valgrind complet :  VALGRIND=1 ./test_ircserv.sh
  - Bonus : /dcc send + /dcc get (md5sum identique, gros fichier, binaire) ; bot
  - Preparer la modification en direct (NOTICE, +n/+m, message de bienvenue, taille de ligne...)
EOF
}

# ----------------------------------------------------------------------------
#  Nettoyage
# ----------------------------------------------------------------------------
cleanup() {
	closeall 2>/dev/null
	[ -n "$SRV_PID" ] && kill -9 "$SRV_PID" 2>/dev/null
	rm -rf "$TMP"
}
trap cleanup EXIT
trap 'echo; echo "interrompu"; exit 130' INT TERM

# ----------------------------------------------------------------------------
#  Main
# ----------------------------------------------------------------------------
ALL="build readme static args net compat format partial edge kill flood auth privmsg relay channel perms opx kick invite mode robust cpu lowmem fdlimit syscalls signals"
LIVE="net compat format partial edge kill flood auth privmsg relay channel perms opx kick invite mode robust cpu"
SEL=("$@"); [ ${#SEL[@]} -eq 0 ] && SEL=($ALL)
want() { local s; for s in "${SEL[@]}"; do [ "$s" = "$1" ] && return 0; done; return 1; }

if [ "$VALGRIND" = 1 ] && ! command -v valgrind > /dev/null 2>&1; then
	echo "valgrind introuvable"; exit 2
fi
[ "$VALGRIND" = 1 ] && WAIT=$(awk -v w="$WAIT" 'BEGIN{print w*3}')

want build  && t_build
want readme && t_readme
want static && t_static
if [ ! -x "$BIN" ]; then
	echo "$BIN introuvable : lance 'make' ou 'build' d'abord"; [ ${#SEL[@]} -gt 0 ] && want build || exit 2
fi
want args && t_args

NEED=0
for s in $LIVE; do want "$s" && NEED=1; done
if [ $NEED = 1 ]; then
	if start_server; then
		for s in $LIVE; do want "$s" && "t_$s"; done
		closeall
		server_alive && ok "le serveur est toujours vivant apres tous les tests" || ko "le serveur est mort pendant les tests"
		stop_server INT
		[ "$SRV_RC" -ge 128 ] && ko "arret final : code $SRV_RC" || ok "arret propre a la fin (exit $SRV_RC)"
		check_valgrind "serveur principal"
	else
		section "Demarrage"; ko "impossible de demarrer $BIN $PORT $PASSW (port occupe ?)"; stop_server KILL
	fi
fi
want lowmem  && t_lowmem
want fdlimit && t_fdlimit
want syscalls && t_syscalls
want signals && t_signals

manual_list
printf '\n%s================ RESUME ================%s\n' "$BLU" "$RST"
printf '  %sOK %d%s   %sFAIL %d%s   %sWARN %d%s   %sSKIP %d%s\n' \
	"$GRN" $PASS_N "$RST" "$RED" $FAIL_N "$RST" "$YEL" $WARN_N "$RST" "$YEL" $SKIP_N "$RST"
if [ $FAIL_N -gt 0 ]; then
	printf '\n%sEchecs :%s\n' "$RED" "$RST"
	for f in "${FAILED[@]}"; do printf '  - %s\n' "$f"; done
fi
if [ $WARN_N -gt 0 ]; then
	printf '\n%sAvertissements :%s\n' "$YEL" "$RST"
	for f in "${WARNED[@]}"; do printf '  - %s\n' "$f"; done
fi
[ $FAIL_N -gt 0 ] && exit 1
exit 0
