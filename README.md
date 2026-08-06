<h1> Ft_IRC </h1>

<h3>Protocols internet:</h3>

<center>Stream Sockets (TCP):</center>


<center><h4>Datagram Sockets( TCP) (Transmission Control Protocol) - est un protocole fondamental de la couche transport des réseaux informatiques. Il garantit une transmission fiable, ordonnée et sans erreur des données entre des applications connectées à Internet, fonctionnant de concert avec le protocole IP.Fonctionnement principalMode connecté : Une liaison est établie entre l'émetteur et le récepteur avant tout échange de données.Poignée de main en trois étapes (3-way handshake) : La connexion s'initialise via l'envoi successif de signaux SYN, SYN-ACK et ACK.Fiabilité : Chaque paquet envoyé possède un numéro de séquence et nécessite un accusé de réception (ACK).Correction des pertes : Si un segment est perdu en chemin, TCP le détecte et demande sa retransmission.Contrôle de flux et congestion : Le protocole ajuste dynamiquement la vitesse d'envoi pour éviter de saturer le réseau ou la machine réceptrice.* </h4></center>

<center>

[▶️ Voir la vidéo TCP](mdusages/tcp.gif)

</center>


<center>UDP:</center>


<center>
	<h4>UDP (User Datagram Protocol) - est un protocole de la couche transport (couche 4) des modèles OSI et TCP/IP. Il fonctionne en mode non connecté : il envoie des données (datagrammes) rapidement sans vérifier la liaison, sans accuser réception et sans garantir l'ordre d'arrivée des paquets.Caractéristiques principales d'UDPNon orienté connexion : aucun échange préalable (pas de handshake) n'est requis avant l'envoi.Léger et rapide : l'en-tête fait seulement 8 octets, ce qui réduit la surcharge réseau.Non fiable : pas de retransmission en cas de perte, de duplication ou de désordre des données.*
	<h4>
</center>

<center>

[▶️ Voir la vidéo UDP](mdusages/udp.gif)

</center>

<h3>IRC:</h3>

<center>
	<h4>Internet Relay Chat (IRC ; en français : « discussion relayée par Internet ») est un protocole de communication textuel sur Internet. Il sert à la communication instantanée principalement sous la forme de discussions en groupe par l’intermédiaire de canaux de discussion, mais il peut aussi être utilisé pour de la communication entre deux personnes seulement. Il peut par ailleurs être utilisé pour faire du transfert de fichier
	</h2>
</center>

Par construction IRC utilise le protocol TCP (sans perte de donnees) pour communiquer entre plusieurs clients de tel sorte que :



<center>

![Image](mdusages/protocole_irc.png)

</center>


<h3>Programmes de sockets client/serveur : appels de sockets bloquants, non bloquants et asynchrones</h3>

<center>
	Mode bloquant (Default mode)
</center>



<h3>Vocabulaire</h3>

socket = une connexion réseau



