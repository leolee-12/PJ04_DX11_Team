#ifndef Engine_Define_h__
#define Engine_Define_h__

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <d3dcompiler.h>

#include <fx11/d3dx11effect.h>
#include <DirectTK/DDSTextureLoader.h>
#include <DirectTK/WICTextureLoader.h>
#include <DirectTK/SpriteBatch.h>
#include <DirectTK/SpriteFont.h>
#include <DirectTK/ScreenGrab.h>
#include <DirectTK/PrimitiveBatch.h>
#include <DirectTK/VertexTypes.h>
#include <DirectTK/Effects.h>
#include "nlohmann/json.hpp"

namespace Engine {
	using json = nlohmann::ordered_json;
}

using namespace DirectX;


#include <vector>
#include <list>
#include <map>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <functional>
#include <string>
#include <ctime>
#include <random>

using namespace std;

#include "Engine_Enum.h"
#include "Engine_Macro.h"
#include "Engine_Struct.h"
#include "Engine_Typedef.h"
#include "Engine_Function.h"
#include "Engine_Model_Data.h"

namespace Engine
{
	static const _wstring g_strTransformTag = TEXT("Com_Transform");
	static const _uint g_iNumMeshBones = { 512 };

	//const unsigned int g_iMaxWidth = 2560;
	//const unsigned int g_iMaxHeight = 1440;
	// const unsigned int g_iMaxWidth = 1280;
	// const unsigned int g_iMaxHeight = 720;

	/*
	const unsigned int g_iMaxWidth = 16384;
	const unsigned int g_iMaxHeight = 9216;
	*/
	const unsigned int g_iMaxWidth = 8192;
	const unsigned int g_iMaxHeight = 4608;
}

#define DIRECTINPUT_VERSION	0x0800
#include <dinput.h>

#pragma warning(disable : 4251)

#ifdef _DEBUG

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifndef DBG_NEW 

#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ ) 
#define new DBG_NEW 

#endif
#endif


using namespace Engine;

#endif // Engine_Define_h__
