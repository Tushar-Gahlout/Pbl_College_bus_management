#ifndef BUSMAPWIDGET_H
#define BUSMAPWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <vector>
#include <string>
#include "../services/FileManager.h"

struct MapStop {
    QString name;
    QPointF pos;
    bool isOrigin;
    bool isDestination;
    QString scheduledTime;
};

/**
 * @brief Dedicated Canvas Widget for rendering vector road maps and animated bus GPS tracking.
 */
class MapCanvas : public QWidget {
    Q_OBJECT

private:
    float busProgress; // 0.0 to 1.0 along the route
    int speedKmh;
    std::string busId;
    std::string shiftType;
    std::vector<MapStop> stops;

    QPointF getInterpolatedBusPos() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

public:
    explicit MapCanvas(QWidget* parent = nullptr);
    ~MapCanvas() override = default;

    void setRouteStops(const std::vector<std::string>& stopNames);
    void updateBusState(float progress, int speed, const std::string& id, const std::string& shift);
    void recalculatePositions();
    const std::vector<MapStop>& getStops() const { return stops; }
};

/**
 * @brief Comprehensive Live GPS Bus Tracking Module.
 * 
 * Strict native C++17 + Qt 6 Widgets implementation using QPainter and QTimer.
 * No web engines, no QML, no external dependencies.
 */
class BusMapWidget : public QWidget {
    Q_OBJECT

private:
    FileManager* fileManager;

    QComboBox* busSelectorCombo;
    QLabel* shiftBadgeLbl;
    QLabel* emptySeatsBadgeLbl;
    QLabel* liveGpsIndicatorLbl;

    QLabel* currentStopValLbl;
    QLabel* nextStopValLbl;
    QLabel* speedValLbl;
    QLabel* travelTimeValLbl;
    QLabel* etaValLbl;

    QPushButton* playPauseBtn;
    QPushButton* speedToggleBtn;
    QPushButton* nextStopBtn;
    QPushButton* resetBtn;
    QPushButton* bookEmptySeatBtn;

    QLabel* timelineLbl;

    MapCanvas* mapCanvas;
    QTimer* animTimer;

    float busProgress;
    bool isLiveTracking;
    int simSpeedMultiplier;
    int currentSpeedKmh;
    int lastPassedStopIdx;

    void setupUI();
    void updateMapForSelectedBus();
    void updateTelemetry();

protected:
    void showEvent(QShowEvent* event) override;

private slots:
    void handleBusChanged(int index);
    void handleToggleTracking();
    void handleToggleSpeed();
    void handleNextStop();
    void handleReset();
    void handleBookEmptySeat();
    void onGpsTick();

public slots:
    void refreshData();

public:
    explicit BusMapWidget(FileManager* fm, QWidget* parent = nullptr);
    ~BusMapWidget() override = default;

    void selectBus(const QString& busId);
};

#endif // BUSMAPWIDGET_H
