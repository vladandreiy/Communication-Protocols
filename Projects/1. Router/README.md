Tema 1 - Protocoale de comunicatii - 321CB - Vasile Vlad-Andrei

Functionalitatile router-ului, intr-o bucla infinita sunt (ordinea difera
fata de cum au fost prezentate in cerinta temei):
1. Router-ul primeste un pachet prin functia get_packet()
2. Daca este un pachet de tip ARP, verifica daca este de tip ARP_REPLY sau
ARP_REQUEST. Daca este un ARP_REQUEST, router-ul va verifica daca adresa IP
destinatie a pachetului corespunde cu una din adresele IP ale router-ului si
va retine numarul interfetei. Va trimite un pachet ARP REPLY cu destinatia
catre destinatia ce are IP-ul si MAC-ul egale cu sursa de unde a primit
router-ul ARP REQUEST iar IP-ul si MAC-ul sursa sunt cele ale interfetei
router-ului mentionate anterior, pachetul fiind de asemenea trimis prin
aceasta interfata.
3. Daca este un pachet de tip ARP REPLY, se va verifica daca acesta este
destinat pentru router. Daca este destinat pentru router, va adauga in
tabela ARP a router-ului adresa IP si adresa MAC primite si va verifica 
daca coada de pachete contine vreun pachet. Daca coada contine pachete,
le va completa header-ul de ethernet corespunzator noii tabele ARP si 
le va dirija
4. Se verifica checksum-ul pachetului, daca acesta este gresit, se va arunca
pachetul.
5. Se verifica TTL-ul pachetului, daca acesta este mai mic decat 1, se va
trimite un ICMP_TIME_EXCEEDED cu adresele IP si MAC inversate
6. Daca router-ul primeste un pachet de tip ICMP, verifica daca acesta
este de tip ECHO REQUEST. Daca este adevarat, acesta va trimite un 
pachet ICMP de tip ECHO REPLY cu adresele IP si MAC inversate, pe acceasi
interfata pe care a venit si cu acelasi id si numar de secventa.
7. Decrementeaza TTL-ul si recalculeaza checksum-ul
8. Gaseste in tabela de routare cea mai buna potrivire pentru adresa IP
destinatie a pachetului. Cautarea se realizeaza in O(logn), intrucat este
una binara, tabela de routare fiind sortata la inceputul programului.
Tabela de routare este sortata dupa prefix apoi dupa masca
9. Se cauta in tabela ARP adresa MAC a next-hop-ului. Daca aceasta nu se
gaseste, se va trimite un ARP_REQUEST catre IP-ul next-hop-ului, prin 
interfata regasita in tabela de routare si IP-ul sursa corespunzator
interfetei. Se va adauga in coada pachetul.
In cazul in care se stie adresa MAC a next-hop-ului, se va completa
header-ul de ethernet corespunzator.
10. Se dirijeaza pachetul folosind functia send_pachet()