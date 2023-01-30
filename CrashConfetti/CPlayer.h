/*
CPlayer.h, copyright Chris/abstractedfox 2022. Implementation of Microsoft's Media Foundation API

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