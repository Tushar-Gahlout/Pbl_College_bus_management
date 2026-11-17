#ifndef SEATALLOCATIONVIEW_H
#define SEATALLOCATIONVIEW_H

#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include "../services/FileManager.h"

/**
 * @brief Seat Allocation Module for assigning, visualizing, and releasing bus seats.
 */
class SeatAllocationView : public QWidget {
    Q_OBJECT

private:
    FileManager* fileManager;

    QComboBox* busSelectorCombo;
    QLabel* busCapacityLbl;
    QLabel* occupiedCountLbl;
    QLabel* availableCountLbl;
    QLabel* shiftInfoLbl;

    QWidget* seatGridContainer;
    QGridLayout* seatGridLayout;

    QTableWidget* seatTable;
    QPushButton* assignSeatBtn;
    QPushButton* quickBookEmptyBtn;
    QPushButton* releaseSeatBtn;
    QPushButton* waitingQueueBtn;
    QPushButton* refreshBtn;

    void setupUI();
    void renderSeatGrid(const std::string& busId);

private slots:
    void handleBusChanged(int index);
    void handleAssignSeat();
    void handleQuickBookEmpty(int preselectedSeat = -1);
    void handleReleaseSeat();
    void handleWaitingQueue();

public slots:
    void refreshData();

public:
    explicit SeatAllocationView(FileManager* fm, QWidget* parent = nullptr);
    ~SeatAllocationView() override = default;

    void setReadOnly(bool readOnly);
    void selectBus(const QString& busId);
};

#endif // SEATALLOCATIONVIEW_H
