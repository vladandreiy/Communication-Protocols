Tema 3 PC - 321CB - Vasile Vlad-Andrei

Am ales folosirea biblioteca nlohmann pentru parsarea obiectelor de tip JSON,
intrucat am rezolvat tema in C++. Consider ca aceasta a fost o alegere utila,
intrucat lucrul cu biblioteca a fost usor.
Am pornit rezolvarea temei plecand de la scheletul oferit in Laboratorul 10.
In fisierul requests.cpp, exista 3 functii utile pe care le voi folosi in 
fisierul principal, client.cpp:

1. compute_get_request - returneaza un char* reprezentand un mesaj de tip GET
ce poate fi trimis prin intermediul protocolului HTTP. Pentru a construi acest
mesaj, se primesc ca parametru: url-ul unde se doreste sa se ajunga, parametrii
din intermediul query-ului, un vector de string-uri de headere optionale si
un vector de string-uri de cookie-uri. Pentru a se construi mesajul, se
va adauga fiecare linie din mesaj pe rand, cu ajutorul functie compute_message
care va adauga o linie la mesaj-ul deja construit si va adauga in plus "\r\n".
Mai intai se adauga o linie cu metoda GET, apoi o linie pentru fiecare header
din vectorul de headere primit, apoi o linie ce va contine toate cookie-urile
separate ";" si apoi o linie goala.

2. compute_post_request - analog compute_get_request, primeste in plus un
vector de string-uri body_data reprezentand continutul mesajului POST si un
string content_type, reprezentand tipul continutului (in cadrul acestei 
aplicatii, acesta va fi mereu application/json). Operatiile in plus care se fac
fata de functia anterioara sunt: construirea unui buffer auxiliar in care
se va pune continutul interior al mesajului. Se va calcula lungimea acestui
buffer, si impreuna cu lungimea headerelor se va seta campul Content-Length
din mesajul HTTP. Se vor adauga headerele necesare ca linii separate 
(Content-Type si Content-Length), se vor adauga cookie-urile, datele din
buffer-ul creat si o linie noua la sfarsit.

3. compute_delete_request - analog compute_get_request, cu exceptia ca in loc
de comanda GET se va trimite comanda DELETE

In fisierul client.cpp:
Functia main preia comenzi de la STDIN, pana in momentul cand da de comanda
"exit", moment in care se inchide conexiunea la server si se iese din program.
In aceasta bucla se creeaza conexiunea catre server la fiecare parcurgere, si
se va inchide la finalul buclei. Am luat aceasta decizie pentru a nu exista
probleme prin care conexiunea cu server-ul se pierde sau se inchide automat
dupa un anumit interval.
1. Comanda "register", se va construi un obiect json cu ajutorul librariei nlohmann. Acest obiect va avea username-ul si parola primite
primite de la input. Se va trimite un mesaj de POST cu campurile specifice.
2. Comanda login "login", se va construi un obiect json cu 
ajutorul librariei nlohmann. Acest obiect va avea username-ul si parola
primite la input si se va trimite un mesaj de POST cu campurile specifice. Din 
raspunsul primit de la server, se va extrage cookie-ul primit din interiorul
mesajului si se va salva in caz ca va fi nevoie mai tarziu.
3. Comanda "enter_library", se va construi un mesaj de tip GET,
ce va avea cookie-ul setat la login in cazul in care a fost data 
si se va trimite catre server. Se va extrage token-ul din body-ul mesajului
cu ajutorul librariei externe nlohmann prin functia parse.
si parola primite la input si se va trimite un mesaj de POST cu campurile
specifice. Din raspunsul primit de la server, se va extrage cookie-ul primit
din interiorul mesajului si se va salva in caz ca va fi nevoie mai tarziu.
4. Comanda "get_books" va trimite un mesaj GET server-ului in care va atasa
token-ul JWT primit de la comanda enter_library, in caz ca acesta exista.
Va extrage din raspunsul primit de la server cu ajutorul librariei nlohmann
si functia parse, din body-ul mesajului obiectele json si se vor afisa
cu functia dump.
5. Comanda "get_book" va astepta sa primeasca un ID si va trimite un mesaj
GET serverului cu id-ul primit de la input si token-ul JWT primit dupa apelul
enter_library va extrage cu ajutorul librariei nlohmann din body-ul mesajului
informatiile despre carte ce se vor afisa apoi cu functia dump.
6. Comanda "add_book" va astepta sa primeasca de la input informatiile 
specifice unei carti, se va forma un obiect json cu acestea ce se vor introduce
in body-ul mesajului ce urmeaza sa fie trimis. In mesajul POST se va adauga si 
token-ul JWT si se va trimite serverului. 
7. Comanda "delete_book" va astepta sa primeasca de la input id-ul cartii
si va trimite un mesaj de tip DELETE catre server, caruia ii va fi atasat si
token-ul JWT
8. Comanda "logout" va trimite un mesaj GET catre server cu cookie-ul sesiunii
curente.
In cadrul acestor comenzi, se verifica daca cod-ul HTTP primit de la server
este unul de eroare, caz in care se va afisa un mesaj sugestiv.