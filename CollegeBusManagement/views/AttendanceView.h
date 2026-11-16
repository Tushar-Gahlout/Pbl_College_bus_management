#ifndef ATTENDANCEVIEW_H
#define ATTENDANCEVIEW_H

#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QLabel>
#include "../services/FileManager.h"

/**
 * @brief Attendance Management Module for daily passenger tracking.
 */
class AttendanceView : public QWidget {
    Q_OBJECT

private:
    FileManager* fileManager;

    QComboBox* busCombo;
    QDateEdit* datePicker;
    QTableWidget* attendanceTable;

    QPushButton* saveAttendanceBtn;
    QPushButton* markAllPresentBtn;
    QPushButton* markAllAbsentBtn;
    QPushButton* viewHistoryBtn;
    QPushButton* refreshBtn;

    void setupUI();
    void showHistoryDialog();

private slots:
    void handleBusOrDateChanged();
    void handleSaveAttendance();
    void handleMarkAll(bool present);

public slots:
    void refreshData();

public:
    explicit AttendanceView(FileManager* fm, QWidget* parent = nullptr);
    ~AttendanceView() override = default;
};

#endif // ATTENDANCEVIEW_H
