#pragma once


#include <mfidl.h> //maybe this will fix linker errors
#include <mfapi.h>

#include <Windows.h>
#include <string>

//these two are for message boxes for debugging
#include <iostream>
#include <winuser.h>

#include <mfobjects.h>

#include <assert.h>
#include <mfidl.h>
#include <mferror.h>

#include <Shlwapi.h>
#include <evr.h>

//extra includes idk
#include <ShObjIdl.h>
#include <strsafe.h>
#include <new>

const UINT WM_APP_PLAYER_EVENT = WM_APP + 1; //linker got angy when this wasn't const so now it's const

enum PlayerState
{
    Closed = 0,     // No session.
    Ready,          // Session was created, ready to open a file. 
    OpenPending,    // Session is opening a file.
    Started,        // Session is playing a file.
    Paused,         // Session is paused.
    Stopped,        // Session is stopped (ready to play). 
    Closing         // Application has closed the session, but is waiting for MESessionClosed.
};

template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

class CPlayer : public IMFAsyncCallback
{
public:
    static HRESULT CreateInstance(HWND hVideo, HWND hEvent, CPlayer **ppPlayer, int width, int height);

    //IUnknown methods
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    //IMFAsyncCallback methods
    STDMETHODIMP GetParameters(DWORD*, DWORD*) {
        return E_NOTIMPL;
    }
    STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult);

    //Playback
    HRESULT OpenURL(const WCHAR* sURL);
    HRESULT Play();
    HRESULT Pause();
    HRESULT Stop();
    HRESULT Shutdown();
    HRESULT HandleEvent(UINT_PTR uUnkPtr);
    PlayerState GetState() const { return m_state; }

    //Video functionality
    HRESULT Repaint();
    HRESULT ResizeVideo(WORD width, WORD height);
    BOOL HasVideo() const { return (m_pVideoDisplay != NULL); }


    int globalW, globalH;



protected:
    //Constructor is private, use CreateInstance to instantiate
    CPlayer(HWND hVideo, HWND hEvent);

    //Docs say the caller should call IUnknown::Release instead of using the destructor
    virtual ~CPlayer();

    HRESULT Initialize();
    HRESULT CreateSession();
    HRESULT CloseSession();
    HRESULT StartPlayback();

    //Media event handlers
    virtual HRESULT OnTopologyStatus(IMFMediaEvent* pEvent);
    virtual HRESULT OnPresentationEnded(IMFMediaEvent* pEvent);
    virtual HRESULT OnNewPresentation(IMFMediaEvent* pEvent);

    //Override to handle additional sessions
    virtual HRESULT OnSessionEvent(IMFMediaEvent*, MediaEventType){
        return S_OK;
    }

protected:
    long m_nRefCount; //Reference count
    
    IMFMediaSession* m_pSession;
    IMFMediaSource* m_pSource;
    IMFVideoDisplayControl* m_pVideoDisplay;

    HWND m_hwndVideo;
    HWND m_hwndEvent;
    PlayerState m_state;
    HANDLE m_hCloseEvent;

};