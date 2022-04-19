/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#include "CFSAcceptorBase.h"
namespace fsutils
{
CFSAcceptorBase::CFSAcceptorBase(IPSReactor *pThreadNetwork)
    : m_pReactor(pThreadNetwork)
    , m_pSink(NULL)
{

}

CFSAcceptorBase::~CFSAcceptorBase()
{
}

DWORD CFSAcceptorBase::AddReference()
{
    return CAWReferenceControlSingleThread::AddReference();
}

DWORD CFSAcceptorBase::ReleaseReference()
{
    return CAWReferenceControlSingleThread::ReleaseReference();
}

BOOL CFSAcceptorBase::IsConnector()
{
    return FALSE;
}
}//namespace fsutils

