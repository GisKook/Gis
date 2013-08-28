
namespace MUSGISKOOK
{

class MyUnicodeString
{
public:
	MyUnicodeString();
	~MyUnicodeString();
public:
	void* m_pTemp;
	int m_nCapacity;
	int m_nLength;
	short* m_pData;
	short m_sFlag;
	short m_sShortLength[7];
};

}
