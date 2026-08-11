# 📚 Vocabulaire réseau & IRC

Ce document regroupe les principaux termes utilisés dans l'étude des réseaux informatiques, du protocole IRC et du projet `ft_irc`.

---

# Sommaire

## 🌐 1. Réseaux et protocoles

1. [Protocole de communication](protocole-comm.md)
2. [RFC](rfc.md)
3. [Modèle OSI](osi_modele.md)
4. [Internet](internet.md)
5. [IP](ip.md)
6. [DNS](dns.md)
7. [DHCP](dhcp.md)
8. [WWW](WWW.md)

---

## 🔌 2. Communication réseau

9. [Socket](../functions/socket()/SOCKET.md)
10. [TCP (Stream Sockets)](TCP.md)
11. [UDP (User Datagram Protocol)](udp.md)
12. [En-tête TCP](en-tete-tcp.md)
13. [3-Way Handshake](3-Way%20Handshake.md)
14. [SYN](SYN.md)
15. [SYN-ACK](SYN-ACK.md)
16. [ACK](ACK.md)
17. [Handshake](handshake.md)
18. [TLS / SSL](tls-ssl.md)

---

## ⚙️ 3. Gestion des entrées / sorties

19. [Buffer](buffer.md)
20. [EAGAIN](EAGAIN.md)
21. [EWOULDBLOCK](EWOULDBLOCK.md)
22. [Communication asynchrone](comm_asynchrone.md)
23. [Événements / Events](event.md)
24. [Microservices](microservices.md)
25. [Proxy](proxy.md)

---

## 💬 4. IRC

26. [Clients IRC](client_irc.md)
27. [Serveur IRC / IRCd](ircd.md)
28. [Modes IRC](modes-irc.md)
29. [Channel IRC](channel-irc.md)
30. [Grades IRC](grades.md)
31. [Commandes IRC](commandes.md)
32. [Format des messages IRC](message-format-irc.md)
33. [Découpage des messages IRC](decoupage-message-irc.md)
34. [Enregistrement IRC](login-irc.md)
35. [Numeric Replies](numeric-replies.md)
36. [Robots IRC](bot.md)
37. [Bouncer / BNC](bouncer_bnc.md)
38. [DCC (Direct Client-to-Client)](DCC.md)

---

## 📁 5. Transfert de fichiers

39. [Protocole de transfert de fichiers](file-transfer.md)

---


# 📌 Les distinctions essentielles

| Terme            | À retenir                                            |
| ---------------- | ---------------------------------------------------- |
| **Socket**       | Interface utilisée par un programme pour communiquer |
| **TCP**          | Transport fiable sous forme de flux                  |
| **UDP**          | Transport sous forme de datagrammes                  |
| **IP**           | Adressage et acheminement des paquets                |
| **Paquet**       | Unité de données d'une couche réseau, couramment IP  |
| **Segment**      | Unité de données TCP                                 |
| **Datagramme**   | Unité de données UDP                                 |
| **Buffer**       | Zone de stockage temporaire                          |
| **DNS**          | Résolution de noms                                   |
| **DHCP**         | Configuration automatique du réseau                  |
| **IRCd**         | Serveur IRC                                          |
| **Client IRC**   | Programme utilisé pour se connecter à IRC            |
| **Bot IRC**      | Client IRC automatisé                                |
| **BNC**          | Intermédiaire maintenant une connexion IRC           |
| **Proxy**        | Intermédiaire entre client et serveur                |
| **TLS**          | Sécurisation de la communication                     |
| **Handshake**    | Procédure d'établissement/négociation                |
| **SYN**          | Demande/initiation TCP                               |
| **SYN-ACK**      | Réponse du serveur au SYN                            |
| **ACK**          | Accusé de réception TCP                              |
| **EAGAIN**       | Ressource momentanément indisponible                 |
| **EWOULDBLOCK**  | L'opération bloquerait en mode non bloquant          |
| **RFC**          | Document de spécification technique                  |
| **DCC**          | Communication directe entre clients IRC              |
| **Microservice** | Service logiciel indépendant et spécialisé           |
| **Event**        | Fait qui s'est produit dans un système               |
