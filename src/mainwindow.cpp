#include "mainwindow.h"
#include "json.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QHeaderView>
#include <QNetworkRequest>
#include <QDebug>

using json = nlohmann::json;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);

    auto *topRow = new QHBoxLayout();
    btnRefresh = new QPushButton("Refresh", this);
    auto *autoCheck = new QCheckBox("Auto-refresh (5s)", this);
    topRow->addWidget(btnRefresh);
    topRow->addWidget(autoCheck);
    topRow->addStretch();

    table = new QTableWidget(this);
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels({"Symbol", "Price (USD)"});
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSortingEnabled(true);

    layout->addLayout(topRow);
    layout->addWidget(table);
    setCentralWidget(central);
    setWindowTitle("Crypto Prices");
    resize(400, 600);

    // Network manager — one instance for the lifetime of the window
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &MainWindow::onReplyFinished);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::refresh);
    connect(autoCheck, &QCheckBox::toggled, [this](bool checked)
            { checked ? timer->start(5000) : timer->stop(); });
    connect(btnRefresh, &QPushButton::clicked, this, &MainWindow::refresh);

    refresh();
}

void MainWindow::refresh()
{
    QNetworkRequest request(QUrl("https://api.coinbase.com/v2/exchange-rates?currency=USD"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "CryptoViewer/1.0");
    manager->get(request); // async — returns immediately, onReplyFinished fires when done
    setWindowTitle("Crypto Prices — fetching...");
    btnRefresh->setEnabled(false);
}

void MainWindow::onReplyFinished(QNetworkReply *reply)
{
    btnRefresh->setEnabled(true);
    reply->deleteLater(); // important: Qt won't clean this up automatically

    if (reply->error() != QNetworkReply::NoError)
    {
        setWindowTitle(QString("Network error: %1").arg(reply->errorString()));
        return;
    }

    populateTable(reply->readAll());
}

void MainWindow::populateTable(const QByteArray &rawData)
{
    json data;

    try
    {
        data = json::parse(rawData.begin(), rawData.end());
    }
    catch (const json::parse_error &e)
    {
        setWindowTitle(QString("JSON parse error: %1").arg(e.what()));
        return;
    }

    if (!data.contains("data") || !data["data"].contains("rates"))
    {
        qDebug() << "Unexpected response:" << rawData.left(300);
        setWindowTitle("Error: unexpected API structure");
        return;
    }

    auto &rates = data["data"]["rates"];

    table->setSortingEnabled(false);
    table->setRowCount(0);

    for (auto it = rates.begin(); it != rates.end(); ++it)
    {
        if (!it.value().is_string())
            continue;

        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(it.key())));
        table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(it.value().get<std::string>())));
    }

    table->setSortingEnabled(true);
    setWindowTitle(QString("Crypto Prices — %1 pairs").arg(table->rowCount()));
}