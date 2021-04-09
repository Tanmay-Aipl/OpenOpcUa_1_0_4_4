//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  AddItemDlg.cpp
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************
// RtThread.h: interface for the CRtThread class.
//
//////////////////////////////////////////////////////////////////////
#include "VpiString.h"
#if !defined(AFX_ACQCTRLTHREAD_H__EB417B84_B4D0_11D1_AEFD_006097758E14__INCLUDED_)
#define AFX_ACQCTRLTHREAD_H__EB417B84_B4D0_11D1_AEFD_006097758E14__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


Vpi_Void MainProc(Vpi_Void* pParam);


typedef struct _THREAD_PARAM
{
	void	*pThread;
	void	*pParam;
} THREAD_PARAM;



typedef struct tagTHREADNAME_INFO
{
  Vpi_UInt16	uiType; // must be 0x1000
  Vpi_String	szName; // pointer to name (in user addr space)
  Vpi_Int16	iThreadID; // thread ID (-1=caller thread)
  Vpi_UInt16	uiFlags; // reserved for future use, must be zero
} THREADNAME_INFO;


/// <summary>
/// Class to handle Threads
/// </summary>
class   CRtThread  
{
public:
	CRtThread();
	virtual ~CRtThread();

	// Properties
protected:
	Vpi_Thread			m_hThread;
	//HANDLE					m_hThread;
	DWORD	    			m_dwThreadID;
	DWORD					m_dwExitThreadStatus;
	Vpi_String			m_szName;

	//int						m_nPriority;
	Vpi_Semaphore			m_hEventRunSem;
	BOOL					m_bWaitEnd;
	BOOL					m_bAutoDelete;
	BOOL					m_bRun;
	DWORD					m_dwTimeOut;
	THREAD_PARAM			m_xThreadParam;
	
	// Methods
public:

	inline	void			SetName(Vpi_String szName) { Vpi_String_CopyTo(&szName,&m_szName);}
	inline	Vpi_String	GetName(){return m_szName;}
	inline  DWORD			GetExitThreadStatus(){return m_dwExitThreadStatus;}
	inline  DWORD			GetThreadID(){return m_dwThreadID;}
	inline  void			SetThreadID(DWORD dwThreadID){m_dwThreadID = dwThreadID;}
	inline  Vpi_Thread	GetThread(){return m_hThread;}
	inline  void			SetAutoDelete(BOOL bAutoDelete=TRUE){m_bAutoDelete=bAutoDelete;}
	inline	BOOL			IsRunning(){return m_bRun;}
	inline	void			SetTimeOut(DWORD dwTimeOut){m_dwTimeOut = dwTimeOut;}
	inline	DWORD			GetTimeOut(){return m_dwTimeOut;}
	
	virtual	BOOL			Run(void *pParam = NULL);
			void			SetThreadName( Vpi_Int16 iThreadID, Vpi_String szThreadName);

	virtual BOOL			Quit();
	virtual DWORD			Main(void *pParam = NULL);

protected:
	virtual BOOL		ExitInstance();
};

#endif // !defined(AFX_ACQCTRLTHREAD_H__EB417B84_B4D0_11D1_AEFD_006097758E14__INCLUDED_)
