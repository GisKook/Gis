// GKGeometry�����е�������Ļ���,�Ƕ����������еĵ�����е�������ĳ���.�����ĵ���Ҫ����Geometry��������
// ��һ���Geometry�Ͳ����������˰�
// �涨д���ļ���blob�ֶ��е����ݱ�����С�˵�.
// �����Ȳ�����Geometry�İ汾����,ֻ�������ݿ��д洢��Ԥ�����ֽ�,����Ϊ0,����Ժ�GeometryҲҪ���ǰ汾�ʹ���ѡ���ֽ���Ϊ�汾�Ŀ���
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
// �洢�ṹΪ:��С+����+����
// ���ڴ�С�˵�����,��Engine��Geometry���м�㴦���.������SetBuffer/GetBufferʱ�õ��Ķ��Ǻ͵�ǰϵͳһ�µĴ洢,�������ǰϵͳ
// �Ǵ�˵ĳ�����תʱ��ʹ�õĴ��,�����С�˵ĳ�����ת�ľ���С��.
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

	// �õ�Geomtry�����ݿ��д洢��gid
	GKBASE::GKulong GetID() const
	{
		return m_nID;
	}

	// ����Geomtry��gid
	// param nID
	void SetID(GKBASE::GKulong nID)
	{
		m_nID = nID;
	}

	// �õ�Geomtry������
	virtual GKBASE::GKuint8 GetType() const = 0;

public:
	virtual ~GKGeometry(){};
	GKGeometry():m_nOptions(0), m_nID(0){};

private:
	// Geometry�Ĵ�С,����,�汾 4(��С)+1(����)+1(�汾) + 2(not use)
	GKBASE::GKuint32 m_nOptions;

	// Geometry��gid,����Ϊ�洢�����ݿ���ļ��е�����.
	GKBASE::GKulong m_nID;
};

}
#endif