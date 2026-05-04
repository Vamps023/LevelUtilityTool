
#ifndef __EXTRUDER2_WORLD_LOGIC_H__
#define __EXTRUDER2_WORLD_LOGIC_H__

#include <UnigineLogic.h>
#include <UnigineStreams.h>

class Extruder2WorldLogic : public Unigine::WorldLogic
{

public:
	Extruder2WorldLogic();
	~Extruder2WorldLogic() override;

	int init() override;

	int update() override;
	int postUpdate() override;
	int updatePhysics() override;

	int shutdown() override;

	int save(const Unigine::StreamPtr &stream) override;
	int restore(const Unigine::StreamPtr &stream) override;
};

#endif // __EXTRUDER2_WORLD_LOGIC_H__
