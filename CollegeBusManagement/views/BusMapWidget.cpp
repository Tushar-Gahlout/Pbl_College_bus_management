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
      busId("BUS01"), shiftType("Full Day") {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumHeight(380);
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
        stops.push_back(ms);
    }
    recalculatePositions();
    update();
}

void MapCanvas::recalculatePositions() {
    if (stops.size() < 2) return;

    float w = static_cast<float>(width() > 100 ? width() : 800);
    float h = static_cast<float>(height() > 100 ? height() : 400);

    float startX = 70.0f;
    float endX = w - 70.0f;
    float midY = h * 0.52f;

    int n = static_cast<int>(stops.size());
    for (int i = 0; i < n; ++i) {
        float frac = static_cast<float>(i) / (n - 1);
        float x = startX + frac * (endX - startX);
        // Realistic curved transit highway trajectory
        float y = midY + std::sin(frac * 3.14159f * 1.6f) * (h * 0.24f);
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

void MapCanvas::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 1. Canvas Base Map
    painter.fillRect(rect(), QColor("#f8fafc"));

    // 2. City Road Grid Pattern
    painter.setPen(QPen(QColor("#e2e8f0"), 1, Qt::DotLine));
    for (int y = 30; y < height(); y += 35) {
        painter.drawLine(0, y, width(), y);
    }
    for (int x = 30; x < width(); x += 35) {
        painter.drawLine(x, 0, x, height());
    }

    if (stops.size() < 2) return;

    // 3. Draw Highway Road Underlay (Wide Grey Border)
    QPainterPath roadPath;
    roadPath.moveTo(stops[0].pos);
    for (size_t i = 1; i < stops.size(); ++i) {
        roadPath.lineTo(stops[i].pos);
    }

    painter.setPen(QPen(QColor("#cbd5e1"), 16, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawPath(roadPath);

    // 4. Draw Active Transit Lane (Vibrant Electric Indigo)
    painter.setPen(QPen(QColor("#4f46e5"), 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawPath(roadPath);

    // 5. Render Route Stops & Landmarks
    for (size_t i = 0; i < stops.size(); ++i) {
        const auto& s = stops[i];

        if (s.isDestination) {
            // College Campus
            painter.setBrush(QColor("#059669"));
            painter.setPen(QPen(Qt::white, 3));
            painter.drawEllipse(s.pos, 16, 16);

            painter.setPen(QColor("#1e1b4b"));
            painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
            painter.drawText(s.pos.x() - 65, s.pos.y() + 30, 130, 20, Qt::AlignCenter, "🎓 " + s.name);
        } else if (s.isOrigin) {
            // Origin Terminal
            painter.setBrush(QColor("#4f46e5"));
            painter.setPen(QPen(Qt::white, 3));
            painter.drawEllipse(s.pos, 14, 14);

            painter.setPen(QColor("#1e1b4b"));
            painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
            painter.drawText(s.pos.x() - 65, s.pos.y() + 28, 130, 20, Qt::AlignCenter, "🚩 " + s.name);
        } else {
            // Intermediate Transit Stop
            painter.setBrush(QColor("#ffffff"));
            painter.setPen(QPen(QColor("#4f46e5"), 3));
            painter.drawEllipse(s.pos, 11, 11);

            // Stop number
            painter.setPen(QColor("#4f46e5"));
            painter.setFont(QFont("Segoe UI", 8, QFont::Bold));
            painter.drawText(QRectF(s.pos.x() - 10, s.pos.y() - 10, 20, 20), Qt::AlignCenter, QString::number(i));

            painter.setPen(QColor("#334155"));
            painter.setFont(QFont("Segoe UI", 9, QFont::Bold));
            painter.drawText(s.pos.x() - 60, s.pos.y() + 24, 120, 20, Qt::AlignCenter, s.name);
        }
    }

    // 6. Draw Live Moving Bus at Interpolated Position
    QPointF busPos = getInterpolatedBusPos();

    // Concentric Radar GPS Pulse Rings
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(79, 70, 229, 45));
    painter.drawEllipse(busPos, 34, 34);
    painter.setBrush(QColor(79, 70, 229, 85));
    painter.drawEllipse(busPos, 22, 22);

    // Bus Core Marker
    painter.setBrush(QColor("#f59e0b"));
    painter.setPen(QPen(Qt::white, 2));
    painter.drawEllipse(busPos, 14, 14);

    // Floating Live Vehicle Telemetry Card
    QRectF cardRect(busPos.x() - 85, busPos.y() - 54, 170, 36);
    painter.setBrush(QColor("#1e1b4b"));
    painter.setPen(QPen(QColor("#818cf8"), 1.5));
    painter.drawRoundedRect(cardRect, 6, 6);

    painter.setPen(Qt::white);
    painter.setFont(QFont("Segoe UI", 9, QFont::Bold));
    painter.drawText(cardRect, Qt::AlignCenter,
        QString("🚌 %1 | %2 km/h\n%3").arg(QString::fromStdString(busId)).arg(speedKmh).arg(QString::fromStdString(shiftType)));
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
    busSelectLbl->setStyleSheet("font-weight: bold; font-size: 13px; color: #1e1b4b;");
    row1->addWidget(busSelectLbl);

    busSelectorCombo = new QComboBox(topCard);
    busSelectorCombo->setMinimumWidth(140);
    busSelectorCombo->setStyleSheet("font-size: 13px; font-weight: 600; padding: 6px 10px;");
    row1->addWidget(busSelectorCombo);

    row1->addSpacing(8);

    // Live Shift Badge
    shiftBadgeLbl = new QLabel("Shift: Full Day", topCard);
    shiftBadgeLbl->setStyleSheet("background-color: #ede9fe; color: #4338ca; padding: 6px 12px; border-radius: 12px; font-weight: bold; font-size: 12px; border: 1px solid #ddd6fe;");
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
        "QPushButton { background-color: #ffffff; color: #312e81; border: 1.5px solid #c7d2fe; padding: 8px 16px; border-radius: 6px; font-weight: 700; font-size: 13px; min-width: 105px; min-height: 38px; }"
        "QPushButton:hover { background-color: #eef2ff; border-color: #818cf8; }"
    );
    speedToggleBtn->setCursor(Qt::PointingHandCursor);

    nextStopBtn = new QPushButton("⏭ Skip to Next Stop", topCard);
    nextStopBtn->setStyleSheet(
        "QPushButton { background-color: #ffffff; color: #312e81; border: 1.5px solid #c7d2fe; padding: 8px 16px; border-radius: 6px; font-weight: 700; font-size: 13px; min-width: 145px; min-height: 38px; }"
        "QPushButton:hover { background-color: #eef2ff; border-color: #818cf8; }"
    );
    nextStopBtn->setCursor(Qt::PointingHandCursor);

    resetBtn = new QPushButton("🔄 Reset Route", topCard);
    resetBtn->setStyleSheet(
        "QPushButton { background-color: #ffffff; color: #312e81; border: 1.5px solid #c7d2fe; padding: 8px 16px; border-radius: 6px; font-weight: 700; font-size: 13px; min-width: 115px; min-height: 38px; }"
        "QPushButton:hover { background-color: #eef2ff; border-color: #818cf8; }"
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

    // 2. Telemetry HUD Bar (Deep Indigo Theme)
    auto* hudCard = new QFrame(this);
    hudCard->setStyleSheet("background-color: #1e1b4b; border-radius: 8px; padding: 12px 20px; border: 1px solid #312e81;");
    auto* hudLayout = new QHBoxLayout(hudCard);

    currentStopValLbl = new QLabel("📍 Current Station: Origin Terminal", hudCard);
    currentStopValLbl->setStyleSheet("color: #a5b4fc; font-weight: 700; font-size: 13px;");
    hudLayout->addWidget(currentStopValLbl);

    hudLayout->addSpacing(25);

    nextStopValLbl = new QLabel("➔ Next Stop: Loading...", hudCard);
    nextStopValLbl->setStyleSheet("color: #fde047; font-weight: 700; font-size: 13px;");
    hudLayout->addWidget(nextStopValLbl);

    hudLayout->addSpacing(25);

    speedValLbl = new QLabel("Speed: 42 km/h", hudCard);
    speedValLbl->setStyleSheet("color: #34d399; font-weight: 700; font-size: 13px;");
    hudLayout->addWidget(speedValLbl);

    hudLayout->addSpacing(25);

    etaValLbl = new QLabel("ETA: 3 mins", hudCard);
    etaValLbl->setStyleSheet("color: #ffffff; font-weight: 700; font-size: 13px;");
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

    // Build timeline string
    QStringList tl;
    for (size_t i = 0; i < stopList.size(); ++i) {
        tl << QString("[%1] %2").arg(i + 1).arg(QString::fromStdString(stopList[i]));
    }
    timelineLbl->setText("Route Progression: " + tl.join("  ➔  "));
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

    currentStopValLbl->setText(QString("📍 Current Station: %1").arg(currStop));
    nextStopValLbl->setText(QString("➔ Next Stop: %1").arg(nextStop));
    speedValLbl->setText(QString("Speed: %1 km/h").arg(currentSpeedKmh));

    int mins = 1 + (static_cast<int>(std::ceil((1.0f - (scaled - currIdx)) * 4.0f)));
    etaValLbl->setText(QString("ETA: %1 mins").arg(mins));

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
            "QPushButton { background-color: #4f46e5; color: #ffffff; padding: 8px 16px; border-radius: 6px; font-weight: 700; font-size: 13px; min-width: 125px; }"
            "QPushButton:hover { background-color: #4338ca; }"
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
