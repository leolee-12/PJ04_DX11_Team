#include "GigatzoBullet.h"
#include "GameInstance.h"
#include "GameContent_const.h"
#include "Effect_Loader.h"

CGigatzoBullet::CGigatzoBullet(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CProjectile{ pDevice, pContext }
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	m_fSpeed = 18.f;
	m_fLifeTime = 4.5f;
	m_fDamage = 1.f;
	m_fHitRadius = 2.5f;
}

CGigatzoBullet::CGigatzoBullet(const CGigatzoBullet& Prototype)
	: CProjectile (Prototype)
{
}

void CGigatzoBullet::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);   
    if (!m_bAlive)
        return;

    if (m_pAnimatorCom)
    {
        m_pAnimatorCom->Update(fTimeDelta);
        if (m_bIntro && m_pAnimatorCom->Is_Finished())
        {
            m_pAnimatorCom->Play("Move", true, true);
            m_bIntro = false;
        }
    }
}

HRESULT CGigatzoBullet::Render()
{
    if (!m_bAlive)
        return S_OK;
    if (nullptr == m_pModelCom || nullptr == m_pShaderCom)
        return S_OK;

    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", 0, MTEX_TYPE::DIFFUSE, 0)))
        return E_FAIL;
    if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", 0, MTEX_TYPE::NORMALS, 0)))
        return E_FAIL;
    if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", 0, MTEX_TYPE::METALNESS, 0)))
        return E_FAIL;
    if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", 0)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Begin(1)))
        return E_FAIL;
    if (FAILED(m_pModelCom->Render(0)))
        return E_FAIL;
    return S_OK;
}

HRESULT CGigatzoBullet::Render_Shadow()
{
    if (!m_bAlive)
        return S_OK;
    if (nullptr == m_pModelCom || nullptr == m_pShaderCom)
        return S_OK;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrixPtr())))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::VIEW))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::PROJ))))
        return E_FAIL;

    if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", 0)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Begin(7)))     
        return E_FAIL;
    if (FAILED(m_pModelCom->Render(0)))
        return E_FAIL;
    return S_OK;
}

void CGigatzoBullet::Launch(const _float3& vPos, const _float3& vDir)
{
    __super::Launch(vPos, vDir);   

    _vector vUp = XMVector3Normalize(XMLoadFloat3(&vDir));

    _vector vRef = XMVectorSet(0.f, 0.f, 1.f, 0.f);
    if (fabsf(XMVectorGetX(XMVector3Dot(vUp, vRef))) > 0.99f)
        vRef = XMVectorSet(1.f, 0.f, 0.f, 0.f);

    _vector vRight = XMVector3Normalize(XMVector3Cross(vRef, vUp));
    _vector vLook = XMVector3Cross(vRight, vUp);

    _float3 vScaled = m_pTransformCom->Get_Scaled();
    m_pTransformCom->Set_State(STATE::RIGHT, XMVectorSetW(vRight * vScaled.x, 0.f));
    m_pTransformCom->Set_State(STATE::UP, XMVectorSetW(vUp * vScaled.y, 0.f));
    m_pTransformCom->Set_State(STATE::LOOK, XMVectorSetW(vLook * vScaled.z, 0.f));
}

void CGigatzoBullet::Configure(_float fSpeed, _float fDamage, _float fLifeSec)
{
    m_fSpeed = fSpeed;
    m_fDamage = fDamage;
    m_fLifeTime = fLifeSec;
}

HRESULT CGigatzoBullet::Ready_Visual()
{
    m_pShaderCom = Add_Component<CShader>(Shader_AnimMesh_PBR.iLevelID,
        Shader_AnimMesh_PBR.szProtoTag, TEXT("Com_Shader"));
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, MODEL_PROTO_TAG, TEXT("Com_Model"));
    if (nullptr == m_pModelCom)
        return E_FAIL;

    CAnimator::ANIMATOR_DESC ad{};
    ad.pModel = m_pModelCom;
    m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
    if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&ad)))
        return E_FAIL;

    return S_OK;
}

void CGigatzoBullet::On_Activated()
{
    m_bIntro = true;
    if (m_pAnimatorCom)
        m_pAnimatorCom->Play("MoveStart", false, true);
}

void CGigatzoBullet::On_Impact()
{
    // Effect Ãß°¡


    Kill();
}

HRESULT CGigatzoBullet::Bind_ShaderResources()
{
    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;
    return S_OK;
}

CGigatzoBullet* CGigatzoBullet::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CGigatzoBullet* pInstance = new CGigatzoBullet(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CGigatzoBullet");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CGigatzoBullet::Clone(void* pArg)
{
    CGigatzoBullet* pInstance = new CGigatzoBullet(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CGigatzoBullet");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CGigatzoBullet::Free()
{
    __super::Free();
}
