
#ifndef __EXTRUDER2EDITORLOGIC_H__
#define __EXTRUDER2EDITORLOGIC_H__

/* ---------- headers */

#include <UnigineLogic.h>

/* ---------- constants */

/* ---------- definitions */

class Extruder2EditorLogic :
    public Unigine::EditorLogic
{
public:
    Extruder2EditorLogic();

    /* ---------- Unigine::EditorLogic interface */

    virtual ~Extruder2EditorLogic() override;
    virtual int init() override;
    virtual int shutdown() override;
    virtual int update() override;
    virtual int render(const Unigine::EngineWindowViewportPtr &window) override;
    virtual int worldInit() override;
    virtual int worldShutdown() override;
    virtual int worldSave() override;
};

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

#endif // __EXTRUDER2EDITORLOGIC_H__
