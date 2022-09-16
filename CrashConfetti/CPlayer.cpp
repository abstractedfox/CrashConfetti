#include "CPlayer.h"

HRESULT CPlayer::CreateInstance(
	HWND hVideo, //Video window
	HWND hEvent, //Window to receive notifications
	CPlayer** ppPlayer) //Ptr to the CPlayer object
{
	if (ppPlayer == NULL) return E_POINTER;

	CPlayer* pPlayer = new (std::nothrow) CPlayer(hVideo, hEvent);
	if (pPlayer == NULL) return E_OUTOFMEMORY;

	HRESULT hr = pPlayer->Initialize();

	if (SUCCEEDED(hr)) {
		*ppPlayer = pPlayer;
	}

	else pPlayer->Release();
	return hr;
}

HRESULT CPlayer::Initialize() {
	//Start media foundation platform
	HRESULT hr = MFStartup(MF_VERSION);
	
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

	IMFTopology* pTopology = NULL;
	IMFPresentationDescriptor* pSourcePD = NULL;

	HRESULT hr = CreateSession();
	if (FAILED(hr)) goto done;

	hr = CreateMediaSource(sURL, &m_pSource);
	if (FAILED(hr)) goto done;

	hr = m_pSource -> CreatePresentationDescriptor(&pSourcePD);
	if (FAILED(hr)) goto done;

	hr = CreatePlaybackTopology(m_pSource, pSourcePD, m_hwndVideo, &pTopology);
	if (FAILED(hr)) goto done;

	hr = m_pSession->SetTopology(0, pTopology);
	if (FAILED(hr)) goto done;

	m_state = OpenPending;

	// If SetTopology succeeds, the media session will queue an MESessionTopologySet event.

done:
	if (FAILED(hr)) m_state = Closed;

	SafeRelease(*pSourcePD);
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