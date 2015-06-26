#include "Geometry/GKGeometryPoint.h"
#include "Geometry/GKGeometryLine.h"
#include "Geometry/GKGeometryPointMulti.h"
#include <stdio.h>

using namespace GKBASE;
using namespace GKGEOMETRY;

int main(){
	GKPoint2d pt(0.1,0.2);
	GKPoint2d pt2(1,2);
	GKPoint2d pt3 = pt2;
	GKPoint2d pt4;
	pt4 = pt2;
	
	GKLine2d line(pt, pt2);
	GKLine2d line2(line);
	GKLine2d line3;
	line3 = line2;

	GKGeometryPoint gpt1(pt);
	GKGeometryPoint gpt2(100,200);
	GKGeometryPoint gpt3;
	gpt3 = gpt1; 

	GKGeometryLine gline1(pt, pt2);
	GKGeometryLine gline3;
	gline3 = gline1;
	GKPoint2d ptone = gline1[0];
	GKPoint2d pttwo = gline1[1];
	gline1[1].x = 1000;
	gline1[1].y = 2000;

	// pointmulti
	GKGeometryMultiPoint geomp;
	geomp.AddPoint(1,2);
	geomp.AddPoint(1,2);
	geomp.AddPoint(1,2);
	geomp.AddPoint(1,2);
	geomp.AddPoint(1,2);
	geomp.AddPoint(1,2);
	geomp.AddPoint(1,2);
	geomp.AddPoint(1,2);
	geomp.AddPoint(1,2);
	geomp.AddPoint(1,2);
	geomp.AddPoint(1,2);
	geomp.AddPoint(1,2);
	geomp[0].x = 10;
	geomp[0].y = 10;


	

	return 0;
}