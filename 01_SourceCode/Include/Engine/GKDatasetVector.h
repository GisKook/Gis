/*
	ʸ�����ݼ���,�洢ʸ������.
*/
#ifndef SOURCECODE_INCLUDE_ENGINE_GKDATASETVECTOR_H_H
#define SOURCECODE_INCLUDE_ENGINE_GKDATASETVECTOR_H_H

class ENGINE_API GKDatasetVector : public GKDataset
{
public:
	// ���Ҫ��
	// param [in] Ҫ��ӵ�Ҫ��
	struct GKFeature;
	virtual GKBASE::GKErrorCode AddFeature(const GKFeature&) = 0;
	
	// ɾ��Ҫ��
	// param [in] Ҫɾ����Ҫ��
	virtual GKBASE::GKErrorCode DeleteFeature(const GKFeature&) = 0;

	// �õ�Feature
	// param [in] Ҫ������
	virtual std::vector<const GKFeature > GetFeatures(const GKString & strName) = 0;

	// �޸�Feature
	// param [in] Ҫ������
	virtual GKBASE::GKErrorCode ModifyFeature(const GKuint32, )
	
	// �������Features
	virtual GKBASE::GKErrorCode ClearAllFeatures() = 0;

	// �õ�Feature�ĸ���
	const GKuint32 GetFeatureCount() const {return m_nFeatureCount;}

private:
	// Feature�ĸ���
	GKBASE::GKuint32 m_nFeatureCount;
};
#endif