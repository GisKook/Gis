#include "Geometry/GKGeometry.h"

using namespace GKBASE;
using namespace GKGEOMETRY;

GKBASE::GKPoint2d & GKGEOMETRY::GKGeometry::operator[]( GKBASE::GKuint32 index )
{
	size_t ptcount = m_points.size();
	if(ptcount > 0){
		if(index<0){
			return m_points[0];
		}else if(index > ptcount - 1){
			return m_points[ptcount-1];
		}else{ 
			return m_points[index];
		}
	}
}

