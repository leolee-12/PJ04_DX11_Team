#pragma once
#include "Editor_Defines.h"
#include "Camera.h"

NS_BEGIN(Editor)

class CEditCamera final : public CCamera
{
public:
	typedef struct tagEditCameraFreeDesc final : public CCamera::CAMERA_DESC
	{
		_float		fMouseSensor;
	}EDIT_CAMERA_FREE_DESC;

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_EditCameraFree";

private:
	CEditCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEditCamera(const CEditCamera& Prototype);
	virtual ~CEditCamera() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

public:
	void Set_Active(_bool b) { m_bActive = b; }

	void Set_PreviewMode(_bool b);
	void Set_PreviewPose(const _float3& vEye, const _float3& vFwd, const _float3& vUp, _float fFovDeg)
	{
		m_vPrevEye = vEye; m_vPrevFwd = vFwd; m_vPrevUp = vUp; m_fPrevFov = fFovDeg;
	}

private:
	_float  m_fMouseSensor = {};
	_bool   m_bActive = { false };

	_bool   m_bPreview = { false };
	_float3 m_vPrevEye = {};
	_float3 m_vPrevFwd = { 0.f, 0.f, 1.f };
	_float3 m_vPrevUp = { 0.f, 1.f, 0.f };
	_float  m_fPrevFov = { 50.f };
	_float  m_fNearKeep = { 0.1f };
	_float  m_fFarKeep = { 1000.f };

private:
	virtual HRESULT Ready_Events() override { return S_OK; }

public:
	static CEditCamera* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END

