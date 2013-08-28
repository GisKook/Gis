// 线段在数据库中的存储,这里指两点线.内容是size+type+内容
// options + line2d
#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYLINE_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYLINE_H_H

class GEOMETRY_API GKGeometryLine : public GKGeometry
{
public:
	// 得到Geometry存储使用的大小(字节)
	virtual GKuint32 GetBufferSize() const {return sizeof(GKLine2d)};

	// 得到Geomtry的类型
	virtual GKuint8 GetType() const {return  (GKuint8)GKGeometry::GeoLine;} 

	// 得到内容,用于写入到文件或者数据库
	virtual const GKbyte* GetBuffer() const{return (GKbyte*)&m_nContent}; 

	// 设置内容,读取文件或者数据库构造对象
	// param pBuffer[in] buffer
	// param nSize[in] buffer的大小(字节单位)
	virtual void SetBuffer(const GKbyte* pBuffer, GKuint32 nSize){memcpy(&m_nContent, pBuffer, nSize)};

private:
	GKLine2d m_nContent;
};
#endif
