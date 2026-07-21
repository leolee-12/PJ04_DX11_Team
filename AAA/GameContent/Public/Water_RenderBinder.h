#pragma once
#include "Water_Defines.h"

NS_BEGIN(Engine)
class CShader;
NS_END

NS_BEGIN(Client)

CLIENT_DLL HRESULT Bind_WaterRenderDesc(CShader* pShader, const WATER_RENDER_DESC& Desc, _double dGameTime);
CLIENT_DLL void Sanitize_WaterRenderDesc(WATER_RENDER_DESC* pDesc);

NS_END