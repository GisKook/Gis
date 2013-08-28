/*
	矢量数据集类,存储矢量数据.
*/
#ifndef SOURCECODE_INCLUDE_ENGINE_GKDATASETVECTOR_H_H
#define SOURCECODE_INCLUDE_ENGINE_GKDATASETVECTOR_H_H

class ENGINE_API GKDatasetVector : public GKDataset
{
public:
	// 添加要素
	// param [in] 要添加的要素
	struct GKFeature;
	virtual GKBASE::GKErrorCode AddFeature(const GKFeature&) = 0;
	
	// 删除要素
	// param [in] 要删除的要素
	virtual GKBASE::GKErrorCode DeleteFeature(const GKFeature&) = 0;

	// 得到Feature
	// param [in] 要素名称
	virtual std::vector<const GKFeature > GetFeatures(const GKString & strName) = 0;

	// 修改Feature
	// param [in] 要素名称
	virtual GKBASE::GKErrorCode ModifyFeature(const GKuint32, )
	
	// 清除所有Features
	virtual GKBASE::GKErrorCode ClearAllFeatures() = 0;

	// 得到Feature的个数
	const GKuint32 GetFeatureCount() const {return m_nFeatureCount;}

private:
	// Feature的个数
	GKBASE::GKuint32 m_nFeatureCount;
};
#endif