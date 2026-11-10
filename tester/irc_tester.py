#!/usr/bin/env python3
"""
irc_tester.py - testeur pour le projet ft_irc (42)

Usage :
    python3 irc_tester.py --port 6667 --password pass
    python3 irc_tester.py --exe ./ircserv --port 6667 --password pass   # lance et surveille le serveur
    python3 irc_tester.py --port 6667 --password pass --only kick,mode  # filtre par nom
    python3 irc_tester.py --port 6667 --password pass -v                # affiche les echanges en cas d'echec

Resultats : PASS / FAIL (non conforme au sujet) / WARN (comportement optionnel ou
dependant du client de reference, a verifier a la main).
"""
import argparse
import random
import socket
import string
import subprocess
import sys
import time
import traceback

CFG = {}
CLIENTS = []
TESTS = []
TRACE = []      # echanges du test en cours (ecrits dans le fichier log)
T0 = [0.0]


def trace(msg):
    TRACE.append("%8.3f  %s" % (time.perf_counter() - T0[0], msg))



class Fail(Exception):
    pass


class Warn(Exception):
    pass


def check(cond, msg):
    if not cond:
        trace("!! ECHEC : " + msg)
        raise Fail(msg)


def soft(cond, msg):
    if not cond:
        trace("!! A VERIFIER : " + msg)
        raise Warn(msg)


def rnd(n=6):
    return "".join(random.choice(string.ascii_lowercase) for _ in range(n))


def parse(line):
    prefix = None
    if line.startswith(":"):
        prefix, _, line = line[1:].partition(" ")
    cmd, _, rest = line.partition(" ")
    return prefix, cmd.upper(), rest


class Client:
    def __init__(self, nick=None):
        self.s = socket.create_connection((CFG["host"], CFG["port"]), timeout=3)
        self.s.settimeout(0.05)
        self.buf = b""
        self.lines = []
        self.closed = False
        self.nick = nick
        CLIENTS.append(self)

    # ---- envoi
    def send(self, data):
        if isinstance(data, str):
            data = data.encode()
        shown = data.decode(errors="replace")
        if len(shown) > 200:
            shown = shown[:200] + "... (%d octets au total)" % len(data)
        trace("[%s] >> %r" % (self.nick or "?", shown))
        self.s.settimeout(5)
        try:
            self.s.sendall(data)
        finally:
            self.s.settimeout(0.05)

    def cmd(self, line):
        self.send(line + "\r\n")

    # ---- reception
    def pump(self):
        if self.closed:
            time.sleep(0.05)
            return
        try:
            data = self.s.recv(65536)
            if data == b"":
                self.closed = True
                trace("[%s] -- connexion fermee par le serveur" % (self.nick or "?"))
                return
            self.buf += data
            while b"\n" in self.buf:
                raw, self.buf = self.buf.split(b"\n", 1)
                line = raw.rstrip(b"\r").decode(errors="replace")
                self.lines.append(line)
                trace("[%s] << %s" % (self.nick or "?", line if len(line) <= 300 else line[:300] + "..."))
        except socket.timeout:
            pass
        except OSError:
            self.closed = True
            trace("[%s] -- connexion perdue (erreur socket)" % (self.nick or "?"))

    def wait(self, cmds, contains=None, timeout=2.0):
        if isinstance(cmds, str):
            cmds = (cmds,)

        end = time.time() + timeout

        while True:
            # Cherche une réponse déjà reçue sans perdre les autres
            for i in range(len(self.lines)):
                l = self.lines[i]
                _, c, _ = parse(l)

                if c in cmds and (contains is None or contains in l):
                    self.lines.pop(i)
                    return l

            if time.time() >= end or self.closed:
                return None

            self.pump()

    def drain(self, t=0.3):
        end = time.time() + t
        while time.time() < end:
            self.pump()

        self.lines.clear()

    def none(self, cmds, contains=None, timeout=0.5):
        return self.wait(cmds, contains, timeout) is None

    def dump(self):
        return "\n".join("      " + l for l in self.lines[-15:])

    def close(self):
        try:
            self.s.close()
        except OSError:
            pass


# ---------------------------------------------------------------- helpers
def new(nick=None, register=True, password=None):
    nick = nick or "u" + rnd(7)
    c = Client(nick)
    if register:
        c.cmd("PASS " + (CFG["pw"] if password is None else password))
        c.cmd("NICK " + nick)
        c.cmd("USER %s 0 * :Real %s" % (nick, nick))
        check(c.wait("001", timeout=3) is not None, "pas de 001 (RPL_WELCOME) apres PASS/NICK/USER pour " + nick)
        c.drain(0.1)
    return c

def wait_join(c, chan, timeout=2.0):
    response = c.wait("JOIN", contains=chan, timeout=timeout)

    if response is not None:
        return response

    # Recherche une réponse JOIN présente mais mal formée
    for line in c.lines:
        if chan in line and " JOIN " in line:
            _, cmd, _ = parse(line)
            raise Fail(
                "reponse JOIN mal formee : %r "
                "(commande detectee : %r)" % (line, cmd)
            )

    return None

def join(c, chan, key=None):
    c.cmd("JOIN %s%s" % (chan, " " + key if key else ""))

    response = wait_join(c, chan)

    if response is not None:
        return

    # Une ligne contient JOIN, mais parse() ne reconnait pas la commande
    for line in c.lines:
        if chan in line and " JOIN " in line:
            _, cmd, _ = parse(line)
            raise Fail(
                "reponse JOIN mal formee : %r "
                "(commande detectee : %r)" % (line, cmd)
            )

    raise Fail(
        "%s n'a recu aucune reponse JOIN %s"
        % (c.nick, chan)
    )


def setup(n, chan=None):
    chan = chan or "#t" + rnd(5)
    cs = [new() for _ in range(n)]
    for c in cs:
        join(c, chan)
    for c in cs:
        c.drain(0.1)
    return cs, chan


def test(fn):
    TESTS.append(fn)
    return fn


# ---------------------------------------------------------------- authentification
@test
def auth_wrong_password():
    c = new(register=False)
    c.cmd("PASS wrongpass")
    c.cmd("NICK " + c.nick)
    c.cmd("USER %s 0 * :x" % c.nick)
    check(c.wait("001", timeout=1.0) is None, "connexion acceptee avec un mauvais mot de passe")
    soft(c.wait("464", timeout=0.5) is not None or c.closed, "ni 464 (ERR_PASSWDMISMATCH) ni fermeture")


@test
def auth_no_password():
    c = new(register=False)
    c.cmd("NICK " + c.nick)
    c.cmd("USER %s 0 * :x" % c.nick)
    check(c.wait("001", timeout=1.0) is None, "connexion acceptee sans PASS")


@test
def auth_ok():
    new()


@test
def auth_password_after_nick_user():
    c = new(register=False)
    c.cmd("NICK " + c.nick)
    c.cmd("USER %s 0 * :x" % c.nick)
    c.cmd("PASS " + CFG["pw"])
    soft(c.wait("001", timeout=1.5) is not None, "PASS apres NICK/USER non accepte (tolere par certains serveurs)")


@test
def auth_pass_missing_param():
    c = new(register=False)
    c.cmd("PASS")
    soft(c.wait("461", timeout=1) is not None, "pas de 461 pour PASS sans parametre")


@test
def auth_command_before_registration():
    c = new(register=False)

    c.cmd("JOIN #notregistered")
    check(
        c.wait("451", timeout=1.0) is not None,
        "pas de 451 pour JOIN avant inscription"
    )

    c.cmd("PRIVMSG someone :hi")
    check(
        c.wait("451", timeout=1.0) is not None,
        "pas de 451 pour PRIVMSG avant inscription"
    )


@test
def auth_pass_twice_after_register():
    c = new()

    c.cmd("PASS " + CFG["pw"])

    response = c.wait("462", timeout=1.5)

    soft(
        response is not None,
        "pas de 462 (ERR_ALREADYREGISTRED) apres un second PASS"
    )


# ---------------------------------------------------------------- nick / user
@test
def nick_in_use():
    a = new()
    b = new(register=False)
    b.cmd("PASS " + CFG["pw"])
    b.cmd("NICK " + a.nick)
    check(b.wait("433", timeout=1.5) is not None, "pas de 433 (ERR_NICKNAMEINUSE)")


@test
def nick_change():
    a = new()
    new_nick = "n" + rnd(7)
    a.cmd("NICK " + new_nick)
    check(a.wait("NICK", new_nick) is not None, "pas d'echo NICK apres changement")
    a.nick = new_nick


@test
def nick_change_broadcast():
    (a, b), ch = setup(2)
    nn = "n" + rnd(7)

    a.cmd("NICK " + nn)

    check(
        a.wait("NICK", nn) is not None,
        "pas d'echo NICK pour le client qui change de pseudo"
    )

    check(
        b.wait("NICK", nn) is not None,
        "le changement de nick n'est pas diffuse aux membres du canal"
    )


@test
def nick_no_param_and_invalid():
    c = new()

    c.cmd("NICK")
    check(
        c.wait("431", timeout=1) is not None,
        "pas de 431 (ERR_NONICKNAMEGIVEN) pour NICK sans parametre"
    )

    c.cmd("NICK #bad")
    check(
        c.wait("432", timeout=1) is not None,
        "pas de 432 (ERR_ERRONEUSNICKNAME) pour un nick invalide"
    )


@test
def unknown_command():
    c = new()
    c.cmd("FOOBAR a b")
    soft(c.wait("421", timeout=1) is not None, "pas de 421 (ERR_UNKNOWNCOMMAND)")
    c.cmd("PING tok")
    soft(c.wait("PONG", timeout=1) is not None, "PING sans PONG")


# ---------------------------------------------------------------- JOIN / PART
@test
def join_basic():
    c = new()
    ch = "#j" + rnd(5)

    join(c, ch)

    check(
        c.wait("353", timeout=1) is not None,
        "pas de 353 (RPL_NAMREPLY)"
    )
    check(
        c.wait("366", timeout=1) is not None,
        "pas de 366 (RPL_ENDOFNAMES)"
    )


@test
def join_first_is_operator():
    c = new()
    ch = "#o" + rnd(5)
    c.cmd("JOIN " + ch)
    names = c.wait("353", timeout=2)
    check(names is not None, "pas de 353")
    check("@" + c.nick in names, "le createur du canal n'a pas le statut operateur (@) dans NAMES : " + names)


@test
def join_notifies_members():
    (a,), ch = setup(1)
    b = new()
    b.cmd("JOIN " + ch)
    check(wait_join(a, ch) is not None, "les membres ne sont pas notifies du JOIN d'un nouvel arrivant")


@test
def join_no_param():
    c = new()
    c.cmd("JOIN")
    check(c.wait("461", timeout=1) is not None, "pas de 461 pour JOIN sans parametre")


@test
def join_multiple_channels():
    c = new()
    c1, c2 = "#m" + rnd(4), "#m" + rnd(4)
    c.cmd("JOIN %s,%s" % (c1, c2))
    soft(wait_join(c, c1) is not None and wait_join(c, c2) is not None, "JOIN multi-canaux non gere")


@test
def part_basic():
    (a, b), ch = setup(2)
    b.cmd("PART %s :bye" % ch)
    check(b.wait("PART", ch) is not None, "pas d'echo PART")
    check(a.wait("PART", ch) is not None, "membres non notifies du PART")
    a.cmd("PRIVMSG %s :hello" % ch)
    check(b.none("PRIVMSG"), "un client parti recoit encore les messages")


@test
def part_not_on_channel_and_no_such():
    c = new()
    c.cmd("PART #nochan" + rnd(4))
    check(c.wait(("403", "442"), timeout=1) is not None, "pas de 403/442 pour PART sur canal inexistant")


@test
def channel_recreated_after_all_leave():
    (a,), ch = setup(1)
    a.cmd("PART " + ch)
    a.drain(0.2)
    b = new()
    b.cmd("JOIN " + ch)
    names = b.wait("353", timeout=2)
    soft(names is not None and "@" + b.nick in names, "apres vidage du canal, le nouvel arrivant n'est pas operateur")


# ---------------------------------------------------------------- PRIVMSG
@test
def privmsg_channel_broadcast():
    (a, b, c), ch = setup(3)
    a.cmd("PRIVMSG %s :hello everyone" % ch)
    for x in (b, c):
        l = x.wait("PRIVMSG", "hello everyone")
        check(l is not None, "%s n'a pas recu le message de canal" % x.nick)
        check(l.startswith(":" + a.nick), "prefixe incorrect : " + l)
    check(a.none("PRIVMSG"), "l'emetteur recoit son propre message de canal")


@test
def privmsg_channel_isolated():
    (a, b), ch = setup(2)
    (c,), ch2 = setup(1)
    a.cmd("PRIVMSG %s :secret" % ch)
    check(c.none("PRIVMSG"), "message recu par un client hors du canal")


@test
def privmsg_private():
    a, b = new(), new()
    a.cmd("PRIVMSG %s :salut en prive" % b.nick)
    l = b.wait("PRIVMSG", "salut en prive")
    check(l is not None, "message prive non recu")
    check(a.none("PRIVMSG"), "l'emetteur recoit un echo")
    b.cmd("PRIVMSG %s :reponse" % a.nick)
    check(a.wait("PRIVMSG", "reponse") is not None, "reponse non recue")


@test
def privmsg_with_spaces_and_colons():
    a, b = new(), new()
    a.cmd("PRIVMSG %s :a b  c : d :e" % b.nick)
    l = b.wait("PRIVMSG", timeout=1.5)
    check(l is not None and l.endswith(":a b  c : d :e"), "trailing mal gere : %r" % l)


@test
def privmsg_errors():
    a = new()
    a.cmd("PRIVMSG nobody" + rnd(4) + " :hi")
    check(a.wait("401", timeout=1) is not None, "pas de 401 (ERR_NOSUCHNICK)")
    a.cmd("PRIVMSG #nochan" + rnd(4) + " :hi")
    check(a.wait(("403", "401"), timeout=1) is not None, "pas de 403/401 pour canal inexistant")
    a.cmd("PRIVMSG")
    soft(a.wait(("411", "461"), timeout=1) is not None, "pas de 411/461 sans destinataire")
    a.cmd("PRIVMSG " + a.nick)
    soft(a.wait(("412", "461"), timeout=1) is not None, "pas de 412/461 sans texte")


@test
def privmsg_to_channel_not_member():
    (a,), ch = setup(1)
    b = new()
    b.cmd("PRIVMSG %s :intrus" % ch)
    soft(b.wait("404", timeout=1) is not None, "pas de 404 (ERR_CANNOTSENDTOCHAN) pour un non-membre (+n par defaut sur les vrais serveurs)")


@test
def notice():
    a, b = new(), new()
    a.cmd("NOTICE %s :note" % b.nick)
    soft(b.wait("NOTICE", "note") is not None, "NOTICE non gere")


@test
def privmsg_multiple_targets():
    a, b, c = new(), new(), new()
    a.cmd("PRIVMSG %s,%s :multi" % (b.nick, c.nick))
    soft(b.wait("PRIVMSG", "multi") is not None and c.wait("PRIVMSG", "multi") is not None, "cibles multiples non gerees")


# ---------------------------------------------------------------- KICK
@test
def kick_basic():
    (a, b, c), ch = setup(3)
    a.cmd("KICK %s %s :dehors" % (ch, b.nick))
    check(b.wait("KICK", b.nick) is not None, "la victime ne recoit pas le KICK")
    check(c.wait("KICK", b.nick) is not None, "les membres ne recoivent pas le KICK")
    check(a.wait("KICK", b.nick) is not None, "l'operateur ne recoit pas l'echo du KICK")
    c.cmd("PRIVMSG %s :apres kick" % ch)
    check(b.none("PRIVMSG"), "le client kicke recoit encore les messages")
    b.cmd("PRIVMSG %s :je suis la" % ch)
    check(a.none("PRIVMSG", "je suis la"), "le client kicke peut encore ecrire dans le canal")


@test
def kick_without_reason():
    (a, b), ch = setup(2)
    a.cmd("KICK %s %s" % (ch, b.nick))
    check(b.wait("KICK", timeout=1.5) is not None, "KICK sans raison non gere")


@test
def kick_non_operator():
    (a, b, c), ch = setup(3)
    b.cmd("KICK %s %s :nope" % (ch, c.nick))
    check(b.wait("482", timeout=1) is not None, "pas de 482 (ERR_CHANOPRIVSNEEDED)")
    check(c.none("KICK"), "un non-operateur a reussi un KICK")


@test
def kick_errors():
    (a, b), ch = setup(2)
    a.cmd("KICK %s ghost%s" % (ch, rnd(3)))
    check(a.wait(("441", "401"), timeout=1) is not None, "pas de 441/401 pour cible absente")
    a.cmd("KICK #nochan%s %s" % (rnd(3), b.nick))
    check(a.wait(("403", "442"), timeout=1) is not None, "pas de 403/442 pour canal inexistant")
    a.cmd("KICK")
    check(a.wait("461", timeout=1) is not None, "pas de 461 sans parametre")


@test
def kick_can_rejoin():
    (a, b), ch = setup(2)
    a.cmd("KICK %s %s" % (ch, b.nick))
    b.drain(0.3)
    b.cmd("JOIN " + ch)
    check(wait_join(b, ch) is not None, "impossible de rejoindre apres un KICK")


# ---------------------------------------------------------------- INVITE
@test
def invite_basic():
    (a,), ch = setup(1)
    b = new()
    a.cmd("MODE %s +i" % ch)
    check(a.wait("MODE", timeout=1) is not None, "MODE +i sans echo")
    b.cmd("JOIN " + ch)
    check(b.wait("473", timeout=1) is not None, "pas de 473 (ERR_INVITEONLYCHAN)")
    check(b.none("JOIN"), "JOIN accepte sur canal +i sans invitation")
    a.cmd("INVITE %s %s" % (b.nick, ch))
    check(a.wait("341", timeout=1) is not None, "pas de 341 (RPL_INVITING) pour l'inviteur")
    check(b.wait("INVITE", ch) is not None, "l'invite ne recoit pas le message INVITE")
    b.cmd("JOIN " + ch)
    check(wait_join(b, ch) is not None, "JOIN refuse malgre l'invitation")


@test
def invite_non_operator_on_invite_only():
    (a, b), ch = setup(2)
    a.cmd("MODE %s +i" % ch)
    c = new()
    b.cmd("INVITE %s %s" % (c.nick, ch))
    check(b.wait("482", timeout=1) is not None, "pas de 482 : un non-operateur invite sur un canal +i")


@test
def invite_errors():
    (a, b), ch = setup(2)
    a.cmd("INVITE ghost%s %s" % (rnd(3), ch))
    check(a.wait("401", timeout=1) is not None, "pas de 401 pour nick inexistant")
    a.cmd("INVITE %s %s" % (b.nick, ch))
    soft(a.wait("443", timeout=1) is not None, "pas de 443 (ERR_USERONCHANNEL)")
    a.cmd("INVITE")
    check(a.wait("461", timeout=1) is not None, "pas de 461 sans parametre")
    c = new()
    c.cmd("INVITE %s %s" % (a.nick, ch))
    soft(c.wait(("442", "482"), timeout=1) is not None, "pas de 442 pour un inviteur hors du canal")


@test
def invite_removal_of_i():
    (a,), ch = setup(1)
    a.cmd("MODE %s +i" % ch)
    a.cmd("MODE %s -i" % ch)
    a.drain(0.3)
    b = new()
    b.cmd("JOIN " + ch)
    check(wait_join(b, ch) is not None, "JOIN refuse apres MODE -i")


# ---------------------------------------------------------------- TOPIC
@test
def topic_basic():
    (a, b), ch = setup(2)
    a.cmd("TOPIC " + ch)
    check(a.wait(("331", "332"), timeout=1) is not None, "pas de 331 (RPL_NOTOPIC) sans topic")
    a.cmd("TOPIC %s :nouveau sujet" % ch)
    check(b.wait("TOPIC", "nouveau sujet") is not None, "changement de topic non diffuse")
    b.cmd("TOPIC " + ch)
    l = b.wait("332", timeout=1)
    check(l is not None and "nouveau sujet" in l, "pas de 332 correct a la lecture")
    c = new()
    c.cmd("JOIN " + ch)
    check(c.wait("332", "nouveau sujet", timeout=1.5) is not None, "topic non envoye au JOIN")


@test
def topic_restricted_mode_t():
    (a, b), ch = setup(2)
    a.cmd("MODE %s +t" % ch)
    check(a.wait("MODE", timeout=1) is not None, "MODE +t sans echo")
    b.cmd("TOPIC %s :interdit" % ch)
    check(b.wait("482", timeout=1) is not None, "pas de 482 avec +t pour non-operateur")
    check(a.none("TOPIC"), "topic change malgre +t")
    a.cmd("TOPIC %s :autorise" % ch)
    check(b.wait("TOPIC", "autorise") is not None, "operateur ne peut pas changer le topic avec +t")
    b.cmd("TOPIC " + ch)
    check(b.wait("332", timeout=1) is not None, "lecture du topic impossible avec +t")


@test
def topic_unrestricted_without_t():
    (a, b), ch = setup(2)
    a.cmd("MODE %s -t" % ch)
    a.drain(0.2)
    b.cmd("TOPIC %s :libre" % ch)
    check(a.wait("TOPIC", "libre") is not None, "sans +t, un non-operateur doit pouvoir changer le topic")


@test
def topic_errors():
    c = new()
    c.cmd("TOPIC #nochan" + rnd(4))
    check(c.wait(("403", "442", "331"), timeout=1) is not None, "pas de reponse pour canal inexistant")
    c.cmd("TOPIC")
    check(c.wait("461", timeout=1) is not None, "pas de 461 sans parametre")
    (a,), ch = setup(1)
    c.cmd("TOPIC %s :hack" % ch)
    soft(c.wait("442", timeout=1) is not None, "pas de 442 (ERR_NOTONCHANNEL)")


@test
def topic_clear():
    (a, b), ch = setup(2)
    a.cmd("TOPIC %s :abc" % ch)
    b.drain(0.2)
    a.cmd("TOPIC %s :" % ch)
    soft(b.wait("TOPIC", timeout=1) is not None, "effacement du topic (TOPIC #c :) non gere")


# ---------------------------------------------------------------- MODE
@test
def mode_query():
    (a,), ch = setup(1)
    a.cmd("MODE " + ch)
    soft(a.wait("324", timeout=1) is not None, "pas de 324 (RPL_CHANNELMODEIS)")


@test
def mode_key():
    (a,), ch = setup(1)
    a.cmd("MODE %s +k secret" % ch)
    check(a.wait("MODE", "+k", timeout=1) is not None, "MODE +k sans echo")
    b = new()
    b.cmd("JOIN " + ch)
    check(b.wait("475", timeout=1) is not None, "pas de 475 (ERR_BADCHANNELKEY) sans cle")
    b.cmd("JOIN %s wrong" % ch)
    check(b.wait("475", timeout=1) is not None, "pas de 475 avec mauvaise cle")
    check(b.none("JOIN"), "JOIN accepte avec mauvaise cle")
    b.cmd("JOIN %s secret" % ch)
    check(wait_join(b, ch) is not None, "JOIN refuse avec la bonne cle")
    a.cmd("MODE %s -k" % ch)
    a.drain(0.2)
    c = new()
    c.cmd("JOIN " + ch)
    check(wait_join(c, ch) is not None, "JOIN refuse apres MODE -k")


@test
def mode_limit():
    (a, b), ch = setup(2)
    a.cmd("MODE %s +l 3" % ch)
    check(a.wait("MODE", "+l", timeout=1) is not None, "MODE +l sans echo")
    c, d = new(), new()
    c.cmd("JOIN " + ch)
    check(wait_join(c, ch) is not None, "3e membre refuse alors que limite = 3")
    d.cmd("JOIN " + ch)
    check(d.wait("471", timeout=1) is not None, "pas de 471 (ERR_CHANNELISFULL)")
    check(d.none("JOIN"), "JOIN accepte au-dela de la limite")
    a.cmd("MODE %s -l" % ch)
    a.drain(0.2)
    d.cmd("JOIN " + ch)
    check(wait_join(d, ch) is not None, "JOIN refuse apres MODE -l")


@test
def mode_limit_invalid():
    (a,), ch = setup(1)
    a.cmd("MODE %s +l abc" % ch)
    a.cmd("MODE %s +l" % ch)
    a.drain(0.3)
    a.cmd("PING alive")
    soft(a.wait("PONG", timeout=1) is not None, "le serveur ne repond plus apres +l invalide")


@test
def mode_operator_give_take():
    (a, b, c), ch = setup(3)
    a.cmd("MODE %s +o %s" % (ch, b.nick))
    check(b.wait("MODE", "+o", timeout=1) is not None, "MODE +o non diffuse")
    check(c.wait("MODE", "+o", timeout=1) is not None, "MODE +o non diffuse aux membres")
    b.cmd("KICK %s %s" % (ch, c.nick))
    check(c.wait("KICK", timeout=1) is not None, "le nouvel operateur ne peut pas KICK")
    c.cmd("JOIN " + ch)
    c.drain(0.3)
    a.cmd("MODE %s -o %s" % (ch, b.nick))
    check(b.wait("MODE", "-o", timeout=1) is not None, "MODE -o non diffuse")
    b.cmd("KICK %s %s" % (ch, c.nick))
    check(b.wait("482", timeout=1) is not None, "ancien operateur peut encore agir (pas de 482)")


@test
def mode_non_operator():
    (a, b), ch = setup(2)
    b.cmd("MODE %s +i" % ch)
    check(b.wait("482", timeout=1) is not None, "pas de 482 pour MODE par un non-operateur")
    b.cmd("MODE %s +o %s" % (ch, b.nick))
    check(b.wait("482", timeout=1) is not None, "auto-promotion operateur possible")


@test
def mode_errors():
    (a,), ch = setup(1)
    a.cmd("MODE %s +o ghost%s" % (ch, rnd(3)))
    check(a.wait(("401", "441"), timeout=1) is not None, "pas de 401/441 pour +o sur nick absent")
    a.cmd("MODE %s +k" % ch)
    soft(a.wait("461", timeout=1) is not None, "pas de 461 pour +k sans parametre")
    a.cmd("MODE %s +z" % ch)
    soft(a.wait("472", timeout=1) is not None, "pas de 472 (ERR_UNKNOWNMODE)")
    a.cmd("MODE #nochan" + rnd(4) + " +i")
    check(a.wait(("403", "442", "482"), timeout=1) is not None, "pas de reponse pour canal inexistant")


@test
def mode_combined():
    (a,), ch = setup(1)
    a.cmd("MODE %s +itk key1" % ch)
    check(a.wait("MODE", timeout=1) is not None, "MODE combine +itk non gere")
    a.cmd("MODE %s +kl key2 5" % ch)
    soft(a.wait("MODE", timeout=1) is not None, "MODE combine +kl avec 2 parametres non gere")
    a.cmd("MODE %s -itk" % ch)
    soft(a.wait("MODE", timeout=1) is not None, "MODE combine -itk non gere")
    b = new()
    b.cmd("JOIN " + ch)
    check(wait_join(b, ch) is not None, "le canal reste verrouille apres retrait de tous les modes")


# ---------------------------------------------------------------- QUIT / deconnexions
@test
def quit_broadcast():
    (a, b), ch = setup(2)
    b.cmd("QUIT :au revoir")
    check(a.wait("QUIT", timeout=1.5) is not None, "QUIT non diffuse aux membres du canal")
    a.cmd("PRIVMSG %s :toujours la" % ch)
    new()  # le serveur doit rester reactif


@test
def abrupt_disconnect():
    (a, b, c), ch = setup(3)
    import struct
    b.s.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER, struct.pack("ii", 1, 0))
    b.close()
    soft(a.wait("QUIT", timeout=1.5) is not None, "pas de QUIT diffuse apres coupure brutale (RST)")
    a.cmd("PRIVMSG %s :encore la" % ch)
    check(c.wait("PRIVMSG", "encore la") is not None, "le canal ne fonctionne plus apres une coupure brutale")
    d = new()
    d.cmd("JOIN " + ch)
    check(wait_join(d, ch) is not None, "impossible de rejoindre apres coupure brutale d'un client")


@test
def disconnect_op_then_channel_alive():
    (a, b), ch = setup(2)
    a.close()
    b.drain(0.5)
    b.cmd("PRIVMSG %s :seul" % ch)
    new()  # le serveur doit rester reactif


@test
def disconnect_during_partial_command():
    c = new()
    c.send("PRIVMSG xx :incompl")
    c.close()
    time.sleep(0.2)
    new()


@test
def disconnect_before_registration():
    c = new(register=False)
    c.send("PASS " + CFG["pw"] + "\r\nNICK ab")
    c.close()
    time.sleep(0.2)
    new()


# ---------------------------------------------------------------- donnees partielles (nc + Ctrl+D)
@test
def partial_ctrl_d_style():
    a, b = new(), new()
    for chunk in ("PRIV", "MSG " + b.nick + " :hel", "lo wor", "ld\r\n"):
        a.send(chunk)
        time.sleep(0.25)
    check(b.wait("PRIVMSG", "hello world", timeout=1.5) is not None, "commande fragmentee non reconstituee")


@test
def partial_com_man_d():
    """Equivalent du test du sujet : 'com' ^D 'man' ^D 'd\\n' (commande inconnue -> 421 ou silence, mais pas de crash)."""
    c = new()
    for chunk in ("com", "man", "d\n"):
        c.send(chunk)
        time.sleep(0.25)
    c.drain(0.3)
    new()  # le serveur doit rester reactif


@test
def partial_bare_lf():
    a, b = new(), new()
    a.send("PRIVMSG %s :lf only\n" % b.nick)
    check(b.wait("PRIVMSG", "lf only", timeout=1.5) is not None, "terminaison '\\n' seul non geree (nc sans -C)")


@test
def partial_byte_by_byte():
    a, b = new(), new()
    for ch in "PRIVMSG %s :lent\r\n" % b.nick:
        a.send(ch)
        time.sleep(0.02)
    check(b.wait("PRIVMSG", "lent", timeout=2) is not None, "envoi octet par octet non gere (basse bande passante)")


@test
def partial_registration_split():
    c = new(register=False)
    nick = "p" + rnd(6)
    for chunk in ("PA", "SS " + CFG["pw"] + "\r", "\nNICK " + nick[:3], nick[3:] + "\r\nUSER u 0 * :r", "n\r\n"):
        c.send(chunk)
        time.sleep(0.2)
    check(c.wait("001", timeout=2) is not None, "enregistrement fragmente non gere")


@test
def multiple_commands_one_packet():
    a, b = new(), new()
    a.send("PRIVMSG %s :un\r\nPRIVMSG %s :deux\r\nPRIVMSG %s :trois\r\n" % ((b.nick,) * 3))
    for w in ("un", "deux", "trois"):
        check(b.wait("PRIVMSG", ":" + w, timeout=1.5) is not None, "commande '%s' perdue dans un paquet multi-commandes" % w)


@test
def multiple_commands_with_partial_tail():
    a, b = new(), new()
    a.send("PRIVMSG %s :un\r\nPRIVMSG %s :de" % (b.nick, b.nick))
    time.sleep(0.2)
    a.send("ux\r\n")
    check(b.wait("PRIVMSG", ":un") is not None, "1ere commande perdue")
    check(b.wait("PRIVMSG", ":deux") is not None, "2e commande (coupee) non reconstituee")


@test
def empty_lines_and_spaces():
    c = new()
    c.send("\r\n\r\n   \r\n")
    c.drain(0.3)
    new()  # le serveur doit rester reactif


@test
def half_close_partial():
    c = new()
    c.send("PRIVMSG xx :pas fini")
    c.s.shutdown(socket.SHUT_WR)
    time.sleep(0.3)
    new()


# ---------------------------------------------------------------- robustesse
@test
def garbage_binary():
    c = new()
    c.send(bytes(random.randrange(256) for _ in range(2000)) + b"\r\n")
    c.drain(0.3)
    new()


@test
def long_line():
    c = new()
    c.send("PRIVMSG xx :" + "A" * 20000)
    time.sleep(0.2)
    c.send("\r\n")
    c.drain(0.3)
    new()


@test
def very_long_line_no_newline():
    c = new()
    try:
        c.send("A" * 300000)
    except OSError:
        pass
    time.sleep(0.3)
    new()


@test
def huge_params():
    c = new()
    c.cmd("JOIN #" + "x" * 5000)
    c.cmd("NICK " + "n" * 5000)
    c.cmd("MODE #" + "x" * 3000 + " +k " + "k" * 3000)
    c.cmd("PRIVMSG " + ",".join("a%d" % i for i in range(500)) + " :hi")
    c.drain(0.5)
    new()


@test
def weird_commands():
    c = new()
    for l in ("JOIN ,,,", "JOIN #", "JOIN 0", "PART", "PART ,", "KICK # #", "KICK , ,", "MODE", "MODE +", "MODE # +", "MODE # +o",
              "MODE # -", "INVITE ,", "TOPIC #", "PRIVMSG ,", "PRIVMSG : :", "NICK :", "USER", "USER a", "USER a b c",
              ":prefix JOIN #x", "JOIN #x,#x,#x", "MODE #x +ooooo", "MODE #x +kkkk a b c d", "QUIT"):
        try:
            c.cmd(l)
        except OSError:
            break
    c.drain(0.5)
    new()


@test
def unknown_prefix_and_case():
    c = new()
    ch = "#" + rnd(4)
    c.cmd("join " + ch)
    soft(wait_join(c, ch, timeout=1) is not None, "commandes en minuscules non acceptees (clients usuels envoient des majuscules)")

@test
def user_registration_params():
    c = new(register=False)
    c.cmd("PASS " + CFG["pw"])
    c.cmd("USER onlyone")
    check(c.wait("461", timeout=1) is not None, "pas de 461 pour USER incomplet")


@test
def many_clients():
    n = int(CFG.get("many", 40))
    ch = "#many" + rnd(4)
    cs = []
    for _ in range(n):
        cs.append(new())
    for c in cs:
        c.cmd("JOIN " + ch)
    for c in cs:
        c.drain(0.05)
    cs[0].cmd("PRIVMSG %s :broadcast-all" % ch)
    end = time.time() + 5
    missing = [c.nick for c in cs[1:]
               if c.wait("PRIVMSG", "broadcast-all", timeout=max(0.1, end - time.time())) is None]
    check(not missing, "%d/%d clients n'ont pas recu le message" % (len(missing), n - 1))


@test
def many_messages_ordering():
    (a, b), ch = setup(2)
    n = 300
    a.send("".join("PRIVMSG %s :m%d\r\n" % (ch, i) for i in range(n)))
    got = []
    end = time.time() + 5
    while len(got) < n and time.time() < end:
        l = b.wait("PRIVMSG", timeout=1)
        if l is None:
            break
        got.append(l.rsplit(":m", 1)[-1])
    check(len(got) == n, "seulement %d/%d messages recus" % (len(got), n))
    check(got == [str(i) for i in range(n)], "ordre des messages non respecte")


@test
def slow_reader_does_not_block_server():
    (a, b), ch = setup(2)
    payload = "".join("PRIVMSG %s :%s\r\n" % (ch, "X" * 400) for _ in range(2500))
    a.send(payload)  # b ne lit pas pendant ce temps
    t0 = time.time()
    c = new()
    check(time.time() - t0 < 3, "le serveur est bloque par un client qui ne lit pas")
    b.drain(1.0)
    soft(len(b.lines) > 100, "le lecteur lent n'a recu que %d messages" % len(b.lines))


@test
def connect_disconnect_storm():
    for _ in range(100):
        c = Client()
        if random.random() < 0.5:
            c.send("PASS " + CFG["pw"] + "\r\nNICK s" + rnd(6) + "\r\n")
        c.close()
    time.sleep(0.3)
    new()


@test
def simultaneous_nick_race():
    nick = "race" + rnd(3)
    cs = []
    for _ in range(5):
        c = new(register=False)
        c.cmd("PASS " + CFG["pw"])
        c.cmd("NICK " + nick)
        c.cmd("USER x 0 * :x")
        cs.append(c)
    ok = sum(1 for c in cs if c.wait("001", timeout=2) is not None)
    check(ok == 1, "%d clients ont obtenu le meme nick (attendu: 1)" % ok)


@test
def zz_final_liveness():
    c = new()
    c.cmd("PING end")
    soft(c.wait("PONG", timeout=1) is not None, "pas de PONG")


# ---------------------------------------------------------------- affichage / runner
ORDER = ["Authentification", "Nick / User", "Canaux", "Messages", "KICK", "INVITE", "TOPIC", "MODE",
         "Deconnexions", "Donnees partielles", "Robustesse"]
ICONS = {"Authentification": "🔐", "Nick / User": "🪪 ", "Canaux": "📢", "Messages": "💬", "KICK": "🥾",
         "INVITE": "📨", "TOPIC": "📌", "MODE": "⚙️ ", "Deconnexions": "🔌", "Donnees partielles": "🧩",
         "Robustesse": "🛡️ "}
CATS = [("auth", "Authentification"), ("nick", "Nick / User"), ("user", "Nick / User"), ("unknown", "Nick / User"),
        ("join", "Canaux"), ("part_", "Canaux"), ("part", "Canaux"), ("channel", "Canaux"),
        ("privmsg", "Messages"), ("notice", "Messages"), ("kick", "KICK"), ("invite", "INVITE"),
        ("topic", "TOPIC"), ("mode", "MODE"), ("quit", "Deconnexions"), ("abrupt", "Deconnexions"),
        ("disconnect", "Deconnexions"), ("partial", "Donnees partielles"), ("multiple", "Donnees partielles"),
        ("empty", "Donnees partielles"), ("half", "Donnees partielles")]


def category(name):
    for k, v in CATS:
        if name.startswith(k):
            return v
    return "Robustesse"


def main():
    p = argparse.ArgumentParser(description="Testeur ft_irc")
    p.add_argument("--host", default="127.0.0.1")
    p.add_argument("--port", type=int, default=6667)
    p.add_argument("--password", default="pass")
    p.add_argument("--exe", help="chemin de ircserv : le testeur le lance et verifie qu'il ne crashe pas")
    p.add_argument("--only", help="liste de sous-chaines de noms de tests, separees par des virgules")
    p.add_argument("--many", type=int, default=40, help="nombre de clients du test de charge")
    p.add_argument("--log", default="irc_tester.log", help="fichier log detaille (defaut: irc_tester.log)")
    p.add_argument("--no-color", action="store_true", help="desactive les couleurs et les emojis")
    p.add_argument("--show-server", action="store_true", help="affiche les logs du serveur au lieu de ircserv.log")
    p.add_argument("-v", "--verbose", action="store_true")
    a = p.parse_args()
    CFG.update(host=a.host, port=a.port, pw=a.password, many=a.many)

    import os
    fancy = sys.stdout.isatty() and not os.environ.get("NO_COLOR") and not a.no_color

    def col(code, t):
        return "\033[%sm%s\033[0m" % (code, t) if fancy else t

    ok_i, ko_i, wa_i = ("✔", "✘", "⚠") if fancy else ("+", "x", "!")

    def bar(p_, w_, f_, width=36):
        tot = max(p_ + w_ + f_, 1)
        np_, nw_ = round(width * p_ / tot), round(width * w_ / tot)
        nf_ = width - np_ - nw_ if f_ else 0
        if p_ + w_ + f_ and np_ + nw_ + nf_ < width:
            np_ += width - np_ - nw_ - nf_
        return col("32", "█" * np_) + col("33", "█" * nw_) + col("31", "█" * nf_)

    # --- banniere
    inner = "  ft_irc tester  ·  %s:%d  ·  mdp '%s'" % (a.host, a.port, a.password)
    inner = inner.ljust(56)
    print()
    print(col("36", "╔" + "═" * 56 + "╗"))
    print(col("36", "║") + col("1", inner) + col("36", "║"))
    print(col("36", "╚" + "═" * 56 + "╝"))

    logf = open(a.log, "w", encoding="utf-8")

    def L(line=""):
        logf.write(line + "\n")
        logf.flush()

    L("ft_irc tester - log detaille  (%s)" % time.strftime("%Y-%m-%d %H:%M:%S"))
    L("serveur : %s:%d   mot de passe : %r   executable : %s" % (a.host, a.port, a.password, a.exe or "(deja lance)"))
    L(">> = envoye par le testeur    << = recu du serveur    !! = verification echouee")
    L("temps en secondes depuis le debut de chaque test")
    L()

    server = None
    if a.exe:
        log = None if a.show_server else open("ircserv.log", "w")
        server = subprocess.Popen([a.exe, str(a.port), a.password], stdout=log, stderr=log)
        print(col("90", "  serveur lance : %s (logs -> %s)" % (a.exe, "terminal" if a.show_server else "ircserv.log")))
        time.sleep(0.7)

    try:
        socket.create_connection((a.host, a.port), timeout=2).close()
    except OSError as e:
        print(col("31", "\n  %s Impossible de se connecter a %s:%d : %s" % (ko_i, a.host, a.port, e)))
        print(col("90", "  Lancez ./ircserv %d %s, ou utilisez --exe <chemin>." % (a.port, a.password)))
        if server:
            server.terminate()
        return 2

    tests = TESTS
    if a.only:
        keys = a.only.split(",")
        tests = [t for t in TESTS if any(k in t.__name__ for k in keys)]
    tests = sorted(tests, key=lambda t: ORDER.index(category(t.__name__)))  # tri stable par categorie

    counts = {"PASS": 0, "FAIL": 0, "WARN": 0}
    per_cat = {}
    failed, warned = [], []
    cur_cat = None
    total = len(tests)
    t_start = time.perf_counter()

    VERDICT = {"PASS": "VALIDE", "WARN": "A VERIFIER", "FAIL": "NON VALIDE"}

    def log_test(idx, t, cat, status, msg, dt, tb):
        L("=" * 78)
        L("[%s] %s   (%s, test %d/%d, %.2fs)" % (VERDICT[status], t.__name__, cat, idx, total, dt))
        if msg:
            L("  raison : " + msg)
        L("-" * 78)
        ev = list(TRACE)
        if len(ev) > 400:
            ev = ev[:200] + ["          ... %d echanges omis ..." % (len(ev) - 400)] + ev[-200:]
        for e in ev:
            L("  " + e)
        if tb:
            L(tb)
        L()

    for idx, t in enumerate(tests, 1):
        cat = category(t.__name__)
        if cat != cur_cat:
            cur_cat = cat
            print("\n " + col("1;36", "%s  %s" % (ICONS[cat] if fancy else ">", cat.upper())))
            print(col("90", " " + "─" * 56))
        CLIENTS[:] = []
        status, msg, tb = "PASS", "", ""
        TRACE[:] = []
        T0[0] = t0 = time.perf_counter()
        try:
            t()
        except Fail as e:
            status, msg = "FAIL", str(e)
        except Warn as e:
            status, msg = "WARN", str(e)
        except Exception as e:  # noqa
            status, msg = "FAIL", "exception: %r" % e
            tb = traceback.format_exc()
            if a.verbose:
                traceback.print_exc()
        dt = time.perf_counter() - t0
        log_test(idx, t, cat, status, msg, dt, tb)
        counts[status] += 1
        per_cat.setdefault(cat, [0, 0, 0])[("PASS", "WARN", "FAIL").index(status)] += 1
        icon, c_ = {"PASS": (ok_i, "32"), "WARN": (wa_i, "33"), "FAIL": (ko_i, "31")}[status]
        name = t.__name__.replace("_", " ")
        print(" %s %s %s %s" % (col("90", "%3d/%-3d" % (idx, total)), col(c_, icon),
                                col("0" if status == "PASS" else c_, name.ljust(40)), col("90", "%5.2fs" % dt)))
        if msg:
            print("           " + col(c_, "↳ " + msg if fancy else "-> " + msg))
        if status != "PASS" and a.verbose:
            for c in CLIENTS[:4]:
                print(col("90", "           recu par %s%s :" % (c.nick, " (connexion fermee)" if c.closed else "")))
                print(col("90", c.dump() if c.lines else "      (rien recu)"))
        if status == "FAIL":
            failed.append((t.__name__, msg))
        elif status == "WARN":
            warned.append((t.__name__, msg))
        for c in CLIENTS:
            c.close()
        if server and server.poll() is not None:
            L("*** LE SERVEUR A CRASHE pendant '%s' (code %s)" % (t.__name__, server.returncode))
            logf.close()
            print(col("1;31", "\n  💥 LE SERVEUR A CRASHE pendant '%s' (code %s) -> note 0 au sujet !" % (t.__name__, server.returncode)
                      if fancy else "\n  SERVEUR CRASHE pendant '%s' (code %s)" % (t.__name__, server.returncode)))
            return 1
        time.sleep(0.05)

    # --- recapitulatif
    elapsed = time.perf_counter() - t_start
    print("\n" + col("36", "═" * 58))
    print(" " + col("1", "RECAPITULATIF PAR CATEGORIE"))
    for cat in ORDER:
        if cat in per_cat:
            ps, ws, fs = per_cat[cat]
            n = ps + ws + fs
            mark = col("32", ok_i) if not fs else col("31", ko_i)
            print("  %s %-20s %s  %s" % (mark, cat, bar(ps, ws, fs, 24), col("90", "%d/%d" % (ps, n))))
    print()
    ps, ws, fs = counts["PASS"], counts["WARN"], counts["FAIL"]
    score = 100 * ps // max(total, 1)
    print("  " + bar(ps, ws, fs, 40) + "  " + col("1", "%d%%" % score))
    print("  %s  %s  %s   %s" % (col("32", "%s %d reussis" % (ok_i, ps)), col("33", "%s %d warnings" % (wa_i, ws)),
                                  col("31", "%s %d echecs" % (ko_i, fs)), col("90", "(%d tests en %.1fs)" % (total, elapsed))))
    if server:
        alive = server.poll() is None
        print("  Serveur toujours vivant : " + (col("32", "oui " + ok_i) if alive else col("31", "NON " + ko_i)))
        if alive:
            server.terminate()
            server.wait()
    if failed:
        print("\n " + col("1;31", "A CORRIGER"))
        for name, msg in failed:
            print("  %s %s %s" % (col("31", ko_i), col("1", name), col("90", "— " + msg)))
    if warned:
        print("\n " + col("1;33", "A VERIFIER A LA MAIN"))
        for name, msg in warned:
            print("  %s %s %s" % (col("33", wa_i), name, col("90", "— " + msg)))
    if not failed:
        print("\n  " + col("1;32", ("🎉 " if fancy else "") + "Aucun echec !"))
    L("=" * 78)
    L("RESUME : %d valides, %d a verifier, %d non valides (sur %d tests, %.1fs)" % (ps, ws, fs, total, elapsed))
    for name, msg in failed:
        L("  NON VALIDE  %s : %s" % (name, msg))
    for name, msg in warned:
        L("  A VERIFIER  %s : %s" % (name, msg))
    logf.close()
    print(col("90", "  📄 log detaille : %s" % a.log if fancy else "  log detaille : %s" % a.log))
    print()
    return 1 if fs else 0


if __name__ == "__main__":
    sys.exit(main())
