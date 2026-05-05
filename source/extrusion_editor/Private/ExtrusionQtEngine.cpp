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

	// Try to assign extrude property, but don't fail if it doesn't exist
	PropertyPtr prop = Properties::findProperty("extrude");
	if (prop)
	{
		root->setProperty(0, prop);
		Log::message("ExtrusionQtEngine: Assigned 'extrude' property to node\n");
	}
	else
	{
		Log::warning("ExtrusionQtEngine: 'extrude' property not found in project. Node will still work for selection-based extrusion.\n");
	}

	NodeReferencePtr templ = NodeReference::create(nodeFilePath.toUtf8().constData());
	if (templ)
	{
		templ->setName("template");
		templ->setEnabled(false);
		templ->setSaveToWorldEnabled(true);
		root->addChild(templ);
		Log::message("ExtrusionQtEngine: Loaded template: %s\n", templ->getName());
	}
	else
	{
		Log::warning("ExtrusionQtEngine: Failed to load template from %s\n", nodeFilePath.toUtf8().constData());
	}

	NodeDummyPtr spline = NodeDummy::create();
	spline->setName("spline");
	spline->setSaveToWorldEnabled(true);
	root->addChild(spline);

	NodeDummyPtr knot0 = NodeDummy::create();
	knot0->setName("knot0");
	knot0->setPosition(Vec3(-2.0f, 0.0f, 0.0f));  // Local to spline
	knot0->setSaveToWorldEnabled(true);
	spline->addChild(knot0);

	NodeDummyPtr knot1 = NodeDummy::create();
	knot1->setName("knot1");
	knot1->setPosition(Vec3(2.0f, 0.0f, 0.0f));  // Local to spline
	knot1->setSaveToWorldEnabled(true);
	spline->addChild(knot1);

	// Oksygen pattern: make root editor-managed and visible in hierarchy
	// CRITICAL: All children must also be editor-managed to avoid crash
	// when editor iterates the hierarchy
	root->setOwner(false);
	root->setShowInEditorEnabledRecursive(true);

	// Also set owner on all children to ensure consistent editor state
	if (templ) templ->setOwner(false);
	spline->setOwner(false);
	knot0->setOwner(false);
	knot1->setOwner(false);

	Log::message("ExtrusionQtEngine: Created extrude node hierarchy from %s\n", nodeFilePath.toUtf8().constData());
	return true;
}

} // namespace LevelUtility
