#include "StudentPortalView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QTextEdit>
#include <QGroupBox>
#include <iomanip>
#include <sstream>

StudentPortalView::StudentPortalView(FileManager* fm, const User& user, QWidget* parent)
    : QWidget(parent), fileManager(fm), currentUser(user) {
    setupUI();
    refreshData();
}

void StudentPortalView::setCurrentUser(const User& user) {
    currentUser = user;
    refreshData();
}

void StudentPortalView::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(20);

    // Welcome Header
    auto* headerLayout = new QHBoxLayout();
    studentNameLbl = new QLabel("Welcome, Student", this);
    studentNameLbl->setStyleSheet("font-size: 22px; font-weight: bold; color: #0f172a;");
    headerLayout->addWidget(studentNameLbl);
    headerLayout->addStretch();

    liveMapBtn = new QPushButton("🗺️ Track Bus on Map", this);
    liveMapBtn->setProperty("class", "primaryButton");
    liveMapBtn->setCursor(Qt::PointingHandCursor);

    viewSeatsBtn = new QPushButton("💺 View Bus Seats", this);
    viewSeatsBtn->setProperty("class", "secondaryButton");
    viewSeatsBtn->setCursor(Qt::PointingHandCursor);

    viewReceiptBtn = new QPushButton("🧾 Payment Receipt", this);
    viewReceiptBtn->setProperty("class", "secondaryButton");
    viewReceiptBtn->setCursor(Qt::PointingHandCursor);

    refreshBtn = new QPushButton("🔄 Refresh", this);
    refreshBtn->setProperty("class", "secondaryButton");
    refreshBtn->setCursor(Qt::PointingHandCursor);

    headerLayout->addWidget(liveMapBtn);
    headerLayout->addWidget(viewSeatsBtn);
    headerLayout->addWidget(viewReceiptBtn);
    headerLayout->addWidget(refreshBtn);
    mainLayout->addLayout(headerLayout);

    // 3 Cards: Profile, Bus & Transit, Fee Status
    auto* cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(16);

    // Card 1: Student Profile
    auto* profileBox = new QGroupBox("🎓 Student Profile", this);
    profileBox->setStyleSheet("QGroupBox { font-weight: bold; }");
    auto* pLayout = new QVBoxLayout(profileBox);
    studentIdLbl = new QLabel(profileBox);
    deptSemLbl = new QLabel(profileBox);
    phoneLbl = new QLabel(profileBox);
    pLayout->addWidget(studentIdLbl);
    pLayout->addWidget(deptSemLbl);
    pLayout->addWidget(phoneLbl);
    pLayout->addStretch();
    cardsLayout->addWidget(profileBox, 1);

    // Card 2: Transit Details
    auto* busBox = new QGroupBox("🚌 Bus & Seat Assignment", this);
    busBox->setStyleSheet("QGroupBox { font-weight: bold; }");
    auto* bLayout = new QVBoxLayout(busBox);
    busIdLbl = new QLabel(busBox);
    busRegLbl = new QLabel(busBox);
    driverInfoLbl = new QLabel(busBox);
    routeNameLbl = new QLabel(busBox);
    pickupInfoLbl = new QLabel(busBox);
    seatNumLbl = new QLabel(busBox);
    seatNumLbl->setStyleSheet("font-size: 15px; font-weight: bold; color: #d97706;");

    travelStatusLbl = new QLabel(busBox);
    travelStatusLbl->setStyleSheet("font-size: 13px; font-weight: bold; color: #b45309; margin-top: 4px;");

    bLayout->addWidget(busIdLbl);
    bLayout->addWidget(busRegLbl);
    bLayout->addWidget(driverInfoLbl);
    bLayout->addWidget(routeNameLbl);
    bLayout->addWidget(pickupInfoLbl);
    bLayout->addWidget(seatNumLbl);
    bLayout->addWidget(travelStatusLbl);
    cardsLayout->addWidget(busBox, 1);

    // Card 3: Fee Status
    auto* feeBox = new QGroupBox("💳 Transportation Fee Summary", this);
    feeBox->setStyleSheet("QGroupBox { font-weight: bold; }");
    auto* fLayout = new QVBoxLayout(feeBox);
    feeStatusLbl = new QLabel(feeBox);
    feeStatusLbl->setStyleSheet("font-size: 16px; font-weight: bold;");
    feeTotalLbl = new QLabel(feeBox);
    feePaidLbl = new QLabel(feeBox);
    feePendingLbl = new QLabel(feeBox);

    fLayout->addWidget(feeStatusLbl);
    fLayout->addWidget(feeTotalLbl);
    fLayout->addWidget(feePaidLbl);
    fLayout->addWidget(feePendingLbl);
    fLayout->addStretch();
    cardsLayout->addWidget(feeBox, 1);

    mainLayout->addLayout(cardsLayout);

    // Bottom: My Attendance History Table
    auto* attTitle = new QLabel("My Attendance History", this);
    attTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #0f172a; margin-top: 10px;");
    mainLayout->addWidget(attTitle);

    myAttendanceTable = new QTableWidget(this);
    myAttendanceTable->setColumnCount(5);
    myAttendanceTable->setHorizontalHeaderLabels({"Record ID", "Date", "Bus ID", "Status", "Remarks"});
    myAttendanceTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    myAttendanceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    myAttendanceTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    myAttendanceTable->verticalHeader()->setVisible(false);
    mainLayout->addWidget(myAttendanceTable);

    connect(liveMapBtn, &QPushButton::clicked, this, [this]() {
        std::string sId = currentUser.getAssociatedId();
        if (sId.empty()) sId = "STU101";
        Student s = fileManager->getStudentById(sId);
        emit requestLiveMap(QString::fromStdString(s.getBusNumber()));
    });
    connect(viewSeatsBtn, &QPushButton::clicked, this, [this]() {
        std::string sId = currentUser.getAssociatedId();
        if (sId.empty()) sId = "STU101";
        Student s = fileManager->getStudentById(sId);
        emit requestViewSeats(QString::fromStdString(s.getBusNumber()));
    });
    connect(viewReceiptBtn, &QPushButton::clicked, this, &StudentPortalView::handleViewReceipt);
    connect(refreshBtn, &QPushButton::clicked, this, &StudentPortalView::refreshData);
}

void StudentPortalView::refreshData() {
    std::string sId = currentUser.getAssociatedId();
    if (sId.empty()) sId = "STU101";

    Student s = fileManager->getStudentById(sId);
    if (s.getId().empty()) {
        auto all = fileManager->getAllStudents();
        if (!all.empty()) s = all.front();
    }

    studentNameLbl->setText("Welcome back, " + QString::fromStdString(s.getName()));
    studentIdLbl->setText("Student ID: " + QString::fromStdString(s.getId()));
    deptSemLbl->setText("Department: " + QString::fromStdString(s.getDepartment()) + " (Semester " + QString::number(s.getSemester()) + ")");
    phoneLbl->setText("Contact: " + QString::fromStdString(s.getPhone()));

    // Bus info
    std::string bId = s.getBusNumber();
    if (bId.empty() || bId == "None") {
        busIdLbl->setText("Assigned Bus: Unassigned");
        busRegLbl->setText("Bus Reg No: -");
        driverInfoLbl->setText("Assigned Driver: -");
        routeNameLbl->setText("Route: -");
        pickupInfoLbl->setText("Pickup: -");
        seatNumLbl->setText("Seat: Unassigned");
    } else {
        Bus b = fileManager->getBusById(bId);
        busIdLbl->setText("Assigned Bus: " + QString::fromStdString(b.getBusId()) + " [" + QString::fromStdString(b.getShiftType()) + "]");
        busRegLbl->setText("Bus Reg No: " + QString::fromStdString(b.getRegNumber()) + " | 📍 Location: " + QString::fromStdString(b.getCurrentLocation()));
        driverInfoLbl->setText("Driver: " + QString::fromStdString(b.getAssignedDriver()));

        Route r = fileManager->getRouteById(b.getRouteId());
        routeNameLbl->setText("Route: " + QString::fromStdString(r.getRouteName()) + " (" + QString::fromStdString(r.getPickupTime()) + ")");
        pickupInfoLbl->setText("My Pickup Point: " + QString::fromStdString(s.getPickupPoint()));
        seatNumLbl->setText(QString("Allocated Seat: #%1").arg(s.getSeatNumber()));
        travelStatusLbl->setText("⏱️ Live Travel Time: ~40 mins trip (Schedule: " + QString::fromStdString(r.getPickupTime().empty() ? "08:00 AM" : r.getPickupTime()) + " - 08:40 AM)");
    }

    // Fee info
    FeeRecord f = fileManager->getFeeByStudentId(s.getId());
    if (f.getFeeId().empty()) {
        feeStatusLbl->setText("Status: Pending Assessment");
        feeTotalLbl->setText("Annual Bus Fee: ₹15,000.00");
        feePaidLbl->setText("Paid: ₹0.00");
        feePendingLbl->setText("Balance Due: ₹15,000.00");
    } else {
        feeStatusLbl->setText("Status: " + QString::fromStdString(f.getStatus()));
        if (f.getStatus() == "Paid") feeStatusLbl->setStyleSheet("font-size: 16px; font-weight: bold; color: #16a34a;");
        else if (f.getStatus() == "Partial") feeStatusLbl->setStyleSheet("font-size: 16px; font-weight: bold; color: #d97706;");
        else feeStatusLbl->setStyleSheet("font-size: 16px; font-weight: bold; color: #dc2626;");

        feeTotalLbl->setText(QString("Annual Bus Fee: ₹%1").arg(QString::number(f.getTotalAmount(), 'f', 2)));
        feePaidLbl->setText(QString("Paid Amount: ₹%1").arg(QString::number(f.getPaidAmount(), 'f', 2)));
        feePendingLbl->setText(QString("Balance Due: ₹%1").arg(QString::number(f.getPendingAmount(), 'f', 2)));
    }

    // Attendance records
    auto attList = fileManager->getAttendanceByStudent(s.getId());
    myAttendanceTable->setRowCount(0);
    for (size_t i = 0; i < attList.size(); ++i) {
        const auto& a = attList[i];
        myAttendanceTable->insertRow(i);
        myAttendanceTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(a.getRecordId())));
        myAttendanceTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(a.getDate())));
        myAttendanceTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(a.getBusId())));

        auto* statItem = new QTableWidgetItem(QString::fromStdString(a.getStatus()));
        if (a.getStatus() == "Present") statItem->setForeground(QBrush(QColor("#16a34a")));
        else statItem->setForeground(QBrush(QColor("#dc2626")));
        myAttendanceTable->setItem(i, 3, statItem);

        myAttendanceTable->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(a.getRemarks())));
    }
}

void StudentPortalView::handleViewReceipt() {
    std::string sId = currentUser.getAssociatedId();
    if (sId.empty()) sId = "STU101";

    FeeRecord fee = fileManager->getFeeByStudentId(sId);
    if (fee.getFeeId().empty()) {
        QMessageBox::information(this, "Receipt Notice", "No fee receipt is currently generated for your account.");
        return;
    }

    Student s = fileManager->getStudentById(sId);

    QDialog dlg(this);
    dlg.setWindowTitle("My Transportation Fee Receipt");
    dlg.setFixedSize(480, 480);

    auto* layout = new QVBoxLayout(&dlg);
    auto* text = new QTextEdit(&dlg);
    text->setReadOnly(true);
    text->setStyleSheet("font-family: monospace; font-size: 13px; background-color: #f8fafc; border: 1px solid #cbd5e1;");

    std::ostringstream ss;
    ss << "====================================================\n"
       << "       COLLEGE TRANSPORTATION DEPARTMENT RECEIPT    \n"
       << "====================================================\n\n"
       << " Student Name : " << s.getName() << "\n"
       << " Student ID   : " << s.getId() << "\n"
       << " Department   : " << s.getDepartment() << " (Sem " << s.getSemester() << ")\n"
       << " Assigned Bus : " << s.getBusNumber() << " (Seat: #" << s.getSeatNumber() << ")\n"
       << " Pickup Point : " << s.getPickupPoint() << "\n\n"
       << " ---------------- PAYMENT SUMMARY ------------------\n"
       << std::fixed << std::setprecision(2)
       << " Total Fee    : ₹" << fee.getTotalAmount() << "\n"
       << " Paid Amount  : ₹" << fee.getPaidAmount() << "\n"
       << " Balance Due  : ₹" << fee.getPendingAmount() << "\n"
       << " Status       : " << fee.getStatus() << "\n"
       << " Receipt No   : " << (fee.getReceiptNumber().empty() ? "N/A" : fee.getReceiptNumber()) << "\n"
       << " Payment Date : " << (fee.getPaymentDate().empty() ? "Pending" : fee.getPaymentDate()) << "\n\n"
       << "====================================================\n";

    text->setPlainText(QString::fromStdString(ss.str()));
    layout->addWidget(text);

    auto* closeBtn = new QPushButton("Close", &dlg);
    closeBtn->setProperty("class", "primaryButton");
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    layout->addWidget(closeBtn, 0, Qt::AlignRight);

    dlg.exec();
}
