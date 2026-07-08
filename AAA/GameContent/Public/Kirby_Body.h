#pragma once

#include "Kirby_Deform_Model.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

enum class KIRBY_MESH
{
	BODY_BIG, BODY, BODY_VACUUM,
	MOUTH_ANGRY, MOUTH_NORMAL, MOUTH_OPEN, MOUTH_SMILE_CLOSE, MOUTH_SMILE_OPEN,
	LIMBS
};
enum class KIRBY_BODY_STATE { NORMAL, STUFFED, INHALE, END };
enum class KIRBY_MOUTH_STATE { IDLE, OPEN, ANGRY, SMILE_OPEN, SMILE_CLOSE, END };

class CKirby_Body final : public CKirby_Deform_Model
{
	GENERATED_BODY(CKirby_Body)

public:
	struct KIRBY_BODY_DESC : public CKirby_Deform_Model::KIRBY_DEFORM_MODEL_DESC
	{
	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Kirby_Body";
	static constexpr const wchar_t* Kirby_PartTag = L"Body";

private:
	CKirby_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirby_Body(const CKirby_Body& Prototype);
	virtual ~CKirby_Body() = default;

private:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual HRESULT Render() override;
	virtual HRESULT Render_Shadow() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

	virtual HRESULT Ready_AnimEvents(CKirby* pKirby) override;

public:
	void Set_KirbyBody(KIRBY_BODY_STATE eState) { m_eBody = eState; }
	void Set_KirbyMouth(KIRBY_MOUTH_STATE eState) { m_eMouth = eState; }

	KIRBY_BODY_STATE Get_KirbyBody() const { return m_eBody; }
	KIRBY_MOUTH_STATE Get_KirbyMouth() const { return m_eMouth; }

private:
	HRESULT Ready_Components();

	HRESULT Set_VisibleMeshes();
	virtual HRESULT Render_KirbyMesh(_uint iMeshIndex);

private:
	vector<_bool>	m_VisibleMeshes;

	KIRBY_BODY_STATE m_eBody{};
	KIRBY_MOUTH_STATE m_eMouth{};

public:
	static CKirby_Body* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
private:
	virtual void Free();
};

NS_END
