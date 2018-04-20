#pragma once

#ifdef _MSC_VER
static __declspec(noreturn) void builtin_unreachable() {}
#define UNREACHABLE builtin_unreachable()
#else
#define UNREACHABLE __builtin_unreachable()
#endif // _MSC_VER

