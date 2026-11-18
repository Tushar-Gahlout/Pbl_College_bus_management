#ifndef BUSMANAGEMENTVIEW_H
#define BUSMANAGEMENTVIEW_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>
#include "../services/FileManager.h"
#include "BusMapWidget.h"

/**
 * @brief Bus Management Module for adding, editing, deleting, and tracking bus fleet.
 */
class BusManagementView : public QWidget {
    Q_OBJECT

private:
    FileManager* fileManager;

    QTabWidget* tabWidget;
    BusMapWidget* embeddedMapWidget;

    QLineEdit* searchEdit;
    QTableWidget* busTable;

    QPushButton* addBtn;
    QPushButton* editBtn;
    QPushButton* deleteBtn;
    QPushButton* viewPassengersBtn;
    QPushButton* trackMapBtn;
    QPushButton* refreshBtn;

    void setupUI();
    void showBusFormDialog(Bus* existing = nullptr);
    void showPassengersDialog(const std::string& busId);

signals:
    void requestTrackBus(const QString& busId);

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
