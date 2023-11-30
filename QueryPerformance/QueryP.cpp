#include "QueryP.h"
#include <stdio.h>
#include <ctime>
/* 전역함수 */
my_profile g_profile;
my_profile::my_profile(const char* szLogFile)
{
	char szTime[17] = "";
	time_t timer;
	struct tm TM;

	memset(_profile, 0,sizeof(PROFILE_SAMPLE) * PROFILE_MAX_NUM);
	memset(_logFileName, 0, FILE_NAME_MAX);

	time(&timer);
	localtime_s(&TM, &timer);

	sprintf_s(szTime, 17, "%04d%02d%02d_%02d%02d%02d",
		TM.tm_year + 1900,
		TM.tm_mon + 1,
		TM.tm_mday,
		TM.tm_hour,
		TM.tm_min,
		TM.tm_sec);
	
	strcat_s(_logFileName, 64, szLogFile);
	strcat_s(_logFileName, 64, szTime);
	strcat_s(_logFileName, 64, ".txt");

	QueryPerformanceFrequency(&baseTime);
}

my_profile::~my_profile()
{
	SaveLogFile();

}

void my_profile::SaveLogFile()
{
	FILE* pfile;
	errno_t err = fopen_s(&pfile, _logFileName, "a");
	if (err != 0) return;

	fprintf(pfile, "-------------------------------------------------------------------------------\n\n");
	fprintf(pfile, "           Name  |");
	fprintf(pfile, "     Average  |");
	fprintf(pfile, "        Min   |");
	fprintf(pfile, "        Max   |");
	fprintf(pfile, "        Call|\n");
	fprintf(pfile, "-------------------------------------------------------------------------------\n");


	for (int i = 0; i<PROFILE_MAX_NUM; ++i)
	{
		if (_profile[i].lFlag != 0)
		{
			fprintf(pfile, "%17ls |", _profile[i].szName); //C6303
			_profile[i].iTotalTime = _profile[i].iTotalTime - _profile[i].iMax - _profile[i].iMin;
			_profile[i].iCall -= 2;

			fprintf(pfile, "%.4f㎲  |", (double)(_profile[i].iTotalTime / _profile[i].iCall)/10000);
			fprintf(pfile, "%.4f㎲  |", (double)_profile[i].iMin / 10000);
			fprintf(pfile, "%.4f㎲  |", (double)_profile[i].iMax / 10000);
			fprintf(pfile, "%I64d  \n", _profile[i].iCall);
		}
	}

	fprintf(pfile, "-------------------------------------------------------------------------------\n");

	fclose(pfile);
}

void my_profile::ProfileBegin(const WCHAR* szName)
{
	// Begin에서 해야할 일
	// 1. 빈곳에 szName 넣기
	// 2. 안비었다고 표시넣기
	// 3. 들어온 시간 체크하기
	// 4. 누적호출횟수 증가하기
	// 2번째 들어왔을때는 파일 이름으로 찾아야하는데?

	// begin - end 가 들어오면 IStartTime 을 0으로 reset
	// begin - begin이 들어오면 IStartTime에 값이 있을 것임

	bool first_check = false;


	// 같은 WCHAR가 있다면
	for (int i = 0; i < PROFILE_MAX_NUM; ++i)
	{
		if (wcscmp(_profile[i].szName, szName) == 0)
		{
			// 0이 아닌 값이 있다면 그것은 begin-begin 인것임
			if (_profile[i].lStartTime.QuadPart != 0)
			{
				std::cout << "begin - begin" << std::endl;
				break;
			}
			QueryPerformanceCounter(&_profile[i].lStartTime);
			_profile[i].iCall++;
			first_check = true;
			break;
		}
	}
	// 아예 없었다면 빈곳을 찾으러가
	if (!first_check) {
		for (int i = 0; i < PROFILE_MAX_NUM; ++i)
		{
			if (_profile[i].lFlag == 0)
			{
				wcscpy_s(_profile[i].szName, szName);
				_profile[i].lFlag = 1;
				QueryPerformanceCounter(&_profile[i].lStartTime);
				_profile[i].iCall++;
				break;
			}
		}
	}
}

void my_profile::ProfileEnd(const WCHAR* szName)
{
	// 1. szName을 탐색해서 있는지 찾기
	// 2. 만약에 있다면 QuerperFor 시간재고
	// 3. 방금 잰 시간이랑 StartTime이랑 빼고 다시 그 값을 IStartTime에 넣고
	// 4. 그 값을 1000000 (백만) 곱하고 10000(만) 곱해야함
	// 5. 그거를 iTotalTime에 누적저장할 때
	// 6.  iMin 확인 후 보다 작으면 들어오고
	// 7.  iMax 확인 후 보다 크면 들어오고
	/* 예외처리				
	- begin이 없는데 end만 했을경우 -> IStartTime이 0의 값일것임
	- end 하고 end했을때 ->  IStartTime이 0의 값
	
	*/
	double demi = 0;
	for (int i = 0; i < PROFILE_MAX_NUM; ++i)
	{
		if (wcscmp(_profile[i].szName, szName) == 0)
		{
			// 이게 0의 값일수가없음
			if (_profile[i].lStartTime.QuadPart == 0)
			{
				break;
			}
			LARGE_INTEGER end;

			QueryPerformanceCounter(&end);
			demi = ((end.QuadPart - _profile[i].lStartTime.QuadPart) *1000000)/(double)baseTime.QuadPart;
			demi = demi * 10000;

			// begin- begin 예방차원 초기화, 쓸모도 없어짐
			_profile[i].lStartTime.QuadPart = 0;

			// double을 int64로 저장 누적
			// 일단 소수점은 짤린다. 어차피 뒷 4자리까지 필요하니 상관X
			_profile[i].iTotalTime += demi;
			
			// iMAX, iMin 관리
			if (_profile[i].iMax < demi)
			{
				_profile[i].iMax = demi;
			}
			if (_profile[i].iCall ==1)
			{
				_profile[i].iMin = demi;
			}
			if(_profile[i].iMin > demi)
			{
				_profile[i].iMin = demi;
			}
			// 다음껀 할 필요 없으므로 break;
			break;
		}
	}
	
}

/* 전역함수 */
/* my_profile g_profile; */

void PRO_BEGIN(const WCHAR* s)
{
	g_profile.ProfileBegin(s);
}

// 여기부터 하기
void PRO_END(const WCHAR* s)
{
	g_profile.ProfileEnd(s);
}