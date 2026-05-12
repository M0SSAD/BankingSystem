#include <QApplication>
#include "MainWindow.h"
#include "AccountTableModel.h"
#include "AuditLogModel.h"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	app.setStyle(QStringLiteral("Fusion"));

	qRegisterMetaType<QVector<AccountDisplay>>(
	    "QVector<AccountDisplay>");
	qRegisterMetaType<QVector<CustomerDisplay>>(
	    "QVector<CustomerDisplay>");
	qRegisterMetaType<QVector<TransactionDisplay>>(
	    "QVector<TransactionDisplay>");
	qRegisterMetaType<uint64_t>("uint64_t");
	qRegisterMetaType<QVector<uint64_t>>(
	    "QVector<uint64_t>");
	qRegisterMetaType<QPair<uint64_t, QString>>(
	    "QPair<uint64_t,QString>");
	qRegisterMetaType<QVector<QPair<uint64_t, QString>>>(
	    "QVector<QPair<uint64_t,QString>>");

	app.setStyleSheet(R"(
		* {
			font-family: 'Segoe UI', 'Helvetica Neue', 'Ubuntu', Arial, sans-serif;
		}
		QWidget {
			background-color: #F5F7FA;
			color: #1E293B;
			font-size: 13px;
		}
		QMainWindow {
			background-color: #F5F7FA;
		}

		#sidebar {
			background-color: #1E293B;
			border: none;
			border-right: 1px solid #0F172A;
		}
		#sidebarBrand {
			font-size: 14px;
			font-weight: 700;
			color: #FFFFFF;
			letter-spacing: 1.5px;
			background: transparent;
		}
		#sidebarRole {
			font-size: 12px;
			color: #94A3B8;
			background: transparent;
		}
		QPushButton#sidebarBtn {
			background-color: transparent;
			border: none;
			border-radius: 8px;
			padding: 10px 16px;
			color: #94A3B8;
			font-size: 13px;
			font-weight: 500;
			text-align: left;
		}
		QPushButton#sidebarBtn:hover {
			background-color: #334155;
			color: #E2E8F0;
		}
		QPushButton#sidebarBtn:checked {
			background-color: #2563EB;
			color: #FFFFFF;
			font-weight: 600;
		}
		QPushButton#sidebarActionBtn {
			background-color: #334155;
			border: none;
			border-radius: 6px;
			padding: 8px 16px;
			color: #CBD5E1;
			font-weight: 500;
		}
		QPushButton#sidebarActionBtn:hover {
			background-color: #475569;
			color: #FFFFFF;
		}
		QPushButton#sidebarLogoutBtn {
			background-color: transparent;
			border: 1px solid #7F1D1D;
			border-radius: 6px;
			padding: 8px 16px;
			color: #FCA5A5;
			font-weight: 500;
		}
		QPushButton#sidebarLogoutBtn:hover {
			background-color: #7F1D1D;
			color: #FFFFFF;
		}

		#contentPage {
			background-color: #F5F7FA;
		}
		#pageTitle {
			font-size: 22px;
			font-weight: 700;
			color: #0F172A;
			background: transparent;
		}
		#sectionTitle {
			font-size: 15px;
			font-weight: 600;
			color: #334155;
			background: transparent;
		}

		#statCard {
			background-color: #FFFFFF;
			border: 1px solid #E2E8F0;
			border-radius: 12px;
			padding: 16px 20px;
		}
		#statLabel {
			font-size: 12px;
			font-weight: 500;
			color: #64748B;
			background: transparent;
		}
		#statValue {
			font-size: 24px;
			font-weight: 700;
			color: #0F172A;
			background: transparent;
		}

		#formCard {
			background-color: #FFFFFF;
			border: 1px solid #E2E8F0;
			border-radius: 12px;
		}
		#formTitle {
			font-size: 15px;
			font-weight: 600;
			color: #0F172A;
			background: transparent;
		}
		#fieldLabel {
			font-size: 12px;
			font-weight: 600;
			color: #475569;
			background: transparent;
		}
		#hintLabel {
			font-size: 11px;
			color: #94A3B8;
			background: transparent;
		}

		#card {
			background-color: #FFFFFF;
			border: 1px solid #E2E8F0;
			border-radius: 12px;
		}
		#titleLabel {
			font-size: 32px;
			font-weight: bold;
			color: #0F172A;
			background: transparent;
		}
		#subtitleLabel {
			font-size: 14px;
			color: #64748B;
			background: transparent;
		}
		#headingLabel {
			font-size: 18px;
			font-weight: bold;
			color: #0F172A;
			background: transparent;
		}
		#sectionLabel {
			font-size: 12px;
			font-weight: 600;
			color: #475569;
			background: transparent;
		}
		#balanceLabel {
			font-size: 28px;
			font-weight: bold;
			color: #059669;
			background: transparent;
		}

		QPushButton {
			background-color: #FFFFFF;
			border: 1px solid #CBD5E1;
			border-radius: 6px;
			padding: 10px 20px;
			color: #334155;
			font-weight: 500;
			min-height: 20px;
		}
		QPushButton:hover {
			background-color: #F1F5F9;
			border-color: #94A3B8;
		}
		QPushButton:pressed {
			background-color: #E2E8F0;
		}
		QPushButton#primaryBtn {
			background-color: #2563EB;
			border-color: #2563EB;
			color: #FFFFFF;
			font-weight: 600;
		}
		QPushButton#primaryBtn:hover {
			background-color: #1D4ED8;
		}
		QPushButton#primaryBtn:pressed {
			background-color: #1E40AF;
		}
		QPushButton#dangerBtn {
			background-color: #DC2626;
			border-color: #DC2626;
			color: #FFFFFF;
			font-weight: 600;
		}
		QPushButton#dangerBtn:hover {
			background-color: #B91C1C;
		}
		QPushButton#successBtn {
			background-color: #059669;
			border-color: #059669;
			color: #FFFFFF;
			font-weight: 600;
		}
		QPushButton#successBtn:hover {
			background-color: #047857;
		}

		QLineEdit {
			background-color: #FFFFFF;
			border: 1px solid #CBD5E1;
			border-radius: 6px;
			padding: 10px 14px;
			color: #1E293B;
			selection-background-color: #2563EB;
		}
		QLineEdit:focus {
			border-color: #2563EB;
		}
		QLineEdit::placeholder {
			color: #94A3B8;
		}

		QComboBox {
			background-color: #FFFFFF;
			border: 1px solid #CBD5E1;
			border-radius: 6px;
			padding: 10px 14px;
			color: #1E293B;
			min-height: 20px;
		}
		QComboBox:focus {
			border-color: #2563EB;
		}
		QComboBox::drop-down {
			border: none;
			width: 30px;
		}
		QComboBox QAbstractItemView {
			background-color: #FFFFFF;
			border: 1px solid #CBD5E1;
			color: #1E293B;
			selection-background-color: #DBEAFE;
			selection-color: #1E40AF;
		}

		QTableView {
			background-color: #FFFFFF;
			alternate-background-color: #F8FAFC;
			gridline-color: #E2E8F0;
			border: 1px solid #E2E8F0;
			border-radius: 8px;
			selection-background-color: #DBEAFE;
			selection-color: #1E40AF;
		}
		QHeaderView::section {
			background-color: #F1F5F9;
			color: #475569;
			border: none;
			border-bottom: 2px solid #E2E8F0;
			padding: 10px 8px;
			font-weight: 700;
			font-size: 11px;
		}

		QListWidget {
			background-color: #FFFFFF;
			border: 1px solid #CBD5E1;
			border-radius: 6px;
			color: #1E293B;
			outline: none;
		}
		QListWidget::item {
			padding: 6px 10px;
		}
		QListWidget::item:alternate {
			background-color: #F8FAFC;
		}
		QListWidget::item:selected {
			background-color: #DBEAFE;
			color: #1E40AF;
		}
		QListWidget::item:hover {
			background-color: #F1F5F9;
		}

		QScrollBar:vertical {
			background: #F1F5F9;
			width: 8px;
		}
		QScrollBar::handle:vertical {
			background: #CBD5E1;
			border-radius: 4px;
			min-height: 30px;
		}
		QScrollBar::handle:vertical:hover {
			background: #94A3B8;
		}
		QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
			height: 0;
		}
		QScrollBar:horizontal {
			background: #F1F5F9;
			height: 8px;
		}
		QScrollBar::handle:horizontal {
			background: #CBD5E1;
			border-radius: 4px;
			min-width: 30px;
		}
		QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
			width: 0;
		}

		QScrollArea {
			border: none;
		}
		QMessageBox {
			background-color: #FFFFFF;
		}
		QMessageBox QLabel {
			color: #1E293B;
		}
		QMessageBox QPushButton {
			min-width: 80px;
		}
	)");

	MainWindow window;
	window.show();
	return app.exec();
}
