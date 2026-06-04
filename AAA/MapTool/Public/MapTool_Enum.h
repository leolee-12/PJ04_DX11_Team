#pragma once

namespace MapTool
{
    enum class TOOL_LEVEL { STATIC, LOADING, EDIT, END };

    enum class LOG_LEVEL { INFO, WARNING, ERROR_, END };

    // 기즈모 조작 모드 (ImGuizmo::OPERATION 의존 없이 공유 상태로 보관하기 위한 툴 로컬 enum)
    enum class GIZMO_OP { TRANSLATE, ROTATE, SCALE, END };
}
