#pragma once

#include <QString>

namespace LevelUtility
{

class ExtrusionQtEngine
{
public:
	ExtrusionQtEngine();
	~ExtrusionQtEngine();

	bool createExtrudeNode(const QString& nodeFilePath);
};

} // namespace LevelUtility
