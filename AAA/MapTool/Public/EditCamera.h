#pragma once
#include "MapTool_Defines.h"
#include "Camera.h"

NS_BEGIN(MapTool)

class CEditCamera final : public Engine::CCamera
{
public:
	typedef struct tagEditCameraFreeDesc : public Engine::CCamera::CAMERA_DESC
	{
		_float fMouseSensor;
	} EDIT_CAMERA_FREE_DESC;

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_EditCamera";

private:
	CEditCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEditCamera(const CEditCamera& Prototype);
	virtual ~CEditCamera() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void    Priority_Update(_float fTimeDelta) override;
	virtual void    Update(_float fTimeDelta) override;
	virtual void    Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

public:
	void Set_Active(_bool b) { m_bActive = b; }
	void Reset_Rotation();
	void Jump_Local(_float fForwardDistance, _float fRightDistance);

private:
	_float	m_fMouseSensor = {};
	_bool	m_bActive = { false };
	_float3	m_vDefaultEye = {};
	_float3	m_vDefaultAt = {};

private:
	virtual HRESULT Ready_Events() override { return S_OK; }

public:
	static CEditCamera* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END