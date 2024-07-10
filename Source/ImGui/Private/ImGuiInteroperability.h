// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#pragma once

#include "TextureManager.h"

#include <GenericPlatform/ICursor.h>
#include <Input/Events.h>

#include <imgui.h>


// If TCHAR is wider than ImWchar, enable or disable validation of input character before conversions.
#define VALIDATE_INPUT_CHARACTERS !UE_BUILD_SHIPPING

#if VALIDATE_INPUT_CHARACTERS
DEFINE_LOG_CATEGORY_STATIC(LogImGuiInput, Warning, All);
#endif

class FImGuiInputState;

// Utilities to help standardise operations between Unreal and ImGui.
namespace ImGuiInterops
{
	struct ImGuiGamepadKeyRange
	{
		ImGuiKey Negative;
		ImGuiKey Positive;
	};
	
	//====================================================================================================
	// Character conversion
	//====================================================================================================

	template<typename T, std::enable_if_t<(sizeof(T) <= sizeof(ImWchar)), T>* = nullptr>
	ImWchar CastInputChar(T Char)
	{
		return static_cast<ImWchar>(Char);
	}

	template<typename T, std::enable_if_t<!(sizeof(T) <= sizeof(ImWchar)), T>* = nullptr>
	ImWchar CastInputChar(T Char)
	{
#if VALIDATE_INPUT_CHARACTERS
		// We only need a runtime validation if TCHAR is wider than ImWchar.
		// Signed and unsigned integral types with the same size as ImWchar should be safely converted. As long as the
		// char value is in that range we can safely use it, otherwise we should log an error to notify about possible
		// truncations.
		static constexpr auto MinLimit = (std::numeric_limits<std::make_signed_t<ImWchar>>::min)();
		static constexpr auto MaxLimit = (std::numeric_limits<std::make_unsigned_t<ImWchar>>::max)();
		UE_CLOG(!(Char >= MinLimit && Char <= MaxLimit), LogImGuiInput, Error,
			TEXT("TCHAR value '%c' (%#x) is out of range %d (%#x) to %u (%#x) that can be safely converted to ImWchar. ")
			TEXT("If you wish to disable this validation, please set VALIDATE_INPUT_CHARACTERS in ImGuiInputState.cpp to 0."),
			Char, Char, MinLimit, MinLimit, MaxLimit, MaxLimit);
#endif

		return static_cast<ImWchar>(Char);
	}	

	//====================================================================================================
	// Input State Copying
	//====================================================================================================

	template<typename TFlags, typename TFlag>
	static FORCEINLINE constexpr void SetFlag(TFlags& Flags, TFlag Flag, bool bSet)
	{
		Flags = bSet ? Flags | Flag : Flags & ~Flag;
	}	
	
	//====================================================================================================
	// Input Mapping
	//====================================================================================================

	// Set in ImGui IO mapping to recognize indices generated from Unreal input events.
	void SetUnrealKeyMap();
	ImGuiKey GetImGuiKey(const FKey& Key);

	// Map FKey to index in keys buffer.
	uint32 GetKeyIndex(const FKey& Key);

	// Map key event to index in keys buffer.
	uint32 GetKeyIndex(const FKeyEvent& KeyEvent);

	// Map mouse FKey to index in mouse buttons buffer.
	uint32 GetMouseIndex(const FKey& MouseButton);

	ImGuiGamepadKeyRange GetImGuiNavigationAxis(const FKey& Key);
	
	// Map pointer event to index in mouse buttons buffer.
	FORCEINLINE uint32 GetMouseIndex(const FPointerEvent& MouseEvent)
	{
		return GetMouseIndex(MouseEvent.GetEffectingButton());
	}

	// Convert from ImGuiMouseCursor type to EMouseCursor.
	EMouseCursor::Type ToSlateMouseCursor(ImGuiMouseCursor MouseCursor);

	//====================================================================================================
	// Input State Copying
	//====================================================================================================

	// Copy input to ImGui IO.
	// @param IO - Target ImGui IO
	// @param InputState - Input state to copy
	void CopyInput(ImGuiIO& IO, const FImGuiInputState& InputState);


	//====================================================================================================
	// Conversions
	//====================================================================================================

	// Convert from ImGui packed color to FColor.
	FORCEINLINE FColor UnpackImU32Color(ImU32 Color)
	{
		// We use IM_COL32_R/G/B/A_SHIFT macros to support different ImGui configurations.
		return FColor{ (uint8)((Color >> IM_COL32_R_SHIFT) & 0xFF), (uint8)((Color >> IM_COL32_G_SHIFT) & 0xFF),
			(uint8)((Color >> IM_COL32_B_SHIFT) & 0xFF), (uint8)((Color >> IM_COL32_A_SHIFT) & 0xFF) };
	}

	// Convert from ImVec4 rectangle to FSlateRect.
	FORCEINLINE FSlateRect ToSlateRect(const ImVec4& ImGuiRect)
	{
		return FSlateRect{ ImGuiRect.x, ImGuiRect.y, ImGuiRect.z, ImGuiRect.w };
	}

	// Convert from ImVec2 rectangle to FVector2D.
	FORCEINLINE FVector2D ToVector2D(const ImVec2& ImGuiVector)
	{
		return FVector2D{ ImGuiVector.x, ImGuiVector.y };
	}

	// Convert from ImGui Texture Id to Texture Index that we use for texture resources.
	FORCEINLINE TextureIndex ToTextureIndex(ImTextureID Index)
	{
		return static_cast<TextureIndex>(reinterpret_cast<intptr_t>(Index));
	}

	// Convert from Texture Index to ImGui Texture Id that we pass to ImGui.
	FORCEINLINE ImTextureID ToImTextureID(TextureIndex Index)
	{
		return reinterpret_cast<ImTextureID>(static_cast<intptr_t>(Index));
	}
}
