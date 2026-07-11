#pragma once
#include "Property.h"

namespace Engine
{
	using EFFECTPART_ENUM_ITEM = pair<int, wstring>;
	using EFFECTPART_ENUM_ITEMS = vector<EFFECTPART_ENUM_ITEM>;

	inline const EFFECTPART_ENUM_ITEMS& Get_PropertyEnum_RenderID()
	{
		static const EFFECTPART_ENUM_ITEMS Items =
		{
			{ static_cast<int>(RENDERID::PRIORITY),  L"PRIORITY" },
			{ static_cast<int>(RENDERID::SHADOW),    L"SHADOW" },
			{ static_cast<int>(RENDERID::NONBLEND),  L"NONBLEND" },
			{ static_cast<int>(RENDERID::DECAL),     L"DECAL" },
			{ static_cast<int>(RENDERID::NONLIGHT),  L"NONLIGHT" },
			{ static_cast<int>(RENDERID::BLEND),     L"BLEND" },
			{ static_cast<int>(RENDERID::BLEND_HDR), L"BLEND_HDR" },
			{ static_cast<int>(RENDERID::OCCLUSION), L"OCCLUSION" },
		};

		return Items;
	}

	inline const EFFECTPART_ENUM_ITEMS& Get_PropertyEnum_EffectShaderPass()
	{
		static const EFFECTPART_ENUM_ITEMS Items =
		{
			{ 0, L"Default" },
			{ 1, L"AlphaBlend" },
			{ 2, L"Additive" },
		};

		return Items;
	}

	inline const EFFECTPART_ENUM_ITEMS& Get_PropertyEnum_Sampler()
	{
		static const EFFECTPART_ENUM_ITEMS Items =
		{
			{ 0, L"Default" },
			{ 1, L"Mirror" },
		};

		return Items;
	}

	inline const EFFECTPART_ENUM_ITEMS& Get_PropertyEnum_DepthMode()
	{
		static const EFFECTPART_ENUM_ITEMS Items =
		{
			{ 0, L"Default" },
			{ 1, L"Ignore" },
		};

		return Items;
	}

	inline const EFFECTPART_ENUM_ITEMS& Get_PropertyEnum_MaskBlendMode()
	{
		static const EFFECTPART_ENUM_ITEMS Items =
		{
			{ 0, L"Multiply" },
			{ 1, L"Add" },
			{ 2, L"Subtract" },
			{ 3, L"Replace" },
		};

		return Items;
	}

	inline const EFFECTPART_ENUM_ITEMS& Get_PropertyEnum_MaskChannel()
	{
		static const EFFECTPART_ENUM_ITEMS Items =
		{
			{ 0, L"RGBA" },
			{ 1, L"Red" },
			{ 2, L"Green" },
			{ 3, L"Blue" },
			{ 4, L"Alpha" },
		};

		return Items;
	}

	inline const EFFECTPART_ENUM_ITEMS& Get_PropertyEnum_EffectShape()
	{
		static const EFFECTPART_ENUM_ITEMS Items =
		{
			{ 0, L"Point" },
			{ 1, L"Sphere" },
			{ 2, L"Circle" },
			{ 3, L"Box" },
		};

		return Items;
	}

	inline const EFFECTPART_ENUM_ITEMS& Get_PropertyEnum_BoxSpawnMode()
	{
		static const EFFECTPART_ENUM_ITEMS Items =
		{
			{ 0, L"Volume" },
			{ 1, L"Top" },
		};

		return Items;
	}

	inline const EFFECTPART_ENUM_ITEMS& Get_PropertyEnum_EffectVelocityMode()
	{
		static const EFFECTPART_ENUM_ITEMS Items =
		{
			{ 0, L"Shape Outward" },
			{ 1, L"Direction" },
			{ 2, L"Fountain" },
		};

		return Items;
	}

	inline const EFFECTPART_ENUM_ITEMS& Get_PropertyEnum_UVAxis()
	{
		static const EFFECTPART_ENUM_ITEMS Items =
		{
			{ 0, L"U / X" },
			{ 1, L"V / Y" },
		};

		return Items;
	}

	inline const wstring* Find_EffectPartEnumName(const EFFECTPART_ENUM_ITEMS& Items, int iValue)
	{
		for (const auto& Item : Items)
		{
			if (Item.first == iValue)
				return &Item.second;
		}

		return nullptr;
	}

	inline const EFFECTPART_ENUM_ITEMS* Find_EffectPartPropertyEnum(const FPROPERTY& Property)
	{
		const wstring& Name = Property.strName;
		const wstring& Category = Property.strCategory;

		if (Name == L"Render Group" && Category == L"Rendering")
			return &Get_PropertyEnum_RenderID();

		if (Category == L"Effect")
		{
			if (Name == L"Shader Pass")
				return &Get_PropertyEnum_EffectShaderPass();
			if (Name == L"Mirror")
				return &Get_PropertyEnum_Sampler();
			if (Name == L"Depth Ignore")
				return &Get_PropertyEnum_DepthMode();
		}

		if (Category == L"Mask Com")
		{
			if (Name == L"Blend Mode_M")
				return &Get_PropertyEnum_MaskBlendMode();
			if (Name == L"Channel_M")
				return &Get_PropertyEnum_MaskChannel();
		}

		if (Name == L"Shape Type_P" && Category == L"Particle Shape")
			return &Get_PropertyEnum_EffectShape();
		if (Name == L"Shape Type_E" && Category == L"Emitter Shape")
			return &Get_PropertyEnum_EffectShape();

		if (Name == L"Box Spawn Mode_P" && Category == L"Particle Shape - Box")
			return &Get_PropertyEnum_BoxSpawnMode();
		if (Name == L"Box Spawn Mode_E" && Category == L"Emitter Shape - Box")
			return &Get_PropertyEnum_BoxSpawnMode();

		if (Name == L"Velocity Mode_P" && Category == L"Particle Velocity")
			return &Get_PropertyEnum_EffectVelocityMode();
		if (Name == L"Velocity Mode_E" && Category == L"Emitter Velocity")
			return &Get_PropertyEnum_EffectVelocityMode();

		if ((Name == L"Linear Axis_T" && Category == L"Texture Com") ||
			(Name == L"Linear Axis_M" && Category == L"Mask Com") ||
			(Name == L"Linear Axis_D" && Category == L"Diffuse") ||
			(Name == L"Linear Axis_U" && Category == L"Unknown"))
		{
			return &Get_PropertyEnum_UVAxis();
		}

		return nullptr;
	}
}
