// 点类
// 在数据库或文件中点存储的Buffer结构为:size+type+point2d
// options+point2d
#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOINT_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOINT_H_H
#include "Geometry/GKGeometry.h"
#include "Base/GKSystem.h"

namespace GKGeometryNameSpace{

class GEOMETRY_API GKGeometryPoint : public GKGeometry
{
public:
	
	// 得到存储在数据库中点的大小.
	virtual GKuint32 GetBufferSize() const{return sizeof(GKPoint2d);}

	// 得到类型
	virtual GKuint8 GetType() const{return (GKuint8)GKGeometry::GeoPoint;}

	// 得到内容
	virtual const GKbyte* GetBuffer() const	{return &m_nContent;}

	// 设置内容
	virtual void SetBuffer(const GKbyte* pBuffer, GKuint32 nSize){ memcpy(&m_nContent,pBuffer, nSize) };

private:
	GKPoint2d m_nContent;
};

}
#endif