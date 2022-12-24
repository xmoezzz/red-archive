#include "SecondStreamDecoder.h"
#include "dragon.h"
#include "GlobalMap.h"
#include "NodeKeyDef.h"

#define GetTableKeyName(x) SecondTable_##x

Void NTAPI DragonDecoder(PBYTE Buffer, ULONG Length, DWORD Key, BYTE Selector)
{
	PBYTE        InitKey, BaseKey;
	PDWORD       dwBaseKey;
	drECRYPT_ctx S[1];
	

	switch (Selector)
	{
	case 0:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_000);
		break;

	case 1:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_001);
		break;

	case 2:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_002);
		break;

	case 3:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_003);
		break;

	case 4:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_004);
		break;

	case 5:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_005);
		break;

	case 6:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_006);
		break;

	case 7:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_007);
		break;

	case 8:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_008);
		break;

	case 9:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_009);
		break;

	case 10:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_010);
		break;

	case 11:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_011);
		break;

	case 12:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_012);
		break;

	case 13:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_013);
		break;

	case 14:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_014);
		break;

	case 15:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_015);
		break;

	case 16:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_016);
		break;

	case 17:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_017);
		break;

	case 18:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_018);
		break;

	case 19:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_019);
		break;

	case 20:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_020);
		break;

	case 21:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_021);
		break;

	case 22:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_022);
		break;

	case 23:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_023);
		break;

	case 24:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_024);
		break;

	case 25:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_025);
		break;

	case 26:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_026);
		break;

	case 27:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_027);
		break;

	case 28:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_028);
		break;

	case 29:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_029);
		break;

	case 30:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_030);
		break;

	case 31:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_031);
		break;

	case 32:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_032);
		break;

	case 33:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_033);
		break;

	case 34:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_034);
		break;

	case 35:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_035);
		break;

	case 36:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_036);
		break;

	case 37:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_037);
		break;

	case 38:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_038);
		break;

	case 39:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_039);
		break;

	case 40:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_040);
		break;

	case 41:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_041);
		break;

	case 42:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_042);
		break;

	case 43:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_043);
		break;

	case 44:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_044);
		break;

	case 45:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_045);
		break;

	case 46:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_046);
		break;

	case 47:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_047);
		break;

	case 48:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_048);
		break;

	case 49:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_049);
		break;

	case 50:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_050);
		break;

	case 51:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_051);
		break;

	case 52:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_052);
		break;

	case 53:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_053);
		break;

	case 54:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_054);
		break;

	case 55:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_055);
		break;

	case 56:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_056);
		break;

	case 57:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_057);
		break;

	case 58:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_058);
		break;

	case 59:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_059);
		break;

	case 60:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_060);
		break;

	case 61:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_061);
		break;

	case 62:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_062);
		break;

	case 63:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_063);
		break;

	case 64:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_064);
		break;

	case 65:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_065);
		break;

	case 66:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_066);
		break;

	case 67:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_067);
		break;

	case 68:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_068);
		break;

	case 69:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_069);
		break;

	case 70:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_070);
		break;

	case 71:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_071);
		break;

	case 72:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_072);
		break;

	case 73:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_073);
		break;

	case 74:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_074);
		break;

	case 75:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_075);
		break;

	case 76:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_076);
		break;

	case 77:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_077);
		break;

	case 78:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_078);
		break;

	case 79:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_079);
		break;

	case 80:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_080);
		break;

	case 81:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_081);
		break;

	case 82:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_082);
		break;

	case 83:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_083);
		break;

	case 84:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_084);
		break;

	case 85:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_085);
		break;

	case 86:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_086);
		break;

	case 87:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_087);
		break;

	case 88:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_088);
		break;

	case 89:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_089);
		break;

	case 90:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_090);
		break;

	case 91:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_091);
		break;

	case 92:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_092);
		break;

	case 93:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_093);
		break;

	case 94:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_094);
		break;

	case 95:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_095);
		break;

	case 96:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_096);
		break;

	case 97:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_097);
		break;

	case 98:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_098);
		break;

	case 99:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_099);
		break;

	case 100:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_100);
		break;

	case 101:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_101);
		break;

	case 102:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_102);
		break;

	case 103:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_103);
		break;

	case 104:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_104);
		break;

	case 105:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_105);
		break;

	case 106:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_106);
		break;

	case 107:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_107);
		break;

	case 108:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_108);
		break;

	case 109:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_109);
		break;

	case 110:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_110);
		break;

	case 111:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_111);
		break;

	case 112:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_112);
		break;

	case 113:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_113);
		break;

	case 114:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_114);
		break;

	case 115:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_115);
		break;

	case 116:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_116);
		break;

	case 117:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_117);
		break;

	case 118:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_118);
		break;

	case 119:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_119);
		break;

	case 120:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_120);
		break;

	case 121:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_121);
		break;

	case 122:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_122);
		break;

	case 123:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_123);
		break;

	case 124:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_124);
		break;

	case 125:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_125);
		break;

	case 126:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_126);
		break;

	case 127:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_127);
		break;

	case 128:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_128);
		break;

	case 129:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_129);
		break;

	case 130:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_130);
		break;

	case 131:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_131);
		break;

	case 132:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_132);
		break;

	case 133:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_133);
		break;

	case 134:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_134);
		break;

	case 135:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_135);
		break;

	case 136:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_136);
		break;

	case 137:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_137);
		break;

	case 138:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_138);
		break;

	case 139:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_139);
		break;

	case 140:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_140);
		break;

	case 141:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_141);
		break;

	case 142:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_142);
		break;

	case 143:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_143);
		break;

	case 144:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_144);
		break;

	case 145:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_145);
		break;

	case 146:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_146);
		break;

	case 147:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_147);
		break;

	case 148:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_148);
		break;

	case 149:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_149);
		break;

	case 150:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_150);
		break;

	case 151:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_151);
		break;

	case 152:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_152);
		break;

	case 153:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_153);
		break;

	case 154:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_154);
		break;

	case 155:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_155);
		break;

	case 156:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_156);
		break;

	case 157:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_157);
		break;

	case 158:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_158);
		break;

	case 159:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_159);
		break;

	case 160:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_160);
		break;

	case 161:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_161);
		break;

	case 162:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_162);
		break;

	case 163:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_163);
		break;

	case 164:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_164);
		break;

	case 165:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_165);
		break;

	case 166:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_166);
		break;

	case 167:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_167);
		break;

	case 168:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_168);
		break;

	case 169:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_169);
		break;

	case 170:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_170);
		break;

	case 171:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_171);
		break;

	case 172:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_172);
		break;

	case 173:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_173);
		break;

	case 174:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_174);
		break;

	case 175:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_175);
		break;

	case 176:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_176);
		break;

	case 177:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_177);
		break;

	case 178:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_178);
		break;

	case 179:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_179);
		break;

	case 180:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_180);
		break;

	case 181:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_181);
		break;

	case 182:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_182);
		break;

	case 183:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_183);
		break;

	case 184:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_184);
		break;

	case 185:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_185);
		break;

	case 186:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_186);
		break;

	case 187:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_187);
		break;

	case 188:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_188);
		break;

	case 189:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_189);
		break;

	case 190:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_190);
		break;

	case 191:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_191);
		break;

	case 192:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_192);
		break;

	case 193:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_193);
		break;

	case 194:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_194);
		break;

	case 195:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_195);
		break;

	case 196:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_196);
		break;

	case 197:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_197);
		break;

	case 198:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_198);
		break;

	case 199:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_199);
		break;

	case 200:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_200);
		break;

	case 201:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_201);
		break;

	case 202:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_202);
		break;

	case 203:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_203);
		break;

	case 204:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_204);
		break;

	case 205:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_205);
		break;

	case 206:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_206);
		break;

	case 207:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_207);
		break;

	case 208:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_208);
		break;

	case 209:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_209);
		break;

	case 210:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_210);
		break;

	case 211:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_211);
		break;

	case 212:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_212);
		break;

	case 213:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_213);
		break;

	case 214:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_214);
		break;

	case 215:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_215);
		break;

	case 216:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_216);
		break;

	case 217:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_217);
		break;

	case 218:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_218);
		break;

	case 219:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_219);
		break;

	case 220:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_220);
		break;

	case 221:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_221);
		break;

	case 222:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_222);
		break;

	case 223:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_223);
		break;

	case 224:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_224);
		break;

	case 225:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_225);
		break;

	case 226:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_226);
		break;

	case 227:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_227);
		break;

	case 228:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_228);
		break;

	case 229:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_229);
		break;

	case 230:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_230);
		break;

	case 231:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_231);
		break;

	case 232:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_232);
		break;

	case 233:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_233);
		break;

	case 234:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_234);
		break;

	case 235:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_235);
		break;

	case 236:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_236);
		break;

	case 237:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_237);
		break;

	case 238:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_238);
		break;

	case 239:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_239);
		break;

	case 240:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_240);
		break;

	case 241:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_241);
		break;

	case 242:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_242);
		break;

	case 243:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_243);
		break;

	case 244:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_244);
		break;

	case 245:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_245);
		break;

	case 246:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_246);
		break;

	case 247:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_247);
		break;

	case 248:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_248);
		break;

	case 249:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_249);
		break;

	case 250:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_250);
		break;

	case 251:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_251);
		break;

	case 252:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_252);
		break;

	case 253:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_253);
		break;

	case 254:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_254);
		break;

	case 255:
		InitKey = (PBYTE)GlobalMap::GetGlobalMap()->FindNode(SecondTable_255);
		break;
	}

	if (!InitKey)
		Ps::ExitProcess(0);

	BaseKey   = (PBYTE)AllocStack(128);
	dwBaseKey = (PDWORD)BaseKey;
	for (ULONG i = 0; i < 128 / sizeof(DWORD); i++)
	{
		DWORD Temp = InitKey[4 * i + 0] * InitKey[4 * i + 1] * InitKey[4 * i + 2] * InitKey[4 * i + 3];
		dwBaseKey[i] = Key * Temp;
	}

	drECRYPT_keysetup(S, BaseKey, 256, 256);
	drECRYPT_ivsetup (S, BaseKey + 32); 
	RtlZeroMemory(BaseKey, 128);
	
	PBYTE OutputMem = (PBYTE)AllocateMemoryP(Length);

	drECRYPT_process_bytes(0, S, Buffer, OutputMem, Length);
	RtlCopyMemory(Buffer, OutputMem, Length);
	RtlFillMemory(OutputMem, Length, 0xCC);
	FreeMemoryP(OutputMem);
	RtlZeroMemory(S, sizeof(S[0]));
}
