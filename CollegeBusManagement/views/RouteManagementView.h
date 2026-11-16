#ifndef ROUTEMANAGEMENTVIEW_H
#define ROUTEMANAGEMENTVIEW_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include "../services/FileManager.h"

/**
 * @brief Route Management Module for handling stops, timings, and bus route assignments.
 */
class RouteManagementView : public QWidget {
    Q_OBJECT

private:
    FileManager* fileManager;

    QLineEdit* searchEdit;
    QTableWidget* routeTable;

    QPushButton* addBtn;
    QPushButton* editBtn;
    QPushButton* deleteBtn;
    QPushButton* refreshBtn;

    void setupUI();
    void showRouteFormDialog(Route* existing = nullptr);

private slots:
    void handleSearch();
    void handleAdd();
    void handleEdit();
    void handleDelete();

public slots:
    void refreshData();

public:
    explicit RouteManagementView(FileManager* fm, QWidget* parent = nullptr);
    ~RouteManagementView() override = default;
};

#endif // ROUTEMANAGEMENTVIEW_H
