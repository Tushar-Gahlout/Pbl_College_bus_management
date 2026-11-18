#ifndef DRIVERPORTALVIEW_H
#define DRIVERPORTALVIEW_H

#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include "../services/FileManager.h"
#include "../models/User.h"

/**
 * @brief Driver Portal Dashboard for driver staff.
 */
class DriverPortalView : public QWidget {
    Q_OBJECT

private:
    FileManager* fileManager;
    User currentUser;

    QLabel* driverNameLbl;
    QLabel* driverIdLbl;
    QLabel* licenseLbl;
    QLabel* phoneLbl;

    QLabel* busIdLbl;
    QLabel* busRegLbl;
    QLabel* routeNameLbl;
    QLabel* timingLbl;
    QLabel* stopsLbl;
    QLabel* dutyTravelTimeLbl;

    QTableWidget* passengerTable;
    QPushButton* liveMapBtn;
    QPushButton* refreshBtn;

    void setupUI();

signals:
    void requestLiveMap(const QString& busId);

public slots:
    void refreshData();

public:
    explicit DriverPortalView(FileManager* fm, const User& user, QWidget* parent = nullptr);
    ~DriverPortalView() override = default;

    void setCurrentUser(const User& user);
};

#endif // DRIVERPORTALVIEW_H
