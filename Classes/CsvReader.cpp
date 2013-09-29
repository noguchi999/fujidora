#include "CsvReader.h"

CsvReader::CsvReader(string _filePath)
{
	filePath = _filePath;
}


CsvReader::~CsvReader(void)
{
}

vector<string> CsvReader::csvParser(string sorce)
{
    char *pSorce;
    string buf;
	vector<string> data;

    pSorce = &sorce.at(0);
    while( *pSorce != NULL ) {
        if( *pSorce == ',' || *pSorce == '\n' || *pSorce == '\r') {
            data.push_back( buf );
            if( *pSorce == '\n' || *pSorce == '\r' ) break;
            buf = "";
        }
        else buf += *pSorce;
        pSorce++;
    }

    return data;
}

vector<vector<string> > CsvReader::readLine()
{
	vector<vector<string> > results;
	try {
        ifstream ifs("/Users/NoguchiOsamu/Dropbox/ios_farm/Book1.csv");
		string buf;
 
		while(getline(ifs, buf)) {
			buf+="\n";
			results.push_back(csvParser(buf));
		}

	} catch (char *str) {
		cout << str;
	}

    return results;
}

vector<vector<string> > CsvReader::all()
{
	return readLine();
}