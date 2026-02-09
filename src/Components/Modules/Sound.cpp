#include "Sound.hpp"

namespace Components
{
  constexpr auto g_en  (0x1AA4908); // global enable flag
  constexpr auto g_cf  (0x79B174); 	// config value
  constexpr auto f_new (0x454E40); 	// allocator
  constexpr auto f_ini (0x6015D0); 	// ds init
  constexpr auto f_log (0x402500); 	// logger

  const char err[] ("Error: Failed to create DirectSound play buffer\n");

	// The original implementation has a nasty bug in the failure path: if the
  // DirectSound buffer creation fails, it attempts to clean up by calling
  // Release() on the interface pointer. The problem is that if creation failed,
  // the interface pointer is uninitialized, causing an immediate access
  // violation when trying to read the vtable.
  //
	__declspec (naked) int Sound::Init ()
	{
		__asm
		{
			cmp  byte ptr ds:[g_en], 0
			jnz  proceed
			xor  eax, eax
			ret

		proceed:
			push esi
			mov  eax, f_new
			call eax
			mov  esi, eax

			test esi, esi
			jnz  alloc_ok
			pop  esi
			ret

		alloc_ok:
      // Note that we explicitly load ECX/EAX to match the register state
      // for `f_ini`.
			//
			mov  ecx, dword ptr ds:[esi+38h]
			mov  eax, dword ptr ds:[esi+2Ch]
			mov  eax, dword ptr ds:g_cf
			push edi

			push eax
			mov  dword ptr ds:[esi+8], eax

			lea  edi, [esi+4]
			push edi
			mov  eax, f_ini
			call eax
			add  esp, 8

			test eax, eax
			jge  success

			push offset err
			push 9
			mov  eax, f_log
			call eax
			add  esp, 8

			// CRASH FIX:
			//
			// The original code blindly dereferences [EDI] (the buffer pointer) to
      // find the Release() vfunc. But if f_ini failed, [EDI] is likely a
      // invalid pointer. We check it first.
			//
			mov  eax, dword ptr ds:[edi]
			test eax, eax
			jz   fail_clean

			// It's valid, so we release the interface.
			//
			mov  ecx, dword ptr ds:[eax]
			mov  edx, dword ptr ds:[ecx+8] // Release() is usually at offset 8 in IUnknown
			push eax
			call edx

		fail_clean:
			// Otherwise, we zero out the slot and bail out.
			//
			mov dword ptr ds:[edi], 0
			pop edi
			xor eax, eax
			pop esi
			ret

		success:
			pop edi
			mov eax, esi
			pop esi
			ret
		}
	}

  Sound::
  Sound ()
  {
    Utils::Hook (0x0463A80, Sound::Init, HOOK_JUMP).install ()->quick ();
  }
}
