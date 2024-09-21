#include "NiObjects.h"

static TES* g_TES = *(TES**)0x11DEA10;
static PlayerCharacter* g_thePlayerSingleton = *(PlayerCharacter**)0x11DEA3C;
static GridCellArray* g_gridCellArray = g_TES->gridCellArray;

void NiAVObject::Dump(UInt32 level, const char * indent)
{
	UInt32 childCount = 0;
	char locIndent[257];
	strcpy_s(locIndent, 257, indent);
	if (strlen(indent) < 254)
		strcat_s(locIndent, 257, "  ");
	NiNode* niNode = GetAsNiNode();
	if (niNode)
		childCount = niNode->m_children.Length();
	_MESSAGE("[%0.4d]%s - name: '%s' [%0.8X] has %d children", level, locIndent, m_pcName, niNode, childCount);
	for (UInt32 i = 0; i < childCount; i++)
	{
		NiAVObject* child = niNode->m_children[i];
		child->Dump(level+1, locIndent);
	}
	if (0 == level)
		_MESSAGE("\n");
}

__declspec(naked) NiAVObject* __fastcall _GetRayCastObject(RayCastData* rcData)
{

	gLog.Message("_GetRayCastObject");
	__asm
	{
		push	ebx
		mov		ebx, ecx
		mov		eax, [ebp + 0xC]
		mov		ecx, [ebp + 8]
		movups	xmm0, [eax]
		shufps	xmm0, xmm0, 0xC
		movss	xmm1, [eax + 0x18]
		unpcklpd	xmm0, xmm1
		movss	xmm1, [ebp + 0x10]
		shufps	xmm1, xmm1, 0x40
		mulps	xmm0, xmm1
		movups	xmm1, [ecx]
		movaps	xmm2, kUnitConv
		mulps	xmm1, xmm2
		movaps  [ebx], xmm1
		mulps	xmm0, xmm2
		addps	xmm0, xmm1
		movaps  [ebx + 0x10], xmm0
		mov		dword ptr[ebx + 0x40], 0x3F800000
		or      eax, 0xFFFFFFFF
		mov     [ebx + 0x44], eax
		mov     [ebx + 0x50], eax
		xor     eax, eax
		mov     [ebx + 0x20], al
		lea		ecx, [ebx + 0x70]
		mov     [ecx], eax
		mov     [ecx + 0x10], eax
		xorps	xmm0, xmm0
		movaps  [ecx + 0x20], xmm0
		movaps  [ecx + 0x30], xmm0
		mov		eax, g_thePlayerSingleton
		mov		ecx, [eax + 0x68]
		mov		eax, [ecx + 0x138]
		mov		ecx, [eax + 0x594]
		mov		eax, [ecx + 8]
		mov		ecx, [eax + 0x2C]
		mov		eax, [ebp + 0x14]
		mov		edx, 6
		test	eax, eax
		cmovs	eax, edx
		and     eax, 0x7F
		mov		cx, ax
		mov     [ebx + 0x24], ecx
		push	1
		push	ebx
		mov		ecx, g_TES
		CALL_EAX(0x458440)
		pop		ebx
		retn
		ALIGN 16
	kUnitConv:
		EMIT_PS_3(0x3E124DD2)
	}
	gLog.Message("_GetRayCastObject Done");
}

__declspec(naked) bool NiVector4::RayCastCoords(const NiVector3& posVector, float* rotMatRow, float maxRange, SInt32 layerType)
{
	gLog.Message("RayCastCoords");
	__asm
	{
		push	ebp
		mov		ebp, esp
		and		esp, 0xFFFFFFF0
		sub		esp, 0xB0
		push	ecx
		lea		ecx, [esp + 4]
		call	_GetRayCastObject
		pop		ecx
		movaps	xmm0, [esp]
		movaps	xmm1, [esp + 0x10]
		subps	xmm1, xmm0
		pshufd	xmm2, [esp + 0x40], 0
		mulps	xmm1, xmm2
		addps	xmm0, xmm1
		mulps	xmm0, PS_HKUnitCnvrt
		movups  [ecx], xmm0
		test	eax, eax
		jnz		done
		mov		eax, g_TES
		cmp		dword ptr[eax + 0x34], 0
		jnz		done
		cvttps2dq	xmm0, xmm0
		psrad	xmm0, 0xC
		pshuflw	xmm0, xmm0, 2
		mov		ecx, [eax + 8]
		call	GridCellArray::GetCellAtCoord
		test	eax, eax
	done :
		setnz	al
		leave
		retn	0x10
	}
	gLog.Message("RayCastCoordsDone");
}
