#ifndef	VW_VERSION_H
#define VW_VERSION_H

constexpr unsigned long VW_VERSION_MAJOR	{ 1 };
constexpr unsigned long VW_VERSION_MINOR	{ 6 };
constexpr unsigned long VW_VERSION_PATCH	{ 0 };
constexpr unsigned long pluginVersion = (VW_VERSION_MAJOR & 0xFF) << 24 | (VW_VERSION_MINOR & 0xFF) << 16 | (VW_VERSION_PATCH & 0xFF) << 8;

#endif
