// haruno_pushback.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <string>
#include <fstream>
#include <windows.h>

#pragma comment(lib, "user32.lib")

using namespace std;

int main(int argc, char* argv[])
{
	if(argc != 2)
		return -1;

	string filename(argv[1]);
	filename += ".txt";

	fstream text(filename);
	fstream scr(argv[1]);

	string bfilename(argv[1]);
	bfilename += "__comb";
	
	FILE *out = 0;
	out = fopen(bfilename.c_str(), "wb");
	if(out == 0)
	{
		return -1;
	}
	string ReadLine;

	while(getline(scr, ReadLine))
	{
		if(ReadLine[0] != ';' && ReadLine[0] != '[')
		{
			//read from translated text file
			string Text;
			getline(text, Text);
			string truText = Text.substr(12, Text.length());
			if(ReadLine.compare(truText) != 0)
			{
				string info = "differ string in";
				info += argv[1];

				MessageBoxA(NULL, info.c_str(), "pushback", MB_OK);
				return -1;
			}
			Text = "";
			getline(text, Text);
			if(Text[0] != ';')
			{
				string info = "Invalid Chinese line in";
				info += argv[1];

				MessageBoxA(NULL, info.c_str(), "pushback", MB_OK);
				return -1;
			}
			string chineseText = Text.substr(13, Text.length());
			fwrite(chineseText.c_str(), 1, chineseText.length(), out);
			fwrite("\r\n", 1, 2, out);

			getline(text, Text);//empty line
		}
		else
		{
			//copy this line directly
			fwrite(ReadLine.c_str(), 1, ReadLine.length(), out);
			fwrite("\r\n", 1, 2, out);
		}
	}

	scr.close();
	text.close();
	fclose(out);
	return 0;
}

