#include "Base/GKErrors.h"

static char * errstr[]={
	"success",
	"filepath donot support i18n",
	"open shp file .shp file error",
	"open shp file .dbf file error",
};

char * GKErrorStr( GKBASE::GKErrorCode errcode )
{ 
	return errstr[errcode];
}
