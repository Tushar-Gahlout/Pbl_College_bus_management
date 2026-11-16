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

QWidget* AdminDashboardView::createMetricCard(const QString& title, QLabel*& valLabel, const QString& iconText, const QString& accentColor) {
    auto* card = new QFrame(this);
    card->setObjectName("metricCard");
    card->setStyleSheet(QString(
        "QFrame#metricCard { background-color: #ffffff; border: 1px solid #e2e8f0; border-radius: 10px; padding: 15px; border-left: 5px solid %1; }"
        "QFrame#metricCard:hover { border-color: %1; }"
    ).arg(accentColor));

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(6);

    auto* topLayout = new QHBoxLayout();
    auto* titleLbl = new QLabel(title, card);
    titleLbl->setStyleSheet("font-size: 12px; font-weight: bold; color: #64748b; text-transform: uppercase;");
    auto* iconLbl = new QLabel(iconText, card);
    iconLbl->setStyleSheet("font-size: 22px;");
    topLayout->addWidget(titleLbl);
    topLayout->addStretch();
    topLayout->addWidget(iconLbl);

    valLabel = new QLabel("0", card);
    valLabel->setStyleSheet("font-size: 26px; font-weight: bold; color: #0f172a;");

    layout->addLayout(topLayout);
    layout->addWidget(valLabel);

    return card;
}

void AdminDashboardView::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(20);

    // Header section
    auto* headerLayout = new QHBoxLayout();
    auto* title = new QLabel("Administrator Overview", this);
    title->setStyleSheet("font-size: 22px; font-weight: bold; color: #0f172a;");
    headerLayout->addWidget(title);
    headerLayout->addStretch();

    auto* refreshBtn = new QPushButton("🔄 Refresh Stats", this);
    refreshBtn->setStyleSheet("background-color: #2563eb; color: white; padding: 8px 16px; border-radius: 6px; font-weight: bold;");
    refreshBtn->setCursor(Qt::PointingHandCursor);
    connect(refreshBtn, &QPushButton::clicked, this, &AdminDashboardView::refreshData);
    headerLayout->addWidget(refreshBtn);
    mainLayout->addLayout(headerLayout);

    // Metric Cards Grid
    auto* gridLayout = new QGridLayout();
    gridLayout->setSpacing(16);

    gridLayout->addWidget(createMetricCard("Total Students", totalStudentsVal, "🎓", "#3b82f6"), 0, 0);
    gridLayout->addWidget(createMetricCard("Total Buses", totalBusesVal, "🚌", "#10b981"), 0, 1);
    gridLayout->addWidget(createMetricCard("Total Drivers", totalDriversVal, "🧑‍✈️", "#f59e0b"), 0, 2);

    gridLayout->addWidget(createMetricCard("Active Routes", totalRoutesVal, "🗺️", "#8b5cf6"), 1, 0);
    gridLayout->addWidget(createMetricCard("Available Seats", availableSeatsVal, "💺", "#06b6d4"), 1, 1);
    gridLayout->addWidget(createMetricCard("Pending Fees (₹)", pendingFeesVal, "💳", "#ef4444"), 1, 2);

    mainLayout->addLayout(gridLayout);

    // Quick Action Bar
    auto* quickBox = new QFrame(this);
    quickBox->setStyleSheet("background-color: #ffffff; border: 1px solid #e2e8f0; border-radius: 10px; padding: 15px;");
    auto* quickLayout = new QVBoxLayout(quickBox);
    auto* quickTitle = new QLabel("Quick Management Actions", quickBox);
    quickTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #1e293b;");
    quickLayout->addWidget(quickTitle);

    auto* btnRow = new QHBoxLayout();
    btnRow->setSpacing(12);

    auto makeActionBtn = [&](const QString& text, int page) {
        auto* btn = new QPushButton(text, quickBox);
        btn->setStyleSheet("background-color: #f1f5f9; color: #1e293b; border: 1px solid #cbd5e1; border-radius: 6px; padding: 8px 14px; font-weight: 600;");
        btn->setCursor(Qt::PointingHandCursor);
        connect(btn, &QPushButton::clicked, this, [this, page]() { emit navigateTo(page); });
        return btn;
    };

    btnRow->addWidget(makeActionBtn("👥 Manage Students", 1));
    btnRow->addWidget(makeActionBtn("🚌 Manage Buses", 2));
    btnRow->addWidget(makeActionBtn("🗺️ Live Map & GPS", 12));
    btnRow->addWidget(makeActionBtn("💺 Seat Allocation", 5));
    btnRow->addWidget(makeActionBtn("🗺️ View Routes", 3));
    btnRow->addWidget(makeActionBtn("📝 Mark Attendance", 6));
    btnRow->addWidget(makeActionBtn("💰 Fee Collection", 7));
    btnRow->addWidget(makeActionBtn("📊 Export Reports", 9));
    btnRow->addStretch();

    quickLayout->addLayout(btnRow);
    mainLayout->addWidget(quickBox);

    // Bus Status Summary Table
    auto* tableHeaderLbl = new QLabel("Fleet Status & Real-Time Tracking Overview", this);
    tableHeaderLbl->setStyleSheet("font-size: 16px; font-weight: bold; color: #0f172a; margin-top: 10px;");
    mainLayout->addWidget(tableHeaderLbl);

    busSummaryTable = new QTableWidget(this);
    busSummaryTable->setColumnCount(9);
    busSummaryTable->setHorizontalHeaderLabels({
        "Bus ID", "Registration No", "Capacity", "Occupied", "Empty Seats", "Shift Schedule", "Driver", "Status", "Live Location"
    });
    busSummaryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    busSummaryTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    busSummaryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    busSummaryTable->verticalHeader()->setVisible(false);
    mainLayout->addWidget(busSummaryTable);
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
