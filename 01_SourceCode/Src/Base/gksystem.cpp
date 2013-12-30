#include "Base/GKSystem.h.h"
#include "Base/GKExport.h.h"

GKTimer::GKTimer()
{
	m_uOverhead = 0;
	Start();
	m_uOverhead = Stop();
}

void GKTimer::Start()
{ 
	m_uStartCycle = GetCycleCount();
}

GKBASE::GKulong GKTimer::Stop()
{ 
	return GetCycleCount()-m_uStartCycle-m_uOverhead;
}