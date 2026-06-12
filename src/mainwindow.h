#pragma once
#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QTimer>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void refresh();
    void onReplyFinished(QNetworkReply *reply);

private:
    QTableWidget *table;
    QPushButton *btnRefresh;
    QLabel *lblLive;
    QLabel *lblStatus;
    QTimer *timer;
    QNetworkAccessManager *manager;

    void populateTable(const QByteArray &data);
    void applyStylesheet();
};