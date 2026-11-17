#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <vector>

#include "../services/FileManager.h"
#include "../models/User.h"
#include "AdminDashboardView.h"
#include "StudentManagementView.h"
#include "BusManagementView.h"
#include "RouteManagementView.h"
#include "DriverManagementView.h"
#include "SeatAllocationView.h"
#include "AttendanceView.h"
#include "FeeManagementView.h"
#include "MaintenanceView.h"
#include "ReportsView.h"
#include "StudentPortalView.h"
#include "DriverPortalView.h"
#include "BusMapWidget.h"

/**
 * @brief Main Application Window featuring Dark Blue Sidebar, Header Bar, and Stacked Views.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    FileManager* fileManager;
    User currentUser;

    QWidget* sidebarWidget;
    QVBoxLayout* navLayout;
    std::vector<QPushButton*> navButtons;

    QLabel* headerTitleLbl;
    QLabel* userBadgeLbl;
    QPushButton* undoBtn;
    QStackedWidget* stackedWidget;

    // View pointers
    AdminDashboardView* adminDashboardView;
    StudentManagementView* studentManagementView;
    BusManagementView* busManagementView;
    RouteManagementView* routeManagementView;
    DriverManagementView* driverManagementView;
    SeatAllocationView* seatAllocationView;
    AttendanceView* attendanceView;
    FeeManagementView* feeManagementView;
    MaintenanceView* maintenanceView;
    ReportsView* reportsView;
    StudentPortalView* studentPortalView;
    DriverPortalView* driverPortalView;
    BusMapWidget* busMapWidget;

    void setupUI();
    void configureNavigationForRole();
    QPushButton* createNavButton(const QString& text, int pageIndex, const QString& icon = "");

public slots:
    void navigateToPage(int index);
    void handleLogout();
    void handleUndo();

public:
    explicit MainWindow(FileManager* fm, const User& user, QWidget* parent = nullptr);
    ~MainWindow() override = default;

    void setCurrentUser(const User& user);
};

#endif // MAINWINDOW_H
