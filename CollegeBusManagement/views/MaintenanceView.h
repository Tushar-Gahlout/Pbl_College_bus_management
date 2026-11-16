#ifndef MAINTENANCEVIEW_H
#define MAINTENANCEVIEW_H

#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include "../services/FileManager.h"

/**
 * @brief Maintenance Management Module for tracking bus servicing, repairs, and workshop costs.
 */
class MaintenanceView : public QWidget {
    Q_OBJECT

private:
    FileManager* fileManager;

    QComboBox* busFilterCombo;
    QLabel* totalCostLbl;
    QLabel* serviceCountLbl;

    QTableWidget* maintenanceTable;

    QPushButton* addRecordBtn;
    QPushButton* editRecordBtn;
    QPushButton* deleteRecordBtn;
    QPushButton* toggleBusStatusBtn;
    QPushButton* refreshBtn;

    void setupUI();
    void showMaintenanceFormDialog(MaintenanceRecord* existing = nullptr);

private slots:
    void handleFilterChanged();
    void handleAdd();
    void handleEdit();
    void handleDelete();
    void handleToggleStatus();

public slots:
    void refreshData();

public:
    explicit MaintenanceView(FileManager* fm, QWidget* parent = nullptr);
    ~MaintenanceView() override = default;
};

#endif // MAINTENANCEVIEW_H
