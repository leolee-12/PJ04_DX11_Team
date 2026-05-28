#pragma once

#include "Engine_Defines.h"
#include "GameInstance_Proxy.h"
#include "Prototype_Manager.h"

NS_BEGIN(Engine)

class CGraphic_Device;
class CTimer_Manager;
class CLevel_Manager;
class CPrototype_Manager;
class CObject_Manager;
class CRenderer;
class CLevel;
class CGameObject;
class CBase;
class CCamera_Manager;
class CInput_Device;
class CLight_Manager;
class CFont_Manager;
class CUIObject;
class CEventBus;
class CCollision_Manager;
class CSound_Manager;
class CTarget_Manager;
class CShadow_Dir;
class CEffect_Manager;

class ENGINE_DLL CGameInstance 
{
	friend class CGameInstance_Proxy;

private:										
	CGameInstance();
	CGameInstance(const CGameInstance&) = delete;					
	CGameInstance& operator = (const CGameInstance&) = delete;
	~CGameInstance() = default;

#pragma region ENGINE
public:
	static HRESULT Initialize_Engine(const ENGINE_DESC& EngineDesc, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext);
	static void	   DestroyInstance();
	static CGameInstance_Proxy* GetProxy() 
	{ 
		m_pGameInstance_Proxy->AddRef();
		return m_pGameInstance_Proxy;
	}

private:
	void Update_Engine(_float fTimeDelta);
	HRESULT Begin_Draw();
	HRESULT Draw();
	HRESULT End_Draw();
	void Clear_Resources(_int iLevelIndex);

	_float RandomFloat(_float fMin, _float fMax) const;
	_int   RandomInt(_int iMin, _int iMax) const;
#pragma endregion

#pragma region DEVICE
	HRESULT Bind_BackBuffer();
	HRESULT OnResize(_uint iWidth, _uint iHeight);
	_float Get_WindowWidth();
	_float Get_WindowHeight();
#pragma endregion

#pragma region TIMER_MANAGER
private:
	_float Get_TimeDelta(const _wstring& strTimerTag);
	HRESULT	Add_Timer(const _wstring& strTimerTag);
	void Compute_Timer(const _wstring& strTimerTag);
#pragma endregion

#pragma region LEVEL_MANAGER
	HRESULT Change_Level(_int iNewLevelIndex, CLevel* pNewLevel);
#pragma endregion

#pragma region PROTOTYPE_MANAGER
	HRESULT Add_Proto(_uint iLevelIndex, const _wstring& strPrototypeTag, CBase* pPrototype);
	CBase* Clone_Prototype(PROTOTYPE eType, _uint iLevelIndex, const _wstring& strPrototypeTag, void* pArg = nullptr);
	_bool Has_Prototype(_uint iLevelIndex, const _wstring& strPrototypeTag);
#pragma endregion

#pragma region OBJECT_MANAGER
	HRESULT Add_GameObject(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLayerLevelIndex, const _wstring& strLayerTag, const _wstring& strObjectTag, void* pArg);
	HRESULT Add_GameObject_Return (CGameObject** ppOut, _uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLayerLevelIndex, const _wstring& strLayerTag, const _wstring& strObjectTag, void* pArg);
	void	Clear_Objects(_int iLevelIndex);
	void    Destroy_GameObject(CGameObject* pGameObject);
	CGameObject* Find_GameObject(_uint iLevelIndex, const _wstring& strLayerTag, const _wstring& strObjectTag);
#pragma endregion

#pragma region RENDERER
	void Add_RenderGroup(RENDERID eGroupID, CGameObject* pGameObject);
	void Add_RenderGroup_UI(RENDERUIID eGroupID, CUIObject* pUIObject);
#pragma endregion

#pragma region INPUT_DEVICE
	_byte	Get_DIKeyState(_ubyte byKeyID);
	_byte	Get_DIMouseState(DIMB eMouse);
	_long	Get_DIMouseMove(DIMM eMouseState);
	void	Disable_InputDeveice();
	void	Enable_InputDeveice();
	_bool Key_Down(_ubyte byKeyID);
	_bool Key_Up(_ubyte byKeyID);
	_bool Key_Pressing(_ubyte byKeyID);
	_bool Mouse_Down(DIMB eMouse);
	_bool Mouse_Up(DIMB eMouse);
	_bool Mouse_Pressing(DIMB eMouse);
	POINT Get_MousePos();
#pragma endregion

#pragma region CAMERA_MANAGER
	const _float4x4* Get_Matrix(D3DTS eState, PROJ_TYPE eType) const;
	const _float4x4* Get_InverseMatrix_Prespec(D3DTS eState) const;
	const _float4* Get_CamPosition() const;
	void Set_Transform(D3DTS eState, PROJ_TYPE eType, _fmatrix StateMatrix);
	void Set_Transform(D3DTS eState, PROJ_TYPE eType, const _float4x4& StateMatrix);
#pragma endregion

#pragma region LIGHT_MANAGER
	const LIGHT_DESC* Get_LightDesc(_uint iIndex);
	HRESULT Add_Light(const LIGHT_DESC& LightDesc);
#pragma endregion

#pragma region PICKING
	void Compute_PickingRay(
		_float      fNdcX,
		_float      fNdcY,
		_vector* pOutOrigin,
		_vector* pOutDir,
		PROJ_TYPE  eType = PROJ_TYPE::PERSPEC
	);

	_bool Pick_RayToPlane(
		_vector vOrigin,
		_vector vDir,
		_float3* pOutHit,
		_float    fPlaneY = 0.f
	);

	_bool Pick_RayToMesh(
		_vector       vOrigin,
		_vector       vDir,
		const _float3* pVertices, _uint iVertexCount,
		const _uint* pIndices, _uint iIndexCount,
		_float3* pOutHit,
		_float* pOutDist = nullptr
	);
#pragma endregion

#pragma region FONT_MANAGER
	HRESULT Add_Font(const _wstring& strFontTag, const _tchar* pFontFilePath);
	HRESULT Draw_Text(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition,
		_fvector vColor = XMVectorSet(1.f, 1.f, 1.f, 1.f), _float fRotation = 0.f, const _float2& vScale = _float2(1.f, 1.f));
#pragma endregion

#pragma region EVENTBUS
	SUBHANDLE Subscribe(const wstring& EventTag, function<void(void*)> Handler);
	void	  Publish(const wstring& EventTag, void* pData);
	void	  UnSubscribe(const SUBHANDLE& Handle);
	void	  Clear_EventBus();
#pragma endregion

#pragma region COLLISION_MANAGER
	void	Register_Collider(CCollider* pCollider, _uint Group);
	void	Request_Unregister(CCollider* pCollider, _uint Group);
	void	Immediate_Unregister(CCollider* pCollider, _uint Group);
	void	Add_CollisionPool(_uint SrcGroup, _uint DstGroup);
	void	Reset_For_SceneChange();
	void	Clear_CollisionPool();
#pragma endregion
	
#pragma region SOUND_MANAGER
	void Play_Sound(const TCHAR* pSoundKey, _uint iChannelIndex, float fVolume);
	void Play_Sound3D(const TCHAR* pSoundKey, _uint iChannelIndex, float fVolume, _fvector vSoundPos);
	void Play_BGM(const TCHAR* pSoundKey, _uint iChannelIndex, float fVolume);
	void Stop_Sound(_uint iChannelIndex);
	void Stop_SoundAll();
	void Set_Channel_Volume(_uint iChannelIndex, float fVolume);
#pragma endregion

#pragma region TARGET_MANAGER
	HRESULT Add_RenderTarget(const _wstring& strTargetTag, _uint iWidth, _uint iHeight, DXGI_FORMAT ePixelFormat, const _float4& vClearColor);
	HRESULT Add_MRT(const _wstring& strMRTTag, const _wstring& strTargetTag);
	HRESULT Begin_MRT(const _wstring& strMRTTag, ID3D11DepthStencilView* pDSV);
	HRESULT End_MRT();

#ifdef _DEBUG
public:
	HRESULT Ready_RT_Debug(const _wstring& strTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY);
	HRESULT Render_RT_Debug(const _wstring& strMRTTag, class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
#endif
#pragma endregion



private:
	CGraphic_Device*			m_pGraphic_Device = { nullptr };
	CTimer_Manager*				m_pTimer_Manager = { nullptr };
	CLevel_Manager*				m_pLevel_Manager = { nullptr };
	CPrototype_Manager*			m_pPrototype_Manager = { nullptr };
	CObject_Manager*			m_pObject_Manager = { nullptr };
	CRenderer*					m_pRenderer = { nullptr };
	CCamera_Manager*			m_pCamera_Manager = { nullptr };
	CInput_Device*				m_pInput_Device = { nullptr };
	CLight_Manager*				m_pLight_Manager = { nullptr };
	CFont_Manager*				m_pFont_Manager = { nullptr };
	CEventBus*					m_pEventBus = { nullptr };
	CCollision_Manager*			m_pCollision_Manager = { nullptr };
	CSound_Manager*				m_pSound_Manager = { nullptr };
	CTarget_Manager*			m_pTarget_Manager = { nullptr };
	CShadow_Dir*				m_pShadow_Dir = { nullptr };
	CEffect_Manager*			m_pEffect_Manager = { nullptr };

	mutable mt19937             m_RandomGenerator;

private:
	static CGameInstance* m_pInstance;
	static CGameInstance_Proxy* m_pGameInstance_Proxy;

private:
	static CGameInstance* GetInstance();
	void Free();
};

NS_END