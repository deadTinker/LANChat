# LANChat

Real-time LAN messaging application built using Qt and C++.

---

# Overview

LANChat is a desktop-based local network messaging application that allows users connected to the same WiFi/LAN network to discover nearby users and communicate in real time.

The application uses:
- UDP broadcasting for nearby user discovery
- TCP sockets for reliable messaging
- Qt Widgets for the graphical user interface

---

# Features

## Nearby User Discovery
- Automatic LAN/WiFi user detection
- Real-time online user updates
- Visibility toggle system (Visible / Hidden)

## Real-Time Messaging
- Instant TCP-based messaging
- Message delivery failure handling
- Self-message prevention

## User Experience Features
- Typing indicators
- Message timestamps
- Online/offline notifications
- Persistent chat history
- Enter key message sending
- Auto-scroll chat window

---

# Technologies Used

- C++
- Qt 6
- Qt Widgets
- QUdpSocket
- QTcpSocket
- QTcpServer
- QTimer

---

# Screenshots

## Main Chat Interface

![Main Chat](screenshots/main_chat.png)

---

## Join Chat State

![Join Chat](screenshots/join_chat.png)

---

## User Goes Offline

![Offline State](screenshots/typing_indicator.png)

---

# Project Structure

```text
LANChat/
│
├── src/
├── include/
├── ui/
├── screenshots/
├── README.md
└── CMakeLists.txt
```
# How To Run
 Requirements
- Qt 6
- Qt Creator
- MinGW Compiler


# Opening the Project
- Install Qt and Qt Creator
- Open Qt Creator
- Click:
  File → Open File or Project
- Select the CMakeLists.txt file from the project folder
- Choose the Qt kit if prompted
- Configure the project

# Building and Running
- Build the project in Release mode
- Run the application
- Launch multiple instances on devices connected to the same WiFi/LAN network
- Enter a username
- Click "Join Chat"
- Select a nearby user and start messaging

# How It Works

 User Discovery
- Users broadcast their presence over the local network using UDP datagrams.

 Messaging
- Messages are transmitted using TCP connections for reliable delivery.

 Presence System
- Users can toggle visibility. Hidden users automatically disappear from nearby user lists.

# Future Improvements
- File sharing
- Better UI themes
- Notification sounds
- User avatars


# Author

Developed as a Qt/C++ networking project for LAN-based real-time communication.
