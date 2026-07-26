#pragma once
#include "LevelDesignObject.h"
#include "BlendRenderable.h"
#include "MeshLayer_Binder.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CTexture;
NS_END

NS_BEGIN(Client)
struct LD_SPAWN_SPEC;
class CWorld_BlendCollector;

class CLD_Frame final
	: public CLevelDesignObject
	, public IBlendRenderable
{
	GENERATED_BODY(CLD_Frame)

	PROPERTY(_bool, m_bCutReset, L"Cut Reset", L"Frame")	// 체크하면 첫 컷으로. 즉시 자동 해제

public:
	static constexpr const _tchar* OBJECT_NAME = L"CreditKirbyHouseFrame";
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_Frame";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Proto_Component_Model_Frame";
	static constexpr const _tchar* LAYER_TAG = L"Layer_LevelDesign_Gimmick";
	static constexpr const _char* MODEL_PATH = "../../Resources/Map/Gimmick/NonAnim/CreditKirbyHouseFrame/CreditKirbyHouseFrame.ysh";
	static constexpr const _tchar * CUT_TEXTURE_PATH = L"../../Resources/Map/Gimmick/NonAnim/CreditKirbyHouseFrame/_TP_TexturePattern_1.%02d.dds";
	static constexpr _uint CUT_TEXTURE_COUNT = 13u;	//	_1.00 ~ _1.12

	static constexpr _float START_TIMMER = 27.f;

	static constexpr _int CREDIT_FIRST_CUT = 1;    // 첫 크레딧을 띄울 컷 인덱스
	static constexpr _int CREDIT_CUT_STRIDE = 2;   // 이후 몇 컷마다 넘길지
	static constexpr _int CREDIT_COUNT = 4;        // 총 몇 장 보여줄지
	static constexpr _int CREDIT_CLOSE_CUT = 9;

private:
	CLD_Frame(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLD_Frame(const CLD_Frame& Prototype);
	virtual ~CLD_Frame() = default;

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Validate_Initialized() override;

public:
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	virtual HRESULT Render_BlendMesh(_uint iMeshIndex) override;
	virtual HRESULT Apply_EditMeshLayer(_uint iModelSlot, _uint iMesh, const MESH_LAYER_IDX& Layer) override;

	static void Register_LevelDesignSpecs();
	static _bool Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
	static CGameObject* Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CTexture* m_pCutTextureCom = { nullptr };
	CWorld_BlendCollector* m_pBlendCollector = { nullptr };

	LD_STATIC_MODEL_DESC m_tStaticModelDesc = {};
	vector<_uint> m_BlendMeshIndices;
	_float m_fCutCursor = { 0.f };

	_float m_fFirstWait = { 0.f };
	_bool  m_bStarted = { false };

	_float m_fCutHold = { 4.f };
	_float m_fCutFade = { 1.2f };

	_int  m_iLastCutIndex = { 0 };
	_int  m_iCreditFired = { 0 };
	_bool m_bCreditClosed = { false };

private:
	HRESULT Ready_RenderComponents();
	HRESULT Bind_ShaderResources();
	HRESULT Render_Mesh(_uint iMeshIndex, MESH_LAYER_RENDER_KIND eKind);
	void Cache_BlendMeshIndices();
	void Submit_BlendMeshes();
	void Tick_CreditSignal();
	HRESULT Bind_CutTextures();

public:
	static CLD_Frame* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END