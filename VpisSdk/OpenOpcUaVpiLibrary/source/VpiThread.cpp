//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiThread.cpp
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************
/*
 *
 * Permission is hereby granted, for a commerciale use of this 
 * software and associated documentation files (the "Software")
 *
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 * ======================================================================*/
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "stdio.h"
#include "assert.h"
#include "VpiThread.h"

using namespace VpiBuiltinType;
Vpi_Void MainProc(Vpi_Void* pParam)
{
	THREAD_PARAM *pThreadParam = (THREAD_PARAM *)pParam;
	CRtThread *pThread = (CRtThread*) pThreadParam->pThread;
	void *pLocParam = pThreadParam->pParam;

	pThread->Main(pLocParam);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRtThread::CRtThread()
{
	m_hThread		= Vpi_Null;
	m_dwThreadID	= 0;
	Vpi_String_Initialize(&m_szName);
	m_dwExitThreadStatus = 0;
	m_bWaitEnd		= TRUE;
	m_bAutoDelete   = TRUE;
	m_bRun			= FALSE;

	m_dwTimeOut		= INFINITE;
	Vpi_Semaphore_Create(&m_hEventRunSem,0,0x100);
}

CRtThread::~CRtThread()
{

}


void CRtThread::SetThreadName( Vpi_Int16 iThreadID, Vpi_String szThreadName)
{
  THREADNAME_INFO info;
  {
	info.uiType = 0x1000;
	Vpi_String_Initialize(&info.szName);
	Vpi_String_StrnCpy(&info.szName,&szThreadName,Vpi_String_StrLen(&szThreadName));
	info.iThreadID = iThreadID;
	info.uiFlags = 0;
  }
}

//BOOL CRtThread::SetPriority(int nPriority)
//{
//	BOOL bStatus = TRUE;
//
//	if (IsRunning())
//	{
//#ifdef _WIN32_WCE
//		bStatus =  CeSetThreadPriority(GetThreadHandle(),nPriority);
//#else
//		bStatus =  SetThreadPriority(GetThreadHandle(),nPriority);
//#endif
//	}
//	
//	if (bStatus)
//	{
//		m_nPriority = nPriority;
//	}
//
//	return bStatus;
//}




BOOL CRtThread::Run(void *pParam)
{
	Vpi_Boolean bResult=Vpi_False;
	// Init
	//THREAD_PARAM *pThreadParam;
	m_xThreadParam.pThread = this;
	m_xThreadParam.pParam  = pParam;

	// Create Thread Main
	if (Vpi_Thread_Create(&m_hThread,MainProc,(Vpi_Void*)&m_xThreadParam)==Vpi_Good)
	{
		Vpi_Thread_Start(m_hThread);
		m_bRun = Vpi_True;
		bResult = Vpi_True;
	}
	else
		bResult=Vpi_False;
	//SetPriority(m_nPriority);

	//printf("Thread Started : %s : %x\n",m_szName, m_dwThreadID);

	return bResult;		
}



DWORD CRtThread::Main(void *pParam)
{
	(void*)pParam;
	// Wait End of Application..
	Vpi_Semaphore_TimedWait(m_hEventRunSem,m_dwTimeOut);

	return 0;
}



BOOL CRtThread::Quit()
{
	Vpi_Semaphore_Post(m_hEventRunSem,1);
  

	if (m_bWaitEnd)
	{
		Vpi_Semaphore_TimedWait(m_hEventRunSem,VPI_INFINITE);
		//dwThreadStatus=WaitForSingleObject(m_hThread, INFINITE);
	}

	//printf("Thread Finished : %s : %x\n",m_szName, m_dwThreadID);

	if (!ExitInstance()) return FALSE;

	if (m_bAutoDelete)
	{
		delete this;
	}

	return TRUE;
}

BOOL CRtThread::ExitInstance()
{
	Vpi_Semaphore_Delete(&m_hEventRunSem);
	Vpi_Thread_Delete(m_hThread);

	m_bRun=FALSE;

	return TRUE;
}

