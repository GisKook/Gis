/*
     数据集基类,各种数据集应该派生自他,包括DatasetVect/DatasetRaster/Dataset... ...


	 关于数据集的投影还么有一个明确的概念,先处理成srid,也先定义一个描述信息,其实这描述信息可以结构下,当成数据集的元数据
	 来使用吧!现在先用一个GKString 代替吧.如果之后有想法再弄吧.

	 在数据集中使用的编码统一使用Unicode编码!

	 数据集在存储的过程中按照两个表的方式进行存储,一个表存储属性,一个表存储空间数据,存储空间数据的表仅包括GID(GKulong)和空间坐标GEO(blob)
	 存储矢量的表,目前想到的字段段包括 GID(GKulong), L(GKdouble), T(GKdouble), R(GKdouble), B(GKdouble), SRID(GKulong), DES(GKString)

	 Dataset的创建只能由DataSource来创建,释放也由DataSource来进行释放.

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

// 所有的Dataset...Info第一个变量都应该是GKuint32,存储nType
struct GKDatasetVectorInfo{
	GKBASE::GKuint32 nType;
	GKBASE::GKString strName;
};

class GKDataSource;
class ENGINE_API GKDataset
{
public:
	// 打开数据集
	// param [in] 名称
	virtual GKBASE::GKErrorCode Open(const GKBASE::GKString& strDtName) = 0;

	// 打开数据集
	// param [in] 次序
	virtual GKBASE::GKErrorCode Open(GKBASE::GKuint32 nIndex) = 0;

	// 关闭数据集
	virtual GKBASE::GKErrorCode Close() = 0;
	
	// 设置名字
	// strDatasetName[in] 数据集名字
	// 返回错误码
	virtual GKBASE::GKErrorCode SetName(const GKBASE::GKString& strDatasetName){m_strName = strDatasetName; return GKSUCCESS;}

	// 得到名字
	GKBASE::GKString GetName() const {return m_strName;}

	// 得到类型
	GKDatasetType GetDatasetType() const{return m_nDatasetType;}

	// 得到投影信息
	// 关于投影还没有一个特别清晰的认识,先这么搞,就是一个srid的概念,如果之后有修改,在动
	GKBASE::GKulong GetProjection() const {return m_nSrid;}

	// 设置投影
	// nSrid 地图投影编号
	void SetProjection(GKBASE::GKulong nSrid){m_nSrid = nSrid;}

	// 得到边界
	const GKBASE::GKBounds2d GetBounds() const {return m_dBounds;}

public:
	GKDataset(){};
	~GKDataset(){};
	
private:
	// 数据集的bounds 
	GKBASE::GKBounds2d m_dBounds;

	// 数据集名字
	GKBASE::GKString m_strName;

	// 地图投影编号
	GKBASE::GKulong m_nSrid;

	// 数据集的一些options
	GKBASE::GKulong m_nOptions;

	// 数据集的类型
	GKDatasetType m_nDatasetType;

	// 相关联的数据源
	GKENGINE::GKDataSource* m_pDataSource;
};
}
#endif