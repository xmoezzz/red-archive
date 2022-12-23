#include "psb_link_resource.h"
#include "../my.h"


psb_link_resource::psb_link_resource()
{
}


psb_link_resource::~psb_link_resource()
{
}

bool psb_link_resource::compile()
{
	for (uint32_t i = 0; i < resource_table.size();i++)
	{
		chunk_offsets.push_back(chunk_data.get_length());
		chunk_lengths.push_back(resource_table[i].length);
		chunk_data.append(resource_table[i].data, resource_table[i].length);
	}

	chunk_offsets.compile();
	chunk_lengths.compile();

	return true;
}

bool psb_link_resource::load_resource(Json::Value& resource_code,string _res_path)
{
	for (uint32_t i = 0; i < resource_code.size(); i++)
	{
		if (!load_file(i, resource_code[i].asString()))
		{
			return false;
		}
	}

	return true;
}

bool psb_link_resource::load_file(uint32_t i, string filename)
{
	WCHAR  FilePath[260];

	RtlZeroMemory(FilePath, 260 * 2);
	MultiByteToWideChar(CP_UTF8, 0, filename.c_str(), filename.length(), FilePath, 260);
	
	psb_resource item;
	fstream res(FilePath, ios::in | ios::binary);
	if(!res.is_open())
	{
		return false;
	}

	res.seekg(0, ios::end);
	uint32_t length = (uint32_t)res.tellg();
	res.seekg(0, ios::beg);

	item.data = new unsigned char[length];
	item.length = length;
	item.id = i;

	res.read((char*)item.data, item.length);

	resource_table.push_back(item);

	return true;
}