#include <iostream>
#include "snap7.h"
#include "s7.h"
#include <unistd.h>
#include <cstdint>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <ctime>

#define PORT 2000

using namespace std;

TS7Client* Client;
int Rack = 0;
int Slot = 1;
int DB_NUMBER = 6;
int timerOutput = 3000;
const char* address = "192.168.0.150";
uint8_t myDB6[10];

void plc_connect(){
    Client = new TS7Client;
    Client->ConnectTo(address, Rack,Slot);
}

void plc_disconnect(){
    Client->Disconnect();
    delete Client;
}

void leerdatoDB()
{
    Client->DBRead(DB_NUMBER, 0, 2, &myDB6);
    int tres = S7_GetIntAt(myDB6, 0);
    cout << "Alarma Anterior: " << tres << endl;
}

void escribirDB(char a)
{
    S7_SetIntAt(myDB6, 0, a);
    Client->DBWrite(DB_NUMBER, 0, 2, &myDB6);
    cout << "Alarma al PLC: " << a << endl;
    cout << endl;
}

char encuentra_mensaje(char *b)
{
    std::string texto = b;
    size_t inicio = texto.find("<UserString>");
    size_t fin = texto.find("</UserString>");
    const char* m;

    if (inicio != std::string::npos && fin != std::string::npos)
        m = texto.substr(inicio, fin - inicio).c_str();
    else
        std::cout << "No se encontró la etiqueta <UserString>" << std::endl;

    return m[12];
}

char recibeTCP()
{
    int serverSocket;
    int clientSocket;
    char men;
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 5);
    std::cout << "Esperando Alarmas en el puerto " << PORT << std::endl;
    while(1)
    {
        clientSocket = accept(serverSocket, nullptr, nullptr);
        char buffer[1024] = {0};
        recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        men = encuentra_mensaje(buffer);

        std::cout << "Alarma Symphony: " << men << " ";
        std::time_t tiempoActual = std::time(nullptr);
        std::tm* tiempoLocal = std::localtime(&tiempoActual);
        std::cout << " : " << std::asctime(tiempoLocal);

        plc_connect();
        escribirDB(men);
        plc_disconnect();
        close(clientSocket);
    }
    close(serverSocket);

    return men;
}
