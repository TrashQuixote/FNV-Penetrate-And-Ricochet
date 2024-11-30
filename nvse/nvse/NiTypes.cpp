#include "NiTypes.h"


// Copied from JIP LN NVSE
__declspec(naked) float __vectorcall Point3Distance(const NiVector3& pt1, const NiVector3& pt2)
{
	__asm
	{
		movaps	xmm2, PS_XYZ0Mask
		movups	xmm0, [ecx]
		andps	xmm0, xmm2
		movups	xmm1, [edx]
		andps	xmm1, xmm2
		subps	xmm0, xmm1
		mulps	xmm0, xmm0
		xorps	xmm1, xmm1
		haddps	xmm0, xmm1
		haddps	xmm0, xmm1
		comiss	xmm0, xmm1
		jz		done
		movaps	xmm1, xmm0
		rsqrtss	xmm2, xmm0
		mulss	xmm1, xmm2
		mulss	xmm1, xmm2
		movss	xmm3, SS_3
		subss	xmm3, xmm1
		mulss	xmm3, xmm2
		mulss	xmm3, PS_V3_Half
		mulss	xmm0, xmm3
	done :
		retn
	}
}
__m128 __vectorcall Normalize_V4(__m128 inPS);
__declspec(naked) __m128 __vectorcall Normalize_V4(__m128 inPS)
{
	__asm
    {
		movaps	xmm1, xmm0
		movaps	xmm2, xmm1
		mulps	xmm2, xmm2
		xorps	xmm0, xmm0
		haddps	xmm2, xmm0
		haddps	xmm2, xmm0
		comiss	xmm2, PS_Epsilon
		jb		zeroLen
		rsqrtss	xmm3, xmm2
		movss	xmm0, SS_3
		mulss	xmm2, xmm3
		mulss	xmm2, xmm3
		subss	xmm0, xmm2
		mulss	xmm0, xmm3
		mulss	xmm0, PS_V3_Half
		shufps	xmm0, xmm0, 0
		mulps	xmm0, xmm1
	zeroLen:
        retn
    }
}

__declspec(naked) __m128 __vectorcall NiVector3::CrossProduct(const NiVector3& vB) const
{
	__asm
	{
		movups	xmm1, [edx]
		andps	xmm1, PS_XYZ0Mask
		movups	xmm2, [ecx]
		pshufd	xmm0, xmm1, 0xC9
		pshufd	xmm3, xmm2, 0x12
		mulps	xmm0, xmm3
		pshufd	xmm3, xmm1, 0xD2
		pshufd	xmm4, xmm2, 9
		mulps	xmm3, xmm4
		subps	xmm0, xmm3
		jmp		Normalize_V4
	}
}

__declspec(naked) NiVector3& NiVector3::Normalize()
{
	__asm
	{
		mov		eax, ecx
		movups	xmm1, [eax]
		andps	xmm1, PS_XYZ0Mask
		movaps	xmm2, xmm1
		mulps	xmm2, xmm2
		xorps	xmm0, xmm0
		haddps	xmm2, xmm0
		haddps	xmm2, xmm0
		comiss	xmm2, PS_Epsilon
	jb	zeroLen
		rsqrtss	xmm3, xmm2
		movss	xmm0, SS_3
		mulss	xmm2, xmm3
		mulss	xmm2, xmm3
		subss	xmm0, xmm2
		mulss	xmm0, xmm3
		mulss	xmm0, PS_V3_Half
		shufps	xmm0, xmm0, 0xC0
		mulps	xmm0, xmm1
	zeroLen :
		pshufd	xmm2, xmm0, 2
		movlps[eax], xmm0
		movss[eax + 8], xmm2
		retn
	}
}

__declspec(naked) float __vectorcall NiMatrix33::From2Points(__m128 pt1, __m128 pt2)
{
	__asm
	{
		subps	xmm0, xmm1
		andps	xmm0, PS_XYZ0Mask
		xorps	xmm6, xmm6
		movq	xmm7, xmm6
		movaps	xmm1, xmm0
		mulps	xmm1, xmm1
		movaps	xmm2, xmm1
		haddps	xmm2, xmm7
		haddps	xmm2, xmm7
		comiss	xmm2, xmm7
		jz		allZero
		pshufd	xmm3, xmm0, 0xFE
		xorps	xmm3, PS_FlipSignMask0
		unpcklpd	xmm1, xmm7
		haddps	xmm1, xmm7
		comiss	xmm1, xmm7
		jz		zZero
		unpcklpd	xmm2, xmm1
		movaps	xmm4, xmm2
		rsqrtps	xmm6, xmm2
		mulps	xmm2, xmm6
		mulps	xmm2, xmm6
		movss	xmm5, SS_3
		unpcklpd	xmm5, xmm5
		subps	xmm5, xmm2
		mulps	xmm5, xmm6
		mulps	xmm5, PS_V3_Half
		movaps	xmm2, xmm5
		mulps	xmm4, xmm2
		movaps	xmm6, xmm4
		movsldup	xmm2, xmm2
		unpckhpd	xmm4, xmm4
		unpcklps	xmm3, xmm4
		unpcklpd	xmm3, xmm0
		cmpeqps	xmm0, xmm7
		movmskps	eax, xmm0
		and al, 3
		cmp		al, 2
		jz		getSC
		pshufd	xmm0, PS_FlipSignMask0, 0x15
		xorps	xmm3, xmm0
	getSC :
		mulps	xmm3, xmm2
		jmp		makeMat
	zZero :
		movaps	xmm6, xmm3
		pshufd	xmm0, PS_V3_One, 0x3C
		andps	xmm3, PS_FlipSignMask0
		xorps	xmm6, xmm3
		xorps	xmm3, xmm0
		jmp		makeMat
	allZero :
		pshufd	xmm3, PS_V3_One, 0x33
	makeMat :
			pshufd	xmm0, xmm3, 0xA
			pshufd	xmm1, xmm3, 0xF
			movss[ecx], xmm1
			movss[ecx + 0xC], xmm0
			movss[ecx + 0x18], xmm7
			movlps[ecx + 0x1C], xmm3
			shufps	xmm3, xmm7, 1
			mulps	xmm0, xmm3
			movlps[ecx + 4], xmm0
			xor byte ptr[ecx + 7], 0x80
			mulps	xmm3, xmm1
			movlps[ecx + 0x10], xmm3
			xor byte ptr[ecx + 0x17], 0x80
			movq	xmm0, xmm6
			retn
	}
}


