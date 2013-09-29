#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

class CsvReader
{
private:
	string filePath;

	vector<string> csvParser(string sorce);
	vector<vector<string> > readLine();

public:
	CsvReader(string _filePath);
	~CsvReader(void);

	vector<vector<string> > all();
};
