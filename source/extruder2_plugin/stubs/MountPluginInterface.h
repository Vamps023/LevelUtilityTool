/// @file  MountPluginInterface.h
/// @brief Stub for Oksygen's MountPlugin interface.
///        In standalone mode, provides a no-op implementation.

#pragma once
#ifndef __MOUNTPLUGININTERFACE_H__
#define __MOUNTPLUGININTERFACE_H__

/* ---------- headers */

/* ---------- constants */

/* ---------- definitions */

class IMountPlugin
{
public:
    virtual void apply_paths_hack() = 0;
    virtual bool mount_path(const char* virtual_path, const char* absolute_path) = 0;
};

/* ---------- prototypes */

// Standalone stub: returns nullptr (no mount plugin available)
inline IMountPlugin* GetMountPluginInterface()
{
    return nullptr;
}

/* ---------- globals */

/* ---------- public code */

#endif // __MOUNTPLUGININTERFACE_H__
