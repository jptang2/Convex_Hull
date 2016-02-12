#include "MyFile.h"

MyFile::MyFile()
{		
}


MyFile::~MyFile()
{
}

vector<Point3f> MyFile::Open()
{
	vector<Point3f> vPoint;	
	FILE *fp;
	OPENFILENAME ofn;
	TCHAR	szFile[MAX_PATH] = L"points.txt";
	TCHAR	szTitle[MAX_PATH] = L"points";
	static TCHAR szFilter[] = L"file type\0*.*\0file type1\0*.*0";
	ofn.lStructSize = sizeof (OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = szFilter;//类型            
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = szFile;// 文件路径指针            
	ofn.nMaxFile = MAX_PATH;//文件路径大小            
	ofn.lpstrFileTitle = szTitle;//文件名指针     
	ofn.nMaxFileTitle = MAX_PATH;//文件名大小            
	ofn.lpstrInitialDir = NULL;//初始化路径            
	ofn.lpstrTitle = L"Open";//对话框标            
	ofn.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER;//位标记的设置 
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = TEXT("txt");
	ofn.lCustData = 0L;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	if (GetOpenFileName(&ofn) == false) return vPoint;
	errno_t err = _wfopen_s(&fp, ofn.lpstrFile, L"r");	
	if (NULL == fp) return vPoint;

	int num = 0;
	float x = 0, y = 0;
	
	fscanf(fp, "%d", &num);	                      
	for (int i = 0; i <num; i++)
	{
		fscanf(fp, "%f", &x);
		fscanf(fp, "%f", &y);
		Point3f p={ x, y, 0 };
		vPoint.push_back(p);
	}
 	fclose(fp);
	return vPoint;
}

bool MyFile::Save(vector<Point3f> vPoint)
{
	FILE *fp;
	OPENFILENAME ofn;
	TCHAR	szFile[MAX_PATH] = L"points.txt";
	TCHAR	szTitle[MAX_PATH] = L"points";
	static TCHAR szFilter[] = L"file type\0*.*\0file type1\0*.*0";
	ofn.lStructSize = sizeof (OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = szFilter;//类型            
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = szFile;// 文件路径指针            
	ofn.nMaxFile = MAX_PATH;//文件路径大小            
	ofn.lpstrFileTitle = szTitle;//文件名指针     
	ofn.nMaxFileTitle = MAX_PATH;//文件名大小            
	ofn.lpstrInitialDir = NULL;//初始化路径            
	ofn.lpstrTitle = L"Save";//对话框标            
	ofn.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER;//位标记的设置 
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = TEXT("txt");
	ofn.lCustData = 0L;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
	if (GetOpenFileName(&ofn) == false) return false;
	errno_t err = _wfopen_s(&fp, ofn.lpstrFile, L"w");
	
	if (NULL == fp) return false;

	int num = vPoint.size();
	char buffer[50];
	sprintf(buffer, "%d", num);
	fprintf(fp, "%s", buffer);
	fprintf(fp, "\r\n");
	for (int i = 0; i < num; i++)
	{		
		Point3f p = vPoint[i];
		float x = p.x;
		float y = p.y;
		sprintf(buffer, "%f %f", x,y);
		fprintf(fp, "%s", buffer);
		fprintf(fp, "\r\n");
	}
	fclose(fp);
	return true;
}
