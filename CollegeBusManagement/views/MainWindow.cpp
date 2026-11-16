#include "MainWindow.h"
#include "LoginDialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QMessageBox>
#include <QApplication>

MainWindow::MainWindow(FileManager* fm, const User& user, QWidget* parent)
    : QMainWindow(parent), fileManager(fm), currentUser(user) {
    setWindowTitle("College Bus Management System - Transport ERP");
    resize(1200, 780);
    setMinimumSize(1000, 650);

    setupUI();
    setCurrentUser(user);
}

void MainWindow::setupUI() {
    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* rootLayout = new QHBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // 1. Sidebar Container (Dark Blue)
    sidebarWidget = new QWidget(this);
    sidebarWidget->setObjectName("sidebar");
    sidebarWidget->setStyleSheet("background-color: #0f172a; min-width: 230px; max-width: 230px;");

    auto* sidebarLayout = new QVBoxLayout(sidebarWidget);
    sidebarLayout->setContentsMargins(0, 0, 0, 16);
    sidebarLayout->setSpacing(4);

    // Sidebar Header
    auto* sidebarHeader = new QFrame(sidebarWidget);
    sidebarHeader->setObjectName("sidebarHeader");
    sidebarHeader->setStyleSheet("background-color: #1e293b; padding: 16px; border-bottom: 1px solid #334155;");
    auto* sHeaderLayout = new QVBoxLayout(sidebarHeader);

    auto* logoLbl = new QLabel("🚌 CAMPUS TRANSIT", sidebarHeader);
    logoLbl->setStyleSheet("color: #38bdf8; font-size: 15px; font-weight: bold; letter-spacing: 1px;");
    auto* subLogoLbl = new QLabel("College Bus Management", sidebarHeader);
    subLogoLbl->setStyleSheet("color: #94a3b8; font-size: 11px;");

    sHeaderLayout->addWidget(logoLbl);
    sHeaderLayout->addWidget(subLogoLbl);
    sidebarLayout->addWidget(sidebarHeader);

    // Navigation Buttons Container
    navLayout = new QVBoxLayout();
    navLayout->setContentsMargins(8, 12, 8, 8);
    navLayout->setSpacing(4);
    sidebarLayout->addLayout(navLayout);
    sidebarLayout->addStretch();

    // Logout Button at bottom of sidebar
    auto* logoutBtn = new QPushButton("🚪 Log Out", sidebarWidget);
    logoutBtn->setStyleSheet("background-color: #ef4444; color: white; border-radius: 6px; padding: 10px; font-weight: bold; margin: 8px;");
    logoutBtn->setCursor(Qt::PointingHandCursor);
    connect(logoutBtn, &QPushButton::clicked, this, &MainWindow::handleLogout);
    sidebarLayout->addWidget(logoutBtn);

    rootLayout->addWidget(sidebarWidget);

    // 2. Right Main Content Area
    auto* rightArea = new QWidget(this);
    auto* rightLayout = new QVBoxLayout(rightArea);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    // Top Header Bar
    auto* topHeader = new QFrame(rightArea);
    topHeader->setObjectName("topHeader");
    topHeader->setStyleSheet("background-color: #ffffff; border-bottom: 1px solid #e2e8f0; padding: 10px 24px; min-height: 52px;");
    auto* topHeaderLayout = new QHBoxLayout(topHeader);

    headerTitleLbl = new QLabel("Admin Dashboard", topHeader);
    headerTitleLbl->setStyleSheet("font-size: 18px; font-weight: bold; color: #0f172a;");
    topHeaderLayout->addWidget(headerTitleLbl);
    topHeaderLayout->addStretch();

    userBadgeLbl = new QLabel(topHeader);
    userBadgeLbl->setStyleSheet("background-color: #e0f2fe; color: #0369a1; padding: 6px 14px; border-radius: 14px; font-weight: 600; font-size: 12px;");
    topHeaderLayout->addWidget(userBadgeLbl);

    rightLayout->addWidget(topHeader);

    // Stacked Widget for Page Views
    stackedWidget = new QStackedWidget(rightArea);

    adminDashboardView = new AdminDashboardView(fileManager, this);
    studentManagementView = new StudentManagementView(fileManager, this);
    busManagementView = new BusManagementView(fileManager, this);
    routeManagementView = new RouteManagementView(fileManager, this);
    driverManagementView = new DriverManagementView(fileManager, this);
    seatAllocationView = new SeatAllocationView(fileManager, this);
    attendanceView = new AttendanceView(fileManager, this);
    feeManagementView = new FeeManagementView(fileManager, this);
    maintenanceView = new MaintenanceView(fileManager, this);
    reportsView = new ReportsView(fileManager, this);
    studentPortalView = new StudentPortalView(fileManager, currentUser, this);
    driverPortalView = new DriverPortalView(fileManager, currentUser, this);
    busMapWidget = new BusMapWidget(fileManager, this);

    stackedWidget->addWidget(adminDashboardView);       // 0
    stackedWidget->addWidget(studentManagementView);     // 1
    stackedWidget->addWidget(busManagementView);         // 2
    stackedWidget->addWidget(routeManagementView);       // 3
    stackedWidget->addWidget(driverManagementView);      // 4
    stackedWidget->addWidget(seatAllocationView);        // 5
    stackedWidget->addWidget(attendanceView);            // 6
    stackedWidget->addWidget(feeManagementView);         // 7
    stackedWidget->addWidget(maintenanceView);           // 8
    stackedWidget->addWidget(reportsView);               // 9
    stackedWidget->addWidget(studentPortalView);         // 10
    stackedWidget->addWidget(driverPortalView);          // 11
    stackedWidget->addWidget(busMapWidget);              // 12

    rightLayout->addWidget(stackedWidget);
    rootLayout->addWidget(rightArea);

    // Connect dashboard quick action navigation
    connect(adminDashboardView, &AdminDashboardView::navigateTo, this, &MainWindow::navigateToPage);
}

QPushButton* MainWindow::createNavButton(const QString& text, int pageIndex, const QString& /*icon*/) {
    auto* btn = new QPushButton(text, sidebarWidget);
    btn->setProperty("class", "navButton");
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(
        "QPushButton { background: transparent; color: #cbd5e1; border: none; border-radius: 6px; padding: 10px 14px; text-align: left; font-size: 13px; font-weight: 500; margin: 2px 4px; }"
        "QPushButton:hover { background-color: #1e293b; color: #ffffff; }"
        "QPushButton:checked { background-color: #2563eb; color: #ffffff; font-weight: 600; }"
    );

    connect(btn, &QPushButton::clicked, this, [this, pageIndex]() {
        navigateToPage(pageIndex);
    });

    navButtons.push_back(btn);
    navLayout->addWidget(btn);
    return btn;
}

void MainWindow::configureNavigationForRole() {
    // Clear old nav buttons
    for (auto* btn : navButtons) {
        navLayout->removeWidget(btn);
        delete btn;
    }
    navButtons.clear();

    std::string role = currentUser.getRole();

    if (role == "Student") {
        createNavButton("🎓 My Student Portal", 10);
        createNavButton("🗺️ Live Bus Map", 12);
        createNavButton("🚌 Route & Stops", 3);
        navigateToPage(10);
    } else if (role == "Driver") {
        createNavButton("🧑‍✈️ My Driver Portal", 11);
        createNavButton("🗺️ GPS Route Map", 12);
        createNavButton("📝 Mark Attendance", 6);
        createNavButton("🚌 Bus Routes", 3);
        navigateToPage(11);
    } else { // Admin (default)
        createNavButton("📊 Dashboard", 0);
        createNavButton("🎓 Students", 1);
        createNavButton("🚌 Bus Fleet", 2);
        createNavButton("🗺️ Live Map & GPS", 12);
        createNavButton("💺 Seat Allocation", 5);
        createNavButton("🗺️ Routes", 3);
        createNavButton("🧑‍✈️ Drivers", 4);
        createNavButton("📝 Attendance", 6);
        createNavButton("💳 Fee Management", 7);
        createNavButton("🛠️ Maintenance", 8);
        createNavButton("📈 Reports & Export", 9);
        navigateToPage(0);
    }
}

void MainWindow::setCurrentUser(const User& user) {
    currentUser = user;

    QString displayName = QString::fromStdString(user.getFullName().empty() ? user.getUsername() : user.getFullName());
    QString roleStr = QString::fromStdString(user.getRole());
    userBadgeLbl->setText(QString("👤 %1 (%2)").arg(displayName, roleStr));

    studentPortalView->setCurrentUser(user);
    driverPortalView->setCurrentUser(user);

    configureNavigationForRole();
}

void MainWindow::navigateToPage(int index) {
    stackedWidget->setCurrentIndex(index);

    // Update Header title
    switch (index) {
        case 0: headerTitleLbl->setText("Administrator Dashboard"); adminDashboardView->refreshData(); break;
        case 1: headerTitleLbl->setText("Student Management"); studentManagementView->refreshData(); break;
        case 2: headerTitleLbl->setText("Bus Fleet Management"); busManagementView->refreshData(); break;
        case 3: headerTitleLbl->setText("Route Management"); routeManagementView->refreshData(); break;
        case 4: headerTitleLbl->setText("Driver Management"); driverManagementView->refreshData(); break;
        case 5: headerTitleLbl->setText("Seat Allocation & Bus Seating"); seatAllocationView->refreshData(); break;
        case 6: headerTitleLbl->setText("Attendance Management"); attendanceView->refreshData(); break;
        case 7: headerTitleLbl->setText("Fee Management & Receipts"); feeManagementView->refreshData(); break;
        case 8: headerTitleLbl->setText("Bus Maintenance & Servicing"); maintenanceView->refreshData(); break;
        case 9: headerTitleLbl->setText("Reports & CSV Export"); reportsView->refreshData(); break;
        case 10: headerTitleLbl->setText("Student Portal"); studentPortalView->refreshData(); break;
        case 11: headerTitleLbl->setText("Driver Portal"); driverPortalView->refreshData(); break;
        case 12: headerTitleLbl->setText("Live Bus Location & Route GPS Tracker"); busMapWidget->refreshData(); break;
        default: break;
    }

    // Update checked state on nav buttons
    for (auto* btn : navButtons) {
        btn->setChecked(false);
    }
    // Check corresponding button if found
    for (auto* btn : navButtons) {
        // match text keywords
        QString text = btn->text();
        if ((index == 0 && text.contains("Dashboard")) ||
            (index == 1 && text.contains("Students")) ||
            (index == 2 && text.contains("Bus Fleet")) ||
            (index == 3 && text.contains("Route")) ||
            (index == 4 && text.contains("Drivers")) ||
            (index == 5 && text.contains("Seat")) ||
            (index == 6 && text.contains("Attendance")) ||
            (index == 7 && text.contains("Fee")) ||
            (index == 8 && text.contains("Maintenance")) ||
            (index == 9 && text.contains("Reports")) ||
            (index == 10 && text.contains("Student Portal")) ||
            (index == 11 && text.contains("Driver Portal")) ||
            (index == 12 && text.contains("Map"))) {
            btn->setChecked(true);
            break;
        }
    }
}

void MainWindow::handleLogout() {
    auto reply = QMessageBox::question(this, "Log Out", "Are you sure you want to log out of your session?",
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        hide();
        LoginDialog loginDlg(fileManager, nullptr);
        if (loginDlg.exec() == QDialog::Accepted) {
            setCurrentUser(loginDlg.getAuthenticatedUser());
            show();
        } else {
            QApplication::quit();
        }
    }
}
