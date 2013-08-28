// �߶������ݿ��еĴ洢,����ָ������.������size+type+����
// options + line2d
#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYLINE_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYLINE_H_H

class GEOMETRY_API GKGeometryLine : public GKGeometry
{
public:
	// �õ�Geometry�洢ʹ�õĴ�С(�ֽ�)
	virtual GKuint32 GetBufferSize() const {return sizeof(GKLine2d)};

	// �õ�Geomtry������
	virtual GKuint8 GetType() const {return  (GKuint8)GKGeometry::GeoLine;} 

	// �õ�����,����д�뵽�ļ��������ݿ�
	virtual const GKbyte* GetBuffer() const{return (GKbyte*)&m_nContent}; 

	// ��������,��ȡ�ļ��������ݿ⹹�����
	// param pBuffer[in] buffer
	// param nSize[in] buffer�Ĵ�С(�ֽڵ�λ)
	virtual void SetBuffer(const GKbyte* pBuffer, GKuint32 nSize){memcpy(&m_nContent, pBuffer, nSize)};

private:
	GKLine2d m_nContent;
};
#endif
