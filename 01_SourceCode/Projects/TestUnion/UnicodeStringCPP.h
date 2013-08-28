

// namespace MUSGISKOOK
// {
// 	class MyUnicodeString;
// }
// using namespace MUSGISKOOK;

namespace USCPP
{

class UnicodeStringCPP
{
public:
// 	struct MyUnicodeStringEx
// 	{
// 		void* pTemp;
// 		int nCapacity;
// 		int nLength;
// 		short* pData;
// 		short sFlag;
// 		short sShortLength[7];
// 	};
public:
	UnicodeStringCPP();
	~UnicodeStringCPP();
public:
	union
	{
		void* m_pMyUnicodeString;
		struct UnicodeStringEx 
		{
			void* pTemp;
			int nCapacity;
			int nLength;
			short* pData;
			short sFlag;
			short sShortLength[7];
		}*m_pTarget;
	};
	
};
}
