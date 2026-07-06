#include "LD_ArrowBoard.h"
#include "LevelDesign_Registry.h"
#include "Parsing_Utils.h"
#include "GameContent_const.h"

#include "GameInstance.h"

namespace
{
	inline constexpr const _char* ARROWBOARD_MODEL_PATH = "../../Resources/Map/Gimmick/Anim/ArrowBoard/ArrowBoard.ysh";
	inline constexpr const _char* ARROWBOARD_ANIM_ATTACKED = "Attacked";
	inline constexpr const _char* ARROWBOARD_ANIM_WAIT = "Wait";
	inline constexpr const _char* ARROWBOARD_DEG_PATH = "Gimmick.ArrowBoard.MainComponent.Deg";
	inline constexpr const _char* ARROWBOARD_ARROW_BONE_NAME = "ArrowDirL";
	inline constexpr _float ARROWBOARD_ANIM_SPEED = 1.5f;
	inline constexpr _float ARROWBOARD_MIN_HURT_RADIUS = 0.1f;
	inline constexpr _float ARROWBOARD_GLOW_PERIOD = 0.3333f;
	inline constexpr _float ARROWBOARD_GLOW_MAX_INTENSITY = 2.f;
	inline constexpr _uint ARROWBOARD_MATERIAL_TEST_PASS = 8u;
}

NS_BEGIN(Client)

CLD_ArrowBoard::CLD_ArrowBoard(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLD_ArrowBoard::CLD_ArrowBoard(const CLD_ArrowBoard& Prototype)
	: CLevelDesignObject(Prototype)
	, m_tArrowBoardDesc(Prototype.m_tArrowBoardDesc)
	, m_fArrowDeg(Prototype.m_fArrowDeg)
{
}

HRESULT CLD_ArrowBoard::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLD_ArrowBoard::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	m_tArrowBoardDesc = *static_cast<const LD_EVENTOBJECT_DESC*>(pArg);
	m_bHurtBoxRegistered = false;
	m_fArrowDeg = 0.f;
	m_fGlowTime = 0.f;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_RenderComponents()))
		return E_FAIL;

	if (FAILED(Validate_Initialized()))
		return E_FAIL;

	JsonUtils::Try_ReadFloat(m_tArrowBoardDesc.jRaw, ARROWBOARD_DEG_PATH, &m_fArrowDeg);
	m_pAnimatorCom->Play(ARROWBOARD_ANIM_WAIT, true, true, 0.f, ARROWBOARD_ANIM_SPEED);

	return Ready_HurtBox();
}

HRESULT CLD_ArrowBoard::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (!JsonUtils::Equals_NoCase(OBJECT_NAME, m_tArrowBoardDesc.strObjectName.c_str()))
		return E_FAIL;

	if (LD_CATEGORY::GIMMICK != m_tArrowBoardDesc.eCategory)
		return E_FAIL;

	if (1u != m_pModelCom->Get_NumMeshes())
		return E_FAIL;

	if (m_pModelCom->Get_AnimationIndex(ARROWBOARD_ANIM_WAIT) < 0)
		return E_FAIL;

	if (m_pModelCom->Get_AnimationIndex(ARROWBOARD_ANIM_ATTACKED) < 0)
		return E_FAIL;

	if (!m_pAnimatorCom->Has_Bone(ARROWBOARD_ARROW_BONE_NAME))
		return E_FAIL;

	return S_OK;
}

void CLD_ArrowBoard::Update(_float fTimeDelta)
{
	if (!m_bActive || Is_Dead())
		return;

	m_fGlowTime += fTimeDelta;
	if (ARROWBOARD_GLOW_PERIOD <= m_fGlowTime)
	{
		m_bGlow = !m_bGlow;
		m_fGlowTime -= ARROWBOARD_GLOW_PERIOD;
	}

	m_pAnimatorCom->SetBoneRotation(ARROWBOARD_ARROW_BONE_NAME, -m_fArrowDeg, XMVectorSet(0.f, 0.f, 1.f, 0.f));

	m_pAnimatorCom->Update(fTimeDelta);

	if (m_pAnimatorCom->Is_Finished())
		m_pAnimatorCom->Play(ARROWBOARD_ANIM_WAIT, true, true, 0.f, ARROWBOARD_ANIM_SPEED);
}

void CLD_ArrowBoard::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (!m_bActive || Is_Dead())
		return;

	if (m_pHurtBox->Is_Enabled())
	{
		m_pHurtBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pHurtBox);
#endif
	}

	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CLD_ArrowBoard::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	return Render_Model();
}

void CLD_ArrowBoard::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

void CLD_ArrowBoard::Damaged(const ATTACK_INFO& tInfo)
{
	UNREFERENCED_PARAMETER(tInfo);

	if (!m_bActive || Is_Dead())
		return;

	m_pAnimatorCom->Play(ARROWBOARD_ANIM_ATTACKED, false, true, 0.f, ARROWBOARD_ANIM_SPEED);
}

void CLD_ArrowBoard::Register_LevelDesignSpecs()
{
	LD_SPAWN_SPEC Spec{};
	Spec.strObjectName = OBJECT_NAME;
	Spec.strPrototypeTag = PROTOTYPE_TAG;
	Spec.strLayerTag = LAYER_TAG;
	Spec.eCategory = LD_CATEGORY::GIMMICK;
	Spec.wstrModelProtoTag = MODEL_PROTO_TAG;
	Spec.eModelType = MODEL::ANIM;
	Spec.pPrototypeFactory = &Create_Prototype;
	Spec.pBuildDesc = &Build_Desc;
	Spec.ModelRequirements =
	{
			{ MODEL_PROTO_TAG, ARROWBOARD_MODEL_PATH, MODEL::ANIM, false },
	};

	CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
}

_bool CLD_ArrowBoard::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
	UNREFERENCED_PARAMETER(jEntry);

	if (nullptr == pOutEntry)
		return false;

	if (!JsonUtils::Equals_NoCase(OBJECT_NAME, CommonDesc.strObjectName.c_str()))
		return false;

	if (Spec.strPrototypeTag != PROTOTYPE_TAG || Spec.strLayerTag != LAYER_TAG)
		return false;

	if (Spec.eCategory != LD_CATEGORY::GIMMICK || Spec.eModelType != MODEL::ANIM || Spec.wstrModelProtoTag != MODEL_PROTO_TAG)
		return false;

	LD_EVENTOBJECT_DESC Desc{};
	static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
	Desc.eCategory = Spec.eCategory;

	*pOutEntry = Desc;
	return true;
}

CGameObject* CLD_ArrowBoard::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLD_ArrowBoard::Create(pDevice, pContext);
}

HRESULT CLD_ArrowBoard::Ready_RenderComponents()
{
	m_pShaderCom = Add_Component<CShader>(Shader_AnimMesh_PBR.iLevelID, Shader_AnimMesh_PBR.szProtoTag, TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(m_tArrowBoardDesc.iModelProtoLevel, MODEL_PROTO_TAG, TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	CAnimator::ANIMATOR_DESC AnimDesc{};
	AnimDesc.pModel = m_pModelCom;

	m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
	if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_ArrowBoard::Ready_HurtBox()
{
	_float3 vMin{};
	_float3 vMax{};
	m_pModelCom->Get_ModelAABB(&vMin, &vMax);

	if (vMin.x > vMax.x || vMin.y > vMax.y || vMin.z > vMax.z)
		return E_FAIL;

	const _float3 vSize =
	{
			vMax.x - vMin.x,
			vMax.y - vMin.y,
			vMax.z - vMin.z
	};

	_float fRadius = vSize.x;
	if (fRadius < vSize.y)
		fRadius = vSize.y;
	if (fRadius < vSize.z)
		fRadius = vSize.z;

	fRadius = max(fRadius * 0.5f, ARROWBOARD_MIN_HURT_RADIUS);

	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = { (vMin.x + vMax.x) * 0.5f, (vMin.y + vMax.y) * 0.5f, (vMin.z + vMax.z) * 0.5f };
	ColliderDesc.fRadius = fRadius;

	m_pHurtBox = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag, TEXT("Com_ArrowBoardHurtBox"), &ColliderDesc);
	if (nullptr == m_pHurtBox)
		return E_FAIL;

	SetUp_HurtBoxCallback();

	m_pGameInstance_Proxy->Register_Collider(m_pHurtBox, ETOUI(COLLISION_LAYER::ENV_HURT));
	m_bHurtBoxRegistered = true;

	return S_OK;
}

HRESULT CLD_ArrowBoard::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaterialID", &m_iMaterialID, sizeof(_uint))))
		return E_FAIL;

	const _float fGlowIntensity = m_bGlow ? ARROWBOARD_GLOW_MAX_INTENSITY : 0.f;
	const _float4 vEmissiveColor = { fGlowIntensity, fGlowIntensity * 0.8f, fGlowIntensity * 0.05f, 0.f };
	return m_pShaderCom->Bind_RawValue("g_vEmissiveColor", &vEmissiveColor, sizeof(_float4));
}

HRESULT CLD_ArrowBoard::Render_Model()
{
	if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", 0u, MTEX_TYPE::DIFFUSE, 0u)))
		return E_FAIL;

	if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", 0u, MTEX_TYPE::NORMALS, 0u)))
		return E_FAIL;

	if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", 0u, MTEX_TYPE::METALNESS, 0u)))
		return E_FAIL;

	if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", 0u)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(ARROWBOARD_MATERIAL_TEST_PASS)))
		return E_FAIL;

	return m_pModelCom->Render(0u);
}

void CLD_ArrowBoard::SetUp_HurtBoxCallback()
{
	if (nullptr == m_pHurtBox)
		return;

	m_pHurtBox->Set_OnEnter(
		[this](CCollider* pOther)
		{
			Handle_HurtBox(pOther);
		});
}

void CLD_ArrowBoard::Handle_HurtBox(CCollider* pOther)
{
	if (nullptr == pOther)
		return;

	if (ETOUI(COLLISION_LAYER::CAR_BOOST) != pOther->Get_RegisteredGroup())
		return;

	ATTACK_INFO AttackInfo{};
	AttackInfo.pAttacker = pOther->Get_Owner();
	Damaged(AttackInfo);
}

void CLD_ArrowBoard::Unregister_HurtBox(_bool bImmediate)
{
	if (nullptr == m_pHurtBox)
		return;

	m_pHurtBox->Set_Enabled(false);

	if (!m_bHurtBoxRegistered || nullptr == m_pGameInstance_Proxy)
		return;

	const _uint iGroup = ETOUI(COLLISION_LAYER::ENV_HURT);

	if (bImmediate)
		m_pGameInstance_Proxy->Immediate_Unregister(m_pHurtBox, iGroup);
	else
		m_pGameInstance_Proxy->Request_Unregister(m_pHurtBox, iGroup);

	m_bHurtBoxRegistered = false;
}

CLD_ArrowBoard* CLD_ArrowBoard::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLD_ArrowBoard* pInstance = new CLD_ArrowBoard(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLD_ArrowBoard");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLD_ArrowBoard::Clone(void* pArg)
{
	CLD_ArrowBoard* pInstance = new CLD_ArrowBoard(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLD_ArrowBoard");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLD_ArrowBoard::Free()
{
	Unregister_HurtBox(true);

	__super::Free();
}

NS_END