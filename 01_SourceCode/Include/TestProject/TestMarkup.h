#include "Markup.h"
#include "Base/GKString.h"

class TestMarkup
{
public:
	bool ReadTextFile();
	bool WriteTextFile();
	bool AddElemTest();
private:
	static const GKBASE::GKuchar* pSrcFilePath;
	static const GKBASE::GKuchar* pToFilePath;
};