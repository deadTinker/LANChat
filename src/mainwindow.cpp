#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QHostAddress>

#include <QFile>
#include <QTextStream>

#include <QScrollBar>
#include <QApplication>
#include <QMessageBox>

#include <QTime>


// Main Window setup

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setFixedSize(800, 600);


    loadChatHistory();

    setupUdp();
    setupTcp();
    setupTimers();
    setupConnections();

}


//  Chat History setup

void MainWindow::loadChatHistory()
{
    QFile historyFile("chat_history.txt");

    if(historyFile.open(QIODevice::ReadOnly))
    {
        ui->chatBox->append(historyFile.readAll());

        ui->chatBox->verticalScrollBar()->setValue(
            ui->chatBox->verticalScrollBar()->maximum()
            );
    }
}


// UDP Networking setup

void MainWindow::setupUdp()
{
    udpSocket = new QUdpSocket(this);

    udpSocket->bind(
        45454,
        QUdpSocket::ShareAddress |
            QUdpSocket::ReuseAddressHint
        );

    connect(udpSocket,
            &QUdpSocket::readyRead,
            this,
            &MainWindow::processPendingDatagrams);
}


// TCP Networking setup

void MainWindow::setupTcp()
{
    tcpServer = new QTcpServer(this);

    tcpServer->listen(QHostAddress::Any, 5000);

    connect(tcpServer,
            &QTcpServer::newConnection,
            this,
            &MainWindow::handleConnection);
}


// Timers setup

void MainWindow::setupTimers()
{
    broadcastTimer = new QTimer(this);

    connect(broadcastTimer,
            &QTimer::timeout,
            this,
            &MainWindow::broadcastPresence);

    broadcastTimer->start(2000);

    cleanupTimer = new QTimer(this);

    connect(cleanupTimer,
            &QTimer::timeout,
            this,
            [=]()
            {
                QDateTime now =
                    QDateTime::currentDateTime();

                for(auto it = activeUsers.begin();
                     it != activeUsers.end();)
                {
                    if(it.value().secsTo(now) > 5)
                    {
                        QString user = it.key();

                        for(int i = 0;
                             i < ui->usersList->count();
                             i++)
                        {
                            if(ui->usersList->item(i)->text()
                                == user)
                            {
                                QString username =
                                    user.split(" - ").first();

                                QString time =
                                    QTime::currentTime().toString("hh:mm AP");

                                if(!user.contains("(You)"))
                                {
                                    ui->chatBox->append(
                                        "<span style='color:gray;'>[" + time + "]</span> "
                                                                               "<span style='color:#ff6666;'>[System]</span> "
                                        + username + " went offline"
                                        );
                                }

                                ui->chatBox->verticalScrollBar()->setValue(
                                    ui->chatBox->verticalScrollBar()->maximum()
                                    );

                                delete ui->usersList->takeItem(i);

                                break;
                            }
                        }

                        it = activeUsers.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }
            });

    cleanupTimer->start(2000);

    typingTimer = new QTimer(this);

    typingTimer->setSingleShot(true);

    connect(typingTimer,
            &QTimer::timeout,
            [=]()
            {
                ui->typingLabel->clear();
            });
}


// UI Connections setup

void MainWindow::setupConnections()
{
connect(ui->sendButton, &QPushButton::clicked,
        this, &MainWindow::sendMessage);

connect(ui->messageEdit, &QLineEdit::returnPressed,
        this, &MainWindow::sendMessage);

connect(ui->messageEdit, &QLineEdit::textEdited,
        this,
        [=]()
        {
            if(!isVisible)
            {
                return;
            }
            QString username =
                ui->usernameEdit->text().trimmed();

            QString typingMessage =
                "TYPING:" + username;

            udpSocket->writeDatagram(
                typingMessage.toUtf8(),
                QHostAddress::Broadcast,
                45454
                );
        });


connect(ui->visibilityButton, &QPushButton::clicked,
        [=]()
        {
            QString username =
                ui->usernameEdit->text().trimmed();

            if(!hasJoined)
            {
                if(username.isEmpty())
                {
                    QMessageBox::warning(
                        this,
                        "Username Required",
                        "Please enter a username first."
                        );

                    return;
                }

                hasJoined = true;
                isVisible = true;

                ui->usernameEdit->setEnabled(false);

                ui->visibilityButton->setText("Visible");

                return;
            }

            isVisible = !isVisible;

            if(isVisible)
            {
                ui->visibilityButton->setText("Visible");

                ui->usernameEdit->setEnabled(false);
            }
            else
            {
                ui->visibilityButton->setText("Hidden");

                ui->usernameEdit->setEnabled(true);
            }
        });

}


// Cleanup

MainWindow::~MainWindow()
{
    delete ui;
}

// Messaging setup

void MainWindow::sendMessage()
{
    QString msg = ui->messageEdit->text();

    if(msg.trimmed().isEmpty())
    {
        return;
    }

    QListWidgetItem *selectedUser =
        ui->usersList->currentItem();


    QString time =
        QTime::currentTime().toString("hh:mm AP");

    if(!selectedUser)
    {



        ui->chatBox->append(
            "[System] Please select a user first."
            );

        ui->chatBox->verticalScrollBar()->setValue(
            ui->chatBox->verticalScrollBar()->maximum()
            );

        return;
    }


    if(selectedUser)
    {
        QString userText =
            selectedUser->text();

        if(userText.contains("(You)"))
        {
            QMessageBox::information(
                this,
                "Invalid Selection",
                "You cannot message yourself"
                );

            return;
        }


        QFile file("chat_history.txt");

        if(file.open(QIODevice::Append))
        {
            QTextStream out(&file);

            out << "[" << time << "] "
                << "[Me]: "
                << msg << "\n";

        }


        ui->chatBox->append(
            "<span style='color:gray;'>[" + time + "]</span> "
                                                   "[Me]: " + msg
            );


        ui->chatBox->verticalScrollBar()->setValue(
            ui->chatBox->verticalScrollBar()->maximum()
            );


        QString ip =
            userText.split(" - ").last();

        ip = ip.replace(" (You)", "");


        QTcpSocket *socket =
            new QTcpSocket(this);

        socket->connectToHost(ip, 5000);

        if(socket->waitForConnected(3000))
        {
            QString username =
                ui->usernameEdit->text();

            socket->write(
                ("[" + username + "]: " + msg).toUtf8()
                );

            socket->flush();

            socket->waitForBytesWritten(3000);
        }
        else
        {
            QString time =
                QTime::currentTime().toString("hh:mm AP");

            ui->chatBox->append(
                "<span style='color:gray;'>[" + time + "]</span> "
                                                       "<span style='color:#ff6666;'>[System]</span> "
                                                       "Failed to send message"
                );

            ui->chatBox->verticalScrollBar()->setValue(
                ui->chatBox->verticalScrollBar()->maximum()
                );
        }
    }


    ui->messageEdit->clear();
}


// Nearby Users Discovery setup

void MainWindow::broadcastPresence()
{

    if(!isVisible)
    {
        return;
    }

    QString username = ui->usernameEdit->text().trimmed();

    if(username.isEmpty())
    {
        return;
    }


    QString message = username;

    udpSocket->writeDatagram(
        message.toUtf8(),
        QHostAddress::Broadcast,
        45454
        );
}


// Incoming UDP Handling setup

void MainWindow::processPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());

        QHostAddress sender;

        udpSocket->readDatagram(
            datagram.data(),
            datagram.size(),
            &sender
            );

        QString msg(datagram);

        if(msg.startsWith("TYPING:"))
        {
            QString typingUser =
                msg.split(":").last();

            QString localUsername =
                ui->usernameEdit->text().trimmed();

            if(typingUser != localUsername)
            {
                ui->typingLabel->setText(
                    typingUser + " is typing..."
                    );

                typingTimer->start(1000);
            }

            continue;
        }

        QString ip =
            sender.toString().replace("::ffff:", "");

        QString userEntry =
            msg + " - " + ip;


        QString localUsername =
            ui->usernameEdit->text().trimmed();

        if(msg == localUsername)
        {
            userEntry += " (You)";
        }



        activeUsers[userEntry] =
            QDateTime::currentDateTime();

        bool exists = false;



        for(int i = 0; i < ui->usersList->count(); i++)
        {
            if(ui->usersList->item(i)->text() == userEntry)
            {
                exists = true;
                break;
            }
        }

        if(!exists)
        {
            ui->usersList->addItem(userEntry);

            QString time =
                QTime::currentTime().toString("hh:mm AP");

            if(!userEntry.contains("(You)"))
            {

                ui->chatBox->append(
                    "<span style='color:gray;'>[" + time + "]</span> "
                                                           "<span style='color:#4CAF50;'>[System]</span> "
                    + msg + " is online"
                    );

            }

            ui->chatBox->verticalScrollBar()->setValue(
                ui->chatBox->verticalScrollBar()->maximum()
                );
        }
    }
}


// Incoming TCP Handling setup

void MainWindow::handleConnection()
{
    QTcpSocket *client =
        tcpServer->nextPendingConnection();

    connect(client, &QTcpSocket::readyRead,
            [=]()
            {
                QString msg =
                    QString::fromUtf8(client->readAll());


                QString time =
                    QTime::currentTime().toString("hh:mm AP");

                ui->chatBox->append(
                    "<span style='color:gray;'>[" + time + "]</span> "
                    + msg
                    );

                ui->chatBox->verticalScrollBar()->setValue(
                    ui->chatBox->verticalScrollBar()->maximum()
                    );

                QFile file("chat_history.txt");

                if(file.open(QIODevice::Append))
                {
                    QTextStream out(&file);

                    out << "[" << time << "] "
                        << msg << "\n";
                }

            });
}