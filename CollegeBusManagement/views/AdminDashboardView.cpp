#include "AdminDashboardView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QFrame>
#include <iomanip>
#include <sstream>

AdminDashboardView::AdminDashboardView(FileManager* fm, QWidget* parent)
    : QWidget(parent), fileManager(fm) {
    setupUI();
    refreshData();
}

#include <QScrollArea>

QWidget* AdminDashboardView::createMetricCard(const QString& title, QLabel*& valLabel, const QString& iconText, const QString& accentColor) {
    auto* card = new QFrame(this);
    card->setObjectName("metricCard");
    card->setMinimumHeight(92);
    card->setStyleSheet(QString(
        "QFrame#metricCard { background-color: #ffffff; border: 1.5px solid #cbd5e1; border-radius: 10px; padding: 12px 16px; border-left: 5px solid %1; }"
        "QFrame#metricCard:hover { border-color: %1; }"
    ).arg(accentColor));

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setSpacing(4);

    auto* topLayout = new QHBoxLayout();
    auto* titleLbl = new QLabel(title, card);
    titleLbl->setStyleSheet("font-size: 12px; font-weight: bold; color: #64748b; text-transform: uppercase;");
    auto* iconLbl = new QLabel(iconText, card);
    iconLbl->setStyleSheet("font-size: 20px;");
    topLayout->addWidget(titleLbl);
    topLayout->addStretch();
    topLayout->addWidget(iconLbl);

    valLabel = new QLabel("0", card);
    valLabel->setMinimumHeight(34);
    valLabel->setStyleSheet("font-size: 26px; font-weight: 800; color: #78350f; padding: 0px;");
    valLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    layout->addLayout(topLayout);
    layout->addWidget(valLabel);

    return card;
}

void AdminDashboardView::setupUI() {
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");

    auto* scrollContent = new QWidget(scrollArea);
    scrollContent->setStyleSheet("background: transparent;");

    auto* mainLayout = new QVBoxLayout(scrollContent);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(18);

    // Header section
    auto* headerLayout = new QHBoxLayout();
    auto* title = new QLabel("Administrator Overview", scrollContent);
    title->setStyleSheet("font-size: 22px; font-weight: bold; color: #78350f;");
    headerLayout->addWidget(title);
    headerLayout->addStretch();

    auto* refreshBtn = new QPushButton("🔄 Refresh Stats", scrollContent);
    refreshBtn->setStyleSheet(
        "QPushButton { background-color: #d97706; color: #ffffff; padding: 8px 18px; border-radius: 6px; font-weight: 700; font-size: 13px; min-height: 38px; border: 1px solid #b45309; }"
        "QPushButton:hover { background-color: #b45309; }"
    );
    refreshBtn->setCursor(Qt::PointingHandCursor);
    connect(refreshBtn, &QPushButton::clicked, this, &AdminDashboardView::refreshData);
    headerLayout->addWidget(refreshBtn);
    mainLayout->addLayout(headerLayout);

    // Metric Cards Grid
    auto* gridLayout = new QGridLayout();
    gridLayout->setSpacing(16);

    gridLayout->addWidget(createMetricCard("Total Students", totalStudentsVal, "🎓", "#d97706"), 0, 0);
    gridLayout->addWidget(createMetricCard("Total Buses", totalBusesVal, "🚌", "#b45309"), 0, 1);
    gridLayout->addWidget(createMetricCard("Total Drivers", totalDriversVal, "🧑‍✈️", "#f59e0b"), 0, 2);

    gridLayout->addWidget(createMetricCard("Active Routes", totalRoutesVal, "🗺️", "#059669"), 1, 0);
    gridLayout->addWidget(createMetricCard("Available Seats", availableSeatsVal, "💺", "#0284c7"), 1, 1);
    gridLayout->addWidget(createMetricCard("Pending Fees (₹)", pendingFeesVal, "💳", "#e11d48"), 1, 2);

    mainLayout->addLayout(gridLayout);

    // Quick Action Bar - 2 Row Grid for maximum clarity and zero truncation
    auto* quickBox = new QFrame(scrollContent);
    quickBox->setStyleSheet("background-color: #ffffff; border: 1.5px solid #fde68a; border-radius: 10px; padding: 14px;");
    auto* quickLayout = new QVBoxLayout(quickBox);
    auto* quickTitle = new QLabel("Quick Management Actions", quickBox);
    quickTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #78350f; margin-bottom: 4px;");
    quickLayout->addWidget(quickTitle);

    auto* btnGrid = new QGridLayout();
    btnGrid->setSpacing(10);

    auto makeActionBtn = [&](const QString& text, int page) {
        auto* btn = new QPushButton(text, quickBox);
        btn->setStyleSheet(
            "QPushButton { background-color: #fffbeb; color: #78350f; border: 1.5px solid #f59e0b; border-radius: 6px; padding: 8px 14px; font-weight: 700; font-size: 13px; min-height: 38px; text-align: center; }"
            "QPushButton:hover { background-color: #fef3c7; border-color: #d97706; color: #451a03; }"
        );
        btn->setCursor(Qt::PointingHandCursor);
        connect(btn, &QPushButton::clicked, this, [this, page]() { emit navigateTo(page); });
        return btn;
    };

    btnGrid->addWidget(makeActionBtn("👥 Manage Students", 1), 0, 0);
    btnGrid->addWidget(makeActionBtn("🚌 Manage Buses", 2), 0, 1);
    btnGrid->addWidget(makeActionBtn("🗺️ Live Map & GPS", 12), 0, 2);
    btnGrid->addWidget(makeActionBtn("💺 Seat Allocation", 5), 0, 3);
    btnGrid->addWidget(makeActionBtn("🗺️ View Routes", 3), 1, 0);
    btnGrid->addWidget(makeActionBtn("📝 Mark Attendance", 6), 1, 1);
    btnGrid->addWidget(makeActionBtn("💳 Fee Management", 7), 1, 2);
    btnGrid->addWidget(makeActionBtn("📈 Export Reports", 9), 1, 3);

    quickLayout->addLayout(btnGrid);
    mainLayout->addWidget(quickBox);

    // Bus Status Summary Table
    auto* tableHeaderLbl = new QLabel("Fleet Status & Real-Time Tracking Overview", scrollContent);
    tableHeaderLbl->setStyleSheet("font-size: 16px; font-weight: bold; color: #0f172a; margin-top: 6px;");
    mainLayout->addWidget(tableHeaderLbl);

    busSummaryTable = new QTableWidget(scrollContent);
    busSummaryTable->setColumnCount(9);
    busSummaryTable->setHorizontalHeaderLabels({
        "Bus ID", "Registration No", "Capacity", "Occupied", "Empty Seats", "Shift Schedule", "Driver", "Status", "Live Location"
    });
    busSummaryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    busSummaryTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    busSummaryTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    busSummaryTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    busSummaryTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    busSummaryTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    busSummaryTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    busSummaryTable->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);
    busSummaryTable->horizontalHeader()->setSectionResizeMode(7, QHeaderView::ResizeToContents);
    busSummaryTable->horizontalHeader()->setSectionResizeMode(8, QHeaderView::Stretch);

    busSummaryTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    busSummaryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    busSummaryTable->verticalHeader()->setVisible(false);
    busSummaryTable->setMinimumHeight(220);
    mainLayout->addWidget(busSummaryTable);

    scrollArea->setWidget(scrollContent);
    outerLayout->addWidget(scrollArea);
}

void AdminDashboardView::refreshData() {
    auto students = fileManager->getAllStudents();
    auto buses = fileManager->getAllBuses();
    auto drivers = fileManager->getAllDrivers();
    auto routes = fileManager->getAllRoutes();
    auto fees = fileManager->getAllFees();

    int totalAvailSeats = 0;
    for (const auto& b : buses) {
        totalAvailSeats += b.getAvailableSeats();
    }

    double totalPending = 0.0;
    for (const auto& f : fees) {
        totalPending += f.getPendingAmount();
    }

    totalStudentsVal->setText(QString::number(students.size()));
    totalBusesVal->setText(QString::number(buses.size()));
    totalDriversVal->setText(QString::number(drivers.size()));
    totalRoutesVal->setText(QString::number(routes.size()));
    availableSeatsVal->setText(QString::number(totalAvailSeats));

    std::ostringstream oss;
    oss << "₹" << std::fixed << std::setprecision(0) << totalPending;
    pendingFeesVal->setText(QString::fromStdString(oss.str()));

    // Populate Bus summary table
    busSummaryTable->setRowCount(0);
    for (size_t i = 0; i < buses.size(); ++i) {
        const auto& b = buses[i];
        busSummaryTable->insertRow(i);

        busSummaryTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(b.getBusId())));
        busSummaryTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(b.getRegNumber())));
        busSummaryTable->setItem(i, 2, new QTableWidgetItem(QString::number(b.getCapacity())));
        busSummaryTable->setItem(i, 3, new QTableWidgetItem(QString::number(b.getOccupiedSeats())));

        auto* emptySeatsItem = new QTableWidgetItem(QString("🟢 %1 Empty").arg(b.getAvailableSeats()));
        emptySeatsItem->setForeground(QBrush(QColor("#16a34a")));
        busSummaryTable->setItem(i, 4, emptySeatsItem);

        auto* shiftItem = new QTableWidgetItem(QString::fromStdString(b.getShiftType()));
        shiftItem->setForeground(QBrush(b.getShiftType().find("Half") != std::string::npos ? QColor("#d97706") : QColor("#2563eb")));
        busSummaryTable->setItem(i, 5, shiftItem);

        busSummaryTable->setItem(i, 6, new QTableWidgetItem(QString::fromStdString(b.getAssignedDriver())));

        auto* statusItem = new QTableWidgetItem(QString::fromStdString(b.getStatus()));
        if (b.getStatus() == "Active") {
            statusItem->setForeground(QBrush(QColor("#16a34a")));
        } else if (b.getStatus() == "Maintenance") {
            statusItem->setForeground(QBrush(QColor("#d97706")));
        } else {
            statusItem->setForeground(QBrush(QColor("#dc2626")));
        }
        busSummaryTable->setItem(i, 7, statusItem);

        busSummaryTable->setItem(i, 8, new QTableWidgetItem(QString("📍 ") + QString::fromStdString(b.getCurrentLocation())));
    }
}
