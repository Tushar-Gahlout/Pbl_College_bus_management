#ifndef BUSMANAGEMENTVIEW_H
#define BUSMANAGEMENTVIEW_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include "../services/FileManager.h"

/**
 * @brief Bus Management Module for adding, editing, deleting, and tracking bus fleet.
 */
class BusManagementView : public QWidget {
    Q_OBJECT

private:
    FileManager* fileManager;

    QLineEdit* searchEdit;
    QTableWidget* busTable;

    QPushButton* addBtn;
    QPushButton* editBtn;
    QPushButton* deleteBtn;
    QPushButton* viewPassengersBtn;
    QPushButton* refreshBtn;

    void setupUI();
    void showBusFormDialog(Bus* existing = nullptr);
    void showPassengersDialog(const std::string& busId);

private slots:
    void handleSearch();
    void handleAdd();
    void handleEdit();
    void handleDelete();
    void handleViewPassengers();

public slots:
    void refreshData();

public:
    explicit BusManagementView(FileManager* fm, QWidget* parent = nullptr);
    ~BusManagementView() override = default;
};

#endif // BUSMANAGEMENTVIEW_H
