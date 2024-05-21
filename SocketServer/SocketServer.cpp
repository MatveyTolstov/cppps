#define WIN32_LEAN_AND_MEAN

// Включаем заголовочный файл Windows API.
#include <Windows.h>

// Включаем заголовочный файл для операций ввода/вывода в консоль.
#include <iostream>

// Включаем заголовочный файл WinSock2 для программирования с использованием сокетов.
#include <WinSock2.h>

// Включаем заголовочный файл WS2tcpip для дополнительных определений TCP/IP.
#include <WS2tcpip.h>

// Используем стандартное пространство имен, чтобы избежать префиксирования функций стандартной библиотеки "std::".
using namespace std;

int main() {
    setlocale(LC_ALL, "RUS");
    // Структура WSADATA для хранения деталей реализации Windows Sockets.
    WSADATA wsaData;

    // Структура hints для предоставления параметров функции getaddrinfo.
    ADDRINFO hints;

    // Указатель на связанный список структур addrinfo, который содержит результаты getaddrinfo.
    ADDRINFO* addrResult;

    // Сокет для прослушивания входящих подключений.
    SOCKET ListenSocket = INVALID_SOCKET;

    // Сокет для установления соединения с клиентом.
    SOCKET ConnectSocket = INVALID_SOCKET;

    // Буфер для хранения данных, полученных от клиента.
    char recvBuffer[512];

    // Константная строка для отправки клиенту.
    const char* sendBuffer = "Hello from server";

    // Инициализация использования DLL Winsock (версия 2.2).
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        // Если WSAStartup не удалась, выводим сообщение об ошибке и выходим.
        cout << "WSAStartup не удалась с результатом: " << result << endl;
        return 1;
    }

    // Функция ZeroMemory инициализирует структуру hints нулями.
    ZeroMemory(&hints, sizeof(hints));

    // Устанавливаем семейство адресов на IPv4.
    hints.ai_family = AF_INET;

    // Устанавливаем тип сокета на потоковый, что означает TCP сокет.
    hints.ai_socktype = SOCK_STREAM;

    // Устанавливаем протокол на TCP.
    hints.ai_protocol = IPPROTO_TCP;

    // Устанавливаем флаги в пассивный режим, указывая, что сервер будет привязываться к адресу.
    hints.ai_flags = AI_PASSIVE;

    // Разрешаем адрес и порт сервера.
    result = getaddrinfo(NULL, "666", &hints, &addrResult);
    if (result != 0) {
        // Если getaddrinfo не удалась, выводим сообщение об ошибке, освобождаем память и выходим.
        cout << "getaddrinfo не удалась с ошибкой: " << result << endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Создаем сокет для сервера, чтобы прослушивать подключения клиентов.
    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        // Если создание сокета не удалось, выводим сообщение об ошибке, освобождаем память и выходим.
        cout << "Создание сокета не удалось" << endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Привязываем сокет к адресу и порту.
    result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        // Если привязка не удалась, выводим сообщение об ошибке, закрываем сокет, освобождаем память и выходим.
        cout << "Привязка не удалась, ошибка: " << result << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Переводим сокет в режим прослушивания.
    result = listen(ListenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        // Если перевод в режим прослушивания не удался, выводим сообщение об ошибке, закрываем сокет, освобождаем память и выходим.
        cout << "Прослушивание не удалось, ошибка: " << result << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

        // Принимаем входящее соединение от клиента.
        ConnectSocket = accept(ListenSocket, NULL, NULL);
    if (ConnectSocket == INVALID_SOCKET) {
        // Если прием соединения не удался, выводим сообщение об ошибке, закрываем сокет, освобождаем память и выходим.
        cout << "Прием соединения не удался, ошибка: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Закрываем сокет для прослушивания, так как он больше не нужен.
    closesocket(ListenSocket);

    // Цикл для получения и отправки данных.
    do {
        // Обнуляем буфер для получения данных.
        ZeroMemory(recvBuffer, 512);

        // Получаем данные от клиента.
        result = recv(ConnectSocket, recvBuffer, 512, 0);
        if (result > 0) {
            // Если данные успешно получены, выводим количество полученных байт и сами данные.
            cout << "Получено " << result << " байт" << endl;
            cout << "Полученные данные: " << recvBuffer << endl;

            // Отправляем данные обратно клиенту.
            result = send(ConnectSocket, sendBuffer, (int)strlen(sendBuffer), 0);
            if (result == SOCKET_ERROR) {
                // Если отправка не удалась, выводим сообщение об ошибке, закрываем сокет, освобождаем память и выходим.
                cout << "Отправка не удалась, ошибка: " << result << endl;
                closesocket(ConnectSocket);
                freeaddrinfo(addrResult);
                WSACleanup();
                return 1;
            }
        }
        else if (result == 0) {
            // Если соединение закрыто клиентом, выводим сообщение.
            cout << "Закрытие соединения" << endl;
        }
        else {
            // Если получение данных не удалось, выводим сообщение об ошибке, закрываем сокет, освобождаем память и выходим.
            cout << "Получение не удалось, ошибка: " << WSAGetLastError() << endl;
            closesocket(ConnectSocket);
            freeaddrinfo(addrResult);
            WSACleanup();
            return 1;
        }
    } while (result > 0);

    // Завершаем соединение.
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        // Если завершение не удалось, выводим сообщение об ошибке, закрываем сокет, освобождаем память и выходим.
        cout << "Завершение не удалось, ошибка: " << result << endl;
        closesocket(ConnectSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Закрываем сокет для подключения.
    closesocket(ConnectSocket);

    // Освобождаем память, выделенную для адресной информации.
    freeaddrinfo(addrResult);

    // Завершаем использование Winsock DLL.
    WSACleanup();

    // Возвращаем 0 для успешного завершения программы.
    return 0;
}