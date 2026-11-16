#ifndef DRIVERMANAGEMENTVIEW_H
#define DRIVERMANAGEMENTVIEW_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include "../services/FileManager.h"

/**
 * @brief Driver Management Module for handling staff profiles, licenses, and bus assignments.
 */
class DriverManagementView : public QWidget {
    Q_OBJECT

private:
    FileManager* fileManager;

    QLineEdit* searchEdit;
    QTableWidget* driverTable;

    QPushButton* addBtn;
    QPushButton* editBtn;
    QPushButton* deleteBtn;
    QPushButton* viewDetailsBtn;
    QPushButton* refreshBtn;

    void setupUI();
    void showDriverFormDialog(Driver* existing = nullptr);
    void showDriverDetailsDialog(const Driver& driver);

private slots:
    void handleSearch();
    void handleAdd();
    void handleEdit();
    void handleDelete();
    void handleViewDetails();

public slots:
    void refreshData();

public:
    explicit DriverManagementView(FileManager* fm, QWidget* parent = nullptr);
    ~DriverManagementView() override = default;
};

#endif // DRIVERMANAGEMENTVIEW_H
