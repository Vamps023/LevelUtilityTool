#include "ExtruderSystemLogic.h"

using namespace Unigine;

// System logic, it exists during the application life cycle.
// These methods are called right after corresponding system script's (UnigineScript) methods.

Extruder2SystemLogic::Extruder2SystemLogic()
{
}

Extruder2SystemLogic::~Extruder2SystemLogic()
{
}

int Extruder2SystemLogic::init()
{
    Unigine::Log::message("%s\n", __FUNCTION__);
	// Write here code to be called on engine initialization.
	return 1;
}

int Extruder2SystemLogic::update()
{
	// Write here code to be called before updating each render frame.
	return 1;
}

int Extruder2SystemLogic::postUpdate()
{
	// Write here code to be called after updating each render frame.
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// end of the main loop
////////////////////////////////////////////////////////////////////////////////

int Extruder2SystemLogic::shutdown()
{
    Unigine::Log::message("%s\n", __FUNCTION__);
	// Write here code to be called on engine shutdown.
	return 1;
}
