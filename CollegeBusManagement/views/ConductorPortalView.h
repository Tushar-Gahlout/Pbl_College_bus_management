#ifndef CONDUCTORPORTALVIEW_H
#define CONDUCTORPORTALVIEW_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QDateEdit>
#include <QComboBox>
#include <QLineEdit>
#include "../services/FileManager.h"
#include "../models/User.h"

/**
 * @brief Conductor / Bus Helper Portal.
 * 
 * Responsible for student passenger attendance marking, boarding verification,
 * and bus manifest tracking.
 */
class ConductorPortalView : public QWidget {
    Q_OBJECT

private:
    FileManager* fileManager;
    User currentUser;

    QLabel* conductorNameLbl;
    QLabel* conductorIdLbl;
    QLabel* roleBadgeLbl;

    QLabel* busIdLbl;
    QLabel* busRegLbl;
    QLabel* driverNameLbl;
    QLabel* routeNameLbl;
    QLabel* timingLbl;

    QDateEdit* datePicker;
    QLabel* totalStudentsValLbl;
    QLabel* presentCountValLbl;
    QLabel* absentCountValLbl;

    QTableWidget* attendanceTable;

    QPushButton* saveAttendanceBtn;
    QPushButton* markAllPresentBtn;
    QPushButton* markAllAbsentBtn;
    QPushButton* liveMapBtn;
    QPushButton* refreshBtn;

    void setupUI();
    void updateSummaryStats();

signals:
    void requestLiveMap(const QString& busId);

public slots:
    void refreshData();
    void handleSaveAttendance();
    void handleMarkAll(bool present);
    void handleDateChanged();

public:
    explicit ConductorPortalView(FileManager* fm, const User& user, QWidget* parent = nullptr);
    ~ConductorPortalView() override = default;

    void setCurrentUser(const User& user);
};

#endif // CONDUCTORPORTALVIEW_H
