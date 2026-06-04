#pragma once

#include "EnvObject_Defines.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

class CLIENT_DLL CEnvObject abstract : public CGameObject
{
public:
	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_EnvObject";

protected:
	CEnvObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnvObject(const CEnvObject& Prototype);
	virtual ~CEnvObject() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

protected:
	HRESULT Ready_RenderComponents(_uint iModelProtoLevel, const wstring& strModelProtoTag);
	HRESULT Bind_ShaderResources();
	HRESULT Render_Model();
	void	Update_LocalBounds();
	void	Refresh_WorldBounds();
	_bool	Is_VisibleInCurrentView() const;
	_bool	Has_RenderModel() const { return nullptr != m_pModelCom; }

protected:
	ENV_OBJECT_DESC	m_tDesc = {};
	wstring			m_strProtoTag = { PROTOTYPE_TAG };
	CShader*		m_pShaderCom = { nullptr };
	CModel*			m_pModelCom = { nullptr };
	BoundingBox		m_LocalBounds = {};
	BoundingBox		m_WorldBounds = {};
	_bool			m_bRenderable = { true };
	_bool			m_bEnableCulling = { true };
	_bool			m_bVisible = { true };
	_bool			m_bDebugDraw = { false };

private:
	void Apply_TransformFromDesc();
	void Apply_DescDefaults();

protected:
	virtual void Free() override;
};

NS_END
