#pragma once
#include <windows.h>
#include <iostream>
#include <vector>
using namespace std;
struct Point3f
{
	float x;
	float y;
	float z;
	// 	Point3f(){x = 0.0f;y = 0.0f;z = 0.0f;}
	// 	Point3f(float _x, float _y, float _z){ x = _x; y = _y; z = _z; }
	// 	Point3f(std::initializer_list<float> _x) :x(0.0f){}
};
class MyFile
{
public:
	MyFile();
	~MyFile();
	vector<Point3f> Open();
	bool Save(vector<Point3f> vPoint);
private:
	
};

