#include "ReportsView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <iomanip>
#include <sstream>

ReportsView::ReportsView(FileManager* fm, QWidget* parent)
    : QWidget(parent), fileManager(fm) {
    setupUI();
    refreshData();
}

void ReportsView::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // Header & Export Button
    auto* topLayout = new QHBoxLayout();
    auto* title = new QLabel("System Reports & CSV Export", this);
    title->setStyleSheet("font-size: 20px; font-weight: bold; color: #0f172a;");
    topLayout->addWidget(title);
    topLayout->addStretch();

    auto* exportBtn = new QPushButton("📥 Export Current Report to CSV", this);
    exportBtn->setProperty("class", "primaryButton");
    exportBtn->setCursor(Qt::PointingHandCursor);

    auto* refreshBtn = new QPushButton("🔄 Refresh Reports", this);
    refreshBtn->setProperty("class", "secondaryButton");
    refreshBtn->setCursor(Qt::PointingHandCursor);

    topLayout->addWidget(exportBtn);
    topLayout->addWidget(refreshBtn);
    mainLayout->addLayout(topLayout);

    // Tab Widget
    tabWidget = new QTabWidget(this);

    // Tab 1: Students
    studentReportTable = new QTableWidget();
    studentReportTable->setColumnCount(7);
    studentReportTable->setHorizontalHeaderLabels({"Student ID", "Name", "Department", "Semester", "Phone", "Assigned Bus", "Pickup Point"});
    studentReportTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    studentReportTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabWidget->addTab(studentReportTable, "🎓 Student Report");

    // Tab 2: Buses
    busReportTable = new QTableWidget();
    busReportTable->setColumnCount(7);
    busReportTable->setHorizontalHeaderLabels({"Bus ID", "Reg Number", "Capacity", "Occupied", "Available", "Utilization", "Status"});
    busReportTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    busReportTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabWidget->addTab(busReportTable, "🚌 Bus Fleet Report");

    // Tab 3: Route-wise
    auto* routeTabWidget = new QWidget();
    auto* routeTabLayout = new QVBoxLayout(routeTabWidget);
    auto* routeSelectLayout = new QHBoxLayout();
    routeSelectLayout->addWidget(new QLabel("Select Route:", routeTabWidget));
    routeSelectorCombo = new QComboBox(routeTabWidget);
    routeSelectLayout->addWidget(routeSelectorCombo);
    routeSelectLayout->addStretch();
    routeTabLayout->addLayout(routeSelectLayout);

    routeReportTable = new QTableWidget(routeTabWidget);
    routeReportTable->setColumnCount(6);
    routeReportTable->setHorizontalHeaderLabels({"Student ID", "Student Name", "Department", "Semester", "Pickup Point", "Seat"});
    routeReportTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    routeReportTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    routeTabLayout->addWidget(routeReportTable);
    tabWidget->addTab(routeTabWidget, "🗺️ Route-wise Students");

    // Tab 4: Attendance
    attendanceReportTable = new QTableWidget();
    attendanceReportTable->setColumnCount(6);
    attendanceReportTable->setHorizontalHeaderLabels({"Record ID", "Date", "Bus ID", "Student Name", "Status", "Remarks"});
    attendanceReportTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    attendanceReportTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabWidget->addTab(attendanceReportTable, "📝 Attendance Report");

    // Tab 5: Fees
    feeReportTable = new QTableWidget();
    feeReportTable->setColumnCount(7);
    feeReportTable->setHorizontalHeaderLabels({"Student ID", "Student Name", "Total Fee", "Paid Amount", "Pending Balance", "Status", "Receipt No"});
    feeReportTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    feeReportTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabWidget->addTab(feeReportTable, "💰 Fee Report");

    // Tab 6: Maintenance
    maintenanceReportTable = new QTableWidget();
    maintenanceReportTable->setColumnCount(6);
    maintenanceReportTable->setHorizontalHeaderLabels({"Record ID", "Bus ID", "Service Type", "Cost (₹)", "Date", "Workshop"});
    maintenanceReportTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    maintenanceReportTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabWidget->addTab(maintenanceReportTable, "🛠️ Maintenance Report");

    mainLayout->addWidget(tabWidget);

    connect(exportBtn, &QPushButton::clicked, this, &ReportsView::handleExportCSV);
    connect(refreshBtn, &QPushButton::clicked, this, &ReportsView::refreshData);
    connect(routeSelectorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReportsView::loadRouteReport);
}

void ReportsView::refreshData() {
    loadStudentReport();
    loadBusReport();

    routeSelectorCombo->blockSignals(true);
    routeSelectorCombo->clear();
    auto routes = fileManager->getAllRoutes();
    for (const auto& r : routes) {
        routeSelectorCombo->addItem(QString::fromStdString(r.getRouteId()) + " - " + QString::fromStdString(r.getRouteName()),
                                   QString::fromStdString(r.getRouteId()));
    }
    routeSelectorCombo->blockSignals(false);

    loadRouteReport();
    loadAttendanceReport();
    loadFeeReport();
    loadMaintenanceReport();
}

void ReportsView::loadStudentReport() {
    auto list = fileManager->getAllStudents();
    studentReportTable->setRowCount(0);
    for (size_t i = 0; i < list.size(); ++i) {
        const auto& s = list[i];
        studentReportTable->insertRow(i);
        studentReportTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(s.getId())));
        studentReportTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(s.getName())));
        studentReportTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(s.getDepartment())));
        studentReportTable->setItem(i, 3, new QTableWidgetItem(QString::number(s.getSemester())));
        studentReportTable->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(s.getPhone())));
        studentReportTable->setItem(i, 5, new QTableWidgetItem(QString::fromStdString(s.getBusNumber())));
        studentReportTable->setItem(i, 6, new QTableWidgetItem(QString::fromStdString(s.getPickupPoint())));
    }
}

void ReportsView::loadBusReport() {
    auto list = fileManager->getAllBuses();
    busReportTable->setRowCount(0);
    for (size_t i = 0; i < list.size(); ++i) {
        const auto& b = list[i];
        busReportTable->insertRow(i);
        busReportTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(b.getBusId())));
        busReportTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(b.getRegNumber())));
        busReportTable->setItem(i, 2, new QTableWidgetItem(QString::number(b.getCapacity())));
        busReportTable->setItem(i, 3, new QTableWidgetItem(QString::number(b.getOccupiedSeats())));
        busReportTable->setItem(i, 4, new QTableWidgetItem(QString::number(b.getAvailableSeats())));

        double util = (b.getCapacity() > 0) ? (static_cast<double>(b.getOccupiedSeats()) / b.getCapacity() * 100.0) : 0.0;
        busReportTable->setItem(i, 5, new QTableWidgetItem(QString("%1%").arg(QString::number(util, 'f', 1))));
        busReportTable->setItem(i, 6, new QTableWidgetItem(QString::fromStdString(b.getStatus())));
    }
}

void ReportsView::loadRouteReport() {
    if (routeSelectorCombo->count() == 0) return;
    std::string routeId = routeSelectorCombo->currentData().toString().toStdString();
    Route r = fileManager->getRouteById(routeId);

    routeReportTable->setRowCount(0);
    auto allStudents = fileManager->getAllStudents();
    int row = 0;
    for (const auto& s : allStudents) {
        if (s.getBusNumber() == r.getAssignedBus()) {
            routeReportTable->insertRow(row);
            routeReportTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(s.getId())));
            routeReportTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(s.getName())));
            routeReportTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(s.getDepartment())));
            routeReportTable->setItem(row, 3, new QTableWidgetItem(QString::number(s.getSemester())));
            routeReportTable->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(s.getPickupPoint())));
            routeReportTable->setItem(row, 5, new QTableWidgetItem(QString::number(s.getSeatNumber())));
            row++;
        }
    }
}

void ReportsView::loadAttendanceReport() {
    auto list = fileManager->getAllAttendance();
    attendanceReportTable->setRowCount(0);
    for (size_t i = 0; i < list.size(); ++i) {
        const auto& a = list[i];
        attendanceReportTable->insertRow(i);
        attendanceReportTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(a.getRecordId())));
        attendanceReportTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(a.getDate())));
        attendanceReportTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(a.getBusId())));
        attendanceReportTable->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(a.getStudentName())));
        attendanceReportTable->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(a.getStatus())));
        attendanceReportTable->setItem(i, 5, new QTableWidgetItem(QString::fromStdString(a.getRemarks())));
    }
}

void ReportsView::loadFeeReport() {
    auto list = fileManager->getAllFees();
    feeReportTable->setRowCount(0);
    for (size_t i = 0; i < list.size(); ++i) {
        const auto& f = list[i];
        feeReportTable->insertRow(i);
        feeReportTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(f.getStudentId())));
        feeReportTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(f.getStudentName())));
        feeReportTable->setItem(i, 2, new QTableWidgetItem(QString("₹%1").arg(QString::number(f.getTotalAmount(), 'f', 2))));
        feeReportTable->setItem(i, 3, new QTableWidgetItem(QString("₹%1").arg(QString::number(f.getPaidAmount(), 'f', 2))));
        feeReportTable->setItem(i, 4, new QTableWidgetItem(QString("₹%1").arg(QString::number(f.getPendingAmount(), 'f', 2))));
        feeReportTable->setItem(i, 5, new QTableWidgetItem(QString::fromStdString(f.getStatus())));
        feeReportTable->setItem(i, 6, new QTableWidgetItem(QString::fromStdString(f.getReceiptNumber())));
    }
}

void ReportsView::loadMaintenanceReport() {
    auto list = fileManager->getAllMaintenance();
    maintenanceReportTable->setRowCount(0);
    for (size_t i = 0; i < list.size(); ++i) {
        const auto& m = list[i];
        maintenanceReportTable->insertRow(i);
        maintenanceReportTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(m.getRecordId())));
        maintenanceReportTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(m.getBusId())));
        maintenanceReportTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(m.getServiceType())));
        maintenanceReportTable->setItem(i, 3, new QTableWidgetItem(QString("₹%1").arg(QString::number(m.getCost(), 'f', 2))));
        maintenanceReportTable->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(m.getServiceDate())));
        maintenanceReportTable->setItem(i, 5, new QTableWidgetItem(QString::fromStdString(m.getWorkshop())));
    }
}

void ReportsView::handleExportCSV() {
    int currentTab = tabWidget->currentIndex();
    QString defaultName = "report.csv";

    switch (currentTab) {
        case 0: defaultName = "students_report.csv"; break;
        case 1: defaultName = "buses_report.csv"; break;
        case 2: defaultName = "routewise_students.csv"; break;
        case 3: defaultName = "attendance_report.csv"; break;
        case 4: defaultName = "fees_report.csv"; break;
        case 5: defaultName = "maintenance_report.csv"; break;
    }

    QString filename = QFileDialog::getSaveFileName(this, "Export Report to CSV", defaultName, "CSV Files (*.csv)");
    if (filename.isEmpty()) return;

    bool ok = false;
    std::string path = filename.toStdString();

    switch (currentTab) {
        case 0: ok = fileManager->exportStudentsToCSV(path); break;
        case 1: ok = fileManager->exportBusesToCSV(path); break;
        case 2: {
            std::string rId = routeSelectorCombo->currentData().toString().toStdString();
            ok = fileManager->exportRouteWiseStudentsToCSV(rId, path);
            break;
        }
        case 3: ok = fileManager->exportAttendanceToCSV(path); break;
        case 4: ok = fileManager->exportFeesToCSV(path); break;
        case 5: ok = fileManager->exportMaintenanceToCSV(path); break;
    }

    if (ok) {
        QMessageBox::information(this, "Export Complete", QString("Report successfully exported to:\n%1").arg(filename));
    } else {
        QMessageBox::critical(this, "Export Failed", "Failed to write CSV file. Please check file permissions.");
    }
}
