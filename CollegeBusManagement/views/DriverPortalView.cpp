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
    mainLayout->setSpacing(20);

    // Welcome Header
    auto* headerLayout = new QHBoxLayout();
    driverNameLbl = new QLabel("Welcome, Driver", this);
    driverNameLbl->setStyleSheet("font-size: 22px; font-weight: bold; color: #0f172a;");
    headerLayout->addWidget(driverNameLbl);
    headerLayout->addStretch();

    saveAttendanceBtn = new QPushButton("💾 Save Passenger Attendance", this);
    saveAttendanceBtn->setProperty("class", "primaryButton");
    saveAttendanceBtn->setCursor(Qt::PointingHandCursor);

    markAllPresentBtn = new QPushButton("✅ Mark All Present", this);
    markAllPresentBtn->setProperty("class", "successButton");
    markAllPresentBtn->setCursor(Qt::PointingHandCursor);

    refreshBtn = new QPushButton("🔄 Refresh", this);
    refreshBtn->setProperty("class", "secondaryButton");
    refreshBtn->setCursor(Qt::PointingHandCursor);

    headerLayout->addWidget(saveAttendanceBtn);
    headerLayout->addWidget(markAllPresentBtn);
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

    // Bottom: Passenger list table for attendance
    auto* passTitle = new QLabel("Today's Assigned Passengers (Student Attendance)", this);
    passTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #0f172a; margin-top: 10px;");
    mainLayout->addWidget(passTitle);

    passengerTable = new QTableWidget(this);
    passengerTable->setColumnCount(6);
    passengerTable->setHorizontalHeaderLabels({"Student ID", "Name", "Department", "Pickup Stop", "Seat No", "Attendance"});
    passengerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    passengerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    passengerTable->verticalHeader()->setVisible(false);
    mainLayout->addWidget(passengerTable);

    connect(saveAttendanceBtn, &QPushButton::clicked, this, &DriverPortalView::handleSaveAttendance);
    connect(markAllPresentBtn, &QPushButton::clicked, this, &DriverPortalView::handleMarkAllPresent);
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

    // Load today's students
    std::string today = QDate::currentDate().toString("yyyy-MM-dd").toStdString();
    auto allStudents = fileManager->getAllStudents();
    auto todayAttendance = fileManager->getAttendanceByBusAndDate(bId, today);

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

            passengerTable->setItem(row, 4, new QTableWidgetItem(QString::number(s.getSeatNumber())));
            passengerTable->item(row, 4)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

            std::string status = "Present";
            for (const auto& att : todayAttendance) {
                if (att.getStudentId() == s.getId()) {
                    status = att.getStatus();
                    break;
                }
            }

            auto* statusCombo = new QComboBox();
            statusCombo->addItem("Present");
            statusCombo->addItem("Absent");
            statusCombo->setCurrentText(QString::fromStdString(status));
            passengerTable->setCellWidget(row, 5, statusCombo);

            row++;
        }
    }
}

void DriverPortalView::handleMarkAllPresent() {
    for (int r = 0; r < passengerTable->rowCount(); ++r) {
        auto* combo = qobject_cast<QComboBox*>(passengerTable->cellWidget(r, 5));
        if (combo) combo->setCurrentText("Present");
    }
}

void DriverPortalView::handleSaveAttendance() {
    if (passengerTable->rowCount() == 0) {
        QMessageBox::information(this, "Empty List", "No passengers assigned to your bus.");
        return;
    }

    std::string dId = currentUser.getAssociatedId();
    if (dId.empty()) dId = "DRV01";
    Driver d = fileManager->getDriverById(dId);
    std::string bId = d.getAssignedBus();
    std::string today = QDate::currentDate().toString("yyyy-MM-dd").toStdString();

    std::vector<Attendance> records;
    for (int r = 0; r < passengerTable->rowCount(); ++r) {
        std::string sId = passengerTable->item(r, 0)->text().toStdString();
        std::string sName = passengerTable->item(r, 1)->text().toStdString();

        auto* statusCombo = qobject_cast<QComboBox*>(passengerTable->cellWidget(r, 5));
        std::string status = statusCombo ? statusCombo->currentText().toStdString() : "Present";

        std::string recId = "ATT-" + today + "-" + sId;
        records.emplace_back(recId, today, bId, sId, sName, status, "Driver marked");
    }

    try {
        fileManager->markAttendance(records);
        QMessageBox::information(this, "Attendance Saved",
            QString("Successfully marked passenger attendance for %1 (%2 students).")
                .arg(QString::fromStdString(today)).arg(records.size()));
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Save Failed", QString::fromStdString(e.what()));
    }
}
