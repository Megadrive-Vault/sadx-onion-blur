#include "stdafx.h"

// Tails always renders two additional tail models, so we can
// use this to alternate between an alpha of .75 and .5
static bool tails_hack = false;

DataPointer(NJS_ARGB, GlobalSpriteColor, 0x03AB9864);
FunctionPointer(void, njAction_Queue, (NJS_ACTION *action, float frame, QueuedModelFlagsB flags), 0x00405470);

static void __cdecl njAction_Onion(NJS_ACTION* action, float frame)
{
	const auto frame_count = static_cast<float>(action->motion->nbFrame);

	const NJS_ARGB color_orig = GlobalSpriteColor;
	const uint32_t control_3d_orig = _nj_control_3d_flag_;

	_nj_control_3d_flag_ |= NJD_CONTROL_3D_CONSTANT_ATTR | NJD_CONTROL_3D_CONSTANT_MATERIAL;

	BackupConstantAttr();
	AddConstantAttr(0, NJD_FLAG_USE_ALPHA);
	njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_SRCALPHA);
	njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_ONE);

	float alpha = 0.75f;

	for (int i = 0; i < 2; ++i)
	{
		SetMaterialAndSpriteColor_Float(alpha, 1.0, 1.0, 1.0);

		alpha -= 0.25f;
		frame -= 2.0f;

		if (frame < 0.0f)
		{
			frame = frame_count + frame;
		}

		njAction_Queue(action, frame, QueuedModelFlagsB_EnableZWrite);
	}

	RestoreConstantAttr();
	_nj_control_3d_flag_ = control_3d_orig;
	GlobalSpriteColor = color_orig;
}

static const void* loc_494400 = reinterpret_cast<const void*>(0x494400);

void __cdecl sub_494400_o(int a1, CharObj2* a2)
{
	__asm
	{
		mov eax, a1
		mov esi, a2
		call loc_494400
	}
}

void __cdecl sub_494400_c(int anim_index, CharObj2* data2)
{
	if (MetalSonicFlag || anim_index != 13)
	{
		sub_494400_o(anim_index, data2);
		return;
	}

	njAction(data2->AnimationThing.AnimData[anim_index].Animation, data2->AnimationThing.Frame);
	njAction_Onion(data2->AnimationThing.AnimData[anim_index].Animation, data2->AnimationThing.Frame);
}

static void __declspec(naked) sub_494400_asm()
{
	__asm
	{
		push esi // a2
		push eax // a1

		call sub_494400_c

		pop eax // a1
		pop esi // a2
		retn
	}
}

static void __cdecl njAction_TailsWrapper(NJS_ACTION* action, Float frame)
{
	const uint32_t control_3d_orig = _nj_control_3d_flag_;
	const NJS_ARGB color_orig = GlobalSpriteColor;

	_nj_control_3d_flag_ |= NJD_CONTROL_3D_CONSTANT_ATTR | NJD_CONTROL_3D_CONSTANT_MATERIAL;

	BackupConstantAttr();
	AddConstantAttr(0, NJD_FLAG_USE_ALPHA);
	njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_SRCALPHA);
	njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_ONE);

	float alpha = tails_hack ? 0.5f : 0.75f;
	tails_hack = !tails_hack;
	SetMaterialAndSpriteColor_Float(alpha, 1.0f, 1.0f, 1.0f);

#if 1
	// TODO: figure out why this breaks tails' body materials
	njAction_Queue(action, frame, QueuedModelFlagsB_EnableZWrite);
#else
	njAction(action, frame);
#endif

	RestoreConstantAttr();
	_nj_control_3d_flag_ = control_3d_orig;
	GlobalSpriteColor = color_orig;
}

extern "C"
{
	__declspec(dllexport) ModInfo SADXModInfo = { ModLoaderVer };

	__declspec(dllexport) void __cdecl Init(const char* path, const HelperFunctions& helperFunctions)
	{
		// Sonic's motion blur
		WriteCall(reinterpret_cast<void*>(0x004947B7), &sub_494400_asm);
		WriteCall(reinterpret_cast<void*>(0x00494B00), &sub_494400_asm);

		// disabled until the material bug is resolved
	#if 0
		// Tails' tails
		WriteCall(reinterpret_cast<void*>(0x00461156), njAction_TailsWrapper);
		WriteData<5>(reinterpret_cast<void*>(0x004610AF), 0x90i8);
		WriteData<5>(reinterpret_cast<void*>(0x004611A9), 0x90i8);

	#ifdef _DEBUG
		// For debugging - allows tail onion skin to be rendered while paused
		WriteData<2>(reinterpret_cast<void*>(0x0046110E), 0x90i8);
	#endif
	#endif
	}
}
