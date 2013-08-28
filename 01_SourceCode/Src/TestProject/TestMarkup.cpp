#include "TestProject/TestMarkup.h"
using namespace GKBASE;

const GKuchar* TestMarkup::pSrcFilePath=_U("E:/Gis_Kook/01_SourceCode/Src/TestProject/TestMarkup.xml");
bool TestMarkup::ReadTextFile()
{
	CMarkup markup;
	GKString strDoc;
	//return markup.ReadTextFile(pSrcFilePath, strDoc);
	return markup.ReadTextFile(_U("E:/trunk/01_SourceCode/Src/TestProject/TestMarkup.xml"), strDoc);
}

bool TestMarkup::WriteTextFile()
{
	CMarkup markup;
	GKString strDoc(_U("<abc>Gis_Kook</abc>"));
	strDoc+=_U("\n");
	markup.SetDoc(strDoc);
	markup.AddAttrib(_U("map"), 1);
	markup.AddElem(_U("dddd"),_U("dd") );

	markup.WriteTextFile(_U("c:/Test.xml"), markup.GetDoc());

	return TRUE;	
}

bool TestMarkup::AddElemTest()
{
	CMarkup markup;
	markup.AddElem(_U("Gis"), 234);
	markup.AddChildElem(_U("ABC"), _U("FALSE"));
	markup.IntoElem();
	markup.AddElem(_U("zhangkai"), 20);
	return TRUE;
}