/*
     ���ݼ�����,�������ݼ�Ӧ����������,����DatasetVect/DatasetRaster/Dataset... ...


	 �������ݼ���ͶӰ��ô��һ����ȷ�ĸ���,�ȴ����srid,Ҳ�ȶ���һ��������Ϣ,��ʵ��������Ϣ���Խṹ��,�������ݼ���Ԫ����
	 ��ʹ�ð�!��������һ��GKString �����.���֮�����뷨��Ū��.

	 �����ݼ���ʹ�õı���ͳһʹ��Unicode����!

	 ���ݼ��ڴ洢�Ĺ����а���������ķ�ʽ���д洢,һ����洢����,һ����洢�ռ�����,�洢�ռ����ݵı������GID(GKulong)�Ϳռ�����GEO(blob)
	 �洢ʸ���ı�,Ŀǰ�뵽���ֶζΰ��� GID(GKulong), L(GKdouble), T(GKdouble), R(GKdouble), B(GKdouble), SRID(GKulong), DES(GKString)

	 Dataset�Ĵ���ֻ����DataSource������,�ͷ�Ҳ��DataSource�������ͷ�.

*/
#ifndef SOURCECODE_INCLUDE_ENGINE_GKDATASET_H_H
#define SOURCECODE_INCLUDE_ENGINE_GKDATASET_H_H
#include "Base/GKDataType.h"
#include "Base/GKString.h"
#include "Base/GKErrors.h"

namespace GKENGINE{

enum GKDatasetType{
	POINT = 0,
	LINE = 1,
	REGION = 2,
	TEXT = 3,
	RASTER = 4,
	ATTR = 5
};

// ���е�Dataset...Info��һ��������Ӧ����GKuint32,�洢nType
struct GKDatasetVectorInfo{
	GKBASE::GKuint32 nType;
	GKBASE::GKString strName;
};

class GKDataSource;
class ENGINE_API GKDataset
{
public:
	// �����ݼ�
	// param [in] ����
	virtual GKBASE::GKErrorCode Open(const GKBASE::GKString& strDtName) = 0;

	// �����ݼ�
	// param [in] ����
	virtual GKBASE::GKErrorCode Open(GKBASE::GKuint32 nIndex) = 0;

	// �ر����ݼ�
	virtual GKBASE::GKErrorCode Close() = 0;
	
	// ��������
	// strDatasetName[in] ���ݼ�����
	// ���ش�����
	virtual GKBASE::GKErrorCode SetName(const GKBASE::GKString& strDatasetName){m_strName = strDatasetName; return GKSUCCESS;}

	// �õ�����
	GKBASE::GKString GetName() const {return m_strName;}

	// �õ�����
	GKDatasetType GetDatasetType() const{return m_nDatasetType;}

	// �õ�ͶӰ��Ϣ
	// ����ͶӰ��û��һ���ر���������ʶ,����ô��,����һ��srid�ĸ���,���֮�����޸�,�ڶ�
	GKBASE::GKulong GetProjection() const {return m_nSrid;}

	// ����ͶӰ
	// nSrid ��ͼͶӰ���
	void SetProjection(GKBASE::GKulong nSrid){m_nSrid = nSrid;}

	// �õ��߽�
	const GKBASE::GKBounds2d GetBounds() const {return m_dBounds;}

public:
	GKDataset(){};
	~GKDataset(){};
	
private:
	// ���ݼ���bounds 
	GKBASE::GKBounds2d m_dBounds;

	// ���ݼ�����
	GKBASE::GKString m_strName;

	// ��ͼͶӰ���
	GKBASE::GKulong m_nSrid;

	// ���ݼ���һЩoptions
	GKBASE::GKulong m_nOptions;

	// ���ݼ�������
	GKDatasetType m_nDatasetType;

	// �����������Դ
	GKENGINE::GKDataSource* m_pDataSource;
};
}
#endif