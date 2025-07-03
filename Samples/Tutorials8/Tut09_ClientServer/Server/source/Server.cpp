//----------------------------------------------------------------------------
// File: server.cpp
//
// Desc: This simple program builds upon the 5th tutorial, and is the server
//       code for the server in the client/server model
//
// Copyright (c) 2000-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define INITGUID
#define _WIN32_DCOM
#include <dplay8.h>
#include <stdio.h>
#include <wchar.h>

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
IDirectPlay8Server* g_pDPServer = NULL;
IDirectPlay8Address* g_pDeviceAddress = NULL;
IDirectPlay8Address* g_pHostAddress = NULL;

// This GUID allows DirectPlay to find other instances of the same game on
// the network.  So it must be unique for every game, and the same for
// every instance of that game.  // {1AD4CA3B-AC68-4d9b-9522-BE59CD485276}
GUID g_guidApp = {0x1ad4ca3b, 0xac68, 0x4d9b,
	{0x95, 0x22, 0xbe, 0x59, 0xcd, 0x48, 0x52, 0x76}};

//-----------------------------------------------------------------------------
// Miscellaneous helper functions
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p) \
	{ \
		if (p) { \
			delete (p); \
			(p) = NULL; \
		} \
	}
#define SAFE_DELETE_ARRAY(p) \
	{ \
		if (p) { \
			delete[](p); \
			(p) = NULL; \
		} \
	}
#define SAFE_RELEASE(p) \
	{ \
		if (p) { \
			(p)->Release(); \
			(p) = NULL; \
		} \
	}

#define USER_EXIT 1
#define USER_SEND 2

//-----------------------------------------------------------------------------
// Name: IsServiceProviderValid()
// Desc: Return TRUE if the service provider is valid
//-----------------------------------------------------------------------------
static BOOL IsServiceProviderValid(const GUID* /* pGuidSP */)
{
	DWORD dwItems = 0;
	DWORD dwSize = 0;

	HRESULT hr = g_pDPServer->EnumServiceProviders(
		&CLSID_DP8SP_TCPIP, NULL, NULL, &dwSize, &dwItems, 0);

	if (hr != DPNERR_BUFFERTOOSMALL) {
		printf("Failed Enumerating Service Providers:  0x%x\n", hr);
	} else {

		DPN_SERVICE_PROVIDER_INFO* pdnSPInfo =
			(DPN_SERVICE_PROVIDER_INFO*)new BYTE[dwSize];

		if (FAILED(hr = g_pDPServer->EnumServiceProviders(&CLSID_DP8SP_TCPIP,
					   NULL, pdnSPInfo, &dwSize, &dwItems, 0))) {
			printf("Failed Enumerating Service Providers:  0x%x\n", hr);
		} else {

			// There are no items returned so the requested SP is not available
			if (dwItems == 0) {
				hr = E_FAIL;
			}
		}
		SAFE_DELETE_ARRAY(pdnSPInfo);
	}

	if (SUCCEEDED(hr)) {
		return TRUE;
	}
	return FALSE;
}

//-----------------------------------------------------------------------------
// Name: DirectPlayMessageHandler
// Desc: Handler for DirectPlay messages.
//-----------------------------------------------------------------------------
static HRESULT WINAPI DirectPlayMessageHandler(
	PVOID /* pvUserContext */, DWORD dwMessageId, PVOID pMsgBuffer)
{
	HRESULT hr = S_OK;

	switch (dwMessageId) {
	case DPN_MSGID_RECEIVE: {
		PDPNMSG_RECEIVE pMsg = (PDPNMSG_RECEIVE)pMsgBuffer;

		printf("\nReceived Message:  %ls\n", (WCHAR*)pMsg->pReceiveData);
		break;
	}
	}
	return hr;
}

//-----------------------------------------------------------------------------
// Name: InitDirectPlay()
// Desc: Initialize DirectPlay
//-----------------------------------------------------------------------------
static HRESULT InitDirectPlay()
{
	HRESULT hr = S_OK;

	// Create the IDirectPlay8Server Object
	if (FAILED(hr = CoCreateInstance(CLSID_DirectPlay8Server, NULL,
				   CLSCTX_INPROC_SERVER, IID_IDirectPlay8Server,
				   (LPVOID*)&g_pDPServer))) {
		printf("Failed Creating the IDirectPlay8Peer Object:  0x%X\n", hr);
	} else {

		// Init DirectPlay
		if (FAILED(hr = g_pDPServer->Initialize(
					   NULL, DirectPlayMessageHandler, 0))) {
			printf("Failed Initializing DirectPlay:  0x%X\n", hr);
		} else {

			// Ensure that TCP/IP is a valid Service Provider
			if (FALSE == IsServiceProviderValid(&CLSID_DP8SP_TCPIP)) {
				hr = E_FAIL;
				printf("Failed validating CLSID_DP8SP_TCPIP");
			}
		}
	}
	return hr;
}

//-----------------------------------------------------------------------------
// Name: CreateDeviceAddress()
// Desc: Creates a device address
//-----------------------------------------------------------------------------
static HRESULT CreateDeviceAddress()
{
	HRESULT hr = S_OK;

	// Create our IDirectPlay8Address Device Address
	if (FAILED(hr = CoCreateInstance(CLSID_DirectPlay8Address, NULL,
				   CLSCTX_INPROC_SERVER, IID_IDirectPlay8Address,
				   (LPVOID*)&g_pDeviceAddress))) {
		printf("Failed Creating the IDirectPlay8Address Object:  0x%X\n", hr);
	} else {

		// Set the SP for our Device Address
		if (FAILED(hr = g_pDeviceAddress->SetSP(&CLSID_DP8SP_TCPIP))) {
			printf("Failed Setting the Service Provider:  0x%X\n", hr);
		}
	}
	return hr;
}

//-----------------------------------------------------------------------------
// Name: HostSession()
// Desc: Host a DirectPlay session
//-----------------------------------------------------------------------------
static HRESULT HostSession()
{
	HRESULT hr = S_OK;
	DPN_APPLICATION_DESC dpAppDesc;
	DPN_PLAYER_INFO dpPlayerInfo;
	WCHAR wszSession[128];
	WCHAR wszName[] = L"Server";

	ZeroMemory(&dpPlayerInfo, sizeof(DPN_PLAYER_INFO));
	dpPlayerInfo.dwSize = sizeof(DPN_PLAYER_INFO);
	dpPlayerInfo.dwInfoFlags = DPNINFO_NAME;
	dpPlayerInfo.pwszName = wszName;
	dpPlayerInfo.pvData = NULL;
	dpPlayerInfo.dwDataSize = NULL;
	dpPlayerInfo.dwPlayerFlags = 0;

	if (FAILED(hr = g_pDPServer->SetServerInfo(
				   &dpPlayerInfo, NULL, NULL, DPNSETSERVERINFO_SYNC))) {
		printf("Failed Hosting:  0x%X\n", hr);
	} else {

		// Prompt the user for the session name
		printf("\nPlease Enter a Session Name.\n");
		scanf("%ls", wszSession);

		// Now set up the Application Description
		ZeroMemory(&dpAppDesc, sizeof(DPN_APPLICATION_DESC));
		dpAppDesc.dwSize = sizeof(DPN_APPLICATION_DESC);
		dpAppDesc.dwFlags = DPNSESSION_CLIENT_SERVER;
		dpAppDesc.guidApplication = g_guidApp;
		dpAppDesc.pwszSessionName = wszSession;

		// We are now ready to host the app
		if (FAILED(hr = g_pDPServer->Host(&dpAppDesc, // AppDesc
					   &g_pDeviceAddress, 1,          // Device Address
					   NULL, NULL,                    // Reserved
					   NULL,                          // Player Context
					   0)))                           // dwFlags
		{
			printf("Failed Hosting:  0x%X\n", hr);
		} else {
			printf("Currently Hosting...\n");
		}
	}
	return hr;
}

//-----------------------------------------------------------------------------
// Name: SendDirectPlayMessage()
// Desc: Sends a DirectPlay message to all players
//-----------------------------------------------------------------------------
static HRESULT SendDirectPlayMessage()
{
	HRESULT hr = S_OK;
	DPN_BUFFER_DESC dpnBuffer;
	WCHAR wszData[256];

	// Get the data from the user
	printf("\nPlease Enter a String.\n");
	scanf("%ls", wszData);

	dpnBuffer.pBufferData = (BYTE*)wszData;
	dpnBuffer.dwBufferSize = 2 * (wcslen(wszData) + 1);

	if (FAILED(hr = g_pDPServer->SendTo(DPNID_ALL_PLAYERS_GROUP, // dpnid
				   &dpnBuffer,                                   // pBufferDesc
				   1,                                            // cBufferDesc
				   0,                                            // dwTimeOut
				   NULL,          // pvAsyncContext
				   NULL,          // pvAsyncHandle
				   DPNSEND_SYNC | // dwFlags
					   DPNSEND_NOLOOPBACK))) {
		printf("Failed Sending Data:  0x%x\n", hr);
	}
	return hr;
}

//-----------------------------------------------------------------------------
// Name: CleanupDirectPlay()
// Desc: Cleanup DirectPlay
//-----------------------------------------------------------------------------
static void CleanupDirectPlay()
{
	// Shutdown DirectPlay
	if (g_pDPServer) {
		g_pDPServer->Close(0);
	}

	SAFE_RELEASE(g_pDeviceAddress);
	SAFE_RELEASE(g_pHostAddress);
	SAFE_RELEASE(g_pDPServer);
}

//-----------------------------------------------------------------------------
// Name: main()
// Desc: Entry point for the application.
//-----------------------------------------------------------------------------
int main(int /* argc */, char** /* argv */)
{
	HRESULT hr;
	int iUserChoice;

	// Init COM so we can use CoCreateInstance
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// Init the DirectPlay system
	if (FAILED(hr = InitDirectPlay())) {
		printf("Failed Initializing DirectPlay:  0x%X\n", hr);
	} else {

		if (FAILED(hr = CreateDeviceAddress())) {
			printf("Failed CreatingDeviceAddress:  0x%X\n", hr);
		} else {

			if (FAILED(hr = HostSession())) {
				printf("Failed Hosting:  0x%X\n", hr);
			} else {

				// Present User with Choices
				do {
					printf("Please select one.\n1.  Exit\n2.  Send Data\n");
					scanf("%d", &iUserChoice);

					if (iUserChoice == USER_SEND) {
						if (FAILED(hr = SendDirectPlayMessage())) {
							printf("Failed To Send Data:  0x%X\n", hr);
							break;
						}
					}
				} while (iUserChoice != USER_EXIT);
			}
		}
	}
	CleanupDirectPlay();

	// ShutDown COM
	CoUninitialize();

	return 0;
}