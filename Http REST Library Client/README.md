&copy; Pogan Alexandru-Mihail, grupa 325CA, anul II

# Tema 4 - Implementare de client http

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Pentru aceasta tema, am avut de implementat un client, care trebuie sa trimita regulat diverse cereri de tip http catre un server, folosind un ip si un port. De asemenea, in tema am avut la dispozitie o serie de url-uri date dintr-un api, cu care putem efectua respectivele cereri. 

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Ulterior, clientul primeste raspunsul de la server, acceseaza datele propriu-size din cadrul raspunsului si informeaza utilizatorul legat de succes sau eroare, eventual si cauza acestul eveniment.

## Cum se gestioneaza comenzile
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; In acest scop, folosesc un map, in care fiecare actiune valida citita de la tastatura de catre user se asociaza cu o optiune, definita in fisierul client.cpp. Totodata, in functia main din acelasi fisier, imi mai definesc niste variabile ajutatoare, cu care rezolv o serie de edge case-uri, cum ar fi logare dupa logare, inregistrarea dupa logare etc.
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; De asemenea, pentru fiecare comanda este obligatoriu urmatorul set de pasi:
1. conectarea la server
2. trimiterea requestului (eventual dupa verificarea unor constrangeri)
3. primirea requestului
4. informarea utilizatorului 
5. inchiderea conexiunii cu serverul

## Inregistrarea clientului

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Pentru inceput, verific daca utilizatorul este deja inregistrat. In caz afirmativ, afisez un mesaj corespunzator si programul merge mai departe. In caz contrar, intra in functia de inregistrare, do_register. In aceasta functie, imi construiesc un obiect de tip json, unde imi salvez informatiile citite de la tastatura, nu inainte de a verifica daca username-ul si parola sunt corect definite, neavand caractere ilegale, cum ar fi spatiile. Mai departe, deschid o noua conexiune cu serverul, caruia ii trimit ulterior un POST request, cu json-ul creat si serializat, precum si toti parametrii corecti. Dupa ce se primeste raspunsul de la server, se verifica daca json-ul propriu-zis din raspuns contine campul error. In caz afirmativ, extrag informatia si o afisez in consola. in caz contrar, afisez un mesaj de succes. Totodata, am inchis si conexiunea cu serverul.

## Logarea clientului

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Verific daca utilizatorul este deja logat, ca sa evit situatiile in care utilizatorul vrea sa se logheze de doua ori. Ma conectez la server, trimit un POST request catre url-ul de login si in functie de ce primesc de la server, informez utilizatorul de succes sau eroare. Implementarea este foarte similara cu cea de la register. In plus, functia returneaza un cookie de sesiune, folosit la "entry_library". Pentru a obtine acest cookie, am accesat pointerul la substringul din raspuns care incepe cu "connect", precum si pointerul la urmatorul caracter ';' din substringul gasit anterior. Login cookie-ul reprezinta diferenta dintre primul substring si al doilea. 

## Accesarea librariei

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Aici am implementat functia acces_library, in care verific daca userul este deja logat in sistem, dupa ma conectez la server, verificand daca conexiunea s-a stabilit, in caz contrar returnand un sir gol, apoi trimit un POST request cu login cookie-ul obtinut anterior, urmand ca in functie de raspunsul serverului sa informez corect userul.

## Gasirea tuturor cartilor

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Aici spre deosebire de comenzile anterioare, am folosit un request de tip GET, iar in caz de succes, am returnat substringul care incepe cu caracterul "[", inceputul listei de carti.

## Gasirea unei carti cu id dat

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Aici am verificat faptul ca userul este logat si faptul ca id-ul dat ca parametru este un numar. Dupa, am trimis un GET request cu serverul connectat anterior. In cazul in care se gaseste cartea, imi iau un json object, in care salvez substring-ul care incepe cu "{", parsat. In final, pe langa mesajul de succes, mai afisez si cartea cu formatul pretty.

## Adaugarea unei carti

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Aici, pe langa toti pasii comuni cu celelalte comenzi, m-am asigurat ca toate campurile sunt completate corect, inclusiv numarul de pagini ca numar natural, iar celelalte field-uri sa nu fie goale.

## Stergerea unei carti

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Exact la fel ca la gasirea unei carti cu id dat, mai putin faptul ca pentru succes, doar am afisat mesajul corespunzator si request-ul este de tip DELETE.

## Logout

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Exact la fel ca la login.

## Exit

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Aici activez flagul de iesire si apoi programul se opreste.

## Mentiuni

* Pentru aceasta tema am folosit scheletul de laborator de http: https://gitlab.cs.pub.ro/pcom/pcom-laboratoare-public/-/tree/master/lab9. 
* De asemenea, am folosit doua zile de sleep days.
* Pentru a putea lucra mai usor, am folosit parser-ul de json de la nlohmann, datorita simplitatii folosirii acesteia, fiind totodata recomandata si in tema.
