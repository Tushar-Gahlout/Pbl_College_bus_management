#include "DriverPortalView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QComboBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QDate>

DriverPortalView::DriverPortalView(FileManager* fm, const User& user, QWidget* parent)
    : QWidget(parent), fileManager(fm), currentUser(user) {
    setupUI();
    refreshData();
}

void DriverPortalView::setCurrentUser(const User& user) {
    currentUser = user;
    refreshData();
}

void DriverPortalView::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(18);

    // Welcome Header
    auto* headerLayout = new QHBoxLayout();
    driverNameLbl = new QLabel("Welcome, Driver", this);
    driverNameLbl->setStyleSheet("font-size: 22px; font-weight: bold; color: #0f172a;");
    headerLayout->addWidget(driverNameLbl);
    headerLayout->addStretch();

    refreshBtn = new QPushButton("🔄 Refresh", this);
    refreshBtn->setProperty("class", "secondaryButton");
    refreshBtn->setCursor(Qt::PointingHandCursor);
    headerLayout->addWidget(refreshBtn);

    mainLayout->addLayout(headerLayout);

    // Profile & Bus Info Row
    auto* cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(16);

    // Card 1: Driver Profile
    auto* profileBox = new QGroupBox("🧑‍✈️ Driver Profile", this);
    profileBox->setStyleSheet("QGroupBox { font-weight: bold; }");
    auto* pLayout = new QVBoxLayout(profileBox);
    driverIdLbl = new QLabel(profileBox);
    licenseLbl = new QLabel(profileBox);
    phoneLbl = new QLabel(profileBox);
    pLayout->addWidget(driverIdLbl);
    pLayout->addWidget(licenseLbl);
    pLayout->addWidget(phoneLbl);
    pLayout->addStretch();
    cardsLayout->addWidget(profileBox, 1);

    // Card 2: Bus & Route Info
    auto* busBox = new QGroupBox("🚌 Assigned Bus & Route", this);
    busBox->setStyleSheet("QGroupBox { font-weight: bold; }");
    auto* bLayout = new QVBoxLayout(busBox);
    busIdLbl = new QLabel(busBox);
    busRegLbl = new QLabel(busBox);
    routeNameLbl = new QLabel(busBox);
    timingLbl = new QLabel(busBox);
    stopsLbl = new QLabel(busBox);
    stopsLbl->setWordWrap(true);

    bLayout->addWidget(busIdLbl);
    bLayout->addWidget(busRegLbl);
    bLayout->addWidget(routeNameLbl);
    bLayout->addWidget(timingLbl);
    bLayout->addWidget(stopsLbl);
    cardsLayout->addWidget(busBox, 2);

    mainLayout->addLayout(cardsLayout);

    // Bottom: Assigned Passenger List Manifest
    auto* passTitle = new QLabel("📋 Assigned Passenger Roster & Contact Information", this);
    passTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #0f172a; margin-top: 6px;");
    mainLayout->addWidget(passTitle);

    passengerTable = new QTableWidget(this);
    passengerTable->setColumnCount(6);
    passengerTable->setHorizontalHeaderLabels({"Student ID", "Student Name", "Department", "Pickup Stop", "Seat No", "Contact Phone"});
    passengerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    passengerTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    passengerTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    passengerTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    passengerTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    passengerTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    passengerTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);

    passengerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    passengerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    passengerTable->verticalHeader()->setVisible(false);
    mainLayout->addWidget(passengerTable);

    connect(refreshBtn, &QPushButton::clicked, this, &DriverPortalView::refreshData);
}

void DriverPortalView::refreshData() {
    std::string dId = currentUser.getAssociatedId();
    if (dId.empty()) dId = "DRV01";

    Driver d = fileManager->getDriverById(dId);
    if (d.getId().empty()) {
        auto all = fileManager->getAllDrivers();
        if (!all.empty()) d = all.front();
    }

    driverNameLbl->setText("Driver Dashboard: " + QString::fromStdString(d.getName()));
    driverIdLbl->setText("Driver ID: " + QString::fromStdString(d.getId()));
    licenseLbl->setText("License No: " + QString::fromStdString(d.getLicenseNumber()));
    phoneLbl->setText("Contact: " + QString::fromStdString(d.getPhone()));

    std::string bId = d.getAssignedBus();
    if (bId.empty() || bId == "None") {
        busIdLbl->setText("Bus: No bus currently assigned");
        busRegLbl->setText("");
        routeNameLbl->setText("");
        timingLbl->setText("");
        stopsLbl->setText("");
        passengerTable->setRowCount(0);
        return;
    }

    Bus b = fileManager->getBusById(bId);
    Route r = fileManager->getRouteById(b.getRouteId());

    busIdLbl->setText("Assigned Bus: " + QString::fromStdString(b.getBusId()) + " [" + QString::fromStdString(b.getShiftType()) + "] (Capacity: " + QString::number(b.getCapacity()) + ")");
    busRegLbl->setText("Registration Number: " + QString::fromStdString(b.getRegNumber()) + " | 📍 Location: " + QString::fromStdString(b.getCurrentLocation()));
    routeNameLbl->setText("Route: " + QString::fromStdString(r.getRouteName()));
    timingLbl->setText("Shift Timing: Pickup " + QString::fromStdString(r.getPickupTime()) + " | Drop " + QString::fromStdString(r.getDropTime()));
    stopsLbl->setText("Route Stops: " + QString::fromStdString(r.getStopsAsString()));

    // Load assigned students
    auto allStudents = fileManager->getAllStudents();

    passengerTable->setRowCount(0);
    int row = 0;
    for (const auto& s : allStudents) {
        if (s.getBusNumber() == bId) {
            passengerTable->insertRow(row);

            passengerTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(s.getId())));
            passengerTable->item(row, 0)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

            passengerTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(s.getName())));
            passengerTable->item(row, 1)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

            passengerTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(s.getDepartment())));
            passengerTable->item(row, 2)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

            passengerTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(s.getPickupPoint())));
            passengerTable->item(row, 3)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

            QString seatStr = s.getSeatNumber() > 0 ? QString::number(s.getSeatNumber()) : "Unassigned";
            passengerTable->setItem(row, 4, new QTableWidgetItem(seatStr));
            passengerTable->item(row, 4)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

            passengerTable->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(s.getPhone())));
            passengerTable->item(row, 5)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

            row++;
        }
    }
}

