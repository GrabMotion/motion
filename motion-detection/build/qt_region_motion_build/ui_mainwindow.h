/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "drawing/mouse_coordinates.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QWidget *layoutWidget;
    QHBoxLayout *horizontalLayout;
    QLineEdit *network_ip;
    QLabel *label_3;
    QComboBox *ips_combo;
    QPushButton *search_button;
    QPushButton *connect_button;
    QPushButton *disconnect;
    QSpacerItem *horizontalSpacer;
    QLabel *label;
    QLabel *status_label;
    QWidget *layoutWidget1;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_4;
    QHBoxLayout *f_spinner;
    QLabel *label_2;
    QSpacerItem *spinnerLeftSpacer_2;
    QSpacerItem *spinnerRightSpacer_2;
    QTreeView *list_folders;
    QListView *list_files;
    QPushButton *refresh_results;
    QWidget *layoutWidget_2;
    QHBoxLayout *top_spinner;
    QSpacerItem *spinnerLeftSpacer;
    QSpacerItem *spinnerRightSpacer;
    QLabel *output;
    mouse_coordinates *qt_drawing_output;
    QWidget *layoutWidget2;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *scrrenshot;
    QPushButton *save_region;
    QPushButton *start_recognition;
    QCheckBox *rec_with_images;
    QLabel *label_xy;
    QPushButton *get_time;
    QLabel *remote_time;
    QPushButton *set_time;
    QMenuBar *menuBar;
    QMenu *menuMotion_Detection;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(981, 611);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        layoutWidget = new QWidget(centralWidget);
        layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
        layoutWidget->setGeometry(QRect(63, 2, 901, 41));
        horizontalLayout = new QHBoxLayout(layoutWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        network_ip = new QLineEdit(layoutWidget);
        network_ip->setObjectName(QStringLiteral("network_ip"));
        network_ip->setMaximumSize(QSize(80, 16777215));

        horizontalLayout->addWidget(network_ip);

        label_3 = new QLabel(layoutWidget);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setMaximumSize(QSize(30, 16777215));

        horizontalLayout->addWidget(label_3);

        ips_combo = new QComboBox(layoutWidget);
        ips_combo->setObjectName(QStringLiteral("ips_combo"));
        ips_combo->setMaximumSize(QSize(151, 26));

        horizontalLayout->addWidget(ips_combo);

        search_button = new QPushButton(layoutWidget);
        search_button->setObjectName(QStringLiteral("search_button"));

        horizontalLayout->addWidget(search_button);

        connect_button = new QPushButton(layoutWidget);
        connect_button->setObjectName(QStringLiteral("connect_button"));

        horizontalLayout->addWidget(connect_button);

        disconnect = new QPushButton(layoutWidget);
        disconnect->setObjectName(QStringLiteral("disconnect"));

        horizontalLayout->addWidget(disconnect);

        horizontalSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        label = new QLabel(layoutWidget);
        label->setObjectName(QStringLiteral("label"));
        label->setLayoutDirection(Qt::LeftToRight);
        label->setFrameShape(QFrame::NoFrame);
        label->setTextFormat(Qt::AutoText);
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        label->setWordWrap(false);
        label->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

        horizontalLayout->addWidget(label);

        status_label = new QLabel(layoutWidget);
        status_label->setObjectName(QStringLiteral("status_label"));
        status_label->setStyleSheet(QLatin1String("background-color: rgb(255, 3, 20);\n"
"color: rgb(255, 252, 253);"));
        status_label->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(status_label);

        horizontalLayout->setStretch(2, 3);
        layoutWidget1 = new QWidget(centralWidget);
        layoutWidget1->setObjectName(QStringLiteral("layoutWidget1"));
        layoutWidget1->setGeometry(QRect(679, 39, 291, 391));
        verticalLayout = new QVBoxLayout(layoutWidget1);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setSizeConstraint(QLayout::SetMaximumSize);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        f_spinner = new QHBoxLayout();
        f_spinner->setSpacing(6);
        f_spinner->setObjectName(QStringLiteral("f_spinner"));
        label_2 = new QLabel(layoutWidget1);
        label_2->setObjectName(QStringLiteral("label_2"));

        f_spinner->addWidget(label_2);

        spinnerLeftSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        f_spinner->addItem(spinnerLeftSpacer_2);

        spinnerRightSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        f_spinner->addItem(spinnerRightSpacer_2);


        horizontalLayout_4->addLayout(f_spinner);


        verticalLayout->addLayout(horizontalLayout_4);

        list_folders = new QTreeView(layoutWidget1);
        list_folders->setObjectName(QStringLiteral("list_folders"));

        verticalLayout->addWidget(list_folders);

        list_files = new QListView(layoutWidget1);
        list_files->setObjectName(QStringLiteral("list_files"));

        verticalLayout->addWidget(list_files);

        refresh_results = new QPushButton(layoutWidget1);
        refresh_results->setObjectName(QStringLiteral("refresh_results"));

        verticalLayout->addWidget(refresh_results);

        verticalLayout->setStretch(1, 1);
        layoutWidget_2 = new QWidget(centralWidget);
        layoutWidget_2->setObjectName(QStringLiteral("layoutWidget_2"));
        layoutWidget_2->setGeometry(QRect(20, 6, 31, 31));
        top_spinner = new QHBoxLayout(layoutWidget_2);
        top_spinner->setSpacing(6);
        top_spinner->setContentsMargins(11, 11, 11, 11);
        top_spinner->setObjectName(QStringLiteral("top_spinner"));
        top_spinner->setContentsMargins(0, 0, 0, 0);
        spinnerLeftSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        top_spinner->addItem(spinnerLeftSpacer);

        spinnerRightSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        top_spinner->addItem(spinnerRightSpacer);

        output = new QLabel(centralWidget);
        output->setObjectName(QStringLiteral("output"));
        output->setGeometry(QRect(25, 41, 640, 480));
        qt_drawing_output = new mouse_coordinates(centralWidget);
        qt_drawing_output->setObjectName(QStringLiteral("qt_drawing_output"));
        qt_drawing_output->setGeometry(QRect(25, 41, 640, 480));
        layoutWidget2 = new QWidget(centralWidget);
        layoutWidget2->setObjectName(QStringLiteral("layoutWidget2"));
        layoutWidget2->setGeometry(QRect(10, 520, 661, 32));
        horizontalLayout_2 = new QHBoxLayout(layoutWidget2);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        scrrenshot = new QPushButton(layoutWidget2);
        scrrenshot->setObjectName(QStringLiteral("scrrenshot"));

        horizontalLayout_2->addWidget(scrrenshot);

        save_region = new QPushButton(layoutWidget2);
        save_region->setObjectName(QStringLiteral("save_region"));

        horizontalLayout_2->addWidget(save_region);

        start_recognition = new QPushButton(layoutWidget2);
        start_recognition->setObjectName(QStringLiteral("start_recognition"));
        start_recognition->setLayoutDirection(Qt::RightToLeft);
        start_recognition->setAutoFillBackground(false);
        start_recognition->setIconSize(QSize(40, 40));
        start_recognition->setCheckable(true);

        horizontalLayout_2->addWidget(start_recognition);

        rec_with_images = new QCheckBox(layoutWidget2);
        rec_with_images->setObjectName(QStringLiteral("rec_with_images"));

        horizontalLayout_2->addWidget(rec_with_images);

        label_xy = new QLabel(centralWidget);
        label_xy->setObjectName(QStringLiteral("label_xy"));
        label_xy->setGeometry(QRect(570, 50, 111, 16));
        label_xy->setStyleSheet(QStringLiteral("color: rgb(148, 148, 148);"));
        get_time = new QPushButton(centralWidget);
        get_time->setObjectName(QStringLiteral("get_time"));
        get_time->setGeometry(QRect(680, 440, 115, 32));
        remote_time = new QLabel(centralWidget);
        remote_time->setObjectName(QStringLiteral("remote_time"));
        remote_time->setGeometry(QRect(800, 450, 59, 16));
        set_time = new QPushButton(centralWidget);
        set_time->setObjectName(QStringLiteral("set_time"));
        set_time->setGeometry(QRect(860, 440, 115, 32));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 981, 22));
        menuMotion_Detection = new QMenu(menuBar);
        menuMotion_Detection->setObjectName(QStringLiteral("menuMotion_Detection"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuMotion_Detection->menuAction());
        menuMotion_Detection->addSeparator();
        menuMotion_Detection->addSeparator();

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "region", 0));
        network_ip->setText(QApplication::translate("MainWindow", "192.168.1", 0));
        label_3->setText(QApplication::translate("MainWindow", ".255", 0));
        search_button->setText(QApplication::translate("MainWindow", "Lookup", 0));
        connect_button->setText(QApplication::translate("MainWindow", "Connect ", 0));
        disconnect->setText(QApplication::translate("MainWindow", "Dissconnect", 0));
        label->setText(QApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-size:14pt;\">Status:</span></p></body></html>", 0));
        status_label->setText(QApplication::translate("MainWindow", "Disconnected", 0));
        label_2->setText(QApplication::translate("MainWindow", "Remote terminal folders", 0));
        refresh_results->setText(QApplication::translate("MainWindow", "Refresh Results", 0));
        output->setText(QString());
        qt_drawing_output->setText(QString());
        scrrenshot->setText(QApplication::translate("MainWindow", "ScreenShot", 0));
        save_region->setText(QApplication::translate("MainWindow", "Save Region", 0));
        start_recognition->setText(QApplication::translate("MainWindow", "Start Recognition", 0));
        rec_with_images->setText(QApplication::translate("MainWindow", "Save  Images", 0));
        label_xy->setText(QString());
        get_time->setText(QApplication::translate("MainWindow", "Get Time", 0));
        remote_time->setText(QApplication::translate("MainWindow", "TextLabel", 0));
        set_time->setText(QApplication::translate("MainWindow", "Set Time", 0));
        menuMotion_Detection->setTitle(QApplication::translate("MainWindow", "Motion Detection", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
