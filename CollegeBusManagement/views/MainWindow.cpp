#include "MainWindow.h"
#include "LoginDialog.h"
#include "../services/DataStructureManager.h"

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

    // Initialize & Synchronize all 7 Pure C Data Structures from persistent files
    DataStructureManager::getInstance()->reloadAllFromFiles(fileManager);

    setupUI();
    setCurrentUser(user);
}

void MainWindow::setupUI() {
    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* rootLayout = new QHBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // 1. Sidebar Container (Champagne Gold & Soft Ivory Theme)
    sidebarWidget = new QWidget(this);
    sidebarWidget->setObjectName("sidebar");
    sidebarWidget->setStyleSheet("background-color: #fbf8f1; border-right: 1.5px solid #e8decb; min-width: 245px; max-width: 245px;");

    auto* sidebarLayout = new QVBoxLayout(sidebarWidget);
    sidebarLayout->setContentsMargins(0, 0, 0, 16);
    sidebarLayout->setSpacing(4);

    // Sidebar Header
    auto* sidebarHeader = new QFrame(sidebarWidget);
    sidebarHeader->setObjectName("sidebarHeader");
    sidebarHeader->setStyleSheet("background-color: #f3ecd8; padding: 16px; border-bottom: 2px solid #d97706;");
    auto* sHeaderLayout = new QVBoxLayout(sidebarHeader);

    auto* logoLbl = new QLabel("🚌 CAMPUS TRANSIT", sidebarHeader);
    logoLbl->setStyleSheet("color: #92400e; font-size: 15px; font-weight: bold; letter-spacing: 1px;");
    auto* subLogoLbl = new QLabel("College Bus Management", sidebarHeader);
    subLogoLbl->setStyleSheet("color: #78716c; font-size: 11px; font-weight: 600;");

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
    logoutBtn->setStyleSheet("background-color: #e11d48; color: white; border-radius: 6px; padding: 10px; font-weight: bold; margin: 8px 12px; min-height: 38px;");
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
    topHeader->setStyleSheet("background-color: #ffffff; border-bottom: 2px solid #f59e0b; padding: 10px 24px; min-height: 52px;");
    auto* topHeaderLayout = new QHBoxLayout(topHeader);

    headerTitleLbl = new QLabel("Admin Dashboard", topHeader);
    headerTitleLbl->setStyleSheet("font-size: 18px; font-weight: bold; color: #1c1917;");
    topHeaderLayout->addWidget(headerTitleLbl);
    topHeaderLayout->addStretch();

    // Pure C Stack: Undo Button for Administrators
    undoBtn = new QPushButton("↩️ Undo (C Stack)", topHeader);
    undoBtn->setStyleSheet("QPushButton { background-color: #fffbeb; color: #92400e; border: 1.5px solid #f59e0b; border-radius: 6px; padding: 6px 14px; font-size: 12px; font-weight: 700; min-height: 34px; } QPushButton:hover { background-color: #fef3c7; color: #78350f; }");
    undoBtn->setToolTip("Pop and revert recent administrative action from Pure C LIFO Stack");
    undoBtn->setCursor(Qt::PointingHandCursor);
    connect(undoBtn, &QPushButton::clicked, this, &MainWindow::handleUndo);
    topHeaderLayout->addWidget(undoBtn);

    userBadgeLbl = new QLabel(topHeader);
    userBadgeLbl->setStyleSheet("background-color: #fef3c7; color: #92400e; padding: 6px 14px; border-radius: 14px; font-weight: 700; font-size: 12px; border: 1px solid #fde68a;");
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
    conductorPortalView = new ConductorPortalView(fileManager, currentUser, this);
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
    stackedWidget->addWidget(conductorPortalView);       // 13

    rightLayout->addWidget(stackedWidget);
    rootLayout->addWidget(rightArea);

    // Connect dashboard quick action navigation
    connect(adminDashboardView, &AdminDashboardView::navigateTo, this, &MainWindow::navigateToPage);

    // Connect student portal seat view navigation
    connect(studentPortalView, &StudentPortalView::requestViewSeats, this, [this](const QString& busId) {
        if (!busId.isEmpty() && busId != "None") {
            seatAllocationView->selectBus(busId);
        }
        navigateToPage(5);
    });

    // Connect student portal live map navigation
    connect(studentPortalView, &StudentPortalView::requestLiveMap, this, [this](const QString& busId) {
        if (!busId.isEmpty() && busId != "None") {
            busMapWidget->selectBus(busId);
        }
        navigateToPage(12);
    });

    // Connect driver portal live map navigation
    connect(driverPortalView, &DriverPortalView::requestLiveMap, this, [this](const QString& busId) {
        if (!busId.isEmpty() && busId != "None") {
            busMapWidget->selectBus(busId);
        }
        navigateToPage(12);
    });

    // Connect conductor portal live map navigation
    connect(conductorPortalView, &ConductorPortalView::requestLiveMap, this, [this](const QString& busId) {
        if (!busId.isEmpty() && busId != "None") {
            busMapWidget->selectBus(busId);
        }
        navigateToPage(12);
    });

    // Connect bus management track on map navigation
    connect(busManagementView, &BusManagementView::requestTrackBus, this, [this](const QString& busId) {
        if (!busId.isEmpty() && busId != "None") {
            busMapWidget->selectBus(busId);
        }
        navigateToPage(12);
    });
}

QPushButton* MainWindow::createNavButton(const QString& text, int pageIndex, const QString& /*icon*/) {
    auto* btn = new QPushButton(text, sidebarWidget);
    btn->setProperty("class", "navButton");
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setMinimumHeight(40);
    btn->setStyleSheet(
        "QPushButton { background: transparent; color: #44403c; border: none; border-radius: 6px; padding: 10px 14px; text-align: left; font-size: 13px; font-weight: 600; margin: 2px 4px; min-height: 40px; }"
        "QPushButton:hover { background-color: #ebe2ce; color: #78350f; }"
        "QPushButton:checked { background-color: #d97706; color: #ffffff; font-weight: 700; }"
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
        routeManagementView->setReadOnly(true);
        seatAllocationView->setReadOnly(true);
        createNavButton("🎓 My Student Portal", 10);
        createNavButton("💺 View Bus Seats", 5);
        createNavButton("🗺️ Live Bus Map", 12);
        createNavButton("🚌 Route & Stops", 3);
        navigateToPage(10);
    } else if (role == "Driver") {
        routeManagementView->setReadOnly(true);
        seatAllocationView->setReadOnly(true);
        createNavButton("🧑‍✈️ My Driver Portal", 11);
        createNavButton("🗺️ GPS Route Map", 12);
        createNavButton("🚌 Bus Routes", 3);
        navigateToPage(11);
    } else if (role == "Conductor" || role == "Helper") {
        routeManagementView->setReadOnly(true);
        seatAllocationView->setReadOnly(true);
        createNavButton("🎫 Conductor Portal", 13);
        createNavButton("📝 Mark Attendance", 6);
        createNavButton("🗺️ GPS Route Map", 12);
        createNavButton("🚌 Bus Routes", 3);
        navigateToPage(13);
    } else { // Admin (default)
        routeManagementView->setReadOnly(false);
        seatAllocationView->setReadOnly(false);
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

    if (undoBtn) {
        undoBtn->setVisible(role == "Admin");
    }
}

void MainWindow::setCurrentUser(const User& user) {
    currentUser = user;

    QString displayName = QString::fromStdString(user.getFullName().empty() ? user.getUsername() : user.getFullName());
    QString roleStr = QString::fromStdString(user.getRole());
    userBadgeLbl->setText(QString("👤 %1 (%2)").arg(displayName, roleStr));

    studentPortalView->setCurrentUser(user);
    driverPortalView->setCurrentUser(user);
    conductorPortalView->setCurrentUser(user);

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
        case 13: headerTitleLbl->setText("Conductor / Bus Helper Portal"); conductorPortalView->refreshData(); break;
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
            (index == 12 && text.contains("Map")) ||
            (index == 13 && text.contains("Conductor"))) {
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

void MainWindow::handleUndo() {
    auto* dsm = DataStructureManager::getInstance();
    if (!dsm->hasUndoActions()) {
        QMessageBox::information(this, "Undo Stack (Pure C)",
            "The Undo Stack is currently empty.\n(No recent administrative actions recorded in LIFO Stack).");
        return;
    }

    CUndoAction action;
    if (dsm->popUndoAction(action)) {
        QString actionDesc;
        try {
            if (action.action_type == UNDO_STUDENT_ADDED) {
                fileManager->deleteStudent(action.target_id);
                actionDesc = QString("Undid Addition of Student %1").arg(action.target_id);
            } else if (action.action_type == UNDO_STUDENT_DELETED) {
                if (action.previous_state && strlen(action.previous_state) > 0) {
                    Student restored = Student::deserialize(action.previous_state);
                    fileManager->addStudent(restored);
                    actionDesc = QString("Restored Deleted Student %1 (%2)").arg(action.target_id, QString::fromStdString(restored.getName()));
                }
            } else if (action.action_type == UNDO_SEAT_ALLOCATED) {
                fileManager->releaseSeat(action.target_id, atoi(action.target_name));
                actionDesc = QString("Undid Seat Allocation on Bus %1 (Seat #%2)").arg(action.target_id, action.target_name);
            }

            dsm->reloadAllFromFiles(fileManager);
            studentManagementView->refreshData();
            seatAllocationView->refreshData();
            busManagementView->refreshData();
            adminDashboardView->refreshData();

            QMessageBox::information(this, "Action Reverted",
                actionDesc + "\n\n[Pure C Data Structure: LIFO Stack Popped Successfully]");
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Undo Notice", QString("Undo operation encountered: %1").arg(e.what()));
        }
        c_stack_free_action(&action);
    }
}

