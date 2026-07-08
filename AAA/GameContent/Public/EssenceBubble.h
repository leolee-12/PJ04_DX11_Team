#pragma once
#include "Ability_Bubble.h"

NS_BEGIN(Client)

class CEssenceBubble final : public CAbility_Bubble
{
	GENERATED_BODY(CEssenceBubble)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_EssenceBubble";
	
private:
	CEssenceBubble(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEssenceBubble(const CEssenceBubble& Prototype);
	virtual ~CEssenceBubble() = default;

public:
	virtual void				Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
	{
		pOut->strPrototypeTag = PROTOTYPE_TAG;
	}

public:
	static CEssenceBubble*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;

protected:
	virtual void				Free() override;

};

NS_END
