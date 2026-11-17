#ifndef ROUTEMANAGEMENTVIEW_H
#define ROUTEMANAGEMENTVIEW_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "../services/FileManager.h"

/**
 * @brief Route Management Module for handling stops, timings, and bus route assignments.
 */
class RouteManagementView : public QWidget {
    Q_OBJECT

private:
    FileManager* fileManager;
    bool isReadOnly;

    QLabel* titleLbl;
    QLineEdit* searchEdit;
    QTableWidget* routeTable;

    QPushButton* addBtn;
    QPushButton* editBtn;
    QPushButton* deleteBtn;
    QPushButton* stopsListBtn;
    QPushButton* networkGraphBtn;
    QPushButton* refreshBtn;

    void setupUI();
    void showRouteFormDialog(Route* existing = nullptr);

private slots:
    void handleSearch();
    void handleAdd();
    void handleEdit();
    void handleDelete();
    void handleViewStopsList();
    void handleCheckConnectivity();

public slots:
    void refreshData();

public:
    explicit RouteManagementView(FileManager* fm, QWidget* parent = nullptr);
    ~RouteManagementView() override = default;

    // Controls whether user can add/edit/delete routes or only view them
    void setReadOnly(bool readOnly);
};

#endif // ROUTEMANAGEMENTVIEW_H
