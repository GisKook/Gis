// GKGeometry是所有地理几何体的基类,是对世界上所有的地物进行的最基本的抽象.其他的地物要根据Geometry进行派生
// 这一类的Geometry就不包括拓扑了吧
// 规定写到文件或blob字段中的内容必须是小端的.
// 现在先不考虑Geometry的版本问题,只是在数据库中存储有预留的字节,都置为0,如果以后Geometry也要考虑版本就从中选出字节作为版本的考虑
// Geometry:
//	Point;
//	Curve;
// 		LineString;
// 			Line;
// 			LineRing;
// 	Surface;
// 		Polygon;
// 			Triangle;
// 				TIN;
// 		Polyedral Surface;
// 	GeometryCollection;
// 		MultiSurface;
// 			MultiPolygon;
// 		MultiCurve;
// 			MultiLineString;
// 		MultiPoint;
// 存储结构为:大小+类型+内容
// 关于大小端的问题,在Engine和Geometry的中间层处理吧.就是在SetBuffer/GetBuffer时得到的都是和当前系统一致的存储,即如果当前系统
// 是大端的程序流转时就使用的大端,如果是小端的程序流转的就是小端.
// date:2012-7-7 version:1 autor:zhangkai
// date:2015-6-25 version:0.1 delete the database related concepts.Now geometry only a struct in memory.

#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRY_H_H 
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRY_H_H
#include "Base/GKDataType.h"

namespace GKGEOMERTY{

#define GKGEOMETRYOPTIONS(arg1, arg2) (arg1>>32 | arg2 >> 24)

class GEOMETRY_API GKGeometry{
public:
	enum GeoType
	{
		GeoPoint,
		GeoMultiPoint,
		GeoLine,
		GeoPolyline,
		GeoPolyPolyline,
		GeoPolygon,
		GeoPolypolygon	
	};

	// 得到Geomtry在数据库中存储的gid
	GKBASE::GKulong GetID() const
	{
		return m_nID;
	}

	// 设置Geomtry的gid
	// param nID
	void SetID(GKBASE::GKulong nID)
	{
		m_nID = nID;
	}

	// 得到Geomtry的类型
	virtual GKBASE::GKuint8 GetType() const = 0;

public:
	virtual ~GKGeometry(){};
	GKGeometry():m_nOptions(0), m_nID(0){};

private:
	// Geometry的大小,类型,版本 4(大小)+1(类型)+1(版本) + 2(not use)
	GKBASE::GKuint32 m_nOptions;

	// Geometry的gid,不作为存储在数据库或文件中的内容.
	GKBASE::GKulong m_nID;
};

}
#endif