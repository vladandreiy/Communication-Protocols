Tema 2 - Protocoale de comunicatii - 321CB - Vasile Vlad-Andrei

In cadrul implementarii temei am utilizat o structura specifica unui mesaj
UDP care contine un string de maxim 50 de caractere, un byte ce reprezinta
tipul si un string de maxim 1500 de caractere ce reprezinta datele.
Structura specifica unui mesaj tcp este urmatoarea: 4 bytes in care se afla
lungimea totala a mesajului, o structura in care se pastreaza datele 
referitoare la adresa clientului UDP ce a trimis mesajele si o structura
de mesaj UDP mentionata anterior.

Subscriber-ul are urmatoarea functionalitate:
Se conecteaza la server-ul ai carui parametrii sunt dati in linia de comanda,
se trimite un prim mesaj server-ului cu id-ul clientului si se executa intr-o
bucla infinita urmatoarele instructiuni:
	1. Selecteaza din multimea de file descriptori (formata din stdin si 
	socket-ul corespunzator conexiunii TCP cu serverul) pe cel care este activ
	2. Daca sunt primite date de la stdin, acestea se citesc intr-un buffer
	si se va verifica daca este o comanda de tipul subscribe, unsubscribe sau 
	exit.
		a. Comanda de tip subscribe: se trimite pe socket-ul tcp corespunzator
		serverului linia citita, dupa ce aceasta a fost verificata sa fie
		corespunzatoare (topic de maxim 50 de caractere, existenta set 
		flag-ului, etc.)
		b. Comanda de tip unsubscribe: asemanator cu comanda de tip subscribe
		c. Comanda de tip exit: se inchide conexiunea cu server-ul tcp si se
		iese din program.
		Orice alta comanda va duce la afisarea in strem-ul standard de eroare
		a mesajului "Unknown command"
	3. Daca se face select pe socket-ul corespunzator conexiunii cu serverul
	tcp, se vor executa urmatoarele instructiuni:
		a. Se primesc datele intr-un buffer initial.
		b. Se adauga datele primite la sfarsitul unui al doilea buffer. Scopul
		acestui al doilea buffer este pentru a asigura faptul ca in acesta
		va exista mereu (sau cel putin cat timp nu s-au printat toate mesajele)
		inceputul unui mesaj primit de la tcp, astfel incat se va stii lungimea
		intregului mesaj ce va trebui sa fie asteptata inainte de a se printa 
		mesajul
		c. Se aduna intr-un contor numarul de bytes primiti de la server
		d. Cat timp numarul de bytes din contor este mai mare decat lungimea
		primului mesaj din cel de-al doilea buffer, se va afisa mesajul,
		se va sterge acest prim mesaj, fiind inlocuit de urmatoarele date din
		buffer. Din contor se scade valoarea mesajului ce a fost printat, si
		se reia aceasta intructiune.
Functia de printare a unui mesaj afiseaza datele relevante, in formatul
specificat, in functie de ceea ce contine campul corespunzator tipului din
structura mesajului tcp.


In cadrul server-ului, am definit 3 structuri utile:
1. un vector ce contine perechi de string-uri cu int-uri reprezentate de 
id-ul unui client cu socket-ul cu care este acesta conectat in mod curent sau
0 pentru un client ce a fost conectat si a iesit. Cu ajutorul acestei
structuri, se poate afla usor file descriptorul pentru socket-ul unde trebuie
trimis mesajul clientului cu ID-ul respectiv. De asemenea, atunci cand un
client se deconecteaza, sockfd-ul acestuia este setat la 0 si este usor
de determinat cand un client este deconectat, iar daca acesta se reconecteaza
se va schimba valoarea acestuia.
2. un dictionar al carui prim membru (cel dupa care se face cautarea) este un 
string reprezentat de topic iar al doilea membru este reprezentata de un vector
de perechi de tip string, int reprezentand id-ul clientului abonat la topic
si flag-ul setat de acesta (un topic este stocat o singura data, indiferent de
numarul de clienti care sunt abonati la acesta). Cu ajutorul acestei structuri
se pot afla toti abonatii unui anumit topic si daca acestia au set flag-ul
setat sau nu. Atunci cand un client da subscribe id-ul acestuia si set flag-ul
sunt adaugate vectorului unui topic iar cand da unsubscribe este scos din acel
vector.
3. un vector de perechi de mesaje de tip tcp si un vector de string-uri
reprezentand id-uri unde se vor stoca mesajele ce trebuie trimise unor clienti
ce nu sunt conectati. (un mesaj este stocat o singura data, indiferent de numarul de clienti carora trebuie sa fie trimis, cand s-a trimis tuturor 
clientilor acesta este sters din memorie). Atunci cand un client se conecteaza,
se verifica daca id-ul acestuia se afla prin id-urile acestui vector si 
i se trimit mesajele corespunzatoarea. Dupa ce ii este trimis mesajul,
id-ul ii este eliminat din vector, iar daca vectorul de id-uri pentru un mesaj
dat ramane gol, mesajul va fi sters din memorie.

Server-ul are urmatoarea functionalitate:
Asteapta conexiuni atat de tip tcp cat si de tip udp pe portul primit ca
parametru in linia de comanda. Intr-o bucla infinita, acesta executa 
urmatoarele instructiuni:
	1. Selecteaza din multimea de file descriptori (formata din stdin, 
	socket-ul corespunzator clientilor udp si orice alt socket reprezentat
	de un client tcp)
	2. Daca sunt primite date de la stdin, se verifica daca acesta este un
	mesaj de tip exit ce va inchide toate conexiunile prezente ale server-ului
	si va iesi din program.
	3. Daca sunt primite date de la un client udp:
		a. Daca mesajul contine un topic nou se va seta vectorul de clienti
		abonati la null
		b. Se parcurge lista de clienti ce sunt abonati la topic-ul respectiv.
		Daca clientul este abonat si este online in momentul respectiv,
		i se trimite mesajul in acel moment. Altfel, se va adauga mesajul
		in vectorul de mesaje stocate daca acesta nu a fost adaugat inainte.
	4. Daca se conecteaza un nou client tcp:
		a. Se verifica daca exista un client cu acelasi id conectat deja
		b. In caz contrar, se adauga noul socket la multimea de file
		descriptori ce vor fi parcursi in bucla
		c. Daca sunt mesaje ce au fost stocate pentru id-ul clientului,
		acestea se trimit acum. Daca acesta era ultimul client caruia trebuia
		sa ii fie trimis un mesaj anume, acel mesaj se sterge.
	5. Daca se primesc date de pe un socket corespunzator unei conexinui tcp,
	se verifica daca acesta este o comanda de tip subscribe sau unsubscribe
		a. Comanda de tip subscribe: Daca client-ul este abonat la topic, 
		se afiseaza un mesaj de eroare. Altfel, id-ul acestuia va fi marcat
		alaturi de set flag in dictionar.
		b. Daca este un mesaj de tip unsubscribe, acesta va fi sters din 
		dictionar (in cazul in care era abonat la topic)