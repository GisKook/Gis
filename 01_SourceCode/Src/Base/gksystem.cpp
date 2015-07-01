#include "Base/GKSystem.h"
#include "Base/GKExport.h"

#if defined(GK_WIN)
static inline unsigned long long GetCycleCount(){ 
	__asm{ _emit 0x0F}
	__asm{_emit 0x31}
}
#endif

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

unsigned long long GKTimer::Stop()
{ 
	return GetCycleCount()-m_uStartCycle-m_uOverhead;
}
