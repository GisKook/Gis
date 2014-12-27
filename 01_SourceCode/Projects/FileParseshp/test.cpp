#include <stdio.h>
#include "FileParseshp/GKFileParseshp.h"
using namespace GKFILEPARSE;
int main(){
	FILE *fp;
	fp = freopen("E:/Gis/01_SourceCode/Projects/FileParseshp/alabama_natural/dump.txt", "a+", stdout);
	GKFileParseshp fileparseshp;
	fileparseshp.Open("E:/Gis/01_SourceCode/Projects/FileParseshp/alabama_natural/alabama_natural.shp");
	fileparseshp.OpenDBF("E:/Gis/01_SourceCode/Projects/FileParseshp/alabama_natural/alabama_natural.dbf");
//	fileparseshp.Open("E:/Gis/01_SourceCode/Projects/FileParseshp/alabama_highway/alabama_highway.shp");
//	fileparseshp.OpenDBF("E:/Gis/01_SourceCode/Projects/FileParseshp/alabama_highway/alabama_highway.dbf");
	fileparseshp.LoadInfo();
	fileparseshp.LoadDBFInfo();
	fprintf(stdout, "entities %d, shapetype %d, Minx %f, Miny %f, Minz %f, Minm %f, Maxx %f, Maxy %f, Maxz %f, Maxm %f\n", fileparseshp.GetEntities(), 
		fileparseshp.GetShapetype(), fileparseshp.GetMinBound()[0], fileparseshp.GetMinBound()[1],fileparseshp.GetMinBound()[2],
		fileparseshp.GetMinBound()[3], fileparseshp.GetMaxBound()[0], fileparseshp.GetMaxBound()[1], fileparseshp.GetMaxBound()[2],
		fileparseshp.GetMaxBound()[3]);
	int i = 0;
	for(;i < fileparseshp.GetFieldCount(); ++i){
		fprintf(stdout, "name: %s, type: %d,  width: %d, decimals: %d\n", fileparseshp.GetFiledName(i), fileparseshp.GetFieldType(i), fileparseshp.GetFieldWidth(i), fileparseshp.GetFieldDecimals(i));
	}
	SHPObject * object = NULL;
	int j;
	for(i =  0;i<fileparseshp.GetEntities();++i){
		object = fileparseshp.GetEntity(i);
		for(j = 0; j < fileparseshp.GetFieldCount(); ++j){ 
			if(GKFileParseshp::FTInteger == fileparseshp.GetFieldType(j)){
				fprintf(stdout, "%s %d ", fileparseshp.GetFiledName(j), fileparseshp.ReadIntegerAttr(GKFileParseshp::GetEntityID(object), j));
			}else if(GKFileParseshp::FTString == fileparseshp.GetFieldType(j)){
				fprintf(stdout, "%s %s ", fileparseshp.GetFiledName(j), fileparseshp.ReadStringAttr(GKFileParseshp::GetEntityID(object), j));
			}
		}
		fprintf(stdout,"\n");
		fileparseshp.DestroyEntity(object);
	}

	fclose(fp);
	system("pause");
	return 0; 
}