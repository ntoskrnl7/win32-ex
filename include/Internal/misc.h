﻿#pragma once

#define __W(x) L##x
#define _W(x) __W(x)

#ifndef Add2Ptr
#define Add2Ptr(P, I) ((PVOID)((ULONG_PTR)(P) + (I)))
#endif

#ifndef PtrOffset
#define PtrOffset(B, O) ((ULONG)((ULONG_PTR)(O) - (ULONG_PTR)(B)))
#endif

//
//  플래그 설정, 제거, 검사 매크로.
//

#ifndef FlagOn
#define FlagOn(_F, _SF) ((_F) & (_SF))
#endif

#ifndef BooleanFlagOn
#define BooleanFlagOn(F, SF) ((bool)(((F) & (SF)) != 0))
#endif

#ifndef SetFlag
#define SetFlag(_F, _SF) ((_F) |= (_SF))
#endif

#ifndef ClearFlag
#define ClearFlag(_F, _SF) ((_F) &= ~(_SF))
#endif

//
//  크기/정렬 메크로.
//

#ifndef ROUND_TO_SIZE
//
//  This macro takes a length & rounds it up to a multiple of the alignment
//  Alignment is given as a power of 2
//

#define ROUND_TO_SIZE(_length, _alignment) ((((ULONG_PTR)(_length)) + ((_alignment)-1)) & ~((ULONG_PTR)(_alignment)-1))

#endif

#ifndef IS_ALIGNED

//
//  Checks if 1st argument is aligned on given power of 2 boundary specified
//  by 2nd argument
//

#define IS_ALIGNED(_pointer, _alignment) ((((ULONG_PTR)(_pointer)) & ((_alignment)-1)) == 0)

#endif