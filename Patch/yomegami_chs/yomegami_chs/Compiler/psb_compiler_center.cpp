#include "psb_compiler_center.h"
#include <direct.h>
psb_compiler_center pcc;


psb_compiler_center::psb_compiler_center()
{
}


psb_compiler_center::~psb_compiler_center()
{
}
string psb_compiler_center::get_directory(string src_file)
{
	char path[512];
	strcpy_s(path,src_file.c_str());
	char* pos = strrchr(path,'\\');
	if(pos != NULL){
		pos++;
		*pos = 0;
	}else{
		_getcwd(path,sizeof(path));
		strcat_s(path,"\\");
	}
	return path;
}

bool psb_compiler_center::can_load_resource()
{
	return true;
}
bool psb_compiler_center::require_compile(Json::Value& root)
{
	source_code = root;
	return true;
}

bool psb_compiler_center::compile()
{
	return _compiler.cc(source_code);
}

bool psb_compiler_center::link()
{
	return _link.link(_compiler, resource_code, NULL);
}

bool psb_compiler_center::write_file()
{
	return true;
}
