#pragma once

#include "def.h"
#include "psb_link.h"

class psb_compiler_center
{
public:
	psb_compiler_center();
	~psb_compiler_center();

	bool require_compile(Json::Value& root);
	string get_directory(string src_file);
	bool compile();
	bool link();
	bool write_file();

	bool can_load_resource();

//protected:

	Json::Value source_code;
	Json::Value resource_code;
	Json::Reader source_reader;
	Json::Reader resource_reader;

	psb_cc _compiler;
	psb_link _link;

};

extern psb_compiler_center pcc;