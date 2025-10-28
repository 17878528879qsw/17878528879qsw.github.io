#ifndef PRODUCTWIDGET_H
#define PRODUCTWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>
#include <QGroupBox>
#include <QTextEdit>
#include <QDateTime>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QTableWidgetItem>
#include <QPixmap>
#include <QDoubleSpinBox>

class ProductWidget : public QWidget {
    Q_OBJECT

public:
    explicit ProductWidget(QWidget *parent = nullptr);

private slots:
    void onShelveProductClicked();
    void onUnshelveProductClicked();
    void onSearchTextChanged();
    void onResetClicked();
    void onSortChanged();

private:
    void setupUI();
    void setupSearchBar();
    void setupProductTable();
    void setupButtons();
    void loadProducts();
    void updateTableFromResults(const QList<QVariantMap>& results);
    QPixmap createThumbnail(const QString& imagePath, const QSize& size = QSize(100, 100));

    // UI 组件
    QLineEdit *searchEdit;
    QComboBox *searchFieldCombo;
    QPushButton *searchBtn;
    QPushButton *resetBtn;
    QComboBox *sortFieldCombo;
    QComboBox *sortOrderCombo;
    QPushButton *sortBtn;
    QPushButton *shelveBtn;
    QPushButton *unshelveBtn;

    QTableWidget *productTable;
    int currentProductId;

    // 布局组件
    QHBoxLayout *searchBarLayout;
    QHBoxLayout *buttonLayout;
};

#endif
