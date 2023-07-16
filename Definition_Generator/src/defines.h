#pragma once

#define b_switch(val) (val) = (val) ? false : true
#define closed_interval(low, val, high) ((val) >= (low) && (val) <= (high))
#define open_interval(low, val, high) ((val) > (low) && (val) < (high))

#define EPSILON 0.0001f

#define KILOBYTE(n) ((n) * 1024ULL)
#define MEGABYTE(n) (KILOBYTE((n)) * 1024ULL)
#define GIGABYTE(n) (MEGABYTE((n)) * 1024ULL)

#define MILLISECONDS(milli) (milli) * 0.001f;
#define MICROSECONDS(micro) (micro) * 0.000001f;
#define NANOSECONDS(nano) (nano) * 0.000000001f;

#define PI 3.1415936f
#define U8_MAX 0xFF
#define U16_MAX 0xFFFF
#define U32_MAX 0xFFFFFFFF
#define U64_MAX 0xFFFFFFFFFFFFFFFF

#define sy_SIZE(array) (sizeof(array) / sizeof(array[0]))

#define sy(...) __VA_ARGS__

#define get_bit(val, bit)
#define set_bit(val, bit) (val) |= (bit)
#define unset_bit(val, bit) (val) &= ~(bit)
#define switch_bit(val, bit) (val) ^= (bit)
#define check_bit(val, bit) (((val) & (bit)) == (bit))

#define BIT_64 0x8000000000000000
#define BIT_63 0x4000000000000000
#define BIT_62 0x2000000000000000
#define BIT_61 0x1000000000000000
#define BIT_60 0x800000000000000
#define BIT_59 0x400000000000000
#define BIT_58 0x200000000000000
#define BIT_57 0x100000000000000
#define BIT_56 0x80000000000000
#define BIT_55 0x40000000000000
#define BIT_54 0x20000000000000
#define BIT_53 0x10000000000000
#define BIT_52 0x8000000000000
#define BIT_51 0x4000000000000
#define BIT_50 0x2000000000000
#define BIT_49 0x1000000000000
#define BIT_48 0x800000000000
#define BIT_47 0x400000000000
#define BIT_46 0x200000000000
#define BIT_45 0x100000000000
#define BIT_44 0x80000000000
#define BIT_43 0x40000000000
#define BIT_42 0x20000000000
#define BIT_41 0x10000000000
#define BIT_40 0x8000000000
#define BIT_39 0x4000000000
#define BIT_38 0x2000000000
#define BIT_37 0x1000000000
#define BIT_36 0x800000000
#define BIT_35 0x400000000
#define BIT_34 0x200000000
#define BIT_33 0x100000000
#define BIT_32 0x80000000
#define BIT_31 0x40000000
#define BIT_30 0x20000000
#define BIT_29 0x10000000
#define BIT_28 0x8000000
#define BIT_27 0x4000000
#define BIT_26 0x2000000
#define BIT_25 0x1000000
#define BIT_24 0x800000
#define BIT_23 0x400000
#define BIT_22 0x200000
#define BIT_21 0x100000
#define BIT_20 0x80000
#define BIT_19 0x40000
#define BIT_18 0x20000
#define BIT_17 0x10000
#define BIT_16 0x8000
#define BIT_15 0x4000
#define BIT_14 0x2000
#define BIT_13 0x1000
#define BIT_12 0x800
#define BIT_11 0x400
#define BIT_10 0x200
#define BIT_9 0x100
#define BIT_8 0x80
#define BIT_7 0x40
#define BIT_6 0x20
#define BIT_5 0x10
#define BIT_4 0x8
#define BIT_3 0x4
#define BIT_2 0x2
#define BIT_1 0x1

#define true 1
#define false 0

typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

typedef int64_t b64;
typedef int32_t b32;
typedef int16_t b16;
typedef int8_t b8;

typedef double f64;
typedef float f32;

#define global static
#define internal static
#define presist static

#if 1
size_t __cdecl strlen(_In_z_ char const* _Str);
int __cdecl strcmp(_In_z_ char const* _Str1, _In_z_ char const* _Str2);
void* __cdecl memset(void* _Dst, _In_ int _Val, _In_ size_t _Size);
void* __cdecl memcpy(void* _Dst, void const* _Src, _In_ size_t _Size);
#endif
