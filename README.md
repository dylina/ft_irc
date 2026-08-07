<h1> Ft_IRC </h1>

<center><h2>Protocols internet:</h2></center>

<center><h3>Stream Sockets (TCP):</h3></center>


<center><h5>Datagram Sockets( TCP) (Transmission Control Protocol) - est un protocole fondamental de la couche transport des réseaux informatiques. Il garantit une transmission fiable, ordonnée et sans erreur des données entre des applications connectées à Internet, fonctionnant de concert avec le protocole IP.Fonctionnement principalMode connecté : Une liaison est établie entre l'émetteur et le récepteur avant tout échange de données.Poignée de main en trois étapes (3-way handshake) : La connexion s'initialise via l'envoi successif de signaux SYN, SYN-ACK et ACK.Fiabilité : Chaque paquet envoyé possède un numéro de séquence et nécessite un accusé de réception (ACK).Correction des pertes : Si un segment est perdu en chemin, TCP le détecte et demande sa retransmission.Contrôle de flux et congestion : Le protocole ajuste dynamiquement la vitesse d'envoi pour éviter de saturer le réseau ou la machine réceptrice.* </h5></center>

<center>

![](mdusages/tcp.gif)

</center>


<center><h3>UDP:</h3></center>


<center>
	<h5>
		UDP (User Datagram Protocol) - est un protocole de la couche transport (couche 4) des modèles OSI et TCP/IP. Il fonctionne en mode non connecté : il envoie des données (datagrammes) rapidement sans vérifier la liaison, sans accuser réception et sans garantir l'ordre d'arrivée des paquets.Caractéristiques principales d'UDPNon orienté connexion : aucun échange préalable (pas de handshake) n'est requis avant l'envoi.Léger et rapide : l'en-tête fait seulement 8 octets, ce qui réduit la surcharge réseau.Non fiable : pas de retransmission en cas de perte, de duplication ou de désordre des données.*
	<h5>
</center>

<center>

![](mdusages/udp.gif)

</center>

<center><h3>IRC (Internet Relay Chat):</h3></center>

<center>
	<h5>Internet Relay Chat (IRC ; en français : « discussion relayée par Internet ») est un protocole de communication textuel sur Internet. Il sert à la communication instantanée principalement sous la forme de discussions en groupe par l’intermédiaire de canaux de discussion, mais il peut aussi être utilisé pour de la communication entre deux personnes seulement. Il peut par ailleurs être utilisé pour faire du transfert de fichier


	IRC est un protocole utilisant TCP et de façon optionnelle TLS. Un serveur IRC peut se connecter à d’autres serveurs IRC. Un réseau IRC est un ensemble de serveurs IRC connectés l’un à l’autre relayant si nécessaire les messages, formant ainsi une architecture répartie. L’utilisateur utilise un logiciel client IRC afin de se connecter à un des serveurs du réseau, le choix du serveur étant indifférent.

</h5>
</center>




<center>

![](mdusages/ircmodele.png)

</center>


<h5>Programmes de sockets client/serveur : appels de

[sockets](#socket) :

[bloquants](#mode-bloquant-(default-mode)),
[non bloquants](#mode-non-bloquant) et
[asynchrones](#asynchrones)</h5>


<center><h2>Specificite d'IRC</h2></center>
<h5>La plupart des serveurs IRC ne nécessitent pas l’enregistrement par l’intermédiaire d’un compte, l'utilisateur pouvant alors choisir son nickname (surnom) librement tant qu'il est disponible.</h5>

<center>
<h3>Moyen d'accès:</h3>
</center>

<h5>L'accès d'utilisateur à un réseau IRC se fait historiquement au moyen d'un logiciel dit « Client IRC ».
</h5>

<center>
	<h3>
	Channel
	</h3>
</center>

<h5>
	L’élément de base pour communiquer sur un réseau est le canal (channel dans la spécification). Un canal est défini par une liste d’utilisateurs connectés sur celui-ci. En français, un canal est comparable au mot "salon"

<center>

[Mode de canal](#modes-de-canal)

</center>

</h5>


<center>
	<h3>
	Mode bloquant (Default mode)
	</h3>
</center>


<center>
	<h5>Le programme s'arrête complètement et attend que l'action soit terminée avant de continuer. Par exemple, s'il attend une connexion d'un client, rien d'autre ne se passe tant que ce client n'arrive pas. C'est simple à programmer mais ça peut « geler » l'application.
	</h5>
</center>

<center>
	<h3>
		Mode non bloquant
	</h3>
</center>

<center>
	<h3>
		asynchrones
	</h3>
</center>




https://www.youtube.com/watch?v=D26sUZ6DHNQ

<center>
# Modes de canal
</center>











<center>
	<h3>
		Vocabulaire
	</h3>
</center>


- <h5>
	modes :
  </h5>
	Les modes sont des options positionnées sur des canaux ou des utilisateurs, et qui affectent leur mode de fonctionnement, leurs privilèges et leurs interactions avec les autres entités de l’IRC.

- <h5>
	socket :
  </h5>  une connexion réseau

- <h5>
	RFC (Request for comments) :</h5> Les Requests for comments (RFC), littéralement « demandes de commentaires », sont une série numérotée de documents décrivant les aspects et spécifications techniques d'Internet, ou de différents matériels informatiques (routeurs, serveur DHCP). Peu de RFC sont des standards, mais tous les documents publiés par l'IETF sont des RFC.
  </h5>

- <h5>DHCP (Dynamic Host Configuration Protocol)</h5> DHCP, protocole de configuration dynamique des hôtes est un protocole réseau dont le rôle est d’assurer la configuration automatique des paramètres IP d’une station ou d'une machine, notamment en lui attribuant automatiquement une adresse IP et un masque de sous-réseau. DHCP peut aussi configurer l’adresse de la passerelle par défaut et configurer des serveurs de noms comme DNS ou NBNS

- <h5>IP</h5> nternet Protocol (littéralement protocole Internet, abrégé en IP) est une famille de protocoles de communication de réseaux informatiques conçus pour être utilisés sur Internet. Les protocoles IP sont au niveau 3 dans le modèle OSI. Les protocoles IP s'intègrent dans la suite des protocoles Internet et permettent un service d'adressage unique pour l'ensemble des terminaux connectés.

- <h5>Internet</h5> Internet est le réseau informatique mondial accessible au public.Il est constitué de millions de réseaux tant publics que privés, universitaires, commerciaux et gouvernementaux, à commutation de paquets, sans centre névralgique. Ceux-ci sont regroupés en réseaux autonomes. Il existe plus de 91 000 réseaux distincts en 2019[1].Les données échangées sont retransmises par Internet grâce à un ensemble standardisé de protocoles de transfert de données, lesquels permettent différentes applications comme le courrier électronique, le World Wide Web, la messagerie instantanée, le partage de fichiers en pair-à-pair, le streaming, etc.

- <h5>
	WWW (World Wide Web)</h5> Le World Wide Web (/ˌwɜːld waɪd ˈweb/[a]), abrégé en WWW, W3, le Web, la toile mondiale ou simplement la toile[1], est un système hypertexte public fonctionnant sur Internet. Le Web permet de consulter, à l'aide d'un navigateur, des pages regroupées en sites. Web signifie littéralement « toile (d’araignée) », image représentant les hyperliens qui lient les pages web entre elles[b].Le Web est une des applications d'Internet[2], qui est distincte d'autres applications comme les réseaux sociaux, le courrier électronique et la visioconférence. Inventé en 1989-1990 au CERN par Tim Berners-Lee épaulé de Robert Cailliau, le Web popularise Internet[3]. Depuis, il est fréquemment confondu avec ce dernier[4].
</h5>

 - <h5>client IRC </h5>
	permet de se connecter à un serveur pour discuter en direct par texte sur le réseau IRC (Internet Relay Chat). Il sert à échanger des messages de groupe dans des salons ou en privé, et à partager des fichiers.

	
https://youtu.be/hDLvHKEASRE?si=Y0SBig43sWt_B7g0
