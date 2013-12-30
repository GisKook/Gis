// ����
// �����ݿ���ļ��е�洢��Buffer�ṹΪ:size+type+point2d
// options+point2d
#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOINT_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOINT_H_H
#include "Geometry/GKGeometry.h"
#include "Base/GKSystem.h"

namespace GKGeometryNameSpace{

class GEOMETRY_API GKGeometryPoint : public GKGeometry
{
public:
	
	// �õ��洢�����ݿ��е�Ĵ�С.
	virtual GKuint32 GetBufferSize() const{return sizeof(GKPoint2d);}

	// �õ�����
	virtual GKuint8 GetType() const{return (GKuint8)GKGeometry::GeoPoint;}

	// �õ�����
	virtual const GKbyte* GetBuffer() const	{return &m_nContent;}

	// ��������
	virtual void SetBuffer(const GKbyte* pBuffer, GKuint32 nSize){ memcpy(&m_nContent,pBuffer, nSize) };

private:
	GKPoint2d m_nContent;
};

}
#endif