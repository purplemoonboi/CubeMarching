#pragma once

/** core */
#include "Core/Layer/Layer.h"
#include "Core/Log/Log.h"
#include "Core/Application/Application.h"
#include "Core/Core.h"

/** time */
#include "Core/Time/DeltaTime.h"
#include "Core/Time/AppTimeManager.h"

/** input */
#include "Core/Input/Input.h"
#include "Core/Input/KeyCodes.h"
#include "Core/Input/MouseButtonCodes.h"

/** rendering */
#include "Framework/Renderer/Api/RendererAPI.h"
#include "Renderer/Renderer3D/RenderInstruction.h"
#include "Renderer/Renderer3D/Renderer3D.h"
#include "Renderer/Resources/Shader.h"
#include "Renderer/Buffers/FrameBuffer.h"

/** events */
#include <Framework/Core/Events/MouseEvent.h>
#include <Framework/Core/Events/AppEvents.h>
#include <Framework/Core/Events/KeyEvent.h>

/** components */
//#include "Camera/MainCamera.h"

/** scene */
#include "Framework/Scene/Scene.h"

/** imgui */