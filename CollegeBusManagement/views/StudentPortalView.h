#ifndef STUDENTPORTALVIEW_H
#define STUDENTPORTALVIEW_H

#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include "../services/FileManager.h"
#include "../models/User.h"

/**
 * @brief Student Portal Dashboard for student users.
 */
class StudentPortalView : public QWidget {
    Q_OBJECT

private:
    FileManager* fileManager;
    User currentUser;

    QLabel* studentNameLbl;
    QLabel* studentIdLbl;
    QLabel* deptSemLbl;
    QLabel* phoneLbl;

    QLabel* busIdLbl;
    QLabel* busRegLbl;
    QLabel* driverInfoLbl;
    QLabel* routeNameLbl;
    QLabel* pickupInfoLbl;
    QLabel* seatNumLbl;

    QLabel* feeStatusLbl;
    QLabel* feeTotalLbl;
    QLabel* feePaidLbl;
    QLabel* feePendingLbl;

    QTableWidget* myAttendanceTable;
    QPushButton* viewReceiptBtn;
    QPushButton* viewSeatsBtn;
    QPushButton* refreshBtn;

    void setupUI();

signals:
    void requestViewSeats(const QString& busId);

private slots:
    void handleViewReceipt();

public slots:
    void refreshData();

public:
    explicit StudentPortalView(FileManager* fm, const User& user, QWidget* parent = nullptr);
    ~StudentPortalView() override = default;

    void setCurrentUser(const User& user);
};

#endif // STUDENTPORTALVIEW_H
