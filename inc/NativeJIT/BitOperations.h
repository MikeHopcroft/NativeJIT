#pragma once

#include <cstdint>
#include <intrin.h> // Intrinsic instructions.
#include <type_traits>


namespace NativeJIT
{
    // WARNING: the methods in this file are thin wrappers around compiler
    // intrinsic methods. As such, they are meant to have no or minimal added
    // overhead which is why only compile-time checks are done, but no runtime
    // checks for valid input range or similar are performed.
    namespace BitOp
    {
        extern const bool c_isPopCntSupported;
        extern char const * const c_bitsSetInByte;

        // Reinterprets the cast from FROM* to TO*, ensuring that FROM and
        // TO are indeed of the same size.
        // This is necessary since bit intrinsics take e.g. pointer to long
        // and won't accept pointer to int32_t (i.e. int) even though long and
        // int have the same size. The cast doesn't verify that signedness is
        // the same since bit intrinsics work the same way regardless of sign.
        template <typename TO, typename FROM>
        __forceinline
        TO* SameTargetSizeCast(FROM* pointer)
        {
            static_assert(sizeof(FROM) == sizeof(TO),
                          "The pointers must point to types of the same size");

            return reinterpret_cast<TO*>(pointer);
        }


        // Fallback routine for counting the number of 1 bits in a value if
        // the popcnt intrinsic is not supported.
        template <typename T>
        /* __forceinline */
        uint8_t GetNonZeroBitCountFallback(T value)
        {
            static_assert(std::is_integral<T>::value, "The type must be integral");

            uint8_t numBitsSet = 0;
            auto const valueBytes = reinterpret_cast<uint8_t const *>(&value);

            for (unsigned i = 0; i < sizeof(T); ++i)
            {
                numBitsSet += c_bitsSetInByte[valueBytes[i]];
            }

            return numBitsSet;
        }


        // Returns the count of 1 bits in the value.
        // Requires SSE4 support.
        // See https://en.wikipedia.org/wiki/SSE4#POPCNT_and_LZCNT
        // Note that processors from around 2008 and onwards support POPCNT, the
        // fallback method is implemented only for unit test runs in potentially
        // old lab machines/virtual machines.
        __forceinline
        uint8_t GetNonZeroBitCount(uint32_t value)
        {
            return c_isPopCntSupported
                ? static_cast<uint8_t>(_mm_popcnt_u32(value))
                : GetNonZeroBitCountFallback(value);
        }


        // Returns the count of 1 bits in the value.
        // Requires SSE4 support.
        // See https://en.wikipedia.org/wiki/SSE4#POPCNT_and_LZCNT
        __forceinline
        uint8_t GetNonZeroBitCount(uint64_t value)
        {
            return c_isPopCntSupported
                ? static_cast<uint8_t>(_mm_popcnt_u64(value))
                : GetNonZeroBitCountFallback(value);
        }


        // Stores the index of the lowest 1 bit and returns true. Returns
        // false and leaves lowestBitSetIndex in an underterminate state
        // if the value has no bits set.
        // Uses BSF instruction. See http://felixcloutier.com/x86/BSF.html
        __forceinline
        bool GetLowestBitSet(uint64_t value, unsigned* lowestBitSetIndex)
        {
            return _BitScanForward64(SameTargetSizeCast<unsigned long>(lowestBitSetIndex),
                                     value)
                   ? true
                   : false;
        }

        // Stores the index of the highest 1 bit and returns true. Returns
        // false and leaves highestBitSetIndex in an underterminate state
        // if the value has no bits set.
        // Uses BSR instruction. See http://felixcloutier.com/x86/BSR.html
        __forceinline
        bool GetHighestBitSet(uint64_t value, unsigned* highestBitSetIndex)
        {
            return _BitScanReverse64(SameTargetSizeCast<unsigned long>(highestBitSetIndex),
                                     value)
                   ? true
                   : false;
        }


        // Returns a boolean indicating whether the specified bit is set or not.
        // WARNING: Does not verify that bitIndex is in valid range.
        __forceinline
        bool TestBit(uint64_t value, unsigned bitIndex)
        {
            return _bittest64(SameTargetSizeCast<const long long>(&value),
                              bitIndex)
                   ? true
                   : false;
        }


        // Returns a boolean indicating whether the specified bit is set or not.
        // WARNING: Does not verify that bitIndex is in valid range.
        template <typename T>
        __forceinline
        bool TestBit(T value, unsigned bitIndex)
        {
            static_assert(std::is_integral<T>::value, "Value must be integral");
            static_assert(sizeof(T) <= sizeof(uint32_t), "Value must not be larger than 32 bits");

            const uint32_t value32 = value;
            return _bittest(SameTargetSizeCast<const long>(&value32),
                            bitIndex)
                   ? true
                   : false;
        }


        // Sets the specified bit to 1 and returns whether the bit was previously
        // set or not.
        // WARNING: Does not verify that bitIndex is in valid range.
        __forceinline
        bool TestAndSetBit(uint32_t* value, unsigned bitIndex)
        {
            return _bittestandset(SameTargetSizeCast<long>(value),
                                  bitIndex)
                   ? true
                   : false;
        }


        // Sets the specified bit to 1 and returns whether the bit was previously
        // set or not.
        // WARNING: Does not verify that bitIndex is in valid range.
        __forceinline
        bool TestAndSetBit(uint64_t* value, unsigned bitIndex)
        {
            return _bittestandset64(SameTargetSizeCast<long long>(value),
                                    bitIndex)
                   ? true
                   : false;
        }


        // A (naming) convenience wrapper for TestAndSetBit.
        // WARNING: Does not verify that bitIndex is in valid range.
        template <typename T>
        __forceinline
        void SetBit(T* value, unsigned bitIndex)
        {
            TestAndSetBit(value, bitIndex);
        }


        // Sets the specified bit to 0 and returns whether the bit was previously
        // set or not.
        // WARNING: Does not verify that bitIndex is in valid range.
        __forceinline
        bool TestAndClearBit(uint32_t* value, unsigned bitIndex)
        {
            return _bittestandreset(SameTargetSizeCast<long>(value),
                                    bitIndex)
                   ? true
                   : false;
        }


        // Sets the specified bit to 1 and returns whether the bit was previously
        // set or not.
        // WARNING: Does not verify that bitIndex is in valid range.
        __forceinline
        bool TestAndClearBit(uint64_t* value, unsigned bitIndex)
        {
            return _bittestandreset64(SameTargetSizeCast<long long>(value),
                                      bitIndex)
                   ? true
                   : false;
        }


        // A (naming) convenience wrapper for TestAndClearBit.
        // WARNING: Does not verify that bitIndex is in valid range.
        template <typename T>
        __forceinline
        void ClearBit(T* value, unsigned bitIndex)
        {
            TestAndClearBit(value, bitIndex);
        }
    }
}