/*
CPlayer.cpp, an implementation of the Media Foundation library that follows 
Microsoft's implementation example.
Copyright Chris/abstractedfox 2022-2023 where applicable.

This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.

In addition to the standard GPL v3.0, this program includes an additional condition:
Redistribution or modification of this code may not strike credit of the original author, Chris/abstractedfox

Contact: chriswhoprograms@gmail.com
*/

#pragma once
#include "CPlayer.h"
#pragma comment(lib, "shlwapi")

//Forward declarations
HRESULT CreatePlaybackTopology(
	IMFMediaSource* pSource, //the media
	IMFPresentationDescriptor* pPD, //presentation descriptor
	HWND hVideoWnd,
	IMFTopology** ppTopology);
HRESULT CreateMediaSource(PCWSTR sURL, IMFMediaSource** ppSource);
HRESULT AddBranchToPartialTopology(
	IMFTopology* pTopology,
	IMFMediaSource* pSource,
	IMFPresentationDescriptor* pPD,
	DWORD iStream,
	HWND hVideoWnd);
HRESULT CreateMediaSinkActivate(
	IMFStreamDescriptor* pSourceSD,
	HWND hVideoWindow, //handle to the video clipping window
	IMFActivate** ppActivate
);
HRESULT AddSourceNode(
	IMFTopology* pTopology,
	IMFMediaSource* pSource,
	IMFPresentationDescriptor* pPD,
	IMFStreamDescriptor* pSD,
	IMFTopologyNode** ppNode
); 
HRESULT AddOutputNode(
	IMFTopology* pTopology,
	IMFActivate* pActivate,
	DWORD dwId,
	IMFTopologyNode** ppNode
);

void msgBox(std::wstring output, std::wstring title);

const bool debugboxes = true;


template <class Q>
HRESULT GetEventObject(IMFMediaEvent* pEvent, Q** ppObject) {
	*ppObject = NULL;

	PROPVARIANT var;
	HRESULT hr = pEvent->GetValue(&var);
	if (SUCCEEDED(hr)) {
		if (var.vt == VT_UNKNOWN) {
			hr = var.punkVal->QueryInterface(ppObject);
		}
		else {
			hr = MF_E_INVALIDTYPE;
		}
		PropVariantClear(&var);
	}
	return hr;
}


HRESULT CPlayer::CreateInstance(
	HWND hVideo, //Video window
	HWND hEvent, //Window to receive notifications
	CPlayer** ppPlayer, //Ptr to the CPlayer object
	int width, int height) 
{

	if (ppPlayer == NULL) return E_POINTER;

	CPlayer* pPlayer = new (std::nothrow) CPlayer(hVideo, hEvent);
	if (pPlayer == NULL) return E_OUTOFMEMORY;

	HRESULT hr = pPlayer->Initialize();

	if (SUCCEEDED(hr)) {
		pPlayer->globalW = width;
		pPlayer->globalH = height;
		*ppPlayer = pPlayer;
	}

	else pPlayer->Release();
	return hr;
}

HRESULT CPlayer::Initialize() {
	//Start media foundation platform
	HRESULT hr = MFStartup(MF_VERSION);
	if (SUCCEEDED(hr)) {
		m_hCloseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (m_hCloseEvent == NULL) hr = HRESULT_FROM_WIN32(GetLastError());
	}
	return hr;
}

CPlayer::CPlayer(HWND hVideo, HWND hEvent) :
	m_pSession(NULL),
	m_pSource(NULL),
	m_pVideoDisplay(NULL),
	m_hwndVideo(hVideo),
	m_hwndEvent(hEvent),
	m_state(Closed),
	m_hCloseEvent(NULL),
	m_nRefCount(1)
{
}

CPlayer::~CPlayer() {
	assert(m_pSession == NULL);
	Shutdown();
}

//IUnknown methods
HRESULT CPlayer::QueryInterface(REFIID riid, void** ppv)
{
	static const QITAB qit[] =
	{
		QITABENT(CPlayer, IMFAsyncCallback),
		{ 0 }
	};
	return QISearch(this, qit, riid, ppv);
}

ULONG CPlayer::AddRef()
{
	return InterlockedIncrement(&m_nRefCount);
}

ULONG CPlayer::Release()
{
	ULONG uCount = InterlockedDecrement(&m_nRefCount);
	if (uCount == 0)
	{
		delete this;
	}
	return uCount;
}

HRESULT CPlayer::OpenURL(const WCHAR* sURL) {
	// 1. Create a new media session.
	// 2. Create the media source.
	// 3. Create the topology.
	// 4. Queue the topology [asynchronous]
	// 5. Start playback [asynchronous - does not happen in this method.]
	const bool debug = false;
	const std::wstring debugName = L"CPlayer::OpenURL";

	IMFTopology* pTopology = NULL;
	IMFPresentationDescriptor* pSourcePD = NULL;

	HRESULT hr = CreateSession();
	if (FAILED(hr)) {
		if (debug) MessageBox(NULL, L"CreateSession() failed", debugName.c_str(), MB_OK);
		goto done;
	}

	hr = CreateMediaSource(sURL, &m_pSource);
	
	if (FAILED(hr)) {
		if (debug) MessageBox(NULL, L"CreateMediaSource failed", debugName.c_str(), MB_OK);
	}

	hr = m_pSource -> CreatePresentationDescriptor(&pSourcePD);
	if (FAILED(hr)) {
		if (debug) MessageBox(NULL, L"CreatePresentationDescriptor() failed", debugName.c_str(), MB_OK);
		goto done;
	}

	hr = CreatePlaybackTopology(m_pSource, pSourcePD, m_hwndVideo, &pTopology);
	if (FAILED(hr)) {
		if (debug) MessageBox(NULL, L"CreatePlaybackTopology() failed", debugName.c_str(), MB_OK);
		goto done;
	}

	hr = m_pSession->SetTopology(0, pTopology);
	if (FAILED(hr)) {
		if (debug) MessageBox(NULL, L"SetTopology() failed", debugName.c_str(), MB_OK);
		goto done;
	}

	if (debug) MessageBox(NULL, L"Nothing failed!", debugName.c_str(), MB_OK);

	m_state = OpenPending;


	// If SetTopology succeeds, the media session will queue an MESessionTopologySet event.

done:
	if (FAILED(hr)) m_state = Closed;
	else {
		if (debug) MessageBox(NULL, L"No problems here!", debugName.c_str(), MB_OK);
	}
	SafeRelease(&pSourcePD);
	SafeRelease(&pTopology);
	return hr;
}



HRESULT CreateMediaSource(PCWSTR sURL, IMFMediaSource** ppSource) {
	MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;

	IMFSourceResolver* pSourceResolver = NULL;
	IUnknown* pSource = NULL;

	HRESULT hr = MFCreateSourceResolver(&pSourceResolver);
	if (FAILED(hr)) goto done;

	//The docs say that the synchronous method isn't the most ideal way to do this, but I think this will work fine for confetti

	hr = pSourceResolver->CreateObjectFromURL(
		sURL,
		MF_RESOLUTION_MEDIASOURCE, //docs just call this "create a source object"
		NULL,
		&ObjectType, //Receive the created object type
		&pSource //Receives a pointer to media source
	);
	if (FAILED(hr)) goto done;

	hr = pSource->QueryInterface(IID_PPV_ARGS(ppSource));

done:
	SafeRelease(&pSourceResolver);
	SafeRelease(&pSource);
	return hr;
}

HRESULT CreatePlaybackTopology(
	IMFMediaSource* pSource, //the media
	IMFPresentationDescriptor* pPD, //presentation descriptor
	HWND hVideoWnd,
	IMFTopology** ppTopology) //Receives a ptr to the topology
{
	IMFTopology* pTopology = NULL;
	DWORD cSourceStreams = 0;

	//Create topology
	HRESULT hr = MFCreateTopology(&pTopology);
	if (FAILED(hr)) goto done;

	//get number of streams in the source
	hr = pPD->GetStreamDescriptorCount(&cSourceStreams);
	if (FAILED(hr)) goto done;

	for (DWORD i = 0; i < cSourceStreams; i++) {
		hr = AddBranchToPartialTopology(pTopology, pSource, pPD, i, hVideoWnd);
		if (FAILED(hr)) goto done;
	}

	*ppTopology = pTopology;
	(*ppTopology)->AddRef();

done:
	SafeRelease(&pTopology);
	return hr;
}

//The topology seems to be the part where the stream itself connects to an output node for rendering
HRESULT AddBranchToPartialTopology(
	IMFTopology* pTopology,
	IMFMediaSource* pSource,
	IMFPresentationDescriptor* pPD,
	DWORD iStream,
	HWND hVideoWnd)
{
	IMFStreamDescriptor* pSD = NULL;
	IMFActivate* pSinkActivate = NULL;
	IMFTopologyNode* pSourceNode = NULL;
	IMFTopologyNode* pOutputNode = NULL;

	BOOL fSelected = FALSE;

	HRESULT hr = pPD->GetStreamDescriptorByIndex(iStream, &fSelected, &pSD);
	if (FAILED(hr)) goto done;

	if (fSelected) {
		hr = CreateMediaSinkActivate(pSD, hVideoWnd, &pSinkActivate);
		if (FAILED(hr)) goto done;

		hr = AddSourceNode(pTopology, pSource, pPD, pSD, &pSourceNode);
		if (FAILED(hr)) goto done;

		hr = AddOutputNode(pTopology, pSinkActivate, 0, &pOutputNode);
		if (FAILED(hr)) goto done;

		hr = pSourceNode->ConnectOutput(0, pOutputNode, 0);
	}

done:
	SafeRelease(&pSD);
	SafeRelease(&pSinkActivate);
	SafeRelease(&pSourceNode);
	SafeRelease(&pOutputNode);
	return hr;
}

HRESULT CreateMediaSinkActivate(
	IMFStreamDescriptor* pSourceSD,
	HWND hVideoWindow, //handle to the video clipping window
	IMFActivate** ppActivate
)
{
	IMFMediaTypeHandler* pHandler = NULL;
	IMFActivate* pActivate = NULL;

	HRESULT hr = pSourceSD->GetMediaTypeHandler(&pHandler);
	if (FAILED(hr)) goto done;

	GUID guidMajorType;
	hr = pHandler->GetMajorType(&guidMajorType);
	if (FAILED(hr)) goto done;

	if (MFMediaType_Audio == guidMajorType) hr = MFCreateAudioRendererActivate(&pActivate);
	else if (MFMediaType_Video == guidMajorType) hr = MFCreateVideoRendererActivate(hVideoWindow, &pActivate);
	else hr = E_FAIL;
	if (FAILED(hr)) goto done;

	*ppActivate = pActivate;
	(*ppActivate)->AddRef();

done:
	SafeRelease(&pHandler);
	SafeRelease(&pActivate);
	return hr;
}

HRESULT CPlayer::CreateSession() {
	HRESULT hr = CloseSession();
	if (FAILED(hr)) goto done;

	assert(m_state == Closed);

	hr = MFCreateMediaSession(NULL, &m_pSession);
	if (FAILED(hr)) goto done;

	hr = m_pSession->BeginGetEvent((IMFAsyncCallback*)this, NULL);
	if (FAILED(hr)) goto done;

	m_state = Ready;

done:
	return hr;
}


HRESULT CPlayer::Invoke(IMFAsyncResult* pResult) {
	MediaEventType meType = MEUnknown;

	IMFMediaEvent* pEvent = NULL;
	HRESULT hr = m_pSession->EndGetEvent(pResult, &pEvent);
	if (FAILED(hr)) goto done;

	hr = pEvent->GetType(&meType);
	if (FAILED(hr)) goto done;

	if (meType == MESessionClosed) SetEvent(m_hCloseEvent);
	else {
		hr = m_pSession->BeginGetEvent(this, NULL);
		if (FAILED(hr)) goto done;
	}
	if (m_state != Closing) {
		pEvent->AddRef();

		PostMessage(m_hwndEvent, WM_APP_PLAYER_EVENT, (WPARAM)pEvent, (LPARAM)meType);
	}
done:
	SafeRelease(&pEvent);
	return S_OK;
}

HRESULT CPlayer::HandleEvent(UINT_PTR pEventPtr) {
	const bool debug = false;
	std::wstring debugString = L"";

	HRESULT hrStatus = S_OK;
	MediaEventType meType = MEUnknown;
	
	if (debug) {
		debugString += L"pEventPtr: ";
		debugString += std::to_wstring(pEventPtr);
		
		debugString += L'\n';
		MessageBox(NULL, debugString.c_str(), L"CPlayer::HandleEvent", MB_OK);
	}

	IMFMediaEvent* pEvent = (IMFMediaEvent*)pEventPtr;

	if (pEvent == NULL) {
		if (debug) MessageBox(NULL, L"pEvent == null!!!", L"CPlayer::HandleEvent", MB_OK);
		return E_POINTER;
	}

	HRESULT hr = pEvent->GetType(&meType);
	if (FAILED(hr)) {
		if (debug) MessageBox(NULL, L"pEvent->GetType(&meType) badness", L"CPlayer::HandleEvent", MB_OK);
		goto done;
	}

	hr = pEvent->GetStatus(&hrStatus);
	if (SUCCEEDED(hr) && FAILED(hrStatus)) hr = hrStatus;
	if (FAILED(hr)) {
		std::wstring error = L"Conditional FAILED(hr) met. HRESULT hr = " + std::to_wstring(hr) + L" hrStatus = " + std::to_wstring(hrStatus) 
			+ L" Media Type = " + std::to_wstring(meType);
		
		if (debug) MessageBox(NULL, error.c_str(), L"CPlayer::HandleEvent", MB_OK);
		goto done;
	}

	switch (meType) {
	case MESessionTopologyStatus:
		if (debug) MessageBox(NULL, L"case MESessionTopologyStatus reached", L"CPlayer::HandleEvent", MB_OK);
		hr = OnTopologyStatus(pEvent);
		break;

	case MEEndOfPresentation:
		if (debug) MessageBox(NULL, L"case MEEndOfPresentation reached", L"CPlayer::HandleEvent", MB_OK);
		hr = OnPresentationEnded(pEvent);
		break;

	case MENewPresentation:
		if (debug) MessageBox(NULL, L"case MENewPresentation reached", L"CPlayer::HandleEvent", MB_OK);
		hr = OnNewPresentation(pEvent);
		break;

	default:
		if (debug) MessageBox(NULL, L"case default reached", L"CPlayer::HandleEvent", MB_OK);
		hr = OnSessionEvent(pEvent, meType);
		break;
	}

done:
	SafeRelease(&pEvent);
	return hr;

}

//Fails if the media doesn't have a video stream
HRESULT CPlayer::OnTopologyStatus(IMFMediaEvent* pEvent) {
	UINT32 status;

	HRESULT hr = pEvent->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, &status);
	if (SUCCEEDED(hr) && (status == MF_TOPOSTATUS_READY)) {
		SafeRelease(&m_pVideoDisplay);

		//Fails if the media file doesn't have a video stream
		(void)MFGetService(m_pSession, MR_VIDEO_RENDER_SERVICE, IID_PPV_ARGS(&m_pVideoDisplay));

		hr = StartPlayback();
	}

	return hr;
}

HRESULT CPlayer::OnPresentationEnded(IMFMediaEvent* pEvent) {
	m_state = Stopped;
	return S_OK;
}

HRESULT CPlayer::OnNewPresentation(IMFMediaEvent* pEvent) {
	IMFPresentationDescriptor* pPD = NULL;
	IMFTopology* pTopology = NULL;

	HRESULT hr = GetEventObject(pEvent, &pPD);
	if (FAILED(hr)) goto done;

	hr = CreatePlaybackTopology(m_pSource, pPD, m_hwndVideo, &pTopology);
	if (FAILED(hr)) goto done;

	hr = m_pSession->SetTopology(0, pTopology);
	if (FAILED(hr)) goto done;

	m_state = OpenPending;

done:
	SafeRelease(&pTopology);
	SafeRelease(&pPD);
	return S_OK;
}

HRESULT CPlayer::StartPlayback() {
	const bool debug = false;
	assert(m_pSession != NULL);

	if (debug) {
		MessageBox(NULL, L"StartPlayback() reached", L"CPlayer::StartPlayback()", MB_OK);
	}

	PROPVARIANT varStart;
	PropVariantInit(&varStart);

	HRESULT hr = m_pSession->Start(&GUID_NULL, &varStart);
	if (SUCCEEDED(hr)) {
		if (debug) {
			MessageBox(NULL, L"StartPlayback():m_psession->Start was successful", L"CPlayer::StartPlayback()", MB_OK);
		}
		m_state = Started;
	}
	//Docs point out that start is an asyncrhonous operation but an MESessionStarted event with an error code would be sent if it didn't work
	else {
		if (debug) {
			MessageBox(NULL, L"StartPlayback():m_psession->Start was unsuccessful", L"CPlayer::StartPlayback()", MB_OK);
		}
	}


	PropVariantClear(&varStart);

	this->ResizeVideo(globalW, globalH);

	return hr;
}

HRESULT CPlayer::Play() {
	if (m_state != Paused && m_state != Stopped) return MF_E_INVALIDREQUEST;
	if (m_pSession == NULL || m_pSource == NULL) return E_UNEXPECTED;
	return StartPlayback();
}

//We're going to skip writing out HRESULT CPlayer::Pause(), it shouldn't be necessary for what we're doing

HRESULT CPlayer::Pause() {
	return 0;
}

HRESULT CPlayer::Stop() {
	if (m_state != Started && m_state != Paused) return MF_E_INVALIDREQUEST;
	if (m_pSession == NULL) return E_UNEXPECTED;

	HRESULT hr = m_pSession->Stop();
	if (SUCCEEDED(hr)) m_state = Stopped;

	return hr;
}

//It points out that the 'enhanced video renderer' draws the video in the window on a separate thread.
//This is used to notify the EVR if a WM_PAINT message is sent while stopped or paused
//There should also be a handler for the WM_PAINT message in the main cpp that calls this (see step 6 'control playback')
HRESULT CPlayer::Repaint() {


	if (m_pVideoDisplay) return m_pVideoDisplay->RepaintVideo();
	else return S_OK;
}

HRESULT CPlayer::ResizeVideo(WORD width, WORD height) {
	const bool debug = false;
	if (m_pVideoDisplay) {
		if (debug) MessageBox(NULL, L"Sizey", L"CPlayer::ResizeVideo()", MB_OK);
		RECT rcDest = { 0, 0, width, height };

		//SetAspectRatioMode seems to undo our transparency via SetLayeredWindowAttributes, so we won't use it for now
		//m_pVideoDisplay->SetAspectRatioMode(MFVideoARMode_NonLinearStretch);
		m_pVideoDisplay -> SetAspectRatioMode(MFVideoARMode_None);
		return m_pVideoDisplay->SetVideoPosition(NULL, &rcDest);
	}
	else {
		if (debug) MessageBox(NULL, L"m_pVideoDisplay == false", L"CPlayer::ResizeVideo()", MB_OK);
		return S_OK;
	}
}

HRESULT CPlayer::CloseSession() {
	HRESULT hr = S_OK;

	SafeRelease(&m_pVideoDisplay);

	if (m_pSession) {
		DWORD dwWaitResult = 0;

		m_state = Closing;

		hr = m_pSession->Close();

		if (SUCCEEDED(hr)) {
			dwWaitResult = WaitForSingleObject(m_hCloseEvent, 5000);
			if (dwWaitResult == WAIT_TIMEOUT) assert(FALSE);
		}
	}

	if (SUCCEEDED(hr)) {
		if (m_pSource) (void)m_pSource->Shutdown();
		if (m_pSession) (void)m_pSession->Shutdown();
	}

	SafeRelease(&m_pSource);
	SafeRelease(&m_pSession);
	m_state = Closed;
	return hr;
}

HRESULT CPlayer::Shutdown() {
	HRESULT hr = CloseSession();

	MFShutdown();

	if (m_hCloseEvent) {
		CloseHandle(m_hCloseEvent);
		m_hCloseEvent = NULL;
	}
	return hr;
}

HRESULT AddSourceNode(
	IMFTopology* pTopology,
	IMFMediaSource* pSource,
	IMFPresentationDescriptor* pPD,
	IMFStreamDescriptor* pSD,
	IMFTopologyNode** ppNode
) {
	IMFTopologyNode* pNode = NULL;

	//Create the node
	HRESULT hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pNode);
	if (FAILED(hr)) goto done;

	hr = pNode->SetUnknown(MF_TOPONODE_SOURCE, pSource);
	if (FAILED(hr)) goto done;

	hr = pNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pPD);
	if (FAILED(hr)) goto done;

	hr = pNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSD);
	if (FAILED(hr)) goto done;

	hr = pTopology->AddNode(pNode);
	if (FAILED(hr)) goto done;
	
	*ppNode = pNode;
	(*ppNode)->AddRef();

done:
	SafeRelease(&pNode);
	return hr;
}

HRESULT AddOutputNode(
	IMFTopology* pTopology,
	IMFActivate* pActivate,
	DWORD dwId,
	IMFTopologyNode** ppNode
) {
	IMFTopologyNode* pNode = NULL;

	HRESULT hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode);
	if (FAILED(hr)) goto done;

	hr = pNode->SetObject(pActivate);
	if (FAILED(hr)) goto done;

	hr = pNode->SetUINT32(MF_TOPONODE_STREAMID, dwId);
	if (FAILED(hr)) goto done;

	hr = pNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
	if (FAILED(hr)) goto done;

	hr = pTopology->AddNode(pNode);
	if (FAILED(hr)) goto done;

	*ppNode = pNode;
	(*ppNode)->AddRef();

done:
	SafeRelease(&pNode);
	return hr;
}

void msgBox(std::wstring output, std::wstring title) {
	MessageBox(NULL, output.c_str(), title.c_str(), MB_OK);
}