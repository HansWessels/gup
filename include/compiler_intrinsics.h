// https://stackoverflow.com/questions/11228855/header-files-for-x86-simd-intrinsics
//   and
// https://github.com/yuikns/intrin

#ifdef HAVE_IMMINTRIN_H

#include <immintrin.h>

#if defined(_MSC_VER)
     /* Microsoft C/C++-compatible compiler */
     #include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
     /* GCC-compatible compiler, targeting x86/x86-64 */
     #include <x86intrin.h>
#elif defined(__GNUC__) && defined(__ARM_NEON__)
     /* GCC-compatible compiler, targeting ARM with NEON */
     #include <arm_neon.h>
#elif defined(__GNUC__) && defined(__IWMMXT__)
     /* GCC-compatible compiler, targeting ARM with WMMX */
     #include <mmintrin.h>
#elif (defined(__GNUC__) || defined(__xlC__)) && (defined(__VEC__) || defined(__ALTIVEC__))
     /* XLC or GCC-compatible compiler, targeting PowerPC with VMX/VSX */
     #include <altivec.h>
#elif defined(__GNUC__) && defined(__SPE__)
     /* GCC-compatible compiler, targeting PowerPC with SPE */
     #include <spe.h>
#endif

#ifdef _MSC_VER

#ifdef __cplusplus
extern "C"
{
#endif

static uint32_t inline __builtin_ctz(uint32_t value)
{
    DWORD trailing_zero = 0;

    if (_BitScanForward(&trailing_zero, value))
    {
        return trailing_zero;
    }
    else
    {
        // This is undefined, I better choose 32 than 0
        return 32;
    }
}

static uint32_t inline __builtin_clz(uint32_t value)
{
    DWORD leading_zero = 0;

    if (_BitScanReverse(&leading_zero, value))
    {
        return 31 - leading_zero;
    }
    else
    {
        // Same remarks as above
        return 32;
    }
}

#ifdef __cplusplus
}
#endif

#endif // _MSC_VER

#endif

