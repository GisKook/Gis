// GKGeometry�����е�������Ļ���,�Ƕ����������еĵ�����е�������ĳ���.�����ĵ���Ҫ����Geometry��������
// ��һ���Geometry�Ͳ����������˰�,ֻ����Դ洢���е�
// �涨д���ļ���blob�ֶ��е����ݱ�����С�˵�.
// �����Ȳ�����Geometry�İ汾����,ֻ�������ݿ��д洢��Ԥ�����ֽ�,����Ϊ0,����Ժ�GeometryҲҪ���ǰ汾�ʹ���ѡ���ֽ���Ϊ�汾�Ŀ���
// ���ڰ汾��ȻҲ����ͨ���̶��Ľṹ�����ж�,�������²�
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
// BufferSize�ĸ������һ��,BufferSizeֻ��Geometry�Ĵ�С��������ǰ���Options.
// GetBufferSize()��SetBuffer()����һֱ,���Ƕ��Ǵ����ݿ�д������ʱҪ�õ�,��GetBuffer()�Ǵӳ���д�����ݿ��õ�,����ֻ���������ĵ��ڴ�
// date:2012-7-7 version:1 autor:zhangkai

#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRY_H_H 
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRY_H_H

namespace GKGeometryNameSpace{

#define GKGEOMETRYOPTIONS(arg1, arg2) (arg1>>32 | arg2 >> 24)

class GEOMETRY_API GKGeometry
{

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
	GKulong GetID() const
	{
		return m_nID;
	}

	// ����Geomtry��gid
	// param nID
	void SetID(GKulong nID)
	{
		m_nID = nID;
	}

	// �õ�Geometry�洢ʹ�õĴ�С(�ֽ�)
	// ������Option�Ĵ�С
	virtual GKuint32 GetBufferSize() const = 0;
	
	// �õ�Geomtry������
	virtual GKuint8 GetType() const = 0;

	// �õ�����,����д�뵽�ļ��������ݿ�
	virtual const GKbyte* GetBuffer() const = 0;

	// ��������,��ȡ�ļ��������ݿ⹹�����
	// param pBuffer[in] buffer
	// param nSize[in] buffer�Ĵ�С
	virtual void SetBuffer(const GKbyte* pBuffer, GKuint32 nSize) = 0;

public:
	virtual ~GKGeometry();
	GKGeometry();

private:
	// Geometry�Ĵ�С,����,�汾 4(��С)+1(����)+1(�汾) + 2(not use)
	GKulong m_nOptions;

	// Geometry��gid,����Ϊ�洢�����ݿ���ļ��е�����.
	GKulong m_nID;
};

}
#endif