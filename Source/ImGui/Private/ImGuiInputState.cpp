// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#include "ImGuiInputState.h"

#include <algorithm>
#include <limits>
#include <type_traits>


FImGuiInputState::FImGuiInputState()
{
	Reset();
}

void FImGuiInputState::AddCharacter(TCHAR Char)
{
	imguiIO->AddInputCharacter(ImGuiInterops::CastInputChar(Char));
}

void FImGuiInputState::SetKeyDown(const FKeyEvent& KeyEvent, bool bIsDown)
{
	const FKey& Key = KeyEvent.GetKey();
	SetKeyDown(Key, bIsDown);
}

void FImGuiInputState::SetKeyDown(const FKey& Key, bool bIsDown)
{
	const ImGuiKey imKey = ImGuiInterops::GetImGuiKey(Key);
	imguiIO->AddKeyEvent(imKey, bIsDown);

	bIsLeftControlDown = imKey == ImGuiKey_LeftCtrl && bIsDown;
	bIsRightControlDown = imKey == ImGuiKey_RightCtrl && bIsDown;
	bIsControlDown = bIsLeftControlDown || bIsRightControlDown;
	
	bIsLeftShiftDown = imKey == ImGuiKey_LeftShift && bIsDown;
	bIsRightShiftDown = imKey == ImGuiKey_RightShift && bIsDown;
	bIsShiftDown = bIsLeftShiftDown || bIsRightShiftDown;
	
	bIsLeftAltDown = imKey == ImGuiKey_LeftAlt && bIsDown;
	bIsRightAltDown = imKey == ImGuiKey_RightAlt && bIsDown;
	bIsAltDown = bIsLeftAltDown || bIsRightAltDown;
	
	bIsLeftCommandDown = imKey == ImGuiKey_LeftSuper && bIsDown;
	bIsRightCommandDown = imKey == ImGuiKey_RightSuper && bIsDown;
	bIsCommandDown = bIsLeftCommandDown || bIsRightCommandDown;
}

void FImGuiInputState::SetMouseDown(const FPointerEvent& MouseEvent, bool bIsDown)
{
	const uint32 mouseIndex = ImGuiInterops::GetMouseIndex(MouseEvent);
	imguiIO->AddMouseButtonEvent(mouseIndex, bIsDown);
}

void FImGuiInputState::SetMouseDown(const FKey& MouseButton, bool bIsDown)
{
	const uint32 mouseIndex = ImGuiInterops::GetMouseIndex(MouseButton);
	imguiIO->AddMouseButtonEvent(mouseIndex, bIsDown);	
}

void FImGuiInputState::AddMouseWheelDelta(float DeltaValue)
{
	imguiIO->AddMouseWheelEvent(0, DeltaValue);
	MouseWheelDelta += DeltaValue;
}

void FImGuiInputState::SetMousePosition(const FVector2D& Position)
{
	imguiIO->AddMousePosEvent(Position.X, Position.Y);
	MousePosition = Position;
}

void FImGuiInputState::SetMousePointer(bool bInHasMousePointer)
{
	imguiIO->MouseDrawCursor = bInHasMousePointer;
	bHasMousePointer = bInHasMousePointer;	
}

void FImGuiInputState::SetTouchDown(bool bIsDown)
{
	imguiIO->AddMouseButtonEvent(0, bIsDown);
	bTouchDown = bIsDown;	
}

void FImGuiInputState::SetTouchPosition(const FVector2D& Position)
{
	imguiIO->AddMousePosEvent(Position.X, Position.Y);
}

void FImGuiInputState::SetGamepadNavigationAxis(const FAnalogInputEvent& AnalogInputEvent, float Value)
{
	if(!AnalogInputEvent.GetKey().IsGamepadKey())
	{
		return;
	}
	
	const auto [Negative, Positive] = ImGuiInterops::GetImGuiNavigationAxis(AnalogInputEvent.GetKey());
	
	static constexpr float AxisInputThreshold = 0.166f;

	// Filter out small values to avoid false positives (helpful in case of worn controllers).
	const float AxisValue = FMath::Max(0.f, Value - AxisInputThreshold);

	if (Value < 0.f)
	{
		imguiIO->AddKeyAnalogEvent(Negative, AxisValue > 0.10f, AxisValue);
		imguiIO->AddKeyAnalogEvent(Positive, false, 0.f);		
	}
	else
	{
		imguiIO->AddKeyAnalogEvent(Positive, AxisValue > 0.10f, AxisValue);
		imguiIO->AddKeyAnalogEvent(Negative, false, 0.f);		
	}
}

void FImGuiInputState::SetKeyboardNavigationEnabled(bool bEnabled)
{
	ImGuiInterops::SetFlag(imguiIO->ConfigFlags, ImGuiConfigFlags_NavEnableKeyboard, bEnabled);
	bKeyboardNavigationEnabled = bEnabled;	
}

void FImGuiInputState::SetGamepadNavigationEnabled(bool bEnabled)
{
	ImGuiInterops::SetFlag(imguiIO->ConfigFlags, ImGuiConfigFlags_NavEnableGamepad, bEnabled);
	bGamepadNavigationEnabled = bEnabled;	
}

void FImGuiInputState::SetGamepad(bool bInHasGamepad)
{
	ImGuiInterops::SetFlag(imguiIO->BackendFlags, ImGuiBackendFlags_HasGamepad, bInHasGamepad);
	bHasGamepad = bInHasGamepad;	
}

void FImGuiInputState::ClearUpdateState()
{
	bTouchProcessed = bTouchDown;
	MouseWheelDelta = 0.0f;
}

void FImGuiInputState::ClearMouseAnalogue()
{
	MousePosition = FVector2D::ZeroVector;
	MouseWheelDelta = 0.0f;
}

void FImGuiInputState::ClearModifierKeys()
{
	bIsLeftControlDown = false;
	bIsRightControlDown = false;
	bIsLeftShiftDown = false;
	bIsRightShiftDown = false;
	bIsLeftAltDown = false;
	bIsRightAltDown = false;
	bIsLeftCommandDown = false;
	bIsRightCommandDown = false;
	
	bIsControlDown = false;
	bIsShiftDown = false;
	bIsAltDown = false;
	bIsCommandDown = false;
}
