#include "NetworkImportService.h"

#include <UnigineLog.h>
#include <UnigineWorld.h>
#include <UnigineObjects.h>
#include <QFile>
#include <QXmlStreamReader>

namespace LevelUtility
{

NetworkImportService::NetworkImportService(QObject* parent)
    : QObject(parent)
{
}

NetworkImportService::~NetworkImportService()
{
    clear();
}

bool NetworkImportService::importXml(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit importFinished(false, "Failed to open file: " + filePath);
        return false;
    }

    clear();
    
    QXmlStreamReader xml(&file);
    bool inTrackSegments = false;
    bool inSegmentGeometry = false;
    bool inObjectGeometries = false;
    SegmentGeometry currentSegGeom;
    
    while (!xml.atEnd() && !xml.hasError())
    {
        xml.readNext();
        
        if (xml.isStartElement())
        {
            QString name = xml.name().toString();
            
            if (name == "track_segments")
            {
                inTrackSegments = true;
            }
            else if (name == "object_geometries")
            {
                inObjectGeometries = true;
            }
            else if (name == "track_segment" && inTrackSegments)
            {
                TrackSegment segment;
                segment.segmentId = xml.attributes().value("segment_id").toInt();
                segment.name = xml.attributes().value("name").toString().toStdString();
                trackSegments_.push_back(segment);
            }
            else if (name == "segment_geometry")
            {
                inSegmentGeometry = true;
                currentSegGeom = SegmentGeometry();
                currentSegGeom.segmentId = xml.attributes().value("segment_id").toInt();
            }
            else if (name == "point" && inSegmentGeometry)
            {
                Unigine::Math::Vec3 pt;
                pt.x = xml.attributes().value("x").toDouble();
                pt.y = xml.attributes().value("y").toDouble();
                pt.z = xml.attributes().value("z").toDouble();
                currentSegGeom.points.push_back(pt);
            }
            else if (name == "object_geometry" && inObjectGeometries)
            {
                ObjectGeometry obj;
                obj.objectId = xml.attributes().value("object_id").toInt();
                obj.position.x = xml.attributes().value("x").toDouble();
                obj.position.y = xml.attributes().value("y").toDouble();
                obj.position.z = xml.attributes().value("z").toDouble();
                obj.heading = xml.attributes().value("heading").toFloat();
                obj.pitch = xml.attributes().value("pitch").toFloat();
                obj.roll = xml.attributes().value("roll").toFloat();
                objectGeometries_.push_back(obj);
            }
        }
        else if (xml.isEndElement())
        {
            QString name = xml.name().toString();
            if (name == "track_segments")
            {
                inTrackSegments = false;
            }
            else if (name == "segment_geometry")
            {
                if (currentSegGeom.points.size() >= 2)
                    segmentGeometries_.push_back(currentSegGeom);
                inSegmentGeometry = false;
            }
            else if (name == "object_geometries")
            {
                inObjectGeometries = false;
            }
        }
    }
    
    file.close();
    
    if (xml.hasError())
    {
        emit importFinished(false, "XML parse error: " + xml.errorString());
        return false;
    }
    
    filePath_ = filePath;
    
    QString message = QString("Imported %1 segments, %2 segment geometries, %3 objects")
                          .arg(trackSegments_.size())
                          .arg(segmentGeometries_.size())
                          .arg(objectGeometries_.size());
    
    Unigine::Log::message("NetworkImportService: %s\n", message.toStdString().c_str());
    
    emit importFinished(true, message);
    return true;
}

void NetworkImportService::clear()
{
    clearVisuals();
    trackSegments_.clear();
    segmentGeometries_.clear();
    objectGeometries_.clear();
    filePath_.clear();
    emit dataCleared();
}

bool NetworkImportService::hasData() const
{
    return !segmentGeometries_.empty();
}

void NetworkImportService::setVisualizationMode(VisualizationMode mode)
{
    if (visualizationMode_ != mode)
    {
        visualizationMode_ = mode;
        if (hasData())
        {
            generateVisuals();
        }
    }
}

void NetworkImportService::generateVisuals()
{
    clearVisuals();
    
    if (segmentGeometries_.empty())
        return;
    
    // Draw each segment_geometry as a separate visible spline
    for (size_t i = 0; i < segmentGeometries_.size(); ++i)
    {
        const auto& seg = segmentGeometries_[i];
        if (seg.points.size() < 2)
            continue;
        
        auto lineNode = createLineNode(seg.points, Unigine::Math::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
        if (lineNode)
        {
            QString nodeName = QString("Segment_%1").arg(seg.segmentId);
            lineNode->setName(nodeName.toStdString().c_str());
            lineNode->setSaveToWorldEnabled(true);
            visualNodes_.push_back(lineNode);
        }
    }
    
    Unigine::Log::message("NetworkImportService: Created %d segment splines\n",
                          static_cast<int>(visualNodes_.size()));
    
    emit visualsUpdated();
}

void NetworkImportService::setVisualsVisible(bool visible)
{
    for (auto& node : visualNodes_)
    {
        if (node)
            node->setEnabled(visible);
    }
}

bool NetworkImportService::areVisualsVisible() const
{
    if (!visualNodes_.empty() && visualNodes_[0])
    {
        return visualNodes_[0]->isEnabled();
    }
    return false;
}

void NetworkImportService::clearVisuals()
{
    visualNodes_.clear();
    parentNode_.clear();
}

void NetworkImportService::generateSequentialSpline()
{
}

void NetworkImportService::generateSegmentLines()
{
}

// Cubic Hermite Spline interpolation (same as Oksygen CubicHermiteSpline)
static Unigine::Math::Vec3 hermitePosition(
    const Unigine::Math::Vec3& p0, const Unigine::Math::Vec3& m0,
    const Unigine::Math::Vec3& p1, const Unigine::Math::Vec3& m1, double t)
{
    double h00 = (2.0 * t + 1.0) * (1.0 - t) * (1.0 - t);
    double h10 = t * (1.0 - t) * (1.0 - t);
    double h01 = t * t * (-2.0 * t + 3.0);
    double h11 = t * t * (t - 1.0);
    return (p0 * h00) + (m0 * h10) + (p1 * h01) + (m1 * h11);
}

static Unigine::Math::Vec3 hermiteHeading(
    const Unigine::Math::Vec3& p0, const Unigine::Math::Vec3& m0,
    const Unigine::Math::Vec3& p1, const Unigine::Math::Vec3& m1, double t)
{
    double h00 = 6.0 * t * (t - 1.0);
    double h10 = (3.0 * t - 1.0) * (t - 1.0);
    double h01 = -h00;
    double h11 = t * (3.0 * t - 2.0);
    return (p0 * h00) + (m0 * h10) + (p1 * h01) + (m1 * h11);
}

// Adaptive subdivision (same algorithm as Oksygen GenerateSamplingPoints)
static void adaptiveSubdivide(
    const Unigine::Math::Vec3& p0, const Unigine::Math::Vec3& m0,
    const Unigine::Math::Vec3& p1, const Unigine::Math::Vec3& m1,
    double a, double b, double tolerance,
    std::vector<double>& samplePoints, int depth = 0)
{
    if (depth > 12) // Safety limit
    {
        samplePoints.push_back(b);
        return;
    }
    
    double mid = (a + b) * 0.5;
    
    Unigine::Math::Vec3 start = hermitePosition(p0, m0, p1, m1, a);
    Unigine::Math::Vec3 midPt = hermitePosition(p0, m0, p1, m1, mid);
    Unigine::Math::Vec3 end   = hermitePosition(p0, m0, p1, m1, b);
    
    double intervalLen = (end - start).length();
    if (intervalLen < 0.0001)
    {
        samplePoints.push_back(b);
        return;
    }
    
    // Estimate curvature: distance from midpoint to line between endpoints
    Unigine::Math::Vec3 crossVec(
        (midPt.y - start.y) * (midPt.z - end.z) - (midPt.z - start.z) * (midPt.y - end.y),
        (midPt.z - start.z) * (midPt.x - end.x) - (midPt.x - start.x) * (midPt.z - end.z),
        (midPt.x - start.x) * (midPt.y - end.y) - (midPt.y - start.y) * (midPt.x - end.x));
    double distance = crossVec.length() / intervalLen;
    
    if (distance <= tolerance)
    {
        // Refine check with quarter points (same as Oksygen)
        Unigine::Math::Vec3 v1 = hermitePosition(p0, m0, p1, m1, (b - a) * 0.25 + a);
        Unigine::Math::Vec3 v2 = hermitePosition(p0, m0, p1, m1, (b - a) * 0.75 + a);
        
        Unigine::Math::Vec3 cross1(
            (v1.y - start.y) * (v1.z - end.z) - (v1.z - start.z) * (v1.y - end.y),
            (v1.z - start.z) * (v1.x - end.x) - (v1.x - start.x) * (v1.z - end.z),
            (v1.x - start.x) * (v1.y - end.y) - (v1.y - start.y) * (v1.x - end.x));
        Unigine::Math::Vec3 cross2(
            (v2.y - start.y) * (v2.z - end.z) - (v2.z - start.z) * (v2.y - end.y),
            (v2.z - start.z) * (v2.x - end.x) - (v2.x - start.x) * (v2.z - end.z),
            (v2.x - start.x) * (v2.y - end.y) - (v2.y - start.y) * (v2.x - end.x));
        
        distance = (crossVec.length() + cross1.length() + cross2.length()) / intervalLen / 3.0;
    }
    
    if (distance <= tolerance)
    {
        samplePoints.push_back(b);
    }
    else
    {
        adaptiveSubdivide(p0, m0, p1, m1, a, mid, tolerance, samplePoints, depth + 1);
        adaptiveSubdivide(p0, m0, p1, m1, mid, b, tolerance, samplePoints, depth + 1);
    }
}

// Generate smooth points for a segment using cubic hermite with auto-tangents
static std::vector<Unigine::Math::Vec3> generateSmoothSegment(
    const std::vector<Unigine::Math::Vec3>& controlPts, double tolerance)
{
    std::vector<Unigine::Math::Vec3> result;
    
    if (controlPts.size() < 2)
        return result;
    
    // Compute tangents at each control point (finite difference, same as Oksygen Smooth())
    std::vector<Unigine::Math::Vec3> tangents(controlPts.size());
    
    // First tangent
    tangents[0] = (controlPts[1] - controlPts[0]) * 0.7;
    
    // Middle tangents
    for (size_t i = 1; i < controlPts.size() - 1; ++i)
    {
        tangents[i] = (controlPts[i + 1] - controlPts[i - 1]) * 0.35;
    }
    
    // Last tangent
    tangents.back() = (controlPts.back() - controlPts[controlPts.size() - 2]) * 0.7;
    
    // Add the first point
    result.push_back(controlPts[0]);
    
    // For each pair of control points, generate smooth curve
    for (size_t i = 0; i < controlPts.size() - 1; ++i)
    {
        const auto& p0 = controlPts[i];
        const auto& m0 = tangents[i];
        const auto& p1 = controlPts[i + 1];
        const auto& m1 = tangents[i + 1];
        
        std::vector<double> samplePoints;
        adaptiveSubdivide(p0, m0, p1, m1, 0.0, 1.0, tolerance, samplePoints);
        
        for (double t : samplePoints)
        {
            result.push_back(hermitePosition(p0, m0, p1, m1, t));
        }
    }
    
    return result;
}

Unigine::NodePtr NetworkImportService::createLineNode(const std::vector<Unigine::Math::Vec3>& points, const Unigine::Math::Vec4& color)
{
    if (points.size() < 2)
        return Unigine::NodePtr();
    
    // Generate smooth points using cubic hermite spline with adaptive subdivision
    const double tolerance = 0.005; // Same as Oksygen default smoothness
    std::vector<Unigine::Math::Vec3> smoothPts = generateSmoothSegment(points, tolerance);
    
    if (smoothPts.size() < 2)
        return Unigine::NodePtr();
    
    // Create a single ObjectMeshDynamic (same approach as Oksygen spline plugin)
    Unigine::ObjectMeshDynamicPtr lineMesh = Unigine::ObjectMeshDynamic::create();
    if (!lineMesh)
        return Unigine::NodePtr();
    
    lineMesh->setName("TrackSpline");
    lineMesh->setSaveToWorldEnabled(true);
    lineMesh->setEnabled(true);
    lineMesh->setShowInEditorEnabled(true);
    
    lineMesh->clearVertex();
    lineMesh->clearIndices();
    
    const double halfWidth = 0.5; // Half-width of the track ribbon
    
    // Build left+right vertex pairs (same as Oksygen AddVertex)
    std::vector<Unigine::Math::Vec3> vertices;
    vertices.reserve(smoothPts.size() * 2);
    
    for (size_t i = 0; i < smoothPts.size(); ++i)
    {
        Unigine::Math::Vec3 dir;
        if (i == 0)
            dir = smoothPts[1] - smoothPts[0];
        else if (i == smoothPts.size() - 1)
            dir = smoothPts[i] - smoothPts[i - 1];
        else
            dir = smoothPts[i + 1] - smoothPts[i - 1];
        
        double dirLen = dir.length();
        if (dirLen < 0.001)
            dir = Unigine::Math::Vec3(1, 0, 0);
        else
            dir /= dirLen;
        
        // Perpendicular in XY plane
        Unigine::Math::Vec3 side(-dir.y, dir.x, 0);
        double sideLen = side.length();
        if (sideLen < 0.001)
            side = Unigine::Math::Vec3(1, 0, 0);
        else
            side /= sideLen;
        side *= halfWidth;
        
        vertices.push_back(Unigine::Math::Vec3(
            smoothPts[i].x - side.x, smoothPts[i].y - side.y, smoothPts[i].z));
        vertices.push_back(Unigine::Math::Vec3(
            smoothPts[i].x + side.x, smoothPts[i].y + side.y, smoothPts[i].z));
    }
    
    // Create triangle strip and add vertices (same order as Oksygen)
    lineMesh->addTriangleStrip(static_cast<int>(vertices.size()));
    
    double v = 0.0;
    const double v_multiplier = 0.25 / (halfWidth * 2.0);
    
    for (size_t i = 1; i < vertices.size(); i += 2)
    {
        if (i > 1)
            v += (vertices[i] - vertices[i - 2]).length();
        
        lineMesh->addVertex(Unigine::Math::vec3(vertices[i - 1]));
        lineMesh->addTexCoord(Unigine::Math::vec4(1.0f, static_cast<float>(v * v_multiplier), 0.0f, 0.0f));
        
        lineMesh->addVertex(Unigine::Math::vec3(vertices[i]));
        lineMesh->addTexCoord(Unigine::Math::vec4(0.0f, static_cast<float>(v * v_multiplier), 0.0f, 0.0f));
    }
    
    // Finalize mesh (same order as Oksygen)
    lineMesh->updateIndices();
    lineMesh->updateBounds();
    lineMesh->flushVertex();
    lineMesh->flushIndices();
    
    return lineMesh;
}

Unigine::ObjectMeshDynamicPtr NetworkImportService::createLineSegment(
    const Unigine::Math::Vec3& start, 
    const Unigine::Math::Vec3& end,
    const Unigine::Math::Vec4& color)
{
    return Unigine::ObjectMeshDynamicPtr();
}

} // namespace LevelUtility
