// 格式为大小+类型+个数+字节数据
#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOINTMULTI_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOINTMULTI_H_H

namespace GKGeometryNameSpace{

class GEOMETRY_API GKMultiPoint : public GKGeometry
{
public:
	// 得到Geometry存储使用的大小(字节)
	// 总大小+点个数+内容
	virtual GKuint32 GetBufferSize() const {sizeof(GKuint32)+sizeof(GKPoint2d)*m_cPoints.size();}

	// 得到Geomtry的类型
	virtual GKuint8 GetType() const {return (GKuint8)GKGeometry::GeoMultiPoint;} 

	// 得到点数
	GKuint32 GetPointCounts() const{return m_cPoints.size();}

	// 得到点串的内容
	virtual const GKbyte* GetBuffer() const{return (GKbyte*)&m_cPoints[0];}
	
	// 设置
	virtual void SetBuffer(const GKbyte* pBuffer, GKuint32 nSize){
		GKASSERT(nSize%sizeof(GKPoint2d) == 0 && nSize / sizeof(GKPoint2d) != 0) ;
		GKuint32 nPointCounts = nSize/sizeof(GKPoint2d);
		m_cPoints.resize(nPointCounts);
		memcpy(&m_cPoints[0], pBuffer, nSize);
	};

private:
	std::vector<GKPoint2d> m_cPoints;

};
}
#endif