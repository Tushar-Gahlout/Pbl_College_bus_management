#ifndef ADMINDASHBOARDVIEW_H
#define ADMINDASHBOARDVIEW_H

#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include "../services/FileManager.h"

/**
 * @brief Admin Dashboard View displaying key metrics and quick shortcuts.
 */
class AdminDashboardView : public QWidget {
    Q_OBJECT

private:
    FileManager* fileManager;

    QLabel* totalStudentsVal;
    QLabel* totalBusesVal;
    QLabel* totalDriversVal;
    QLabel* totalRoutesVal;
    QLabel* availableSeatsVal;
    QLabel* pendingFeesVal;

    QTableWidget* busSummaryTable;

    QWidget* createMetricCard(const QString& title, QLabel*& valLabel, const QString& iconText, const QString& bgColor);
    void setupUI();

signals:
    void navigateTo(int pageIndex);

public slots:
    void refreshData();

public:
    explicit AdminDashboardView(FileManager* fm, QWidget* parent = nullptr);
    ~AdminDashboardView() override = default;
};

#endif // ADMINDASHBOARDVIEW_H
