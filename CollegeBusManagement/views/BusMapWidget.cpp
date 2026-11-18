#include "BusMapWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPainter>
#include <QPainterPath>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QFrame>
#include <cmath>

// ============================================================================
// MAP CANVAS IMPLEMENTATION
// ============================================================================
MapCanvas::MapCanvas(QWidget* parent)
    : QWidget(parent), busProgress(0.0f), speedKmh(40),
      busId("BUS01"), shiftType("Full Day"), pulsePhase(0) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumHeight(420);

    backgroundMap.load(":/city_map_bg.png");
    if (backgroundMap.isNull()) {
        backgroundMap.load("resources/city_map_bg.png");
    }
    if (backgroundMap.isNull()) {
        backgroundMap.load("../resources/city_map_bg.png");
    }
}

void MapCanvas::setRouteStops(const std::vector<std::string>& stopNames) {
    stops.clear();
    std::vector<std::string> names = stopNames;
    if (names.size() < 2) {
        names = {"City Center", "Metro Gate 3", "Sector 14", "Tech Park", "College Campus"};
    }

    for (size_t i = 0; i < names.size(); ++i) {
        MapStop ms;
        ms.name = QString::fromStdString(names[i]);
        ms.pos = QPointF(0, 0);
        ms.isOrigin = (i == 0);
        ms.isDestination = (i == names.size() - 1);

        int totalRouteMinutes = 40;
        int stopOffsetMins = (names.size() > 1) ? static_cast<int>(i * totalRouteMinutes / (names.size() - 1)) : 0;
        int hour = 8 + (stopOffsetMins / 60);
        int minute = stopOffsetMins % 60;
        ms.scheduledTime = QString("%1:%2 AM")
            .arg(hour, 2, 10, QChar('0'))
            .arg(minute, 2, 10, QChar('0'));

        stops.push_back(ms);
    }
    recalculatePositions();
    update();
}

void MapCanvas::recalculatePositions() {
    if (stops.size() < 2) return;

    float w = static_cast<float>(width() > 100 ? width() : 800);
    float h = static_cast<float>(height() > 100 ? height() : 400);

    float startX = 90.0f;
    float endX = w - 100.0f;
    float midY = h * 0.50f;

    int n = static_cast<int>(stops.size());
    for (int i = 0; i < n; ++i) {
        float frac = static_cast<float>(i) / (n - 1);
        float x = startX + frac * (endX - startX);
        // Realistic S-curve highway trajectory that winds past city districts and across the river
        float y = midY + std::sin(frac * 3.14159265f * 1.5f) * (h * 0.22f) - (std::cos(frac * 3.14159265f * 2.0f) * (h * 0.08f));
        if (y < 80.0f) y = 80.0f;
        if (y > h - 75.0f) y = h - 75.0f;
        stops[i].pos = QPointF(x, y);
    }
}

void MapCanvas::resizeEvent(QResizeEvent* /*event*/) {
    recalculatePositions();
}

void MapCanvas::updateBusState(float progress, int speed, const std::string& id, const std::string& shift) {
    busProgress = progress;
    speedKmh = speed;
    busId = id;
    shiftType = shift;
    update();
}

QPointF MapCanvas::getInterpolatedBusPos() const {
    if (stops.size() < 2) return QPointF(100, 200);

    float totalSegments = static_cast<float>(stops.size() - 1);
    float scaled = busProgress * totalSegments;
    int segIndex = static_cast<int>(std::floor(scaled));

    if (segIndex >= static_cast<int>(stops.size() - 1)) {
        return stops.back().pos;
    }
    if (segIndex < 0) return stops.front().pos;

    float segFrac = scaled - segIndex;
    QPointF p1 = stops[segIndex].pos;
    QPointF p2 = stops[segIndex + 1].pos;

    return QPointF(p1.x() + segFrac * (p2.x() - p1.x()),
                   p1.y() + segFrac * (p2.y() - p1.y()));
}

float MapCanvas::getBusHeadingAngle() const {
    if (stops.size() < 2) return 0.0f;
    float totalSegments = static_cast<float>(stops.size() - 1);
    float scaled = busProgress * totalSegments;
    int segIndex = static_cast<int>(std::floor(scaled));

    if (segIndex >= static_cast<int>(stops.size() - 1)) {
        segIndex = static_cast<int>(stops.size() - 2);
    }
    if (segIndex < 0) segIndex = 0;

    QPointF p1 = stops[segIndex].pos;
    QPointF p2 = stops[segIndex + 1].pos;
    float dx = p2.x() - p1.x();
    float dy = p2.y() - p1.y();
    return std::atan2(dy, dx) * 180.0f / 3.14159265f;
}

void MapCanvas::drawBusVehicle(QPainter& painter, const QPointF& pos, float angleDeg) {
    painter.save();
    painter.translate(pos);
    painter.rotate(angleDeg);

    // 1. Headlights Projection Cone (Illuminates the road ahead)
    QPolygonF lightCone;
    lightCone << QPointF(16, -3) << QPointF(65, -24) << QPointF(65, 24) << QPointF(16, 3);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(254, 240, 138, 75)); // Soft bright yellow translucent beam
    painter.drawPolygon(lightCone);

    // 2. Bus Drop Shadow
    painter.setBrush(QColor(0, 0, 0, 45));
    painter.drawRoundedRect(QRectF(-20, -10, 42, 24), 5, 5);

    // 3. Four Black Rubber Wheels / Tires
    painter.setBrush(QColor("#1c1917"));
    painter.drawRoundedRect(QRectF(-14, -13, 8, 3), 1, 1);
    painter.drawRoundedRect(QRectF(8, -13, 8, 3), 1, 1);
    painter.drawRoundedRect(QRectF(-14, 10, 8, 3), 1, 1);
    painter.drawRoundedRect(QRectF(8, 10, 8, 3), 1, 1);

    // 4. Main Bus Chassis (Bright School Bus Amber/Gold)
    painter.setBrush(QColor("#f59e0b"));
    painter.setPen(QPen(QColor("#b45309"), 1.5));
    painter.drawRoundedRect(QRectF(-20, -11, 40, 22), 4, 4);

    // 5. Roof Accent Stripe (Royal Gold)
    painter.setBrush(QColor("#d97706"));
    painter.setPen(Qt::NoPen);
    painter.drawRect(QRectF(-13, -4, 25, 8));

    // 6. Front Windshield (Aerodynamic Sky Blue Glass)
    painter.setBrush(QColor("#38bdf8"));
    painter.setPen(QPen(QColor("#0284c7"), 1));
    painter.drawRoundedRect(QRectF(13, -8, 5, 16), 2, 2);

    // 7. Side Passenger Windows
    painter.setBrush(QColor("#0284c7"));
    painter.drawRect(QRectF(-16, -9, 5, 4));
    painter.drawRect(QRectF(-9, -9, 5, 4));
    painter.drawRect(QRectF(-2, -9, 5, 4));
    painter.drawRect(QRectF(5, -9, 5, 4));

    painter.drawRect(QRectF(-16, 5, 5, 4));
    painter.drawRect(QRectF(-9, 5, 5, 4));
    painter.drawRect(QRectF(-2, 5, 5, 4));
    painter.drawRect(QRectF(5, 5, 5, 4));

    // 8. Headlights (Bright Yellow Circles in front)
    painter.setBrush(QColor("#fef08a"));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(20, -7), 2, 2);
    painter.drawEllipse(QPointF(20, 7), 2, 2);

    // 9. Rear Red Brake Lights
    painter.setBrush(QColor("#ef4444"));
    painter.drawEllipse(QPointF(-20, -7), 2, 2);
    painter.drawEllipse(QPointF(-20, 7), 2, 2);

    painter.restore();
}

void MapCanvas::drawProceduralMap(QPainter& painter) {
    // Landmass base
    painter.fillRect(rect(), QColor("#f6f4ee"));

    // Water bodies (Lakes & Rivers)
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#bfdbfe")); // Water blue
    QPainterPath river;
    river.moveTo(-20, height() * 0.72);
    river.cubicTo(width() * 0.25, height() * 0.65, width() * 0.45, height() * 0.85, width() * 0.70, height() * 0.65);
    river.cubicTo(width() * 0.85, height() * 0.50, width() * 0.95, height() * 0.40, width() + 20, height() * 0.38);
    painter.setPen(QPen(QColor("#93c5fd"), 24, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawPath(river);

    // North Lake
    painter.setPen(QPen(QColor("#93c5fd"), 2));
    painter.setBrush(QColor("#bfdbfe"));
    painter.drawEllipse(QRectF(width() * 0.10, height() * 0.14, width() * 0.15, height() * 0.20));

    // Green Parks & Gardens
    painter.setPen(QPen(QColor("#86efac"), 2));
    painter.setBrush(QColor("#dcfce7"));
    painter.drawRoundedRect(QRectF(width() * 0.38, height() * 0.15, width() * 0.16, height() * 0.18), 16, 16);
    painter.drawRoundedRect(QRectF(width() * 0.72, height() * 0.65, width() * 0.18, height() * 0.18), 16, 16);

    // Secondary city street grid
    painter.setPen(QPen(QColor("#ffffff"), 4));
    for (int x = 40; x < width(); x += 65) painter.drawLine(x, 0, x, height());
    for (int y = 30; y < height(); y += 50) painter.drawLine(0, y, width(), y);

    // Arterial Expressways (warm peach/orange highways)
    painter.setPen(QPen(QColor("#fed7aa"), 10, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawLine(0, height() * 0.25, width(), height() * 0.75);
    painter.drawLine(width() * 0.50, 0, width() * 0.50, height());
}

void MapCanvas::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Ensure positions are fitted to canvas dimensions
    recalculatePositions();

    // 1. Draw Real City Street Map Background
    if (backgroundMap.isNull()) {
        backgroundMap.load(":/city_map_bg.png");
        if (backgroundMap.isNull()) backgroundMap.load("resources/city_map_bg.png");
        if (backgroundMap.isNull()) backgroundMap.load("../resources/city_map_bg.png");
    }

    if (!backgroundMap.isNull()) {
        painter.drawPixmap(rect(), backgroundMap);
    } else {
        drawProceduralMap(painter);
    }

    if (stops.size() < 2) return;

    // 2. Draw Highway Road Underlay (Wide Casing)
    QPainterPath fullRoadPath;
    fullRoadPath.moveTo(stops[0].pos);
    for (size_t i = 1; i < stops.size(); ++i) {
        fullRoadPath.lineTo(stops[i].pos);
    }
    painter.setPen(QPen(QColor("#78716c"), 16, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawPath(fullRoadPath);
    painter.setPen(QPen(QColor("#f5f5f4"), 12, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawPath(fullRoadPath);

    // Current bus interpolated position & segments
    QPointF busPos = getInterpolatedBusPos();
    float totalSegments = static_cast<float>(stops.size() - 1);
    float scaled = busProgress * totalSegments;
    int segIndex = static_cast<int>(std::floor(scaled));
    if (segIndex >= static_cast<int>(stops.size() - 1)) segIndex = static_cast<int>(stops.size() - 1);
    if (segIndex < 0) segIndex = 0;

    // 3. Traveled Route Segment (Solid Royal Gold Transit Lane)
    QPainterPath traveledPath;
    traveledPath.moveTo(stops[0].pos);
    for (int i = 1; i <= segIndex && i < static_cast<int>(stops.size()); ++i) {
        traveledPath.lineTo(stops[i].pos);
    }
    traveledPath.lineTo(busPos);

    painter.setPen(QPen(QColor("#d97706"), 7, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawPath(traveledPath);

    // 4. Remaining Route Segment (Dashed Lane showing upcoming road to travel)
    QPainterPath remainingPath;
    remainingPath.moveTo(busPos);
    for (size_t i = segIndex + 1; i < stops.size(); ++i) {
        remainingPath.lineTo(stops[i].pos);
    }
    painter.setPen(QPen(QColor("#a8a29e"), 5, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawPath(remainingPath);

    // 5. Render Route Stops & Landmarks
    for (size_t i = 0; i < stops.size(); ++i) {
        const auto& s = stops[i];

        if (s.isDestination) {
            // College Campus Destination Pin
            painter.setBrush(QColor("#059669")); // Emerald
            painter.setPen(QPen(Qt::white, 3));
            painter.drawEllipse(s.pos, 16, 16);

            // Halo ring
            painter.setPen(QPen(QColor(5, 150, 105, 80), 6));
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(s.pos, 22, 22);

            // Label Card
            QRectF destCard(s.pos.x() - 80, s.pos.y() + 22, 160, 42);
            painter.setBrush(QColor(255, 255, 255, 240));
            painter.setPen(QPen(QColor("#059669"), 1.5));
            painter.drawRoundedRect(destCard, 6, 6);

            painter.setPen(QColor("#065f46"));
            painter.setFont(QFont("Segoe UI", 9, QFont::Bold));
            painter.drawText(QRectF(destCard.x(), destCard.y() + 3, destCard.width(), 18), Qt::AlignCenter, "🎓 DESTINATION");

            painter.setPen(QColor("#1e293b"));
            painter.setFont(QFont("Segoe UI", 8, QFont::Bold));
            painter.drawText(QRectF(destCard.x(), destCard.y() + 21, destCard.width(), 16), Qt::AlignCenter, s.name);
        } else if (s.isOrigin) {
            // Origin Terminal Pin
            painter.setBrush(QColor("#d97706")); // Gold
            painter.setPen(QPen(Qt::white, 3));
            painter.drawEllipse(s.pos, 15, 15);

            // Halo ring
            painter.setPen(QPen(QColor(217, 119, 6, 80), 6));
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(s.pos, 21, 21);

            // Label Card
            QRectF origCard(s.pos.x() - 80, s.pos.y() + 22, 160, 42);
            painter.setBrush(QColor(255, 255, 255, 240));
            painter.setPen(QPen(QColor("#d97706"), 1.5));
            painter.drawRoundedRect(origCard, 6, 6);

            painter.setPen(QColor("#92400e"));
            painter.setFont(QFont("Segoe UI", 9, QFont::Bold));
            painter.drawText(QRectF(origCard.x(), origCard.y() + 3, origCard.width(), 18), Qt::AlignCenter, "🚩 ORIGIN");

            painter.setPen(QColor("#1e293b"));
            painter.setFont(QFont("Segoe UI", 8, QFont::Bold));
            painter.drawText(QRectF(origCard.x(), origCard.y() + 21, origCard.width(), 16), Qt::AlignCenter, s.name);
        } else {
            // Intermediate Transit Stop
            painter.setBrush(QColor("#ffffff"));
            painter.setPen(QPen(QColor("#d97706"), 3));
            painter.drawEllipse(s.pos, 11, 11);

            // Stop number
            painter.setPen(QColor("#b45309"));
            painter.setFont(QFont("Segoe UI", 8, QFont::Bold));
            painter.drawText(QRectF(s.pos.x() - 10, s.pos.y() - 10, 20, 20), Qt::AlignCenter, QString::number(i));

            // Station name pill
            QRectF stopPill(s.pos.x() - 55, s.pos.y() + 18, 110, 32);
            painter.setBrush(QColor(255, 255, 255, 235));
            painter.setPen(QPen(QColor("#e2d6c3"), 1));
            painter.drawRoundedRect(stopPill, 4, 4);

            painter.setPen(QColor("#292524"));
            painter.setFont(QFont("Segoe UI", 8, QFont::Bold));
            painter.drawText(QRectF(stopPill.x(), stopPill.y() + 2, stopPill.width(), 15), Qt::AlignCenter, s.name);

            painter.setPen(QColor("#78716c"));
            painter.setFont(QFont("Segoe UI", 7));
            painter.drawText(QRectF(stopPill.x(), stopPill.y() + 16, stopPill.width(), 14), Qt::AlignCenter, s.scheduledTime);
        }
    }

    // 6. Draw Live Moving Bus with Animated GPS Radar Pulse
    pulsePhase = (pulsePhase + 1) % 60;
    float pR1 = 20.0f + (pulsePhase % 30);
    float pR2 = 30.0f + ((pulsePhase + 15) % 30);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(217, 119, 6, static_cast<int>(55 * (1.0f - (pR1 - 20) / 30.0f))));
    painter.drawEllipse(busPos, pR1, pR1);
    painter.setBrush(QColor(217, 119, 6, static_cast<int>(35 * (1.0f - (pR2 - 30) / 30.0f))));
    painter.drawEllipse(busPos, pR2, pR2);

    // Draw the 2D Bus Vehicle oriented in travel direction
    float headingAngle = getBusHeadingAngle();
    drawBusVehicle(painter, busPos, headingAngle);

    // Floating Live Vehicle Telemetry Card with Bus Travelling Time
    QRectF cardRect(busPos.x() - 105, busPos.y() - 72, 210, 52);
    if (cardRect.top() < 48) cardRect.moveTop(busPos.y() + 26);
    if (cardRect.left() < 10) cardRect.moveLeft(10);
    if (cardRect.right() > width() - 10) cardRect.moveRight(width() - 10);

    painter.setBrush(QColor(255, 255, 255, 245));
    painter.setPen(QPen(QColor("#d97706"), 2.0));
    painter.drawRoundedRect(cardRect, 8, 8);

    int totalTripMins = 40;
    int elapsedMins = static_cast<int>(busProgress * totalTripMins);
    int remainingMins = totalTripMins - elapsedMins;
    int progressPct = static_cast<int>(busProgress * 100.0f);

    painter.setPen(QColor("#78350f"));
    painter.setFont(QFont("Segoe UI", 9, QFont::Bold));
    painter.drawText(QRectF(cardRect.x(), cardRect.y() + 4, cardRect.width(), 16), Qt::AlignCenter,
        QString("🚌 %1 | %2 km/h (%3)").arg(QString::fromStdString(busId)).arg(speedKmh).arg(QString::fromStdString(shiftType)));

    painter.setPen(QColor("#b45309"));
    painter.setFont(QFont("Segoe UI", 8, QFont::Bold));
    painter.drawText(QRectF(cardRect.x(), cardRect.y() + 20, cardRect.width(), 16), Qt::AlignCenter,
        QString("⏱️ Traveled: %1m (%2%) | ETA: %3m").arg(elapsedMins).arg(progressPct).arg(remainingMins));

    QString headingStr = (segIndex + 1 < static_cast<int>(stops.size())) ? stops[segIndex + 1].name : stops.back().name;
    painter.setPen(QColor("#059669"));
    painter.setFont(QFont("Segoe UI", 8, QFont::Bold));
    painter.drawText(QRectF(cardRect.x(), cardRect.y() + 35, cardRect.width(), 14), Qt::AlignCenter,
        QString("➔ Heading to: %1").arg(headingStr));

    // 7. Journey Banner at Top of Canvas (showing traveling from here to here)
    QRectF bannerRect(width() * 0.10, 10, width() * 0.80, 32);
    painter.setBrush(QColor(255, 255, 255, 240));
    painter.setPen(QPen(QColor("#d97706"), 1.5));
    painter.drawRoundedRect(bannerRect, 6, 6);

    painter.setPen(QColor("#78350f"));
    painter.setFont(QFont("Segoe UI", 9, QFont::Bold));
    QString journeyText = QString("🗺️ BUS TRANSIT: 🚩 %1  ════ 🚌 BUS MOVING (%2%) ════➔  🎓 %3")
        .arg(stops.front().name)
        .arg(progressPct)
        .arg(stops.back().name);
    painter.drawText(bannerRect, Qt::AlignCenter, journeyText);

    // 8. Map Legend (Bottom Right)
    QRectF legendRect(width() - 170, height() - 78, 160, 68);
    painter.setBrush(QColor(255, 255, 255, 230));
    painter.setPen(QPen(QColor("#e2d6c3"), 1));
    painter.drawRoundedRect(legendRect, 5, 5);

    painter.setFont(QFont("Segoe UI", 7, QFont::Bold));
    painter.setPen(QColor("#78350f"));
    painter.drawText(legendRect.x() + 8, legendRect.y() + 14, "🚩 Origin Stop");
    painter.drawText(legendRect.x() + 8, legendRect.y() + 30, "🎓 Destination (Campus)");
    painter.drawText(legendRect.x() + 8, legendRect.y() + 46, "🚌 Moving Bus (GPS)");
    painter.drawText(legendRect.x() + 8, legendRect.y() + 62, "═ Traveled Highway");
}

// ============================================================================
// BUS MAP WIDGET CONTAINER IMPLEMENTATION
// ============================================================================
BusMapWidget::BusMapWidget(FileManager* fm, QWidget* parent)
    : QWidget(parent), fileManager(fm),
      busProgress(0.10f), isLiveTracking(true),
      simSpeedMultiplier(1), currentSpeedKmh(42),
      lastPassedStopIdx(-1) {

    setupUI();

    animTimer = new QTimer(this);
    connect(animTimer, &QTimer::timeout, this, &BusMapWidget::onGpsTick);
    animTimer->start(40); // 25 FPS smooth real-time animation by default

    refreshData();
}

void BusMapWidget::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(12);

    // 1. Top Control Bar (Clean 2-Row Design - No text clipping)
    auto* topCard = new QFrame(this);
    topCard->setStyleSheet("background-color: #ffffff; border: 1px solid #e0e7ff; border-radius: 10px; padding: 12px;");
    auto* topCardLayout = new QVBoxLayout(topCard);
    topCardLayout->setSpacing(10);
    topCardLayout->setContentsMargins(10, 8, 10, 8);

    // Row 1: Bus Selector & Live Status Badges
    auto* row1 = new QHBoxLayout();
    row1->setSpacing(12);

    auto* busSelectLbl = new QLabel("🚌 Select Bus:", topCard);
    busSelectLbl->setStyleSheet("font-weight: bold; font-size: 13px; color: #1c1917;");
    row1->addWidget(busSelectLbl);

    busSelectorCombo = new QComboBox(topCard);
    busSelectorCombo->setMinimumWidth(140);
    busSelectorCombo->setStyleSheet("font-size: 13px; font-weight: 600; padding: 6px 10px; border: 1.5px solid #d97706;");
    row1->addWidget(busSelectorCombo);

    row1->addSpacing(8);

    // Live Shift Badge
    shiftBadgeLbl = new QLabel("Shift: Full Day", topCard);
    shiftBadgeLbl->setStyleSheet("background-color: #fef3c7; color: #92400e; padding: 6px 12px; border-radius: 12px; font-weight: bold; font-size: 12px; border: 1px solid #fde68a;");
    row1->addWidget(shiftBadgeLbl);

    // Empty Seats Badge
    emptySeatsBadgeLbl = new QLabel("💺 0 Empty Seats", topCard);
    emptySeatsBadgeLbl->setStyleSheet("background-color: #ecfdf5; color: #065f46; padding: 6px 12px; border-radius: 12px; font-weight: bold; font-size: 12px; border: 1px solid #a7f3d0;");
    row1->addWidget(emptySeatsBadgeLbl);

    // GPS Status Indicator
    liveGpsIndicatorLbl = new QLabel("🟢 GPS ONLINE", topCard);
    liveGpsIndicatorLbl->setStyleSheet("background-color: #f0fdf4; color: #15803d; border: 1px solid #bbf7d0; padding: 6px 12px; border-radius: 12px; font-weight: bold; font-size: 12px;");
    row1->addWidget(liveGpsIndicatorLbl);

    row1->addStretch();
    topCardLayout->addLayout(row1);

    // Row 2: Playback & Action Controls (Wide, comfortable, bold, readable)
    auto* row2 = new QHBoxLayout();
    row2->setSpacing(10);

    playPauseBtn = new QPushButton("⏸ Pause Tracking", topCard);
    playPauseBtn->setStyleSheet(
        "QPushButton { background-color: #d97706; color: #ffffff; padding: 8px 16px; border-radius: 6px; font-weight: 700; font-size: 13px; min-width: 125px; min-height: 38px; }"
        "QPushButton:hover { background-color: #b45309; }"
    );
    playPauseBtn->setCursor(Qt::PointingHandCursor);

    speedToggleBtn = new QPushButton("⚡ Speed: 1x", topCard);
    speedToggleBtn->setStyleSheet(
        "QPushButton { background-color: #fffbeb; color: #78350f; border: 1.5px solid #f59e0b; padding: 8px 16px; border-radius: 6px; font-weight: 700; font-size: 13px; min-width: 105px; min-height: 38px; }"
        "QPushButton:hover { background-color: #fef3c7; border-color: #d97706; }"
    );
    speedToggleBtn->setCursor(Qt::PointingHandCursor);

    nextStopBtn = new QPushButton("⏭ Skip to Next Stop", topCard);
    nextStopBtn->setStyleSheet(
        "QPushButton { background-color: #fffbeb; color: #78350f; border: 1.5px solid #f59e0b; padding: 8px 16px; border-radius: 6px; font-weight: 700; font-size: 13px; min-width: 145px; min-height: 38px; }"
        "QPushButton:hover { background-color: #fef3c7; border-color: #d97706; }"
    );
    nextStopBtn->setCursor(Qt::PointingHandCursor);

    resetBtn = new QPushButton("🔄 Reset Route", topCard);
    resetBtn->setStyleSheet(
        "QPushButton { background-color: #fffbeb; color: #78350f; border: 1.5px solid #f59e0b; padding: 8px 16px; border-radius: 6px; font-weight: 700; font-size: 13px; min-width: 115px; min-height: 38px; }"
        "QPushButton:hover { background-color: #fef3c7; border-color: #d97706; }"
    );
    resetBtn->setCursor(Qt::PointingHandCursor);

    row2->addWidget(playPauseBtn);
    row2->addWidget(speedToggleBtn);
    row2->addWidget(nextStopBtn);
    row2->addWidget(resetBtn);

    row2->addStretch();

    bookEmptySeatBtn = new QPushButton("🎟️ Book Empty Seat", topCard);
    bookEmptySeatBtn->setStyleSheet(
        "QPushButton { background-color: #059669; color: #ffffff; padding: 8px 18px; border-radius: 6px; font-weight: 700; font-size: 13px; min-width: 155px; min-height: 38px; }"
        "QPushButton:hover { background-color: #047857; }"
    );
    bookEmptySeatBtn->setCursor(Qt::PointingHandCursor);
    row2->addWidget(bookEmptySeatBtn);

    topCardLayout->addLayout(row2);
    mainLayout->addWidget(topCard);

    // 2. Telemetry HUD Bar (Champagne White & Royal Gold Accent - No Black!)
    auto* hudCard = new QFrame(this);
    hudCard->setStyleSheet("background-color: #ffffff; border-radius: 8px; padding: 12px 20px; border: 2px solid #d97706;");
    auto* hudLayout = new QHBoxLayout(hudCard);

    currentStopValLbl = new QLabel("📍 Current Station: Origin Terminal", hudCard);
    currentStopValLbl->setStyleSheet("color: #78350f; font-weight: 700; font-size: 13px;");
    hudLayout->addWidget(currentStopValLbl);

    hudLayout->addSpacing(25);

    nextStopValLbl = new QLabel("➔ Next Stop: Loading...", hudCard);
    nextStopValLbl->setStyleSheet("color: #b45309; font-weight: 700; font-size: 13px;");
    hudLayout->addWidget(nextStopValLbl);

    hudLayout->addSpacing(25);

    speedValLbl = new QLabel("Speed: 42 km/h", hudCard);
    speedValLbl->setStyleSheet("color: #059669; font-weight: 700; font-size: 13px;");
    hudLayout->addWidget(speedValLbl);

    hudLayout->addSpacing(25);

    travelTimeValLbl = new QLabel("⏱️ Travel Time: 0m / 40m", hudCard);
    travelTimeValLbl->setStyleSheet("color: #d97706; font-weight: 700; font-size: 13px;");
    hudLayout->addWidget(travelTimeValLbl);

    hudLayout->addSpacing(25);

    etaValLbl = new QLabel("ETA: 3 mins", hudCard);
    etaValLbl->setStyleSheet("color: #92400e; font-weight: 700; font-size: 13px;");
    hudLayout->addWidget(etaValLbl);

    hudLayout->addStretch();
    mainLayout->addWidget(hudCard);

    // 3. Map Canvas Area (Fills available space)
    mapCanvas = new MapCanvas(this);
    mainLayout->addWidget(mapCanvas, 1);

    // 4. Route Timeline Progression Bar at Bottom
    timelineLbl = new QLabel("Route Stops Sequence: (Loading...)", this);
    timelineLbl->setStyleSheet("background-color: #ffffff; border: 1px solid #e2e8f0; border-radius: 6px; padding: 8px 14px; font-size: 12px; font-weight: 600; color: #475569;");
    mainLayout->addWidget(timelineLbl);

    // Connect signals
    connect(busSelectorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BusMapWidget::handleBusChanged);
    connect(playPauseBtn, &QPushButton::clicked, this, &BusMapWidget::handleToggleTracking);
    connect(speedToggleBtn, &QPushButton::clicked, this, &BusMapWidget::handleToggleSpeed);
    connect(nextStopBtn, &QPushButton::clicked, this, &BusMapWidget::handleNextStop);
    connect(resetBtn, &QPushButton::clicked, this, &BusMapWidget::handleReset);
    connect(bookEmptySeatBtn, &QPushButton::clicked, this, &BusMapWidget::handleBookEmptySeat);
}

void BusMapWidget::showEvent(QShowEvent* /*event*/) {
    refreshData();
    mapCanvas->recalculatePositions();
    if (isLiveTracking && !animTimer->isActive()) {
        animTimer->start(40);
    }
}

void BusMapWidget::refreshData() {
    QString currentBus = busSelectorCombo->currentText();
    busSelectorCombo->blockSignals(true);
    busSelectorCombo->clear();

    auto buses = fileManager->getAllBuses();
    for (const auto& b : buses) {
        busSelectorCombo->addItem(QString::fromStdString(b.getBusId()));
    }
    busSelectorCombo->blockSignals(false);

    int idx = busSelectorCombo->findText(currentBus);
    if (idx >= 0) {
        busSelectorCombo->setCurrentIndex(idx);
    } else if (busSelectorCombo->count() > 0) {
        busSelectorCombo->setCurrentIndex(0);
    }

    handleBusChanged(busSelectorCombo->currentIndex());
}

void BusMapWidget::handleBusChanged(int /*index*/) {
    if (busSelectorCombo->count() == 0) return;
    std::string bId = busSelectorCombo->currentText().toStdString();
    Bus b = fileManager->getBusById(bId);
    if (b.getBusId().empty()) return;

    // Shift Type Badge
    QString shift = QString::fromStdString(b.getShiftType());
    if (shift.contains("Half")) {
        shiftBadgeLbl->setText("Shift: " + shift);
        shiftBadgeLbl->setStyleSheet("background-color: #ffedd5; color: #c2410c; padding: 6px 12px; border-radius: 12px; font-weight: bold; font-size: 12px; border: 1px solid #fed7aa;");
    } else {
        shiftBadgeLbl->setText("Shift: Full Day");
        shiftBadgeLbl->setStyleSheet("background-color: #ede9fe; color: #4338ca; padding: 6px 12px; border-radius: 12px; font-weight: bold; font-size: 12px; border: 1px solid #ddd6fe;");
    }

    // Empty Seats Badge
    auto emptySeats = fileManager->getEmptySeatsForBus(bId);
    emptySeatsBadgeLbl->setText(QString("💺 %1 Empty Seats").arg(emptySeats.size()));

    updateMapForSelectedBus();
    updateTelemetry();
}

void BusMapWidget::updateMapForSelectedBus() {
    if (busSelectorCombo->count() == 0) return;
    std::string bId = busSelectorCombo->currentText().toStdString();
    Bus b = fileManager->getBusById(bId);
    Route r = fileManager->getRouteById(b.getRouteId());

    std::vector<std::string> stopList;
    std::string startP = r.getStartPoint().empty() ? "Origin Terminal" : r.getStartPoint();
    stopList.push_back(startP);
    for (const auto& s : r.getStops()) {
        if (!s.empty()) stopList.push_back(s);
    }
    std::string endP = r.getEndPoint().empty() ? "College Campus" : r.getEndPoint();
    stopList.push_back(endP);

    mapCanvas->setRouteStops(stopList);

    // Build timeline string with stop scheduled arrival times
    QStringList tl;
    for (size_t i = 0; i < stopList.size(); ++i) {
        int stopOffsetMins = (stopList.size() > 1) ? static_cast<int>(i * 40 / (stopList.size() - 1)) : 0;
        int h = 8 + (stopOffsetMins / 60);
        int m = stopOffsetMins % 60;
        QString timeStr = QString("%1:%2 AM").arg(h, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0'));
        tl << QString("[%1] %2 (%3)").arg(i + 1).arg(QString::fromStdString(stopList[i])).arg(timeStr);
    }
    timelineLbl->setText("Route Progression & Stop Timings: " + tl.join("  ➔  "));
}

void BusMapWidget::onGpsTick() {
    float step = 0.0025f * simSpeedMultiplier;
    busProgress += step;
    if (busProgress >= 1.0f) {
        busProgress = 0.0f; // Loop route
    }

    currentSpeedKmh = 36 + (static_cast<int>(busProgress * 100) % 18);

    std::string bId = busSelectorCombo->currentText().toStdString();
    Bus b = fileManager->getBusById(bId);

    mapCanvas->updateBusState(busProgress, currentSpeedKmh, b.getBusId(), b.getShiftType());
    updateTelemetry();
}

void BusMapWidget::updateTelemetry() {
    const auto& stops = mapCanvas->getStops();
    if (stops.size() < 2) return;

    float totalSegments = static_cast<float>(stops.size() - 1);
    float scaled = busProgress * totalSegments;
    int currIdx = static_cast<int>(std::floor(scaled));
    int nextIdx = (currIdx + 1 < static_cast<int>(stops.size())) ? currIdx + 1 : currIdx;

    QString currStop = stops[currIdx].name;
    QString nextStop = stops[nextIdx].name;

    currentStopValLbl->setText(QString("📍 Current: %1").arg(currStop));

    int totalMins = 40;
    int elapsedMins = static_cast<int>(busProgress * totalMins);
    int remainingMins = totalMins - elapsedMins;
    int nextStopMins = 1 + (static_cast<int>(std::ceil((1.0f - (scaled - currIdx)) * (totalMins / totalSegments))));
    if (nextStopMins < 1) nextStopMins = 1;

    nextStopValLbl->setText(QString("➔ Next: %1 (in %2m)").arg(nextStop).arg(nextStopMins));
    speedValLbl->setText(QString("Speed: %1 km/h").arg(currentSpeedKmh));
    travelTimeValLbl->setText(QString("⏱️ Travel Time: %1m done | %2m left").arg(elapsedMins).arg(remainingMins));
    etaValLbl->setText(QString("🎓 Campus ETA: %1m").arg(remainingMins));

    // Update persistent bus location if passed a new stop
    if (currIdx != lastPassedStopIdx) {
        lastPassedStopIdx = currIdx;
        std::string bId = busSelectorCombo->currentText().toStdString();
        Bus b = fileManager->getBusById(bId);
        if (!b.getBusId().empty()) {
            b.setCurrentLocation(currStop.toStdString());
            fileManager->updateBus(b);
        }
    }
}

void BusMapWidget::handleToggleTracking() {
    isLiveTracking = !isLiveTracking;
    if (isLiveTracking) {
        playPauseBtn->setText("⏸ Pause Tracking");
        playPauseBtn->setStyleSheet(
            "QPushButton { background-color: #d97706; color: #ffffff; padding: 8px 16px; border-radius: 6px; font-weight: 700; font-size: 13px; min-width: 125px; }"
            "QPushButton:hover { background-color: #b45309; }"
        );
        liveGpsIndicatorLbl->setText("🟢 GPS ONLINE");
        liveGpsIndicatorLbl->setStyleSheet("background-color: #f0fdf4; color: #15803d; border: 1px solid #bbf7d0; padding: 6px 12px; border-radius: 12px; font-weight: bold; font-size: 12px;");
        animTimer->start(40);
    } else {
        playPauseBtn->setText("▶ Resume Tracking");
        playPauseBtn->setStyleSheet(
            "QPushButton { background-color: #b45309; color: #ffffff; padding: 8px 16px; border-radius: 6px; font-weight: 700; font-size: 13px; min-width: 125px; }"
            "QPushButton:hover { background-color: #92400e; }"
        );
        liveGpsIndicatorLbl->setText("⏸ GPS PAUSED");
        liveGpsIndicatorLbl->setStyleSheet("background-color: #fef2f2; color: #e11d48; border: 1px solid #fecdd3; padding: 6px 12px; border-radius: 12px; font-weight: bold; font-size: 12px;");
        animTimer->stop();
    }
}

void BusMapWidget::handleToggleSpeed() {
    if (simSpeedMultiplier == 1) {
        simSpeedMultiplier = 2;
        speedToggleBtn->setText("⚡ Speed: 2x");
    } else if (simSpeedMultiplier == 2) {
        simSpeedMultiplier = 4;
        speedToggleBtn->setText("⚡ Speed: 4x");
    } else {
        simSpeedMultiplier = 1;
        speedToggleBtn->setText("⚡ Speed: 1x");
    }
}

void BusMapWidget::handleNextStop() {
    const auto& stops = mapCanvas->getStops();
    if (stops.size() < 2) return;

    float step = 1.0f / (stops.size() - 1);
    busProgress += step;
    if (busProgress >= 1.0f) busProgress = 0.0f;

    std::string bId = busSelectorCombo->currentText().toStdString();
    Bus b = fileManager->getBusById(bId);
    mapCanvas->updateBusState(busProgress, currentSpeedKmh, b.getBusId(), b.getShiftType());
    updateTelemetry();
}

void BusMapWidget::handleReset() {
    busProgress = 0.0f;
    std::string bId = busSelectorCombo->currentText().toStdString();
    Bus b = fileManager->getBusById(bId);
    mapCanvas->updateBusState(busProgress, currentSpeedKmh, b.getBusId(), b.getShiftType());
    updateTelemetry();
}

void BusMapWidget::handleBookEmptySeat() {
    if (busSelectorCombo->count() == 0) return;
    std::string bId = busSelectorCombo->currentText().toStdString();
    Bus b = fileManager->getBusById(bId);

    auto emptySeats = fileManager->getEmptySeatsForBus(bId);
    if (emptySeats.empty()) {
        QMessageBox::warning(this, "Bus Full", QString("Bus %1 has no empty seats remaining!").arg(QString::fromStdString(bId)));
        return;
    }

    QDialog dlg(this);
    dlg.setWindowTitle(QString("Book Empty Seat - Bus %1 (%2)").arg(QString::fromStdString(bId), QString::fromStdString(b.getShiftType())));
    dlg.setFixedSize(450, 320);

    auto* layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    auto* seatCombo = new QComboBox(&dlg);
    for (int sNum : emptySeats) {
        seatCombo->addItem(QString("Seat #%1 (Empty)").arg(sNum), sNum);
    }

    auto* nameEdit = new QLineEdit(&dlg);
    nameEdit->setPlaceholderText("Passenger / Commuter Name");

    auto* contactEdit = new QLineEdit(&dlg);
    contactEdit->setPlaceholderText("Phone Number or Roll Number");

    auto* travelCombo = new QComboBox(&dlg);
    travelCombo->addItems({"Day Pass Traveler", "Ad-hoc Guest", "Faculty / Staff", "Substitute Student"});

    formLayout->addRow("Bus Fleet:", new QLabel(QString::fromStdString(b.getBusId()) + " (" + QString::fromStdString(b.getShiftType()) + ")", &dlg));
    formLayout->addRow("Available Empty Seat *:", seatCombo);
    formLayout->addRow("Passenger Name *:", nameEdit);
    formLayout->addRow("Contact / Roll No *:", contactEdit);
    formLayout->addRow("Travel Pass Type:", travelCombo);

    layout->addLayout(formLayout);

    auto* btnLayout = new QHBoxLayout();
    auto* bookBtn = new QPushButton("Confirm Seat Booking", &dlg);
    bookBtn->setProperty("class", "primaryButton");
    auto* cancelBtn = new QPushButton("Cancel", &dlg);
    cancelBtn->setProperty("class", "secondaryButton");

    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(bookBtn);
    layout->addLayout(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
    connect(bookBtn, &QPushButton::clicked, [&]() {
        QString name = nameEdit->text().trimmed();
        QString contact = contactEdit->text().trimmed();

        if (name.isEmpty() || contact.isEmpty()) {
            QMessageBox::warning(&dlg, "Input Error", "Please provide passenger name and contact.");
            return;
        }

        int seatNum = seatCombo->currentData().toInt();
        std::string travelType = travelCombo->currentText().toStdString();

        try {
            fileManager->quickBookEmptySeat(bId, seatNum, name.toStdString(), contact.toStdString(), travelType);
            QMessageBox::information(&dlg, "Seat Booked!",
                QString("Seat #%1 successfully booked on Bus %2 for %3 (%4).")
                    .arg(seatNum).arg(QString::fromStdString(bId), name, QString::fromStdString(travelType)));
            dlg.accept();
        } catch (const std::exception& e) {
            QMessageBox::critical(&dlg, "Booking Error", QString::fromStdString(e.what()));
        }
    });

    if (dlg.exec() == QDialog::Accepted) {
        refreshData();
    }
}

void BusMapWidget::selectBus(const QString& busId) {
    int idx = busSelectorCombo->findText(busId);
    if (idx != -1) {
        busSelectorCombo->setCurrentIndex(idx);
    }
}
