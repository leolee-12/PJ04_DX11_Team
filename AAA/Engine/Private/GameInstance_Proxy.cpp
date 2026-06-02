#include "GameInstance_Proxy.h"
#include "GameInstance.h"
#include "Target_Manager.h"
#include "Light_Manager.h"
#include "Renderer.h"
#include "Shadow_Dir.h"
#include "Effect_Manager.h"
#include "Timer_Manager.h"
#include "Environment_Manager.h"

#pragma region ENGINE
void CGameInstance_Proxy::Update_Engine(_float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;

	m_pOwner->Update_Engine(fTimeDelta);
}

HRESULT CGameInstance_Proxy::Begin_Draw()
{
	if (m_pOwner == nullptr)
		return E_FAIL;

	return m_pOwner->Begin_Draw();
}

HRESULT CGameInstance_Proxy::Draw()
{
	if (m_pOwner == nullptr)
		return E_FAIL;

	return m_pOwner->Draw();
}

HRESULT CGameInstance_Proxy::End_Draw()
{
	if (m_pOwner == nullptr)
		return E_FAIL;

	return m_pOwner->End_Draw();
}

void    CGameInstance_Proxy::Bind_RenderTarget(ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV, _uint w, _uint h)
{
	if (m_pOwner == nullptr)
		return;

	m_pOwner->m_pRenderer->Bind_RenderTarget(pRTV, pDSV, w, h);
}

void    CGameInstance_Proxy::Clear_Resources(_int iLevelIndex)
{
	if (m_pOwner == nullptr)
		return;

	m_pOwner->Clear_Resources(iLevelIndex);
}

_float CGameInstance_Proxy::RandomFloat(_float fMin, _float fMax) const
{
	if (!IsConnected())
		return 0.f;

	return m_pOwner->RandomFloat(fMin, fMax);
}
_int CGameInstance_Proxy::RandomInt(_int iMin, _int iMax) const
{
	if (!IsConnected())
		return 0;

	return m_pOwner->RandomInt(iMin, iMax);
}
#pragma endregion

#pragma region GRAPHICDEVICE
HRESULT CGameInstance_Proxy::Bind_BackBuffer()
{
	if (!IsConnected())
		return E_FAIL;

	return m_pOwner->Bind_BackBuffer();
}

HRESULT CGameInstance_Proxy::OnResize(_uint iWidth, _uint iHeight)
{
	if (!IsConnected())
		return E_FAIL;

	return m_pOwner->OnResize(iWidth, iHeight);
}

_float CGameInstance_Proxy::Get_WindowWidth()
{
	if (!IsConnected())
		return 0.f;

	return m_pOwner->Get_WindowWidth();
}
_float CGameInstance_Proxy::Get_WindowHeight()
{
	if (!IsConnected())
		return 0.f;

	return m_pOwner->Get_WindowHeight();
}
#pragma endregion

#pragma region TIMERMANAGER
_float	CGameInstance_Proxy::Get_TimeDelta(const _wstring& strTimerTag)
{
	if (m_pOwner == nullptr)
		return 0.f;

	return m_pOwner->Get_TimeDelta(strTimerTag);
}

HRESULT CGameInstance_Proxy::Add_Timer(const _wstring& strTimerTag)
{
	if (m_pOwner == nullptr)
		return E_FAIL;

	return m_pOwner->Add_Timer(strTimerTag);
}

void	CGameInstance_Proxy::Compute_Timer(const _wstring& strTimerTag)
{
	if (m_pOwner == nullptr)
		return;

	m_pOwner->Compute_Timer(strTimerTag);
}
_float CGameInstance_Proxy::Get_RawTimeDelta(const _wstring& strTimerTag)
{
	if (m_pOwner == nullptr)
		return 0.f;

	return m_pOwner->m_pTimer_Manager->Get_RawTimeDelta(strTimerTag);
}
void   CGameInstance_Proxy::Set_TimeScale(_float fScale)
{
	if (m_pOwner == nullptr)
		return;

	m_pOwner->m_pTimer_Manager->Set_TimeScale(fScale);
}
_float CGameInstance_Proxy::Get_TimeScale() const
{
	if (m_pOwner == nullptr)
		return 0.f;

	return m_pOwner->m_pTimer_Manager->Get_TimeScale();
}
#pragma endregion

#pragma region LEVELMANAGER
HRESULT CGameInstance_Proxy::Change_Level(_int iNewLevelIndex, CLevel* pNewLevel)
{
	if (m_pOwner == nullptr)
		return E_FAIL;

	return m_pOwner->Change_Level(iNewLevelIndex, pNewLevel);
}
#pragma endregion

#pragma region PROTOTYPEMANAGER
HRESULT CGameInstance_Proxy::Add_Prototype(_uint iLevelIndex, const _wstring& strTag, CBase* pProto)
{
	if (m_pOwner == nullptr)
		return E_FAIL;

	return m_pOwner->Add_Proto(iLevelIndex, strTag, pProto);
}

CBase* CGameInstance_Proxy::Clone_Prototype(PROTOTYPE eType, _uint iLevelIndex, const _wstring& strTag, void* pArg)
{
	if (m_pOwner == nullptr)
		return nullptr;

	return m_pOwner->Clone_Prototype(eType, iLevelIndex, strTag, pArg);
}

_bool CGameInstance_Proxy::Has_Prototype(_uint iLevelIndex, const _wstring& strPrototypeTag)
{
	if (m_pOwner == nullptr)
		return false;

	return m_pOwner->Has_Prototype(iLevelIndex, strPrototypeTag);
}
#pragma endregion

#pragma region OBJECTMANAGER
HRESULT CGameInstance_Proxy::Add_GameObject(_uint iProtoLevel, const _wstring& strProtoTag, _uint iLayerLevel, const _wstring& strLayerTag, const _wstring& strObjectTag, void* pArg)
{
	if (m_pOwner == nullptr)
		return E_FAIL;

	return m_pOwner->Add_GameObject(iProtoLevel, strProtoTag, iLayerLevel, strLayerTag, strObjectTag, pArg);
}

HRESULT CGameInstance_Proxy::Add_GameObject_Return(CGameObject** ppOut, _uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLayerLevelIndex, const _wstring& strLayerTag, const _wstring& strObjectTag, void* pArg)
{
	if (!IsConnected())
		return E_FAIL;

	return m_pOwner->Add_GameObject_Return(ppOut, iPrototypeLevelIndex, strPrototypeTag, iLayerLevelIndex, strLayerTag, strObjectTag, pArg);
}

void CGameInstance_Proxy::Clear_Objects(_int iLevelIndex)
{
	if (!IsConnected())
		return;

	m_pOwner->Clear_Objects(iLevelIndex);
}

void CGameInstance_Proxy::Destroy_GameObject(CGameObject* pGameObject)
{
	if (!IsConnected())
		return;

	m_pOwner->Destroy_GameObject(pGameObject);
}

CGameObject* CGameInstance_Proxy::Find_GameObject(_uint iLevelIndex, const _wstring& strLayerTag, const _wstring& strObjectTag)
{
	if (!IsConnected())
		return nullptr;

	return m_pOwner->Find_GameObject(iLevelIndex, strLayerTag, strObjectTag);
}
#pragma endregion

#pragma region RENDERER
void	CGameInstance_Proxy::Add_RenderGroup(RENDERID eGroupID, CGameObject* pGameObject)
{
	if (m_pOwner == nullptr)
		return;

	m_pOwner->Add_RenderGroup(eGroupID, pGameObject);
}
void CGameInstance_Proxy::Add_RenderGroup_UI(RENDERUIID eGroupID, CUIObject* pUIObject)
{
	if (!IsConnected())
		return;

	m_pOwner->Add_RenderGroup_UI(eGroupID, pUIObject);
}

#ifdef _DEBUG
void CGameInstance_Proxy::Add_DebugComponent(CComponent* pComponent)
{
	if (!IsConnected())
		return;

	m_pOwner->m_pRenderer->Add_DebugComponent(pComponent);
}
void CGameInstance_Proxy::Toggle_DebugRender()
{
	if (!IsConnected())
		return;

	m_pOwner->m_pRenderer->Toggle_DebugRender();
}
#endif
#pragma endregion

#pragma region INPUTMANAGER
_byte CGameInstance_Proxy::Get_DIKeyState(_ubyte byKeyID)
{
	if (!IsConnected())
		return 0;

	return m_pOwner->Get_DIKeyState(byKeyID);
}

_byte CGameInstance_Proxy::Get_DIMouseState(DIMB eMouse)
{
	if (!IsConnected())
		return 0;

	return m_pOwner->Get_DIMouseState(eMouse);
}

_long CGameInstance_Proxy::Get_DIMouseMove(DIMM eMouseState)
{
	if (!IsConnected())
		return 0;

	return m_pOwner->Get_DIMouseMove(eMouseState);
}

void	CGameInstance_Proxy::Disable_InputDeveice()
{
	if (!IsConnected())
		return;

	m_pOwner->Disable_InputDeveice();
}

void	CGameInstance_Proxy::Enable_InputDeveice()
{
	if (!IsConnected())
		return;

	m_pOwner->Enable_InputDeveice();
}

_bool CGameInstance_Proxy::Key_Down(_ubyte byKeyID)
{
	if (!IsConnected())
		return false;

	return m_pOwner->Key_Down(byKeyID);
}
_bool CGameInstance_Proxy::Key_Up(_ubyte byKeyID)
{
	if (!IsConnected())
		return false;

	return m_pOwner->Key_Up(byKeyID);
}
_bool CGameInstance_Proxy::Key_Pressing(_ubyte byKeyID)
{
	if (!IsConnected())
		return false;

	return m_pOwner->Key_Pressing(byKeyID);
}
_bool CGameInstance_Proxy::Mouse_Down(DIMB eMouse)
{
	if (!IsConnected())
		return false;

	return m_pOwner->Mouse_Down(eMouse);
}
_bool CGameInstance_Proxy::Mouse_Up(DIMB eMouse)
{
	if (!IsConnected())
		return false;

	return m_pOwner->Mouse_Up(eMouse);
}
_bool CGameInstance_Proxy::Mouse_Pressing(DIMB eMouse)
{
	if (!IsConnected())
		return false;

	return m_pOwner->Mouse_Pressing(eMouse);
}
POINT CGameInstance_Proxy::Get_MousePos()
{
	if (!IsConnected())
		return POINT{0, 0};

	return m_pOwner->Get_MousePos();
}
#pragma endregion

#pragma region CAMERAMANAGER
const _float4x4* CGameInstance_Proxy::Get_Matrix(D3DTS eState, PROJ_TYPE eType) const
{
	if (!IsConnected())
		return nullptr;

	return m_pOwner->Get_Matrix(eState, eType);
}
const _float4x4* CGameInstance_Proxy::Get_InverseMatrix_Prespec(D3DTS eState) const
{
	if (!IsConnected())
		return nullptr;

	return m_pOwner->Get_InverseMatrix_Prespec(eState);
}
const _float4* CGameInstance_Proxy::Get_CamPosition() const
{
	if (!IsConnected())
		return nullptr;

	return m_pOwner->Get_CamPosition();
}

void CGameInstance_Proxy::Set_Transform(D3DTS eState, PROJ_TYPE eType, _fmatrix StateMatrix)
{
	if (!IsConnected())
		return;

	return m_pOwner->Set_Transform(eState, eType, StateMatrix);
}

void CGameInstance_Proxy::Set_Transform(D3DTS eState, PROJ_TYPE eType, const _float4x4& StateMatrix)
{
	if (!IsConnected())
		return;

	return m_pOwner->Set_Transform(eState, eType, StateMatrix);
}
#pragma endregion

#pragma region LIGHTMANAGER
const LIGHT_DESC* CGameInstance_Proxy::Get_LightDesc(_uint iIndex)
{
	if (!IsConnected()) {
		static LIGHT_DESC EmptyDesc = {};
		return &EmptyDesc;
	}

	return m_pOwner->Get_LightDesc(iIndex);
}

HRESULT CGameInstance_Proxy::Add_Light(const LIGHT_DESC& LightDesc)
{
	if (!IsConnected())
		return E_FAIL;

	return m_pOwner->Add_Light(LightDesc);
}
HRESULT CGameInstance_Proxy::Render_Light(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	if (!IsConnected())
		return E_FAIL;

	return m_pOwner->m_pLight_Manager->Render(pShader, pVIBuffer);
}
HRESULT CGameInstance_Proxy::Clear_Lights()
{
	if (!IsConnected())
		return E_FAIL;

	return m_pOwner->m_pLight_Manager->Clear_Lights();
}
#pragma endregion

#pragma region PICKING
void CGameInstance_Proxy::Compute_PickingRay(_float fNdcX, _float fNdcY, _vector* pOutOrigin, _vector* pOutDir, PROJ_TYPE eType)
{
	if (!IsConnected())
		return;

	m_pOwner->Compute_PickingRay(fNdcX, fNdcY, pOutOrigin, pOutDir, eType);
}

_bool CGameInstance_Proxy::Pick_RayToPlane(_vector vOrigin, _vector vDir, _float3* pOutHit, _float fPlaneY)
{
	if (!IsConnected())
		return false;

	return m_pOwner->Pick_RayToPlane(vOrigin, vDir, pOutHit, fPlaneY);
}

_bool CGameInstance_Proxy::Pick_RayToMesh(_vector vOrigin, _vector vDir, const _float3* pVertices, _uint iVertexCount, const _uint* pIndices, _uint iIndexCount, _float3* pOutHit, _float* pOutDist)
{
	if (!IsConnected())
		return false;

	return m_pOwner->Pick_RayToMesh(vOrigin, vDir, pVertices, iVertexCount, pIndices, iIndexCount, pOutHit, pOutDist);
}
#pragma endregion

#pragma region FONT_MANAGER
HRESULT CGameInstance_Proxy::Add_Font(const _wstring& strFontTag, const _tchar* pFontFilePath)
{
	if (!IsConnected())
		return E_FAIL;

	return m_pOwner->Add_Font(strFontTag, pFontFilePath);
}

HRESULT CGameInstance_Proxy::Draw_Text(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition, _fvector vColor, _float fRotation, const _float2& vScale)
{
	if (!IsConnected())
		return E_FAIL;

	return m_pOwner->Draw_Text(strFontTag, pText, vPosition, vColor, fRotation, vScale);
}

#pragma endregion

#pragma region EVENTBUS
SUBHANDLE CGameInstance_Proxy::Subscribe(const wstring& EventTag, function<void(void*)> Handler)
{
	if (!IsConnected())
	{
		static SUBHANDLE tEmpty = { {}, 0, 0 };
		return tEmpty;
	}

	return m_pOwner->Subscribe(EventTag, Handler);
}
void CGameInstance_Proxy::UnSubscribe(const SUBHANDLE& Handle)
{
	if (!IsConnected())
		return;

	m_pOwner->UnSubscribe(Handle);
}
void CGameInstance_Proxy::Publish(const wstring& EventTag, void* pData)
{
	if (!IsConnected())
		return;

	m_pOwner->Publish(EventTag, pData);
}
void CGameInstance_Proxy::Clear_EventBus()
{
	if (!IsConnected())
		return;

	m_pOwner->Clear_EventBus();
}
#pragma endregion

#pragma region COLLISION_MANAGER
void	CGameInstance_Proxy::Register_Collider(CCollider* pCollider, _uint Group)
{
	if (!IsConnected())
		return;

	m_pOwner->Register_Collider(pCollider, Group);
}
void	CGameInstance_Proxy::Request_Unregister(CCollider* pCollider, _uint Group)
{
	if (!IsConnected())
		return;

	m_pOwner->Request_Unregister(pCollider, Group);
}
void	CGameInstance_Proxy::Immediate_Unregister(CCollider* pCollider, _uint Group)
{
	if (!IsConnected())
		return;

	m_pOwner->Immediate_Unregister(pCollider, Group);
}
void	CGameInstance_Proxy::Add_CollisionPool(_uint SrcGroup, _uint DstGroup)
{
	if (!IsConnected())
		return;

	m_pOwner->Add_CollisionPool(SrcGroup, DstGroup);
}
void	CGameInstance_Proxy::Reset_For_SceneChange()
{
	if (!IsConnected())
		return;

	m_pOwner->Reset_For_SceneChange();
}
void	CGameInstance_Proxy::Clear_CollisionPool()
{
	if (!IsConnected())
		return;

	m_pOwner->Clear_CollisionPool();
}
#pragma endregion

#pragma region SOUND_MANAGER
void CGameInstance_Proxy::Play_Sound(const TCHAR* pSoundKey, _uint iChannelIndex, float fVolume)
{
	if (!IsConnected())
		return;

	m_pOwner->Play_Sound(pSoundKey, iChannelIndex, fVolume);
}
void CGameInstance_Proxy::Play_Sound3D(const TCHAR* pSoundKey, _uint iChannelIndex, float fVolume, _fvector vSoundPos)
{
	if (!IsConnected())
		return;

	m_pOwner->Play_Sound3D(pSoundKey, iChannelIndex, fVolume, vSoundPos);
}
void CGameInstance_Proxy::Play_BGM(const TCHAR* pSoundKey, _uint iChannelIndex, float fVolume)
{
	if (!IsConnected())
		return;

	m_pOwner->Play_BGM(pSoundKey, iChannelIndex, fVolume);
}
void CGameInstance_Proxy::Stop_Sound(_uint iChannelIndex)
{
	if (!IsConnected())
		return;

	m_pOwner->Stop_Sound(iChannelIndex);
}
void CGameInstance_Proxy::Stop_SoundAll()
{
	if (!IsConnected())
		return;

	m_pOwner->Stop_SoundAll();
}
void CGameInstance_Proxy::Set_Channel_Volume(_uint iChannelIndex, float fVolume)
{
	if (!IsConnected())
		return;

	m_pOwner->Set_Channel_Volume(iChannelIndex, fVolume);
}
#pragma endregion

#pragma region TARGET_MANAGER
HRESULT CGameInstance_Proxy::Add_RenderTarget(const _wstring& strTargetTag, _uint iWidth, _uint iHeight, DXGI_FORMAT ePixelFormat, const _float4& vClearColor)
{
	if (!IsConnected())
		return E_FAIL;

	return m_pOwner->Add_RenderTarget(strTargetTag, iWidth, iHeight, ePixelFormat, vClearColor);
}
HRESULT CGameInstance_Proxy::Add_MRT(const _wstring& strMRTTag, const _wstring& strTargetTag)
{
	if (!IsConnected())
		return E_FAIL;

	return m_pOwner->Add_MRT(strMRTTag, strTargetTag);
}
HRESULT CGameInstance_Proxy::Begin_MRT(const _wstring& strMRTTag, ID3D11DepthStencilView* pDSV)
{
	if (!IsConnected())
		return E_FAIL;

	return m_pOwner->Begin_MRT(strMRTTag, pDSV);
}
HRESULT CGameInstance_Proxy::End_MRT()
{
	if (!IsConnected())
		return E_FAIL;

	return m_pOwner->End_MRT();
}

HRESULT CGameInstance_Proxy::Bind_RT_ShaderResource(const _wstring& strTargetTag, CShader* pShader, const _char* pConstantName)
{
	if (!IsConnected())
		return E_FAIL;

	return m_pOwner->m_pTarget_Manager->Bind_ShaderResource(strTargetTag, pShader, pConstantName);
}

#ifdef _DEBUG
HRESULT CGameInstance_Proxy::Ready_RT_Debug(const _wstring& strTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY)
{
	if (!IsConnected())
		return E_FAIL;

	return m_pOwner->Ready_RT_Debug(strTargetTag, fX, fY, fSizeX, fSizeY);
}
HRESULT CGameInstance_Proxy::Render_RT_Debug(const _wstring& strMRTTag, class CShader* pShader, class CVIBuffer_Rect* pVIBuffer)
{
	if (!IsConnected())
		return E_FAIL;

	return m_pOwner->Render_RT_Debug(strMRTTag, pShader, pVIBuffer);
}
#endif

#pragma endregion

#pragma region SHADOW
const _float4x4* CGameInstance_Proxy::Get_Shadow_Transform(D3DTS eState) const
{
	if (!IsConnected())
		return nullptr;

	return m_pOwner->m_pShadow_Dir->Get_Transform(eState);
}

HRESULT CGameInstance_Proxy::Add_ShadowLight(const SHADOW_LIGHT_DESC& ShadowDesc)
{
	if (!IsConnected())
		return E_FAIL;

	return m_pOwner->m_pShadow_Dir->Add_ShadowLight(ShadowDesc);
}
HRESULT CGameInstance_Proxy::Update_ShadowLight(const SHADOW_LIGHT_DESC& ShadowDesc)
{
	if (!IsConnected())
		return E_FAIL;

	return m_pOwner->m_pShadow_Dir->Add_ShadowLight(ShadowDesc);
}
#pragma endregion

#pragma region EFFECT_MANAGER
HRESULT CGameInstance_Proxy::Spawn_Effect(_uint iLevel, const _wstring& strProtoTag, const CEffect::EFFECT_DESC& desc, CEffect** ppOut)
{
	if (!IsConnected())
		return E_FAIL;

	return m_pOwner->m_pEffect_Manager->Spawn(iLevel, strProtoTag, desc, ppOut);
}
CShader* CGameInstance_Proxy::Get_2DShader()
{
	if (!IsConnected())
		return nullptr;

	return m_pOwner->m_pEffect_Manager->Get_2DShader();
}

CShader* CGameInstance_Proxy::Get_MeshShader()
{
	if (!IsConnected())
		return nullptr;

	return m_pOwner->m_pEffect_Manager->Get_MeshShader();
}
#pragma endregion

#pragma region EDITMODE
void CGameInstance_Proxy::Set_EditMode(_bool bEdit)
{
	if (m_pOwner == nullptr) return;
	m_pOwner->Set_EditMode(bEdit);
}

_bool CGameInstance_Proxy::Is_EditMode() const
{
	if (m_pOwner == nullptr) return false;   // 안전 기본값
	return m_pOwner->Is_EditMode();
}
HRESULT CGameInstance_Proxy::Register_Environment(const _wstring& tag, const _tchar* d, const _tchar* s, _float i)
{
	if (!IsConnected())
		return E_FAIL;

	return m_pOwner->m_pEnvironment_Manager->Register(tag, d, s, i);
}
HRESULT CGameInstance_Proxy::Set_CurrentEnvironment(const _wstring& tag)
{
	if (!IsConnected())
		return E_FAIL;

	return m_pOwner->m_pEnvironment_Manager->Set_Current(tag);
}
const ENVIRONMENT_DESC& CGameInstance_Proxy::Get_CurrentEnvironment() const
{
	if (!IsConnected())
	{
		static ENVIRONMENT_DESC empty{};
		return empty;
	}

	return m_pOwner->m_pEnvironment_Manager->Get_Current();
}
#pragma endregion


CGameInstance_Proxy* CGameInstance_Proxy::Create(CGameInstance* pOwner)
{
	if (pOwner == nullptr)
		return nullptr;

	CGameInstance_Proxy* pInstance = new CGameInstance_Proxy;
	pInstance->m_pOwner = pOwner;

	return pInstance;
}