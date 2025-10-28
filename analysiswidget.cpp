#include "analysiswidget.h"
#include "analysisdatabase.h"
#include <QApplication>
#include <QFont>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QScrollBar>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QPieSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarCategoryAxis>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QDoubleValidator>
#include <QTimer>
#include <QGraphicsSimpleTextItem>

AnalysisWidget::AnalysisWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void AnalysisWidget::setupUI()
{
    setStyleSheet(R"(
        QWidget {
            font-family: "Microsoft YaHei";
            font-size: 16px;
        }
        QGroupBox {
            font-weight: bold;
            border: 1px solid #CCCCCC;
            border-radius: 8px;
            margin-top: 1ex;
            padding-top: 10px;
        }
        QGroupBox::title {
            subline-offset: -2px;
            padding: 0px 5px;
        }
        QLineEdit, QComboBox, QDateEdit {
            padding: 8px;
            border: 1px solid #CCCCCC;
            border-radius: 4px;
            background-color: white;
        }
        QPushButton {
            padding: 8px 16px;
            border: 1px solid #3498DB;
            border-radius: 4px;
            background-color: #3498DB;
            color: white;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #2980B9;
        }
        QPushButton:pressed {
            background-color: #21618C;
        }
        QPushButton:disabled {
            background-color: #BDC3C7;
            border: 1px solid #95A5A6;
        }
        QTableWidget {
            border: 1px solid #CCCCCC;
            border-radius: 4px;
            gridline-color: #E0E0E0;
        }
        QHeaderView::section {
            background-color: #3498DB;
            color: white;
            padding: 6px;
            font-weight: bold;
            border: none;
        }
        QTabWidget::pane {
            border: 1px solid #CCCCCC;
            border-radius: 4px;
        }
        QTabBar::tab {
            background: #E0E0E0;
            border: 1px solid #CCCCCC;
            border-bottom-color: #CCCCCC;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
            min-width: 8ex;
            padding: 8px;
        }
        QTabBar::tab:selected {
            background: #3498DB;
            color: white;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // 创建标签页
    tabWidget = new QTabWidget(this);

    // 销售分析页面
    QWidget *salesPage = new QWidget();
    QVBoxLayout *salesLayout = new QVBoxLayout(salesPage);

    // 筛选栏
    setupFilterBar();
    salesLayout->addLayout(filterBarLayout);

    // 销售图表
    setupSalesChart();
    salesLayout->addWidget(salesChartView);
    salesChartView->setMinimumSize(400, 300);

    // 商品分析页面
    QWidget *productPage = new QWidget();
    QVBoxLayout *productLayout = new QVBoxLayout(productPage);

    // 商品图表
    setupProductChart();
    productLayout->addWidget(productChartView);

    // 客户分析页面
    QWidget *customerPage = new QWidget();
    QVBoxLayout *customerLayout = new QVBoxLayout(customerPage);

    // 客户图表
    setupCustomerChart();
    customerLayout->addWidget(customerChartView);

    // 报表页面
    QWidget *reportPage = new QWidget();
    QVBoxLayout *reportLayout = new QVBoxLayout(reportPage);

    // 报表操作按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    deleteReportBtn = new QPushButton("删除");
    exportReportBtn = new QPushButton("导出");
    deleteReportBtn->setEnabled(false);

    connect(deleteReportBtn, &QPushButton::clicked, this, &AnalysisWidget::onDeleteReportClicked);
    connect(exportReportBtn, &QPushButton::clicked, this, &AnalysisWidget::onExportReportClicked);

    buttonLayout->addWidget(deleteReportBtn);
    buttonLayout->addWidget(exportReportBtn);
    buttonLayout->addStretch();

    // 报表表格
    setupReportTable();
    reportLayout->addLayout(buttonLayout);
    reportLayout->addWidget(reportTable);

    // 添加标签页
    tabWidget->addTab(salesPage, "销售分析");
    tabWidget->addTab(productPage, "商品分析");
    tabWidget->addTab(customerPage, "客户分析");
    tabWidget->addTab(reportPage, "详细报表");

    mainLayout->addWidget(tabWidget);

    // 初始化数据
    QTimer::singleShot(100, this, [this]() {
        onGenerateReportClicked();
    });
}

void AnalysisWidget::setupFilterBar()
{
    // 初始化 filterBarLayout
    filterBarLayout = new QHBoxLayout();

    startDateEdit = new QDateEdit(this);
    startDateEdit->setDate(QDate::currentDate().addDays(-30));
    startDateEdit->setCalendarPopup(true);
    startDateEdit->setMinimumWidth(150);

    endDateEdit = new QDateEdit(this);
    endDateEdit->setDate(QDate::currentDate());
    endDateEdit->setCalendarPopup(true);
    endDateEdit->setMinimumWidth(150);

    analysisTypeCombo = new QComboBox(this);
    analysisTypeCombo->addItem("按日统计");
    analysisTypeCombo->addItem("按周统计");
    analysisTypeCombo->addItem("按月统计");

    generateReportBtn = new QPushButton("生成报表", this);
    connect(generateReportBtn, &QPushButton::clicked, this, &AnalysisWidget::onGenerateReportClicked);

    filterBarLayout->addWidget(new QLabel("日期范围:"));
    filterBarLayout->addWidget(startDateEdit);
    filterBarLayout->addWidget(new QLabel("至"));
    filterBarLayout->addWidget(endDateEdit);
    filterBarLayout->addSpacing(20);
    filterBarLayout->addWidget(new QLabel("统计方式:"));
    filterBarLayout->addWidget(analysisTypeCombo);
    filterBarLayout->addWidget(generateReportBtn);
    filterBarLayout->addStretch();
}

void AnalysisWidget::setupSalesChart()
{
    QChart *chart = new QChart();
    chart->setTitle("销售统计图表");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    salesChartView = new QChartView(chart);
    salesChartView->setRenderHint(QPainter::Antialiasing);
    salesChartView->setMinimumHeight(400);
}

void AnalysisWidget::setupProductChart()
{
    QChart *chart = new QChart();
    chart->setTitle("商品销售占比");
    chart->legend()->show();
    chart->legend()->setAlignment(Qt::AlignRight);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    productChartView = new QChartView(chart);
    productChartView->setRenderHint(QPainter::Antialiasing);
    productChartView->setMinimumHeight(400);
}

void AnalysisWidget::setupCustomerChart()
{
    QChart *chart = new QChart();
    chart->setTitle("客户购买金额排行");
    chart->legend()->hide();
    chart->setAnimationOptions(QChart::SeriesAnimations);

    customerChartView = new QChartView(chart);
    customerChartView->setRenderHint(QPainter::Antialiasing);
    customerChartView->setMinimumHeight(400);
}

void AnalysisWidget::setupReportTable()
{
    reportTable = new QTableWidget(this);
    reportTable->setColumnCount(6);
    QStringList headers = {"排名", "商品名称", "销售数量", "销售额", "销售占比", "订单数"};
    reportTable->setHorizontalHeaderLabels(headers);
    reportTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    reportTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    reportTable->verticalHeader()->setVisible(false);
    reportTable->horizontalHeader()->setStretchLastSection(true);
    reportTable->setAlternatingRowColors(true);
    reportTable->setStyleSheet("QTableWidget { border: 1px solid #ccc; }");

    // 设置列宽
    reportTable->setColumnWidth(0, 60);   // 排名
    reportTable->setColumnWidth(1, 150);  // 商品名称
    reportTable->setColumnWidth(2, 100);  // 销售数量
    reportTable->setColumnWidth(3, 100);  // 销售额
    reportTable->setColumnWidth(4, 100);  // 销售占比
    reportTable->setColumnWidth(5, 100);  // 订单数

    connect(reportTable, &QTableWidget::itemSelectionChanged, this, &AnalysisWidget::onTableSelectionChanged);
}

void AnalysisWidget::onGenerateReportClicked()
{
    QString startDate = startDateEdit->date().toString("yyyy-MM-dd");
    QString endDate = endDateEdit->date().toString("yyyy-MM-dd");
    QString analysisType = analysisTypeCombo->currentText();

    // 更新销售图表
    updateSalesChart(startDate, endDate, analysisType);

    // 更新商品图表
    updateProductChart(startDate, endDate);

    // 更新客户图表
    updateCustomerChart(startDate, endDate);

    // 更新报表表格
    updateReportTable(startDate, endDate);
}

void AnalysisWidget::onDeleteReportClicked()
{
    int row = reportTable->currentRow();
    if (row >= 0) {
        QString productName = reportTable->item(row, 1)->text();
        if (QMessageBox::question(this, "确认", QString("确定要删除 %1 的记录吗？").arg(productName))
            == QMessageBox::Yes) {
            // 添加删除数据库记录的逻辑
            if (AnalysisDatabaseManager::instance().deleteOrderItem(productName)) {
                QMessageBox::information(this, "提示", "记录已删除");
                onGenerateReportClicked(); // 刷新报表
            } else {
                QMessageBox::warning(this, "错误", "记录删除失败，请检查网络连接或数据库状态");
            }
        }
    }
}

void AnalysisWidget::onExportReportClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出报表", "report.csv", "CSV Files (*.csv)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);

            // 写入表头
            out << "排名,商品名称,销售数量,销售额,销售占比,订单数\n";

            // 写入数据
            for (int row = 0; row < reportTable->rowCount(); ++row) {
                QStringList rowData;
                for (int col = 0; col < reportTable->columnCount(); ++col) {
                    QTableWidgetItem *item = reportTable->item(row, col);
                    rowData << (item ? item->text() : "");
                }
                out << rowData.join(",") << "\n";
            }

            file.close();
            QMessageBox::information(this, "导出成功", "报表已成功导出到 " + fileName);
        } else {
            QMessageBox::warning(this, "导出失败", "无法写入文件");
        }
    }
}

void AnalysisWidget::onTableSelectionChanged()
{
    bool hasSelection = !reportTable->selectedItems().isEmpty();
    deleteReportBtn->setEnabled(hasSelection);
}

void AnalysisWidget::updateSalesChart(const QString& startDate, const QString& endDate, const QString& analysisType)
{
    QList<QVariantMap> salesData;
    if (!AnalysisDatabaseManager::instance().getSalesDataByPeriod(startDate, endDate, analysisType, salesData)) {
        return;
    }

    // 创建全新的 chart 对象
    QChart *newChart = new QChart();
    newChart->setAnimationOptions(QChart::SeriesAnimations);
    newChart->legend()->setVisible(true);
    newChart->legend()->setAlignment(Qt::AlignBottom);

    if (salesData.isEmpty()) {
        newChart->setTitle("销售统计图表 - 暂无数据");
        salesChartView->setChart(newChart);
        return;
    }

    // 使用折线图，确保连线可见
    QLineSeries *series = new QLineSeries();
    series->setName("销售额");
    series->setPointsVisible(true); // 确保点可见

    QStringList categories;
    double maxAmount = 0;
    double minAmount = std::numeric_limits<double>::max();

    for (int i = 0; i < salesData.size(); ++i) {
        QString period = salesData[i]["period"].toString();
        double amount = salesData[i]["amount"].toDouble();

        QString formattedPeriod = formatPeriod(period, analysisType);
        series->append(i, amount);
        categories << formattedPeriod;

        if (amount > maxAmount) maxAmount = amount;
        if (amount < minAmount) minAmount = amount;
    }

    newChart->addSeries(series);

    // 创建X轴（分类轴）
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setTitleText("时间");
    newChart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // 创建Y轴（数值轴）
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("销售额 (元)");

    // 设置Y轴范围
    double range = maxAmount - minAmount;
    if (range < 1.0) {
        range = maxAmount > 0 ? maxAmount : 1.0;
    }

    axisY->setRange(minAmount - range * 0.1, maxAmount + range * 0.1);
    newChart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    newChart->setTitle(QString("销售统计图表 (%1 至 %2)").arg(startDate).arg(endDate));

    // 设置新图表
    salesChartView->setChart(newChart);

    // 强制重绘视图
    salesChartView->repaint();

    // 延迟添加数据标签，确保图表已经完全渲染
    QTimer::singleShot(100, this, [this, salesData, analysisType]() {
        addDataLabels(salesData, analysisType);
    });
}

// 新增函数：添加数据标签
void AnalysisWidget::addDataLabels(const QList<QVariantMap>& salesData, const QString& analysisType)
{
    if (!salesChartView->chart() || salesChartView->chart()->series().count() == 0) {
        return;
    }

    // 注意：现在系列类型是 QLineSeries
    QLineSeries *series = qobject_cast<QLineSeries*>(salesChartView->chart()->series().at(0));
    if (!series) {
        return;
    }

    // 清除之前可能存在的标签
    QList<QGraphicsItem*> items = salesChartView->scene()->items();
    for (QGraphicsItem *item : items) {
        if (item->data(0).toString() == "data_label") {
            salesChartView->scene()->removeItem(item);
            delete item;
        }
    }

    // 添加新的数据标签
    for (int i = 0; i < salesData.size(); ++i) {
        double amount = salesData[i]["amount"].toDouble();

        QGraphicsSimpleTextItem *textItem = new QGraphicsSimpleTextItem();
        textItem->setData(0, "data_label");
        textItem->setText(QString::number(amount, 'f', 2));
        textItem->setBrush(QBrush(Qt::darkBlue));
        textItem->setFont(QFont("Arial", 10, QFont::Bold));

        // 获取数据点的位置
        QPointF point = series->at(i);
        QPointF chartPos = salesChartView->chart()->mapToPosition(point, series);

        // 将图表坐标转换为视图坐标，再转换为场景坐标
        QPoint viewPos = salesChartView->mapFromScene(chartPos);
        QPointF finalPos = salesChartView->mapToScene(viewPos);

        textItem->setPos(finalPos.x() - textItem->boundingRect().width()/2,
                         finalPos.y() - textItem->boundingRect().height() - 15);

        salesChartView->scene()->addItem(textItem);
    }
}

void AnalysisWidget::updateProductChart(const QString& startDate, const QString& endDate)
{
    QList<QVariantMap> productData;
    if (!AnalysisDatabaseManager::instance().getProductSalesData(startDate, endDate, productData)) {
        return;
    }

    // 创建全新的 chart 对象
    QChart *newChart = new QChart();
    newChart->setTitle("商品销售占比");
    newChart->legend()->show();
    newChart->legend()->setAlignment(Qt::AlignRight);
    newChart->setAnimationOptions(QChart::SeriesAnimations);

    if (productData.isEmpty()) {
        newChart->setTitle("商品销售占比 - 暂无数据");
        productChartView->setChart(newChart);
        return;
    }

    QPieSeries *series = new QPieSeries();
    series->setName("商品销售");

    int maxItems = 10;
    double otherAmount = 0;
    double totalAmount = 0;
    int count = 0;

    for (const auto& data : productData) {
        totalAmount += data["amount"].toDouble();
    }

    for (const auto& data : productData) {
        if (count < maxItems) {
            QString name = data["product_name"].toString();
            double amount = data["amount"].toDouble();

            if (name.length() > 10) {
                name = name.left(7) + "...";
            }

            QPieSlice *slice = series->append(name, amount);
            slice->setLabelVisible();
            double percentage = (totalAmount > 0) ? (amount / totalAmount * 100) : 0;
            slice->setLabel(QString("%1\n%2%").arg(name).arg(percentage, 0, 'f', 1));
        } else {
            otherAmount += data["amount"].toDouble();
        }
        count++;
    }

    if (otherAmount > 0) {
        QPieSlice *slice = series->append("其他", otherAmount);
        slice->setLabelVisible();
        double percentage = (totalAmount > 0) ? (otherAmount / totalAmount * 100) : 0;
        slice->setLabel(QString("其他\n%1%").arg(percentage, 0, 'f', 1));
    }

    newChart->addSeries(series);
    newChart->setTitle(QString("商品销售占比 (%1 至 %2)").arg(startDate).arg(endDate));

    productChartView->setChart(newChart);
}

void AnalysisWidget::updateCustomerChart(const QString& startDate, const QString& endDate)
{
    QList<QVariantMap> customerData;
    if (!AnalysisDatabaseManager::instance().getCustomerSalesData(startDate, endDate, customerData)) {
        return;
    }

    // 创建全新的 chart 对象
    QChart *newChart = new QChart();
    newChart->setTitle("客户购买金额排行");
    newChart->legend()->hide();
    newChart->setAnimationOptions(QChart::SeriesAnimations);

    if (customerData.isEmpty()) {
        newChart->setTitle("客户购买金额排行 - 暂无数据");
        customerChartView->setChart(newChart);
        return;
    }

    QBarSeries *series = new QBarSeries();
    QBarSet *set = new QBarSet("客户购买金额");

    QStringList categories;
    int maxItems = 15; // 最多显示15个客户
    double maxAmount = 0;
    int count = 0;

    for (const auto& data : customerData) {
        if (count >= maxItems) break;

        QString name = data["customer_name"].toString();
        double amount = data["amount"].toDouble();

        // 如果客户名称过长，截断显示
        if (name.length() > 8) {
            name = name.left(5) + "...";
        }

        *set << amount;
        categories << name;
        if (amount > maxAmount) maxAmount = amount;
        count++;
    }

    series->append(set);
    newChart->addSeries(series);

    // 显示数值标签
    series->setLabelsVisible(true);
    series->setLabelsFormat("@value");

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setTitleText("客户");
    newChart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("购买金额 (元)");
    axisY->setRange(0, maxAmount * 1.2);
    newChart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    newChart->setTitle(QString("客户购买金额排行 (%1 至 %2)").arg(startDate).arg(endDate));

    // 关键：设置新图表，旧图表会被自动删除
    customerChartView->setChart(newChart);
}

void AnalysisWidget::updateReportTable(const QString& startDate, const QString& endDate)
{
    reportTable->setRowCount(0);

    QList<QVariantMap> reportData;
    if (!AnalysisDatabaseManager::instance().getDetailedReportData(startDate, endDate, reportData)) {
        return;
    }

    if (reportData.isEmpty()) {
        return;
    }

    double totalAmount = 0;
    for (const auto& data : reportData) {
        totalAmount += data["amount"].toDouble();
    }

    for (int i = 0; i < reportData.size(); ++i) {
        const auto& data = reportData[i];
        int row = reportTable->rowCount();
        reportTable->insertRow(row);

        reportTable->setItem(row, 0, new QTableWidgetItem(QString::number(i + 1)));
        reportTable->setItem(row, 1, new QTableWidgetItem(data["product_name"].toString()));
        reportTable->setItem(row, 2, new QTableWidgetItem(QString::number(data["quantity"].toInt())));

        double amount = data["amount"].toDouble();
        reportTable->setItem(row, 3, new QTableWidgetItem(QString::number(amount, 'f', 2)));

        double percentage = (totalAmount > 0) ? (amount / totalAmount * 100) : 0;
        reportTable->setItem(row, 4, new QTableWidgetItem(QString::number(percentage, 'f', 1) + "%"));
        reportTable->setItem(row, 5, new QTableWidgetItem(QString::number(data["order_count"].toInt())));
    }
}

//void AnalysisWidget::clearChart(QChart *chart)
//{
//    // 清除所有系列
//    for (QAbstractSeries *series : chart->series()) {
//        chart->removeSeries(series);
//        delete series;
//    }

//    // 清除所有轴
//    for (QAbstractAxis *axis : chart->axes()) {
//        chart->removeAxis(axis);
//        delete axis;
//    }
//}

QString AnalysisWidget::formatPeriod(const QString &period, const QString &type)
{
    if (type == "按日统计") {
        return period; // 日期格式已经是 yyyy-MM-dd
    } else if (type == "按周统计") {
        // period 格式为 yyyy-ww (年-周)
        QStringList parts = period.split("-");
        if (parts.size() == 2) {
            return QString("%1年第%2周").arg(parts[0]).arg(parts[1].toInt());
        }
    } else if (type == "按月统计") {
        // period 格式为 yyyy-MM
        QStringList parts = period.split("-");
        if (parts.size() == 2) {
            return QString("%1年%2月").arg(parts[0]).arg(parts[1].toInt());
        }
    }
    return period;
}
