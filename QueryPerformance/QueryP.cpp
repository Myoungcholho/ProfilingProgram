#include "QueryP.h"
#include <stdio.h>
#include <ctime>
/* �����Լ� */
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

			fprintf(pfile, "%.4f��  |", (double)(_profile[i].iTotalTime / _profile[i].iCall)/10000);
			fprintf(pfile, "%.4f��  |", (double)_profile[i].iMin / 10000);
			fprintf(pfile, "%.4f��  |", (double)_profile[i].iMax / 10000);
			fprintf(pfile, "%I64d  \n", _profile[i].iCall);
		}
	}

	fprintf(pfile, "-------------------------------------------------------------------------------\n");

	fclose(pfile);
}

void my_profile::ProfileBegin(const WCHAR* szName)
{
	// Begin���� �ؾ��� ��
	// 1. ����� szName �ֱ�
	// 2. �Ⱥ���ٰ� ǥ�óֱ�
	// 3. ���� �ð� üũ�ϱ�
	// 4. ����ȣ��Ƚ�� �����ϱ�
	// 2��° ���������� ���� �̸����� ã�ƾ��ϴµ�?

	// begin - end �� ������ IStartTime �� 0���� reset
	// begin - begin�� ������ IStartTime�� ���� ���� ����

	bool first_check = false;


	// ���� WCHAR�� �ִٸ�
	for (int i = 0; i < PROFILE_MAX_NUM; ++i)
	{
		if (wcscmp(_profile[i].szName, szName) == 0)
		{
			// 0�� �ƴ� ���� �ִٸ� �װ��� begin-begin �ΰ���
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
	// �ƿ� �����ٸ� ����� ã������
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
	// 1. szName�� Ž���ؼ� �ִ��� ã��
	// 2. ���࿡ �ִٸ� QuerperFor �ð����
	// 3. ��� �� �ð��̶� StartTime�̶� ���� �ٽ� �� ���� IStartTime�� �ְ�
	// 4. �� ���� 1000000 (�鸸) ���ϰ� 10000(��) ���ؾ���
	// 5. �װŸ� iTotalTime�� ���������� ��
	// 6.  iMin Ȯ�� �� ���� ������ ������
	// 7.  iMax Ȯ�� �� ���� ũ�� ������
	/* ����ó��				
	- begin�� ���µ� end�� ������� -> IStartTime�� 0�� ���ϰ���
	- end �ϰ� end������ ->  IStartTime�� 0�� ��
	
	*/
	double demi = 0;
	for (int i = 0; i < PROFILE_MAX_NUM; ++i)
	{
		if (wcscmp(_profile[i].szName, szName) == 0)
		{
			// �̰� 0�� ���ϼ�������
			if (_profile[i].lStartTime.QuadPart == 0)
			{
				break;
			}
			LARGE_INTEGER end;

			QueryPerformanceCounter(&end);
			demi = ((end.QuadPart - _profile[i].lStartTime.QuadPart) *1000000)/(double)baseTime.QuadPart;
			demi = demi * 10000;

			// begin- begin �������� �ʱ�ȭ, ���� ������
			_profile[i].lStartTime.QuadPart = 0;

			// double�� int64�� ���� ����
			// �ϴ� �Ҽ����� ©����. ������ �� 4�ڸ����� �ʿ��ϴ� ���X
			_profile[i].iTotalTime += demi;
			
			// iMAX, iMin ����
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
			// ������ �� �ʿ� �����Ƿ� break;
			break;
		}
	}
	
}

/* �����Լ� */
/* my_profile g_profile; */

void PRO_BEGIN(const WCHAR* s)
{
	g_profile.ProfileBegin(s);
}

// ������� �ϱ�
void PRO_END(const WCHAR* s)
{
	g_profile.ProfileEnd(s);
}