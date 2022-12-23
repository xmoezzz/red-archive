#include <iostream>
#include <fstream>
#include <string>
#include <direct.h>
#include "../json/json.h"
#include "psb.hpp"
#include "compress.h"
#include "../my.h"
#include "../zlib/zlib.h"
#include "../zlib/zconf.h"

using namespace std;

bool
traversal_object_tree(psb_t& psb,
	const psb_objects_t *objects, string object_name, Json::Value &root);

bool
traversal_offsets_tree(psb_t& psb,
	const psb_collection_t *offsets, string entry_name, Json::Value &root) {
	psb_value_t *value = NULL;

	for (uint32_t i = 0; i < offsets->size(); i++) {
		unsigned char* entry_buff = offsets->get(i);
		psb.unpack(value, entry_buff);


		if (value != NULL) {
			//=======================================================
			if (value->get_type() == psb_value_t::TYPE_COLLECTION) {

				Json::Value node(Json::arrayValue);
				auto ret = traversal_offsets_tree(psb, (const psb_collection_t *)value, entry_name, node);
				if (!ret)
				{
					node.clear();
					return ret;
				}
				root.append(node);

			}
			//=======================================================
			if (value->get_type() == psb_value_t::TYPE_OBJECTS) {

				Json::Value node(Json::objectValue);
				auto ret = traversal_object_tree(psb, (const psb_objects_t *)value, entry_name, node);
				if (!ret)
				{
					node.clear();
					return ret;
				}
				root.append(node);
			}

			if (value->get_type() == psb_value_t::TYPE_TRUE || value->get_type() == psb_value_t::TYPE_FALSE) {

				Json::Value node(Json::booleanValue);
				psb_boolean_t *psb_boolean = (psb_boolean_t*)value;
				node = psb_boolean->get_boolean();
				root.append(node);
			}

			//=======================================================
			if (value->get_type() == psb_value_t::TYPE_STRING_N1) {

				Json::Value node(Json::stringValue);
				psb_string_t *psb_string = (psb_string_t*)value;
				node = psb_string->get_string();
				root.append(node);
			}
			//=======================================================
			if (value->get_type() == psb_value_t::TYPE_NUMBER_N0 || value->get_type() == psb_value_t::TYPE_NUMBER_N1 ||
				value->get_type() == psb_value_t::TYPE_NUMBER_N2 || value->get_type() == psb_value_t::TYPE_NUMBER_N3 ||
				value->get_type() == psb_value_t::TYPE_NUMBER_N4) {

				Json::Value node(Json::intValue);
				psb_number_t *number = (psb_number_t*)value;
				node = number->get_integer();
				root.append(node);
			}


			if (value->get_type() == psb_value_t::TYPE_FLOAT0 || value->get_type() == psb_value_t::TYPE_FLOAT || value->get_type() == psb_value_t::TYPE_DOUBLE) {

				Json::Value node(Json::realValue);

				psb_number_t *number = (psb_number_t*)value;
				if (value->get_type() == psb_value_t::TYPE_FLOAT || value->get_type() == psb_value_t::TYPE_FLOAT0) {
					node = number->get_float();
				}
				if (value->get_type() == psb_value_t::TYPE_DOUBLE) {
					node = number->get_double();
				}

				root.append(node);
			}

			if (value->get_type() == psb_value_t::TYPE_NUMBER_N5 || value->get_type() == psb_value_t::TYPE_NUMBER_N6 ||
				value->get_type() == psb_value_t::TYPE_NUMBER_N7 || value->get_type() == psb_value_t::TYPE_NUMBER_N8) {

				Json::Value node(Json::intValue);
				psb_number_t *number = (psb_number_t*)value;
				node = number->get_integer();
				root.append(node);
			}

			if (value->get_type() == psb_value_t::TYPE_NULL) {
				Json::Value node(Json::nullValue);
				root.append(node);
			}
			if (value->get_type() == psb_value_t::TYPE_RESOURCE_N1 || value->get_type() == psb_value_t::TYPE_RESOURCE_N2 ||
				value->get_type() == psb_value_t::TYPE_RESOURCE_N3 || value->get_type() == psb_value_t::TYPE_RESOURCE_N4) {
				psb_resource_t *resource = (psb_resource_t *)value;
				Json::Value node(Json::stringValue);
				char temp[32];
				_itoa_s(resource->get_index(), temp, 10);
				node = "#resource#" + (string)temp;
				root.append(node);
			}
		}

		else {
			//printf("invalid_type:%s,%02X\n", entry_name.c_str(), entry_buff[0]);
			root.clear();
			return false;
		}
	}
	return true;
}

bool
traversal_object_tree(psb_t& psb,
	const psb_objects_t *objects, string object_name, Json::Value &root) {
	psb_value_t *value = NULL;
	for (uint32_t i = 0; i < objects->size(); i++) {
		string entry_name = objects->get_name(i);
		unsigned char* entry_buff = objects->get_data(i);

		psb.unpack(value, entry_buff);

		if (value != NULL) {
			//=======================================================

			if (value->get_type() == psb_value_t::TYPE_COLLECTION) {

				Json::Value node(Json::arrayValue);
				auto ret = traversal_offsets_tree(psb, (const psb_collection_t *)value, entry_name, node);
				if (!ret)
				{
					return ret;
					node.clear(); 
				}
				root[entry_name] = node;
			}
			//=======================================================

			if (value->get_type() == psb_value_t::TYPE_OBJECTS) {

				Json::Value node(Json::objectValue);
				auto ret = traversal_object_tree(psb, (const psb_objects_t *)value, entry_name, node);
				if (!ret)
				{
					return ret;
					node.clear();
				}
				root[entry_name] = node;
			}

			if (value->get_type() == psb_value_t::TYPE_TRUE || value->get_type() == psb_value_t::TYPE_FALSE) {

				Json::Value node(Json::booleanValue);
				psb_boolean_t *psb_boolean = (psb_boolean_t*)value;
				node = psb_boolean->get_boolean();
				root[entry_name] = node;
			}

			//=======================================================

			if (value->get_type() == psb_value_t::TYPE_STRING_N1) {

				Json::Value node(Json::stringValue);
				psb_string_t *psb_string = (psb_string_t*)value;
				node = psb_string->get_string();
				root[entry_name] = node;
			}
			//=======================================================
			if (value->get_type() == psb_value_t::TYPE_NUMBER_N0 || value->get_type() == psb_value_t::TYPE_NUMBER_N1 ||
				value->get_type() == psb_value_t::TYPE_NUMBER_N2 || value->get_type() == psb_value_t::TYPE_NUMBER_N3 ||
				value->get_type() == psb_value_t::TYPE_NUMBER_N4) {

				Json::Value node(Json::intValue);
				psb_number_t *number = (psb_number_t*)value;
				node = number->get_integer();
				root[entry_name] = node;
			}

			if (value->get_type() == psb_value_t::TYPE_FLOAT0 || value->get_type() == psb_value_t::TYPE_FLOAT || value->get_type() == psb_value_t::TYPE_DOUBLE) {

				Json::Value node(Json::realValue);
				
				psb_number_t *number = (psb_number_t*)value;
				if (value->get_type() == psb_value_t::TYPE_FLOAT || value->get_type() == psb_value_t::TYPE_FLOAT0) {
					node = number->get_float();
				}
				if (value->get_type() == psb_value_t::TYPE_DOUBLE) {
					node = number->get_double();
				}
				root[entry_name] = node;
			}

			if (value->get_type() == psb_value_t::TYPE_NUMBER_N5 || value->get_type() == psb_value_t::TYPE_NUMBER_N6 ||
				value->get_type() == psb_value_t::TYPE_NUMBER_N7 || value->get_type() == psb_value_t::TYPE_NUMBER_N8) {

				Json::Value node(Json::intValue);
				psb_number_t *number = (psb_number_t*)value;
				node = number->get_integer();
				root[entry_name] = node;
			}

			//=======================================================
			if (value->get_type() == psb_value_t::TYPE_NULL) {
				Json::Value node(Json::nullValue);
				root[entry_name] = node;
			}

			if (value->get_type() == psb_value_t::TYPE_RESOURCE_N1 || value->get_type() == psb_value_t::TYPE_RESOURCE_N2 ||
				value->get_type() == psb_value_t::TYPE_RESOURCE_N3 || value->get_type() == psb_value_t::TYPE_RESOURCE_N4) {
				psb_resource_t *resource = (psb_resource_t *)value;

				Json::Value node(Json::stringValue);
				char temp[32];
				_itoa_s(resource->get_index(), temp, 10);
				node = "#resource#" + (string)temp;
				root[entry_name] = node;
			}
		}
		else {
			root.clear();
			return false;
		}
	}
	return true;
}

#include <shlobj.h>

bool export_res(psb_t &psb, LPCWSTR source_name, Json::Value& ResourceJson)
{
	Json::Value res(Json::arrayValue);

	wstring PathName;

	WCHAR CurDir[MAX_PATH];
	RtlZeroMemory(CurDir, CONST_STRSIZE(CurDir));

	Nt_GetExeDirectory(CurDir, MAX_PATH);
	
	PathName = CurDir;
	PathName += L"\\Project\\";
	PathName += source_name;
	PathName += L"_resource";

	if (Nt_GetFileAttributes(PathName.c_str()) == 0xFFFFFFFF)
	{
		auto Ret = SHCreateDirectory(0, PathName.c_str());
		if (Ret != ERROR_SUCCESS)
			return false;
	}
	
	for (uint32_t i = 0; i < psb.chunk_offsets->size(); i++)
	{
		WCHAR filename[256];
		CHAR  UTF8Name[1000];

		RtlZeroMemory(UTF8Name, CONST_STRSIZE(UTF8Name));

		uint32_t offset = psb.chunk_offsets->get(i);
		uint32_t length = psb.chunk_lengths->get(i);

		swprintf_s(filename, L"%s\\%d", PathName.c_str(), i);

		fstream output(filename, ios::trunc | ios::binary | ios::out);
		if (output.is_open())
		{
			output.write((const char*)psb.chunk_data + offset, length);
			output.flush();
			output.close();

			WideCharToMultiByte(CP_UTF8, 0, filename, StrLengthW(filename), UTF8Name, CONST_STRSIZE(UTF8Name), 0, 0);

			res[i] = UTF8Name;
		}
	}

	ResourceJson = res;
	return true;
}

struct MDFHDR {
	unsigned char signature[4];
	unsigned long size;
};

NTSTATUS DecompilePSBFile(PBYTE Buffer, ULONG Size, LPCWSTR FileName, Json::Value& DecompiledResult, Json::Value& ResourceResult)
{
	Json::Value root;
	MDFHDR      hdr;
	PBYTE       PsbBuffer;
	ULONG       PsbSize;

	PsbBuffer = NULL;

	if (strncmp((const char *)Buffer, "mdf", 3) == 0)
	{
		memcpy(&hdr, Buffer, sizeof(hdr));
		PsbBuffer = new unsigned char[hdr.size];
		PsbSize = hdr.size;

		if (uncompress(PsbBuffer, &PsbSize, &Buffer[sizeof(hdr)], Size - sizeof(hdr)) != Z_OK) {
			delete[] PsbBuffer;
			return false;
		}

		Size   = PsbSize;
		Buffer = PsbBuffer;
	}

	psb_t psb((unsigned char*)Buffer);
	const psb_objects_t *objects = psb.get_objects();

	auto Ret = traversal_object_tree(psb, objects, "", root);
	if (!Ret)
		return STATUS_UNSUCCESSFUL;

	DecompiledResult = root;
	export_res(psb, FileName, ResourceResult);

	if (PsbBuffer)
		delete[] PsbBuffer;

	return STATUS_SUCCESS;
}
