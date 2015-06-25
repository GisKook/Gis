#include "Geometry/GKGeometryPoint.h"
#include "Geometry/GKGeometryLine.h"
#include <stdio.h>

using namespace GKBASE;
using namespace GKGEOMERTY;

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
	GKGeometryLine gline2(line3);
	GKGeometryLine gline3;
	GKPoint2d ptt = gline2[0];
	gline3 = gline2;


	return 0;
}