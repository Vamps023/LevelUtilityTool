#include "ExtrusionQtEngine.h"

#include <UnigineLog.h>
#include <UnigineNodes.h>
#include <UnigineWorld.h>
#include <UnigineProperties.h>
#include <UnigineMathLib.h>

using namespace Unigine;
using namespace Unigine::Math;

namespace LevelUtility
{

ExtrusionQtEngine::ExtrusionQtEngine() = default;
ExtrusionQtEngine::~ExtrusionQtEngine() = default;

bool ExtrusionQtEngine::createExtrudeNode(const QString& nodeFilePath)
{
	if (nodeFilePath.isEmpty())
		return false;

	Vec3 pos = Vec3(0.0f, 0.0f, 2.0f);

	NodeDummyPtr root = NodeDummy::create();
	root->setName("new_extrude");
	root->setWorldPosition(pos);
	root->setSaveToWorldEnabled(true);

	// Oksygen pattern: NO property assignment (property may cause editor crash)
	// Just use name-based identification instead

	// Template: use plain NodeDummy instead of NodeReference to avoid crash
	// NodeReference can cause editor crash when expanding parent
	NodeDummyPtr templ = NodeDummy::create();
	templ->setName("template");
	templ->setEnabled(false);
	templ->setSaveToWorldEnabled(true);
	root->addChild(templ);

	NodeDummyPtr spline = NodeDummy::create();
	spline->setName("spline");
	spline->setSaveToWorldEnabled(true);
	root->addChild(spline);

	NodeDummyPtr knot0 = NodeDummy::create();
	knot0->setName("knot0");
	knot0->setPosition(Vec3(-2.0f, 0.0f, 0.0f));
	knot0->setSaveToWorldEnabled(true);
	spline->addChild(knot0);

	NodeDummyPtr knot1 = NodeDummy::create();
	knot1->setName("knot1");
	knot1->setPosition(Vec3(2.0f, 0.0f, 0.0f));
	knot1->setSaveToWorldEnabled(true);
	spline->addChild(knot1);

	// Oksygen pattern: set owner AFTER all children added
	// Only root gets setOwner(false) - children inherit visibility via recursive flag
	root->setOwner(false);
	root->setShowInEditorEnabledRecursive(true);

	Log::message("ExtrusionQtEngine: Created extrude node hierarchy from %s\n", nodeFilePath.toUtf8().constData());
	return true;
}

} // namespace LevelUtility
