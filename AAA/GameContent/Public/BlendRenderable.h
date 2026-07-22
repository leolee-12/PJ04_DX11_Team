#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Client)

// 모델 내부의 반투명 메시를 CWorld_BlendCollector가 대신 그릴 수 있는 객체.
class IBlendRenderable
{
protected:
	virtual ~IBlendRenderable() = default;

public:
	virtual HRESULT Render_BlendMesh(_uint iMeshIndex) = 0;
};

NS_END