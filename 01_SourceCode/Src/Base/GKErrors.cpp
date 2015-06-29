#include "Base/GKErrors.h"

static char * errstr[]={
	"success",
	"filepath donot support i18n",
};

char * GKErrorStr( GKBASE::GKErrorCode errcode )
{ 
	return errstr[errcode];
}
