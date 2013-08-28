#include "Toolkit/GKFile.h"
#include <io.h>
namespace GKBASE{

GKBASE::GKbool GKBASE::GKFile::IsExist( const GKBASE::StringPiece strPath )
{
	return 0==GKACCESS(strPath.Data(), 0);
}


GKBASE::GKbool GKFile::Delete( const GKBASE::StringPiece strPath )
{
	return 0==GKREMOVE(strPath.Data());
}

}