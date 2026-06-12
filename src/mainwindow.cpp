#include "mainwindow.h"
#include "json.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QHeaderView>
#include <QNetworkRequest>
#include <QDebug>
#include <QFile>
#include <QDir>

using json = nlohmann::json;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    resize(460, 580);
    setMinimumSize(460, 580);

    applyStylesheet();

    auto *central = new QWidget(this);
    central->setObjectName("centralWidget");
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // ── Top bar ──────────────────────────────────────────────────────────────
    auto *topBar = new QWidget(this);
    topBar->setObjectName("topBar");
    auto *topRow = new QHBoxLayout(topBar);
    topRow->setContentsMargins(18, 14, 18, 14);
    topRow->setSpacing(10);

    auto *lblTitle = new QLabel("Market", this);
    lblTitle->setObjectName("lblTitle");

    lblLive = new QLabel("LIVE", this);
    lblLive->setObjectName("lblLive");

    auto *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto *autoCheck = new QCheckBox("Auto", this);
    autoCheck->setObjectName("autoCheck");

    btnRefresh = new QPushButton("↻  Refresh", this);
    btnRefresh->setObjectName("btnRefresh");
    btnRefresh->setCursor(Qt::PointingHandCursor);

    topRow->addWidget(lblTitle);
    topRow->addWidget(lblLive);
    topRow->addWidget(spacer);
    topRow->addWidget(autoCheck);
    topRow->addWidget(btnRefresh);

    // ── Table ────────────────────────────────────────────────────────────────
    table = new QTableWidget(this);
    table->setObjectName("cryptoTable");
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels({"Symbol", "Price (USD)"});
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setStretchLastSection(false);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table->verticalHeader()->hide();
    table->setShowGrid(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSortingEnabled(true);

    // ── Status bar ───────────────────────────────────────────────────────────
    auto *statusBar = new QWidget(this);
    statusBar->setObjectName("statusBar");
    auto *statusRow = new QHBoxLayout(statusBar);
    statusRow->setContentsMargins(18, 8, 18, 8);

    lblStatus = new QLabel("—", this);
    lblStatus->setObjectName("lblStatus");
    auto *lblCount = new QLabel("0 assets", this);
    lblCount->setObjectName("lblCount");

    statusRow->addWidget(lblStatus);
    statusRow->addStretch();
    statusRow->addWidget(lblCount);

    layout->addWidget(topBar);
    layout->addWidget(table);
    layout->addWidget(statusBar);

    setCentralWidget(central);
    setWindowTitle("Crypto Prices");
    resize(460, 580);

    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &MainWindow::onReplyFinished);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::refresh);
    connect(autoCheck, &QCheckBox::toggled, [this](bool on)
            { on ? timer->start(5000) : timer->stop(); });
    connect(btnRefresh, &QPushButton::clicked, this, &MainWindow::refresh);

    refresh();
}

void MainWindow::refresh()
{
    QNetworkRequest request(QUrl("https://api.coinbase.com/v2/exchange-rates?currency=USD"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "CryptoViewer/1.0");
    manager->get(request);
    setWindowTitle("Crypto Prices — fetching...");
    btnRefresh->setEnabled(false);
}

void MainWindow::onReplyFinished(QNetworkReply *reply)
{
    btnRefresh->setEnabled(true);
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
    {
        setWindowTitle(QString("Network error: %1").arg(reply->errorString()));
        return;
    }
    setWindowTitle("Crypto Prices - Fetched");
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
        lblStatus->setText(QString("Parse error: %1").arg(e.what()));
        return;
    }

    if (!data.contains("data") || !data["data"].contains("rates"))
    {
        qDebug() << "Unexpected response:" << rawData.left(300);
        lblStatus->setText("Error: unexpected API structure");
        return;
    }

    auto &rates = data["data"]["rates"];

    table->setSortingEnabled(false);
    table->setRowCount(0);

    QFont monoFont("JetBrains Mono");
    monoFont.setPointSize(12);

    for (auto it = rates.begin(); it != rates.end(); ++it)
    {
        if (!it.value().is_string())
            continue;

        int row = table->rowCount();
        table->insertRow(row);
        table->setRowHeight(row, 44);

        // Symbol column — left aligned, Inter
        auto *symItem = new QTableWidgetItem(QString::fromStdString(it.key()));
        symItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        table->setItem(row, 0, symItem);

        // Price column — right aligned, monospace
        auto *priceItem = new QTableWidgetItem(
            QString::fromStdString(it.value().get<std::string>()));
        priceItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
        priceItem->setFont(monoFont);
        priceItem->setForeground(QColor("#c8c8c8"));
        table->setItem(row, 1, priceItem);
    }

    table->setSortingEnabled(true);

    // Update status bar instead of window title
    lblStatus->setText(QString("Updated just now"));
    findChild<QLabel *>("lblCount")->setText(QString("%1 assets").arg(table->rowCount()));
}

void MainWindow::applyStylesheet()
{
    QFile file(":/src/styles.qss");
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        setStyleSheet(QString::fromUtf8(file.readAll()));
        file.close();
    }
}