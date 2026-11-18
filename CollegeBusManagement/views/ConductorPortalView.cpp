#include "ConductorPortalView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>

ConductorPortalView::ConductorPortalView(FileManager* fm, const User& user, QWidget* parent)
    : QWidget(parent), fileManager(fm), currentUser(user) {
    setupUI();
    refreshData();
}

void ConductorPortalView::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // 1. Welcome & Actions Header
    auto* headerLayout = new QHBoxLayout();
    conductorNameLbl = new QLabel("Conductor & Helper Portal", this);
    conductorNameLbl->setStyleSheet("font-size: 22px; font-weight: bold; color: #78350f;");
    headerLayout->addWidget(conductorNameLbl);
    headerLayout->addStretch();

    saveAttendanceBtn = new QPushButton("💾 Save Attendance", this);
    saveAttendanceBtn->setProperty("class", "primaryButton");
    saveAttendanceBtn->setCursor(Qt::PointingHandCursor);

    markAllPresentBtn = new QPushButton("✅ All Present", this);
    markAllPresentBtn->setProperty("class", "successButton");
    markAllPresentBtn->setCursor(Qt::PointingHandCursor);

    markAllAbsentBtn = new QPushButton("❌ All Absent", this);
    markAllAbsentBtn->setProperty("class", "dangerButton");
    markAllAbsentBtn->setCursor(Qt::PointingHandCursor);

    liveMapBtn = new QPushButton("🗺️ Live Route Map", this);
    liveMapBtn->setProperty("class", "secondaryButton");
    liveMapBtn->setCursor(Qt::PointingHandCursor);

    refreshBtn = new QPushButton("🔄 Refresh", this);
    refreshBtn->setProperty("class", "secondaryButton");
    refreshBtn->setCursor(Qt::PointingHandCursor);

    headerLayout->addWidget(saveAttendanceBtn);
    headerLayout->addWidget(markAllPresentBtn);
    headerLayout->addWidget(markAllAbsentBtn);
    headerLayout->addWidget(liveMapBtn);
    headerLayout->addWidget(refreshBtn);
    mainLayout->addLayout(headerLayout);

    // 2. Info Cards (3 Column Grid)
    auto* cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(16);

    // Card 1: Conductor / Helper Profile
    auto* profileBox = new QGroupBox("🎫 Conductor / Helper Profile", this);
    profileBox->setStyleSheet("QGroupBox { font-weight: bold; color: #78350f; }");
    auto* pLayout = new QVBoxLayout(profileBox);
    pLayout->setSpacing(6);

    conductorIdLbl = new QLabel(profileBox);
    roleBadgeLbl = new QLabel("Role: Bus Conductor / Assistant", profileBox);
    auto* statusDutyLbl = new QLabel("Duty Status: 🟢 Active On-Board", profileBox);
    statusDutyLbl->setStyleSheet("font-weight: bold; color: #15803d;");

    pLayout->addWidget(conductorIdLbl);
    pLayout->addWidget(roleBadgeLbl);
    pLayout->addWidget(statusDutyLbl);
    pLayout->addStretch();
    cardsLayout->addWidget(profileBox, 1);

    // Card 2: Assigned Bus & Driver Information
    auto* busBox = new QGroupBox("🚌 Assigned Bus & Driver", this);
    busBox->setStyleSheet("QGroupBox { font-weight: bold; color: #78350f; }");
    auto* bLayout = new QVBoxLayout(busBox);
    bLayout->setSpacing(6);

    busIdLbl = new QLabel(busBox);
    busRegLbl = new QLabel(busBox);
    driverNameLbl = new QLabel(busBox);
    routeNameLbl = new QLabel(busBox);
    timingLbl = new QLabel(busBox);

    bLayout->addWidget(busIdLbl);
    bLayout->addWidget(busRegLbl);
    bLayout->addWidget(driverNameLbl);
    bLayout->addWidget(routeNameLbl);
    bLayout->addWidget(timingLbl);
    cardsLayout->addWidget(busBox, 1);

    // Card 3: Today's Attendance Marking Overview
    auto* summaryBox = new QGroupBox("📝 Today's Roster Status", this);
    summaryBox->setStyleSheet("QGroupBox { font-weight: bold; color: #78350f; }");
    auto* sLayout = new QVBoxLayout(summaryBox);
    sLayout->setSpacing(8);

    auto* dateRow = new QHBoxLayout();
    auto* dateTitle = new QLabel("Date:", summaryBox);
    dateTitle->setStyleSheet("font-weight: bold; color: #44403c;");
    datePicker = new QDateEdit(QDate::currentDate(), summaryBox);
    datePicker->setCalendarPopup(true);
    datePicker->setDisplayFormat("yyyy-MM-dd");
    datePicker->setStyleSheet("font-weight: 600; padding: 4px 8px; border: 1.5px solid #d97706;");
    dateRow->addWidget(dateTitle);
    dateRow->addWidget(datePicker);
    dateRow->addStretch();
    sLayout->addLayout(dateRow);

    auto* statsRow = new QHBoxLayout();
    totalStudentsValLbl = new QLabel("Total: 0", summaryBox);
    totalStudentsValLbl->setStyleSheet("font-weight: bold; color: #78350f; font-size: 13px;");

    presentCountValLbl = new QLabel("🟢 Present: 0", summaryBox);
    presentCountValLbl->setStyleSheet("font-weight: bold; color: #15803d; font-size: 13px;");

    absentCountValLbl = new QLabel("🔴 Absent: 0", summaryBox);
    absentCountValLbl->setStyleSheet("font-weight: bold; color: #b91c1c; font-size: 13px;");

    statsRow->addWidget(totalStudentsValLbl);
    statsRow->addWidget(presentCountValLbl);
    statsRow->addWidget(absentCountValLbl);
    sLayout->addLayout(statsRow);
    sLayout->addStretch();

    cardsLayout->addWidget(summaryBox, 1);
    mainLayout->addLayout(cardsLayout);

    // 3. Passenger Attendance Roster Table
    auto* tableTitle = new QLabel("📋 Bus Passenger Manifest & Attendance Marking Roster", this);
    tableTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #78350f; margin-top: 6px;");
    mainLayout->addWidget(tableTitle);

    attendanceTable = new QTableWidget(this);
    attendanceTable->setColumnCount(7);
    attendanceTable->setHorizontalHeaderLabels({
        "Student ID", "Student Name", "Department", "Pickup Stop", "Seat No", "Attendance Status", "Remarks / Boarding Notes"
    });

    attendanceTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    attendanceTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    attendanceTable->setColumnWidth(1, 160);
    attendanceTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    attendanceTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Interactive);
    attendanceTable->setColumnWidth(3, 160);
    attendanceTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    attendanceTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Interactive);
    attendanceTable->setColumnWidth(5, 140);
    attendanceTable->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);

    attendanceTable->verticalHeader()->setDefaultSectionSize(42);
    attendanceTable->verticalHeader()->setVisible(false);
    attendanceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mainLayout->addWidget(attendanceTable);

    // Connect signals
    connect(saveAttendanceBtn, &QPushButton::clicked, this, &ConductorPortalView::handleSaveAttendance);
    connect(markAllPresentBtn, &QPushButton::clicked, this, [this]() { handleMarkAll(true); });
    connect(markAllAbsentBtn, &QPushButton::clicked, this, [this]() { handleMarkAll(false); });
    connect(datePicker, &QDateEdit::dateChanged, this, &ConductorPortalView::handleDateChanged);
    connect(refreshBtn, &QPushButton::clicked, this, &ConductorPortalView::refreshData);

    connect(liveMapBtn, &QPushButton::clicked, this, [this]() {
        std::string bId = currentUser.getAssociatedId();
        if (bId.empty()) bId = "BUS01";
        emit requestLiveMap(QString::fromStdString(bId));
    });
}

void ConductorPortalView::setCurrentUser(const User& user) {
    currentUser = user;
    refreshData();
}

void ConductorPortalView::refreshData() {
    QString name = QString::fromStdString(currentUser.getFullName().empty() ? currentUser.getUsername() : currentUser.getFullName());
    conductorNameLbl->setText("Conductor Portal: " + name);
    conductorIdLbl->setText("Conductor / Helper: " + QString::fromStdString(currentUser.getUsername()));

    std::string bId = currentUser.getAssociatedId();
    if (bId.empty()) bId = "BUS01";

    Bus b = fileManager->getBusById(bId);
    if (b.getBusId().empty()) {
        auto allBuses = fileManager->getAllBuses();
        if (!allBuses.empty()) {
            b = allBuses.front();
            bId = b.getBusId();
        }
    }

    Route r = fileManager->getRouteById(b.getRouteId());

    busIdLbl->setText("Assigned Bus: " + QString::fromStdString(b.getBusId()) + " [" + QString::fromStdString(b.getShiftType()) + "]");
    busRegLbl->setText("Registration No: " + QString::fromStdString(b.getRegNumber()) + " | 📍 " + QString::fromStdString(b.getCurrentLocation()));
    driverNameLbl->setText("Assigned Driver: " + QString::fromStdString(b.getAssignedDriver()));
    routeNameLbl->setText("Route: " + QString::fromStdString(r.getRouteName()));
    timingLbl->setText("Shift Timing: Pickup " + QString::fromStdString(r.getPickupTime().empty() ? "08:00 AM" : r.getPickupTime()));

    handleDateChanged();
}

void ConductorPortalView::handleDateChanged() {
    std::string bId = currentUser.getAssociatedId();
    if (bId.empty()) bId = "BUS01";
    std::string dateStr = datePicker->date().toString("yyyy-MM-dd").toStdString();

    auto allStudents = fileManager->getAllStudents();
    auto existingAttendance = fileManager->getAttendanceByBusAndDate(bId, dateStr);

    attendanceTable->setRowCount(0);
    int row = 0;
    for (const auto& s : allStudents) {
        if (s.getBusNumber() == bId) {
            attendanceTable->insertRow(row);

            // Student ID
            auto* idItem = new QTableWidgetItem(QString::fromStdString(s.getId()));
            idItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            idItem->setFont(QFont(font().family(), 10, QFont::Bold));
            attendanceTable->setItem(row, 0, idItem);

            // Name
            auto* nameItem = new QTableWidgetItem(QString::fromStdString(s.getName()));
            nameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            attendanceTable->setItem(row, 1, nameItem);

            // Department
            auto* deptItem = new QTableWidgetItem(QString::fromStdString(s.getDepartment()));
            deptItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            attendanceTable->setItem(row, 2, deptItem);

            // Pickup Point
            auto* pickupItem = new QTableWidgetItem(QString::fromStdString(s.getPickupPoint()));
            pickupItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            attendanceTable->setItem(row, 3, pickupItem);

            // Seat No
            QString seatStr = (s.getSeatNumber() > 0) ? QString("Seat #%1").arg(s.getSeatNumber()) : "Unassigned";
            auto* seatItem = new QTableWidgetItem(seatStr);
            seatItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            attendanceTable->setItem(row, 4, seatItem);

            // Existing status & remarks
            std::string status = "Present";
            std::string remarks = "Regular Boarding";
            for (const auto& att : existingAttendance) {
                if (att.getStudentId() == s.getId()) {
                    status = att.getStatus();
                    remarks = att.getRemarks();
                    break;
                }
            }

            // Status Combo Widget with Green / Red coloring
            auto* statusCombo = new QComboBox();
            statusCombo->addItem("Present");
            statusCombo->addItem("Absent");
            statusCombo->setCurrentText(QString::fromStdString(status));

            auto applyComboStyle = [statusCombo, this]() {
                if (statusCombo->currentText() == "Present") {
                    statusCombo->setStyleSheet(
                        "QComboBox { font-size: 13px; font-weight: bold; padding: 4px 10px; "
                        "border: 1.5px solid #16a34a; border-radius: 6px; background-color: #f0fdf4; color: #15803d; }"
                    );
                } else {
                    statusCombo->setStyleSheet(
                        "QComboBox { font-size: 13px; font-weight: bold; padding: 4px 10px; "
                        "border: 1.5px solid #dc2626; border-radius: 6px; background-color: #fef2f2; color: #b91c1c; }"
                    );
                }
                updateSummaryStats();
            };
            applyComboStyle();
            connect(statusCombo, &QComboBox::currentTextChanged, applyComboStyle);
            attendanceTable->setCellWidget(row, 5, statusCombo);

            // Remarks LineEdit Widget
            auto* remarksEdit = new QLineEdit();
            remarksEdit->setText(QString::fromStdString(remarks));
            remarksEdit->setPlaceholderText("Conductor notes / boarding status...");
            remarksEdit->setStyleSheet(
                "QLineEdit { font-size: 13px; padding: 6px 12px; border: 1.5px solid #e8decb; "
                "border-radius: 6px; background-color: #ffffff; color: #44403c; }"
                "QLineEdit:focus { border: 1.5px solid #d97706; background-color: #fffdfa; }"
            );
            attendanceTable->setCellWidget(row, 6, remarksEdit);

            row++;
        }
    }

    updateSummaryStats();
}

void ConductorPortalView::updateSummaryStats() {
    int total = attendanceTable->rowCount();
    int present = 0;
    int absent = 0;

    for (int r = 0; r < total; ++r) {
        auto* statusCombo = qobject_cast<QComboBox*>(attendanceTable->cellWidget(r, 5));
        if (statusCombo && statusCombo->currentText() == "Present") {
            present++;
        } else {
            absent++;
        }
    }

    totalStudentsValLbl->setText(QString("Total: %1").arg(total));
    presentCountValLbl->setText(QString("🟢 Present: %1").arg(present));
    absentCountValLbl->setText(QString("🔴 Absent: %1").arg(absent));
}

void ConductorPortalView::handleMarkAll(bool present) {
    QString target = present ? "Present" : "Absent";
    for (int r = 0; r < attendanceTable->rowCount(); ++r) {
        auto* statusCombo = qobject_cast<QComboBox*>(attendanceTable->cellWidget(r, 5));
        if (statusCombo) {
            statusCombo->setCurrentText(target);
        }
    }
    updateSummaryStats();
}

void ConductorPortalView::handleSaveAttendance() {
    std::string bId = currentUser.getAssociatedId();
    if (bId.empty()) bId = "BUS01";
    std::string dateStr = datePicker->date().toString("yyyy-MM-dd").toStdString();

    std::vector<Attendance> records;
    for (int r = 0; r < attendanceTable->rowCount(); ++r) {
        std::string sId = attendanceTable->item(r, 0)->text().toStdString();
        std::string sName = attendanceTable->item(r, 1)->text().toStdString();

        auto* statusCombo = qobject_cast<QComboBox*>(attendanceTable->cellWidget(r, 5));
        std::string status = statusCombo ? statusCombo->currentText().toStdString() : "Present";

        auto* remarksEdit = qobject_cast<QLineEdit*>(attendanceTable->cellWidget(r, 6));
        std::string remarks = remarksEdit ? remarksEdit->text().trimmed().toStdString() : "";

        std::string recId = "ATT-" + dateStr + "-" + sId;
        records.emplace_back(recId, dateStr, bId, sId, sName, status, remarks);
    }

    try {
        fileManager->markAttendance(records);
        QMessageBox::information(this, "Attendance Saved",
            QString("Successfully recorded passenger attendance for Bus %1 on %2 (%3 students).")
                .arg(QString::fromStdString(bId), QString::fromStdString(dateStr)).arg(records.size()));
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error Saving Attendance", QString::fromStdString(e.what()));
    }
}
