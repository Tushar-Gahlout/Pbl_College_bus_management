#ifndef REPORTSVIEW_H
#define REPORTSVIEW_H

#include <QWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include "../services/FileManager.h"

/**
 * @brief Reports Module providing tabular analysis and CSV exports using C++ File Handling.
 */
class ReportsView : public QWidget {
    Q_OBJECT

private:
    FileManager* fileManager;

    QTabWidget* tabWidget;

    // Tables for each report
    QTableWidget* studentReportTable;
    QTableWidget* busReportTable;
    QTableWidget* routeReportTable;
    QTableWidget* attendanceReportTable;
    QTableWidget* feeReportTable;
    QTableWidget* maintenanceReportTable;

    QComboBox* routeSelectorCombo;

    void setupUI();
    void loadStudentReport();
    void loadBusReport();
    void loadRouteReport();
    void loadAttendanceReport();
    void loadFeeReport();
    void loadMaintenanceReport();

private slots:
    void handleExportCSV();

public slots:
    void refreshData();

public:
    explicit ReportsView(FileManager* fm, QWidget* parent = nullptr);
    ~ReportsView() override = default;
};

#endif // REPORTSVIEW_H
