// GKGeometry是所有地理几何体的基类,是对世界上所有的地物进行的最基本的抽象.其他的地物要根据Geometry进行派生
// 这一类的Geometry就不包括拓扑了吧,只是针对存储进行的
// 规定写到文件或blob字段中的内容必须是小端的.
// 现在先不考虑Geometry的版本问题,只是在数据库中存储有预留的字节,都置为0,如果以后Geometry也要考虑版本就从中选出字节作为版本的考虑
// 对于版本当然也可以通过固定的结构进行判断,这是下下策
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
// BufferSize的概念调整一下,BufferSize只是Geometry的大小并不包括前面的Options.
// GetBufferSize()和SetBuffer()概念一直,他们都是从数据库写到程序时要用的,而GetBuffer()是从程序写到数据库用的,所以只包括连续的点内存
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

	// 得到Geomtry在数据库中存储的gid
	GKulong GetID() const
	{
		return m_nID;
	}

	// 设置Geomtry的gid
	// param nID
	void SetID(GKulong nID)
	{
		m_nID = nID;
	}

	// 得到Geometry存储使用的大小(字节)
	// 不包括Option的大小
	virtual GKuint32 GetBufferSize() const = 0;
	
	// 得到Geomtry的类型
	virtual GKuint8 GetType() const = 0;

	// 得到内容,用于写入到文件或者数据库
	virtual const GKbyte* GetBuffer() const = 0;

	// 设置内容,读取文件或者数据库构造对象
	// param pBuffer[in] buffer
	// param nSize[in] buffer的大小
	virtual void SetBuffer(const GKbyte* pBuffer, GKuint32 nSize) = 0;

public:
	virtual ~GKGeometry();
	GKGeometry();

private:
	// Geometry的大小,类型,版本 4(大小)+1(类型)+1(版本) + 2(not use)
	GKulong m_nOptions;

	// Geometry的gid,不作为存储在数据库或文件中的内容.
	GKulong m_nID;
};

}
#endif