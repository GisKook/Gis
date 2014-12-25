#include <stdio.h>
#include "FileParseshp/GKFileParseshp.h"
using namespace GKFILEPARSE;
int main(){
	GKFileParseshp fileparseshp;
	fileparseshp.Open("E:/Gis/01_SourceCode/Projects/FileParseshp/alabama_natural/alabama_natural.shp");
	fileparseshp.GetInfo();
	fprintf(stdout, "entities %d, shapetype %d, Minx %f, Miny %f, Minz %f, Minm %f, Maxx %f, Maxy %f, Maxz %f, Maxm %f\n", fileparseshp.GetEntities(), 
		fileparseshp.GetShapetype(), fileparseshp.GetMinBound()[0], fileparseshp.GetMinBound()[1],fileparseshp.GetMinBound()[2],
		fileparseshp.GetMinBound()[3], fileparseshp.GetMaxBound()[0], fileparseshp.GetMaxBound()[1], fileparseshp.GetMaxBound()[2],
		fileparseshp.GetMaxBound()[3]);

	system("pause");
	return 0; 
}