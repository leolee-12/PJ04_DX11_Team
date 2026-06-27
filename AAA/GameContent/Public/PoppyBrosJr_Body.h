#pragma once
#include "MonsterPart.h"

NS_BEGIN(Engine)
class CTexture;
NS_END

NS_BEGIN(Client)

class CPoppyBrosJr_Body final : public CMonsterPart
{
	GENERATED_BODY(CPoppyBrosJr_Body)

public:
	struct POPPYBROSJR_BODY_DESC : public CMonsterPart::MONSTERPART_DESC
	{

	};

	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_PoppyBrosJr_Body";

private:
	CPoppyBrosJr_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPoppyBrosJr_Body(const CPoppyBrosJr_Body& Prototype);
	virtual ~CPoppyBrosJr_Body() = default;

private:
	virtual HRESULT				Initialize_Prototype() override;
	virtual HRESULT				Initialize(void* pArg) override;

	virtual void				Update(_float fTimeDelta) override;
	virtual HRESULT				Render() override;                  // 특수 렌더 필요 시 자식이 오버라이드

public:
	virtual void				Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

	void						Set_Eye(_uint iIndex) { m_iEyeIndex = (iIndex < EYE_COUNT) ? iIndex : 0; }
	_uint						Get_Eye() const { return m_iEyeIndex; }

private:
	CTexture*					m_pEyeTextureCom = { nullptr };
	static constexpr _uint		EYE_COUNT = { 11 };
	_uint						m_iEyeIndex = { 0 };

	static constexpr _uint		s_iFaceMeshIndex = { 1 };

private:
	virtual HRESULT				Ready_Components() override;

public:
	static CPoppyBrosJr_Body*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;

protected:
	virtual void				Free() override;
};

NS_END