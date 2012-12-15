#include <iostream>
#include <fstream>
#include <pthread.h>
#include <winsock2.h>
#include "Gamepad.h"
#include "KBSend.h"
#define SERVER 1

using namespace std;

short static disable = 0;
void* communication(void* data);
#if SERVER
short nb_clients = 1;
#endif

int main()
{
    // ******************************EN TETE******************************
    cout << "=============================================" << endl;
    cout << "Remote PAD Disabler v1.2\nby Bastnt" << endl;
    cout << "=============================================" << endl;
    // *******************************************************************


    // ******************* Lecture du fichier PADS.txt *******************
    string nom1; // Nom du PAD#1
    string nom2; // Nom du PAD#2
    char lect; // Caractère de lecture du nom dans le fichiers PADS.txt
    short config1[4]; // Map du PAD#1
    short config2[4]; // Map du PAD#2
    short id_pad1; // ID USB du PAD#1
    short id_pad2; // ID USB du PAD#2
    // /!\ Avertissement : Les IDs des périphériques sont juste leur position dans la pile des périphériques USB connectés.
    bool duo_pad=false; // Boolean : Y'a t-il 2 PADS ?
    bool enter=false; // Le serveur appuie sur la touche Entrée
    ifstream fich("PADS.txt"); // Ouverture de PADS.txt

    #if SERVER
    string nom3; // Nom du PAD Admin
    short config3[2]; // Touche d'administration
    short id_pad3; // ID USB du PAD Admin
    ifstream fich2("ADMIN_PAD.txt"); // Ouverture
    cout << "\nHello I'm the server !" << endl;
    cout << "Number of clients: ";
    cin >> nb_clients;
    #endif

    if(fich){ // S'il existe bien
        short lout=0;
        while(1) // Si on dépasse 100 caractères, c'est qu'il y a une erreur !
        {
            fich >> lect;
            if(lect!='=')
                nom1+=lect;
            else
                break;

            if(++lout > 100)
            {
                cout << "Fichier PADS.txt incorrect." << endl;
                system("PAUSE");
                exit(EXIT_FAILURE);
            }
        }

        for(int i=0;i<4;i++)
            fich >> config1[i];

        if(fich >> lect) // S'il y a un 2ème PAD dans le fichier
        {
            duo_pad=true;
            nom2+=lect;

            short lout=0;
            while(1) // Si on dépasse 100 caractères, c'est qu'il y a une erreur !
            {
                fich >> lect;
                if(lect!='=')
                    nom2+=lect;
                else
                    break;

                if(++lout > 100)
                {
                    cout << "Fichier PADS.txt incorrect." << endl;
                    system("PAUSE");
                    exit(EXIT_FAILURE);
                }
            }

            for(int i=0;i<4;i++)
                fich >> config2[i];
        }
        fich.close();
    }
    else{ // On quitte le programme et on créé le fichier en cas d'échec
        cout << "Le fichier PADS.txt est vide !" << endl;
        ofstream("PADS.txt");
        system("PAUSE");
        exit(EXIT_FAILURE);
    }

    #if SERVER
    if(fich2)
    {
        short lout=0;
            while(1) // Si on dépasse 100 caractères, c'est qu'il y a une erreur !
            {
                fich2 >> lect;
                if(lect!='=')
                    nom3+=lect;
                else
                    break;

                if(++lout > 100)
                {
                    cout << "Fichier ADMIN_PAD.txt incorrect." << endl;
                    system("PAUSE");
                    exit(EXIT_FAILURE);
                }
            }

            for(int i=0;i<4;i++)
                fich2 >> config3[i];
    }
    else
    {
        cout << "Le fichier ADMIN_PAD.txt est vide !" << endl;
        ofstream("ADMIN_PAD.txt");
        system("PAUSE");
        exit(0);
    }
    fich2.close();
    #endif
    // *******************************************************************

    cout << endl << "Looking for PADS : " <<endl;
    cout << nom1 << endl;
    if(duo_pad)
        cout << nom2 << endl;
    #if SERVER
    cout << "Looking for Admin PAD : " << endl;
    cout << nom3 << endl;
    #endif

    cout << endl;

    // ********* Connexion aux périphériques avec Gamepad class ***********

    Gamepad* pad1 = new Gamepad();
    Gamepad* pad2 = new Gamepad();
    #if SERVER
    Gamepad* pad3 = new Gamepad();
    #endif

    if((id_pad1=pad1->open(nom1))) // Le PAD1 a été trouvé
    {
        cout << "Found : " << endl;
        cout << "(" << id_pad1 << ") " << pad1->getName() << endl;
        if(duo_pad && (id_pad2=pad2->open(nom2, id_pad1))) // Recherche de PAD2 excluant l'ID du
                                  // premier pad trouvé.
        {
            cout << "(" << id_pad2 << ") " << pad2->getName() << endl;
        }
        else
        {
            duo_pad=false;
        }

    }
    else
    {
        cout << "No PADS have been found." << endl;
        system("PAUSE");
        exit(0);
    }

    #if SERVER
    if((id_pad3=pad3->open(nom3)))
    {
        cout << "\nFound ADMIN : " << endl;
        cout << "(" << id_pad3 << ") " << pad3->getName() << endl;
    }
    else
    {
        cout << "No Admin PAD has been found.\n" << endl;
        system("PAUSE");
        exit(0);
    }

    cout << "\nListening port 4242 : " << endl;
    #endif
    // *******************************************************************

    // ******************Création des Threads et Sockets******************
    pthread_t thread1;
    pthread_create(&thread1, NULL, communication, NULL);
    // *******************************************************************


    BYTE *boutons1; // La liste des boutons du PAD
    BYTE *boutons2;
    bool press_pad1[4] = {0}; // Liste Mémoire pour savoir si à n-1 le bouton i était appuyé
    bool press_pad2[4] = {0};
    short keymap1[] = {37,40,38,39}; // Keymappage des touches claviers à utiliser
    short keymap2[] = {100,98,104,102};

    #if SERVER
    BYTE *boutons3;
    bool press_pad3[1] = {0};
    #endif

    // *************** Capture de l'entrée PAD en boucle *****************
    do{
        #if SERVER
        if(!pad1->synch() || !pad3->synch()) // Si on perd le signal avec l'un des PAD
            break; // On arrête le programme
        #else
        if(!pad1->synch()) // Si on perd le signal avec l'un des PAD
            break; // On arrête le programme
        #endif

        #if SERVER
        boutons3 = pad3->getButtons();

        if(boutons3[config3[0]-1] && !press_pad3[0])
        {
            disable=!disable;
            press_pad3[0]=1;
            cout << (disable ? "D " : "E ");
        }
        else if(press_pad3[0] && !boutons3[config3[0]-1])
        {
            press_pad3[0]=0;
        }

        #endif


        boutons1 = pad1->getButtons();

        if(/* !SERVER && */disable) // Si le serveur a envoyer DISABLE, on ne peut qu'appuyer sur la touche BAS.
        {
            KB_press(&press_pad1[1], boutons1[config1[1]-1], keymap1[1]);
            if(enter)
            {
                KB_enter();
                enter=false;
            }

        }
        else
         {
            for(int i=0;i<4;i++)
                KB_press(&press_pad1[i], boutons1[config1[i]-1], keymap1[i]);
            if(!enter)
                enter=true;
         }

        if(duo_pad)
        {
            if(!pad2->synch())
                break;

            boutons2 = pad2->getButtons();

            if(/* !SERVER && */disable) // Si le serveur a envoyer DISABLE, on ne peut qu'appuyer sur la touche BAS.
            {
                KB_press(&press_pad2[1], boutons2[config2[1]-1], keymap2[1]);
            }
            else
            {
            for(int i=0;i<4;i++)
                KB_press(&press_pad2[i], boutons2[config2[i]-1], keymap2[i]);
            }
        }


        Sleep(10);
    }while(1);
    // *******************************************************************

    disable=-1;
    pthread_join(thread1, NULL);
    pad1->close();
    pad2->close();
    free(pad1);
    free(pad2);

    #if SERVER
    pad3->close();
    free(pad3);
    #endif

    return 0;
}

#if SERVER
void* communication(void* data)
{
    WSADATA WSAData;
    WSAStartup(MAKEWORD(2,2), &WSAData); // Version 2

    SOCKET sock;
    SOCKADDR_IN sin;

    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_family      = AF_INET;
    sin.sin_port        =htons(4242);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if(bind(sock, (SOCKADDR *)&sin, sizeof(sin)) == SOCKET_ERROR)
    {
        cout << "\n\nImpossible de BIND." << endl;
        exit(EXIT_FAILURE);
    }

    if(listen(sock, 5) == SOCKET_ERROR)
    {
        cout << "\n\nImpossible de LISTEN." << endl;
        exit(EXIT_FAILURE);
    }

    SOCKET csock[10];
    SOCKADDR_IN csin[10];
    int sizeof_csin = sizeof(csin[0]);

    for(short i=0;i<nb_clients;i++)
    {
        cout << "Waiting for Client#" << (i+1) << "... ";
        csock[i]=accept(sock, (SOCKADDR *)&csin[i], &sizeof_csin);
        cout << "OK !" << endl;
    }


    cout << "All Clients connected." << endl;
    cout << endl << "*** READY TO PLAY ***" << endl;

    short val;
    while(disable != -1)
    {
        val = disable;
        for(short i=0;i<nb_clients;i++)
            send(csock[i], (char*)&val, sizeof(short), 0);
        Sleep(1000);
    }

    for(short i=0;i<nb_clients;i++)
        closesocket(csock[i]);

    closesocket(sock);

    WSACleanup();
    return NULL;
}

#else

void* communication(void* data)
{
    WSADATA WSAData;
    WSAStartup(MAKEWORD(2,2), &WSAData); // Version 2

    SOCKET sock;
    SOCKADDR_IN sin;
    char answer;
    int sizeof_answer = sizeof(short);

    ifstream fich_addr("server_addr.txt");
    string ip;

    if(fich_addr)
    {
        fich_addr >> ip;
        fich_addr.close();
    }
    else
    {
        ofstream fich_addr("server_addr.txt");
        fich_addr.close();

        cout << "\n\n Le fichier server_addr.txt est vide !" << endl;
        system("pause");
        exit(EXIT_FAILURE);
    }


    sin.sin_addr.s_addr = inet_addr(ip.c_str());
    sin.sin_family      = AF_INET;
    sin.sin_port        =htons(4242);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if(connect(sock, (SOCKADDR*)&sin, sizeof(sin)) == -1)
    {
        cout << "\n\nImpossible de se connecter au serveur." << endl;
        closesocket(sock);
        WSACleanup();
        system("pause");
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << endl << "-> Connected to the server." << endl;
        cout << endl << "*** READY TO PLAY ***" << endl;
    }


        while(disable != -1)
        {
            if(recv(sock, &answer, sizeof_answer, 0) == -1)
            {
                cout << "Disconnected from Server" << endl;
                break;
            }
            if(disable != -1)
            {
                if((short)answer == 1)
                    disable=1;
                else
                    disable=0;
            }
        }


    closesocket(sock);
    WSACleanup();

    return NULL;
}
#endif
