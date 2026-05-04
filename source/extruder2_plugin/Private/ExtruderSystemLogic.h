#ifndef __EXTRUDER2_SYSTEM_LOGIC_H__
#define __EXTRUDER2_SYSTEM_LOGIC_H__

#include <UnigineLogic.h>

class Extruder2SystemLogic : public Unigine::SystemLogic
{
public:
	Extruder2SystemLogic();
	~Extruder2SystemLogic() override;

	int init() override;

	int update() override;
	int postUpdate() override;

	int shutdown() override;
};

#endif // __EXTRUDER2_SYSTEM_LOGIC_H__
