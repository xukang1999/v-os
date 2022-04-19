
#ifndef SO_INCLUDE_EXPORT_H_
#define SO_INCLUDE_EXPORT_H_

#if !defined(SO_EXPORT)

#if defined(SO_SHARED_LIBRARY)
#if defined(_WIN32)

#if defined(SO_COMPILE_LIBRARY)
#define SO_EXPORT __declspec(dllexport)
#else
#define SO_EXPORT __declspec(dllimport)
#endif  // defined(SO_COMPILE_LIBRARY)

#else  // defined(_WIN32)
#if defined(SO_COMPILE_LIBRARY)
#define SO_EXPORT __attribute__((visibility("default")))
#else
#define SO_EXPORT
#endif
#endif  // defined(_WIN32)

#else  // defined(SO_SHARED_LIBRARY)
#define SO_EXPORT
#endif

#endif  // !defined(SO_EXPORT)

#endif  // SO_INCLUDE_EXPORT_H_
