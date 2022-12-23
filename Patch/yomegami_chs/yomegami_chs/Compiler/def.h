#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include "../json/json.h"
#include "../Decompiler/psb.hpp"
#include "psb_cc.h"

using namespace std;

#ifdef _WIN32

#define bzero(ptr, size) memset(ptr, 0, size)

#endif