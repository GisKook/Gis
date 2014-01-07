// ����ϵͳ��ص��ö���װ������ļ���
#ifndef SOURCECODE_INCLUDE_BASE_SYSTEM_H_H
#define SOURCECODE_INCLUDE_BASE_SYSTEM_H_H
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// ��С��
static union{
	unsigned int a;
	char b;
}endian = {1};

#define GKBIGENDIAN endian.b!=1
#define GKSWAP2BYTES(args) ((args & 0x00FF) << 8 | (args & 0xFF00) >> 8)
#define GKSWAP4BYTES(args) ((args & 0x000000FF) << 24 | (args & 0x0000FF00) << 8 | \
	(args & 0x00FF0000) >> 8 | (args & 0xFF000000) >> 24)
#define GKSWAP8BYTES(args) ((args & 0x00000000000000FF) << 56 | (args & 0x000000000000FF00) << 40 | \
	(args & 0x0000000000FF0000) << 24 | (args & 0x00000000FF000000) << 8 | \
	(args & 0x000000FF00000000) >> 8 | (args & 0x0000FF0000000000) >> 24 | \
	(args & 0x00FF000000000000) >> 40 | (args & 0xFF00000000000000) >> 56)

#ifdef GK_WIN
inline void BASE_API gkassert( bool b )
{
	if (!b){ __asm{ INT 3; } }
}
#define GKASSERT gkassert 
#else
#define GKASSERT assert
#endif
#define GKSPRINTF swprintf

#ifdef GK_WIN
// �����ļ�����
#define GKFOPEN _wfopen
#define GKFSEEK fseek
#define GKFTELL ftell
#define GKFREAD fread
#define GKFWRITE fwrite  
#define GKREMOVE _wremove 
#define GKACCESS _waccess
#else
#endif

// brief �õ��߾���ʱ�� 
// brief ��ȡʱ�������
#if defined(GK_WIN)
inline GKulong GetCycleCount(){ 
	__asm{ _emit 0x0F}
	__asm{_emit 0x31}
}
#endif

class BASE_API GKTimer{
public:
	// brief ��ʼ��
	GKTimer();

	// brief ��ʼ
	void Start();

	// brief ����
	GKulong Stop(); 
public:
	GKulong m_uOverhead;

private:
	GKulong m_uStartCycle;
}

#endif // SOURCECODE_INCLUDE_BASE_SYSTEM_H_H