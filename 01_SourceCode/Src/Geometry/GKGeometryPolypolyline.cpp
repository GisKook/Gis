#include "Geometry/GKGeometryPolypolyline.h"
#include "Geometry/GKGeometryPolyLine.h"

using namespace GKBASE;
using namespace GKGEOMETRY;

void GKGEOMETRY::GKGeometryPolypolyline::AddSubLine( GKGeometryPolyline & subline ){ 
	size_t pointcount = subline.GetSize();
	for(size_t i = 0; i < pointcount; ++i){
		m_points.push_back(subline[i]);
	}
	m_nSubLinePointCounts.push_back(pointcount);
}

void GKGEOMETRY::GKGeometryPolypolyline::AddSubLine( GKPolyLine2d & subline) {
	return AddSubLine(GKGeometryPolyline(subline)); 
}
