#include "client.h"
#include "sock_defs.h"

#include <SDL3/SDL.h>
#include <string>
#include <iostream>

int main(int argc, char* argv[])
{
#if defined(_WIN32)
	WSAData wsaData;
	if (int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData); wsaResult != 0)
	{
		std::cout << "Failed to initialize Winsock: " << wsaResult << ", " << WSAGetLastError() << std::endl;
		return -1;
	}
#endif

	std::string ip, port;
	if (argc > 2)
	{
		ip = argv[1];
		port = argv[2];
	}
	else
	{
		ip = "127.0.0.1";
		port = "5555";
	}

	Client client("Client", ip, port);
	//client.requestClose();
	client.waitForClose();
#if defined(_WIN32)
	WSACleanup();
#endif
	return 0;
}

//SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
//{
//
//	if (!SDL_SetAppMetadata("IlkpChess", "1.0.0", "com.ilkpchess"))
//		return SDL_AppResult::SDL_APP_FAILURE;
//
//	SDL_Window* window;
//	SDL_Renderer* renderer;
//	if (!SDL_CreateWindowAndRenderer("IlkpChess", 800, 600, 0, &window, &renderer))
//		return SDL_AppResult::SDL_APP_FAILURE;
//
//}

//SDL_AppResult SDL_AppIterate(void* appstate)
//{
//	AppState* appState = (AppState*)appstate;
//	appState->input.onSdlIterate();
//
//	return SDL_AppResult::SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
//{
//	AppState* appState = (AppState*)appstate;
//	appState->input.onSdlEvent(event);
//
//	switch (event->type)
//	{
//	case SDL_EventType::SDL_EVENT_QUIT:
//	{
//		return SDL_APP_SUCCESS;
//	}
//	}
//	return SDL_AppResult::SDL_APP_CONTINUE;
//}
//
//void SDL_AppQuit(void* appstate, SDL_AppResult result)
//{
//	delete(appstate);
//}