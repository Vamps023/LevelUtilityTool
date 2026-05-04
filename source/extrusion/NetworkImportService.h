#pragma once

#include <QObject>
#include <UnigineNode.h>
#include <UnigineObjects.h>
#include <UnigineMathLib.h>
#include <vector>
#include <memory>

namespace LevelUtility
{

/// Track segment data from XML
struct TrackSegment
{
    int segmentId;
    std::string name;
};

/// Segment geometry - the actual track path points
struct SegmentGeometry
{
    int segmentId;
    std::vector<Unigine::Math::Vec3> points;
};

/// Object geometry data from XML
struct ObjectGeometry
{
    int objectId;
    Unigine::Math::Vec3 position;
    float heading;
    float pitch;
    float roll;
};

/// Network import service for loading and visualizing track XML files
class NetworkImportService final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(NetworkImportService)

public:
    enum class VisualizationMode
    {
        SequentialSpline,  ///< One continuous line by object_id order
        SegmentLines       ///< Separate lines per track segment
    };

    explicit NetworkImportService(QObject* parent = nullptr);
    ~NetworkImportService() override;

    /// Import XML file and parse track data
    bool importXml(const QString& filePath);
    
    /// Clear all imported data and visual nodes
    void clear();
    
    /// Check if data is loaded
    bool hasData() const;
    
    /// Get the loaded file path
    QString filePath() const { return filePath_; }
    
    /// Get track segments
    const std::vector<TrackSegment>& trackSegments() const { return trackSegments_; }
    
    /// Get object geometries
    const std::vector<ObjectGeometry>& objectGeometries() const { return objectGeometries_; }
    
    /// Get count of track segments
    int segmentCount() const { return static_cast<int>(trackSegments_.size()); }
    
    /// Get count of object geometries
    int objectCount() const { return static_cast<int>(objectGeometries_.size()); }
    
    /// Get segment geometries (actual track path data)
    const std::vector<SegmentGeometry>& segmentGeometries() const { return segmentGeometries_; }
    int segmentGeometryCount() const { return static_cast<int>(segmentGeometries_.size()); }

    /// Set visualization mode
    void setVisualizationMode(VisualizationMode mode);
    VisualizationMode visualizationMode() const { return visualizationMode_; }
    
    /// Generate visual nodes based on current mode
    void generateVisuals();
    
    /// Show/hide visual nodes
    void setVisualsVisible(bool visible);
    bool areVisualsVisible() const;

signals:
    void importFinished(bool success, const QString& message);
    void dataCleared();
    void visualsUpdated();

private:
    void clearVisuals();
    void generateSequentialSpline();
    void generateSegmentLines();
    Unigine::ObjectMeshDynamicPtr createLineSegment(const Unigine::Math::Vec3& start, const Unigine::Math::Vec3& end, const Unigine::Math::Vec4& color);
    Unigine::NodePtr createLineNode(const std::vector<Unigine::Math::Vec3>& points, const Unigine::Math::Vec4& color);
    
    QString filePath_;
    std::vector<TrackSegment> trackSegments_;
    std::vector<SegmentGeometry> segmentGeometries_;
    std::vector<ObjectGeometry> objectGeometries_;
    
    VisualizationMode visualizationMode_ = VisualizationMode::SequentialSpline;
    std::vector<Unigine::NodePtr> visualNodes_;
    Unigine::NodePtr parentNode_;
};

} // namespace LevelUtility
