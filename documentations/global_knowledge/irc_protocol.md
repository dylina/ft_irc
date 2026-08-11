## IRC (Internet Relay Chat):

</u></h3>


<h5 align="center">Internet Relay Chat (IRC ; en français : « discussion relayée par Internet ») est un protocole de communication textuel sur Internet. Il sert à la communication instantanée principalement sous la forme de discussions en groupe par l’intermédiaire de canaux de discussion, mais il peut aussi être utilisé pour de la communication entre deux personnes seulement. Il peut par ailleurs être utilisé pour faire du transfert de fichier

<br>

IRC est un protocole utilisant TCP et de façon optionnelle TLS. Un serveur IRC peut se connecter à d’autres serveurs IRC. Un réseau IRC est un ensemble de serveurs IRC connectés l’un à l’autre relayant si nécessaire les messages, formant ainsi une architecture répartie. L’utilisateur utilise un logiciel client IRC afin de se connecter à un des serveurs du réseau, le choix du serveur étant indifférent.

<br>

Fonctionnement technique :

	Modèle client-serveur :
	 Un logiciel client (comme HexChat ou Irssi) se connecte à un serveur distant.

	 Réseau de serveurs :
	  Les serveurs peuvent être reliés entre eux pour former un réseau global et propager les messages.

	  Mode texte brut :
	   Les commandes et les messages transitent en clair (généralement en ASCII) sur un port standard (souvent le 6667).

<br>

Les connexions client-serveur IRC fonctionnent via TCP/IP. Les ports standard pour ces connexions sont TCP/6667 pour le texte clair et TCP/6697 pour les connexions TLS.

</h5>







![](mdusages/illustrations/ircmodele.png)




<h5>

Programmes de sockets client/serveur :
- [appel de sockets](#socket)



3 Modes

- [Mode bloquants](#mode-bloquant)

- [Mode non bloquant](#mode-non-bloquant)

- [asynchrones](#asynchrones)

</h5>


## Le transfert de fichier


Le transfert de fichier peut se faire d'ordinateur à périphérique, de serveur à serveur, de client à serveur, ou de client à client par l'intermédiaire d'un serveur de messagerie électronique

Le transfert de fichier via un réseau informatique se fait à l'aide de protocoles réseau et d'applications adaptées. Le FTP, est un protocole spécifique pour le partage de fichiers, ou encore le [SCP](#scp-secure-copy-protocol) ou le [SFTP (SSH File Transfer Protocol)](#sftp-ssh-file-transfer-protocol). Elles peuvent emprunter différents réseaux, tels qu'un réseau X.25 ou l'Internet.
