#include <Windows.h>
#include <iostream>
struct PROFILE_SAMPLE
{
	long			lFlag;				
	WCHAR			szName[64];			

	LARGE_INTEGER	lStartTime;			

	__int64			iTotalTime;			
	__int64			iMin;			
	__int64			iMax;			

	__int64			iCall;				
};

 void PRO_BEGIN(const WCHAR* s);
 void PRO_END(const WCHAR* s);

class my_profile
{
public:
	enum
	{
		PROFILE_MAX_NUM = 10,
		FILE_NAME_MAX = 64
	};
private:
	PROFILE_SAMPLE _profile[PROFILE_MAX_NUM];
	char _logFileName[FILE_NAME_MAX];
	LARGE_INTEGER baseTime;
	LARGE_INTEGER start;
public:
	my_profile(const char* szLogFile ="proFile");
	~my_profile();
private:
	void ProfileBegin(const WCHAR* szName);
	void ProfileEnd(const WCHAR* szName);
	void SaveLogFile();
	friend void PRO_BEGIN(const WCHAR* s);
	friend void PRO_END(const WCHAR* s);

};


#define PROFILE

#ifdef PROFILE
#define PRO_BEGIN(tag) PRO_BEGIN(tag)
#define PRO_END(tag) PRO_END(tag)

#else
#define PRO_BEGIN(tag)
#define PRO_END(tag)
#endif
