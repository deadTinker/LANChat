#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QUdpSocket>
#include <QTimer>

#include <QTcpServer>
#include <QTcpSocket>

#include <QMap>
#include <QDateTime>






QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void sendMessage();

    void broadcastPresence();
    void processPendingDatagrams();

    void handleConnection();

private:
    Ui::MainWindow *ui;

    QUdpSocket *udpSocket;
    QTimer *broadcastTimer;

    QTcpServer *tcpServer;

    QMap<QString, QDateTime> activeUsers;
    QTimer *cleanupTimer;

    QTimer *typingTimer;


    bool isVisible = false;
    bool hasJoined = false;


    void loadChatHistory();

    void setupUdp();
    void setupTcp();
    void setupTimers();
    void setupConnections();

};
#endif // MAINWINDOW_H
