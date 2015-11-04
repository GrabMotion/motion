/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.5.0
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
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QTreeWidget>
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
    QLabel *label_255;
    QComboBox *ips_combo;
    QPushButton *search_button;
    QPushButton *engage_button;
    QCheckBox *mapdrive;
    QSpacerItem *horizontalSpacer_3;
    QLabel *label;
    QSpacerItem *horizontalSpacer_2;
    QLabel *label_speed;
    QComboBox *speedcombo;
    QSpacerItem *horizontalSpacer;
    QLabel *label_up_since;
    QLabel *status_label;
    QWidget *layoutWidget_2;
    QHBoxLayout *top_spinner;
    QSpacerItem *spinnerLeftSpacer;
    QSpacerItem *spinnerRightSpacer;
    QLabel *output;
    mouse_coordinates *qt_drawing_output;
    QLabel *label_xy;
    QLabel *remote_time;
    QLabel *amount_detected;
    QLabel *label_mat_piture;
    QWidget *gridLayoutWidget_2;
    QGridLayout *layout_time;
    QFrame *line;
    QLabel *label_rec_started;
    QLabel *label_terminal_status;
    QLabel *label_terminal_time;
    QFrame *line_2;
    QLabel *instance_started_2;
    QLabel *remote_terminal_time;
    QLabel *computer_time;
    QLabel *instance_started;
    QLabel *label_computer_time;
    QPushButton *set_time;
    QLabel *synched;
    QWidget *gridLayoutWidget_4;
    QGridLayout *layout_instances;
    QTreeWidget *remote_directory;
    QLabel *label_rec_job;
    QPushButton *expand;
    QPushButton *watch_video;
    QComboBox *motionmonth;
    QPushButton *collapse;
    QProgressBar *xml_progress;
    QComboBox *cameracombo;
    QComboBox *rec;
    QHBoxLayout *instance_spinner;
    QSpacerItem *spinnerLeftSpacer_2;
    QSpacerItem *spinnerRightSpacer_2;
    QPushButton *getxml;
    QComboBox *motionday;
    QPushButton *refresh;
    QLabel *label_camera;
    QPushButton *watch_image;
    QSpacerItem *verticalSpacer;
    QWidget *gridLayoutWidget_5;
    QGridLayout *gridLayout_3;
    QComboBox *delay;
    QPushButton *stream;
    QPushButton *save_region;
    QPushButton *picture;
    QLabel *code_label;
    QLineEdit *codename;
    QLabel *delay_label;
    QCheckBox *has_images;
    QProgressBar *mat_progress;
    QPushButton *clear_region;
    QDateTimeEdit *time_from;
    QDateTimeEdit *time_to;
    QLabel *label_name;
    QLineEdit *rec_name;
    QCheckBox *startup;
    QPushButton *rec_new;
    QCheckBox *between;
    QWidget *gridLayoutWidget_3;
    QGridLayout *process_quene;
    QTreeWidget *process_list;
    QLabel *label_process;
    QPushButton *run;
    QComboBox *process;
    QDateTimeEdit *time_process;
    QPushButton *edit_process;
    QPushButton *quene;
    QLabel *label_process_schedule;
    QPushButton *end_process;
    QFrame *line_3;
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QPushButton *start_recognition;
    QPushButton *save_rec;
    QMenuBar *menuBar;
    QMenu *menuMotion_Detection;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(999, 698);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        layoutWidget = new QWidget(centralWidget);
        layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
        layoutWidget->setGeometry(QRect(38, 6, 981, 41));
        horizontalLayout = new QHBoxLayout(layoutWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        network_ip = new QLineEdit(layoutWidget);
        network_ip->setObjectName(QStringLiteral("network_ip"));
        network_ip->setMaximumSize(QSize(70, 16777215));

        horizontalLayout->addWidget(network_ip);

        label_255 = new QLabel(layoutWidget);
        label_255->setObjectName(QStringLiteral("label_255"));
        label_255->setMaximumSize(QSize(30, 16777215));

        horizontalLayout->addWidget(label_255);

        ips_combo = new QComboBox(layoutWidget);
        ips_combo->setObjectName(QStringLiteral("ips_combo"));
        ips_combo->setMaximumSize(QSize(120, 26));

        horizontalLayout->addWidget(ips_combo);

        search_button = new QPushButton(layoutWidget);
        search_button->setObjectName(QStringLiteral("search_button"));
        search_button->setMaximumSize(QSize(70, 16777215));

        horizontalLayout->addWidget(search_button);

        engage_button = new QPushButton(layoutWidget);
        engage_button->setObjectName(QStringLiteral("engage_button"));
        engage_button->setMaximumSize(QSize(70, 16777215));

        horizontalLayout->addWidget(engage_button);

        mapdrive = new QCheckBox(layoutWidget);
        mapdrive->setObjectName(QStringLiteral("mapdrive"));

        horizontalLayout->addWidget(mapdrive);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_3);

        label = new QLabel(layoutWidget);
        label->setObjectName(QStringLiteral("label"));
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        label_speed = new QLabel(layoutWidget);
        label_speed->setObjectName(QStringLiteral("label_speed"));
        label_speed->setMaximumSize(QSize(40, 16777215));
        label_speed->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label_speed);

        speedcombo = new QComboBox(layoutWidget);
        speedcombo->setObjectName(QStringLiteral("speedcombo"));
        speedcombo->setMaximumSize(QSize(80, 16777215));

        horizontalLayout->addWidget(speedcombo);

        horizontalSpacer = new QSpacerItem(10, 20, QSizePolicy::Minimum, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        label_up_since = new QLabel(layoutWidget);
        label_up_since->setObjectName(QStringLiteral("label_up_since"));
        label_up_since->setLayoutDirection(Qt::LeftToRight);
        label_up_since->setFrameShape(QFrame::NoFrame);
        label_up_since->setTextFormat(Qt::AutoText);
        label_up_since->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        label_up_since->setWordWrap(false);
        label_up_since->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

        horizontalLayout->addWidget(label_up_since);

        status_label = new QLabel(layoutWidget);
        status_label->setObjectName(QStringLiteral("status_label"));
        status_label->setStyleSheet(QLatin1String("background-color: rgb(255, 3, 20);\n"
"color: rgb(255, 252, 253);"));
        status_label->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(status_label);

        horizontalLayout->setStretch(2, 3);
        layoutWidget_2 = new QWidget(centralWidget);
        layoutWidget_2->setObjectName(QStringLiteral("layoutWidget_2"));
        layoutWidget_2->setGeometry(QRect(5, 10, 31, 31));
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
        output->setGeometry(QRect(19, 50, 640, 480));
        qt_drawing_output = new mouse_coordinates(centralWidget);
        qt_drawing_output->setObjectName(QStringLiteral("qt_drawing_output"));
        qt_drawing_output->setGeometry(QRect(19, 50, 640, 480));
        label_xy = new QLabel(centralWidget);
        label_xy->setObjectName(QStringLiteral("label_xy"));
        label_xy->setGeometry(QRect(550, 60, 111, 16));
        label_xy->setStyleSheet(QStringLiteral("color: rgb(255, 255, 255);"));
        remote_time = new QLabel(centralWidget);
        remote_time->setObjectName(QStringLiteral("remote_time"));
        remote_time->setGeometry(QRect(768, 62, 0, 0));
        amount_detected = new QLabel(centralWidget);
        amount_detected->setObjectName(QStringLiteral("amount_detected"));
        amount_detected->setGeometry(QRect(1130, 480, 61, 16));
        label_mat_piture = new QLabel(centralWidget);
        label_mat_piture->setObjectName(QStringLiteral("label_mat_piture"));
        label_mat_piture->setGeometry(QRect(550, 480, 111, 16));
        label_mat_piture->setStyleSheet(QStringLiteral("color: rgb(255, 255, 255);"));
        gridLayoutWidget_2 = new QWidget(centralWidget);
        gridLayoutWidget_2->setObjectName(QStringLiteral("gridLayoutWidget_2"));
        gridLayoutWidget_2->setGeometry(QRect(670, 46, 321, 169));
        layout_time = new QGridLayout(gridLayoutWidget_2);
        layout_time->setSpacing(6);
        layout_time->setContentsMargins(11, 11, 11, 11);
        layout_time->setObjectName(QStringLiteral("layout_time"));
        layout_time->setSizeConstraint(QLayout::SetMinimumSize);
        layout_time->setContentsMargins(0, 0, 0, 0);
        line = new QFrame(gridLayoutWidget_2);
        line->setObjectName(QStringLiteral("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        layout_time->addWidget(line, 4, 0, 1, 3);

        label_rec_started = new QLabel(gridLayoutWidget_2);
        label_rec_started->setObjectName(QStringLiteral("label_rec_started"));
        label_rec_started->setMaximumSize(QSize(130, 11));
        label_rec_started->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        layout_time->addWidget(label_rec_started, 6, 0, 1, 1);

        label_terminal_status = new QLabel(gridLayoutWidget_2);
        label_terminal_status->setObjectName(QStringLiteral("label_terminal_status"));
        label_terminal_status->setMaximumSize(QSize(130, 11));
        label_terminal_status->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        layout_time->addWidget(label_terminal_status, 5, 0, 1, 1);

        label_terminal_time = new QLabel(gridLayoutWidget_2);
        label_terminal_time->setObjectName(QStringLiteral("label_terminal_time"));
        label_terminal_time->setMaximumSize(QSize(130, 11));
        label_terminal_time->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        layout_time->addWidget(label_terminal_time, 2, 0, 1, 1);

        line_2 = new QFrame(gridLayoutWidget_2);
        line_2->setObjectName(QStringLiteral("line_2"));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        layout_time->addWidget(line_2, 0, 0, 1, 3);

        instance_started_2 = new QLabel(gridLayoutWidget_2);
        instance_started_2->setObjectName(QStringLiteral("instance_started_2"));
        instance_started_2->setMaximumSize(QSize(16777215, 11));

        layout_time->addWidget(instance_started_2, 6, 1, 1, 2);

        remote_terminal_time = new QLabel(gridLayoutWidget_2);
        remote_terminal_time->setObjectName(QStringLiteral("remote_terminal_time"));

        layout_time->addWidget(remote_terminal_time, 2, 1, 1, 2);

        computer_time = new QLabel(gridLayoutWidget_2);
        computer_time->setObjectName(QStringLiteral("computer_time"));

        layout_time->addWidget(computer_time, 1, 1, 1, 2);

        instance_started = new QLabel(gridLayoutWidget_2);
        instance_started->setObjectName(QStringLiteral("instance_started"));
        instance_started->setMaximumSize(QSize(16777215, 11));

        layout_time->addWidget(instance_started, 5, 1, 1, 2);

        label_computer_time = new QLabel(gridLayoutWidget_2);
        label_computer_time->setObjectName(QStringLiteral("label_computer_time"));
        label_computer_time->setMaximumSize(QSize(130, 11));
        label_computer_time->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        layout_time->addWidget(label_computer_time, 1, 0, 1, 1);

        set_time = new QPushButton(gridLayoutWidget_2);
        set_time->setObjectName(QStringLiteral("set_time"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(set_time->sizePolicy().hasHeightForWidth());
        set_time->setSizePolicy(sizePolicy);
        set_time->setMaximumSize(QSize(16777215, 16777215));
        set_time->setAutoRepeatDelay(302);

        layout_time->addWidget(set_time, 3, 1, 1, 1);

        synched = new QLabel(gridLayoutWidget_2);
        synched->setObjectName(QStringLiteral("synched"));
        synched->setMaximumSize(QSize(100, 16777215));

        layout_time->addWidget(synched, 3, 2, 1, 1);

        gridLayoutWidget_4 = new QWidget(centralWidget);
        gridLayoutWidget_4->setObjectName(QStringLiteral("gridLayoutWidget_4"));
        gridLayoutWidget_4->setGeometry(QRect(670, 215, 323, 321));
        layout_instances = new QGridLayout(gridLayoutWidget_4);
        layout_instances->setSpacing(6);
        layout_instances->setContentsMargins(11, 11, 11, 11);
        layout_instances->setObjectName(QStringLiteral("layout_instances"));
        layout_instances->setContentsMargins(0, 0, 0, 0);
        remote_directory = new QTreeWidget(gridLayoutWidget_4);
        remote_directory->setObjectName(QStringLiteral("remote_directory"));

        layout_instances->addWidget(remote_directory, 4, 0, 1, 4);

        label_rec_job = new QLabel(gridLayoutWidget_4);
        label_rec_job->setObjectName(QStringLiteral("label_rec_job"));
        label_rec_job->setMaximumSize(QSize(55, 16777215));
        label_rec_job->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        layout_instances->addWidget(label_rec_job, 2, 0, 1, 1);

        expand = new QPushButton(gridLayoutWidget_4);
        expand->setObjectName(QStringLiteral("expand"));

        layout_instances->addWidget(expand, 5, 1, 1, 1);

        watch_video = new QPushButton(gridLayoutWidget_4);
        watch_video->setObjectName(QStringLiteral("watch_video"));
        watch_video->setMaximumSize(QSize(16777215, 16777215));

        layout_instances->addWidget(watch_video, 6, 2, 1, 1);

        motionmonth = new QComboBox(gridLayoutWidget_4);
        motionmonth->setObjectName(QStringLiteral("motionmonth"));

        layout_instances->addWidget(motionmonth, 1, 0, 1, 2);

        collapse = new QPushButton(gridLayoutWidget_4);
        collapse->setObjectName(QStringLiteral("collapse"));
        collapse->setMaximumSize(QSize(16777215, 16777215));

        layout_instances->addWidget(collapse, 5, 0, 1, 1);

        xml_progress = new QProgressBar(gridLayoutWidget_4);
        xml_progress->setObjectName(QStringLiteral("xml_progress"));
        xml_progress->setMaximumSize(QSize(60, 16777215));
        xml_progress->setValue(24);

        layout_instances->addWidget(xml_progress, 6, 3, 1, 1);

        cameracombo = new QComboBox(gridLayoutWidget_4);
        cameracombo->setObjectName(QStringLiteral("cameracombo"));

        layout_instances->addWidget(cameracombo, 0, 1, 1, 3);

        rec = new QComboBox(gridLayoutWidget_4);
        rec->setObjectName(QStringLiteral("rec"));
        rec->setMinimumSize(QSize(150, 0));

        layout_instances->addWidget(rec, 2, 1, 1, 3);

        instance_spinner = new QHBoxLayout();
        instance_spinner->setSpacing(6);
        instance_spinner->setObjectName(QStringLiteral("instance_spinner"));
        spinnerLeftSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        instance_spinner->addItem(spinnerLeftSpacer_2);

        spinnerRightSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        instance_spinner->addItem(spinnerRightSpacer_2);


        layout_instances->addLayout(instance_spinner, 5, 2, 1, 1);

        getxml = new QPushButton(gridLayoutWidget_4);
        getxml->setObjectName(QStringLiteral("getxml"));
        getxml->setMaximumSize(QSize(16777215, 16777215));

        layout_instances->addWidget(getxml, 6, 0, 1, 1);

        motionday = new QComboBox(gridLayoutWidget_4);
        motionday->setObjectName(QStringLiteral("motionday"));

        layout_instances->addWidget(motionday, 1, 2, 1, 2);

        refresh = new QPushButton(gridLayoutWidget_4);
        refresh->setObjectName(QStringLiteral("refresh"));
        refresh->setMaximumSize(QSize(16777215, 16777215));

        layout_instances->addWidget(refresh, 5, 3, 1, 1);

        label_camera = new QLabel(gridLayoutWidget_4);
        label_camera->setObjectName(QStringLiteral("label_camera"));
        label_camera->setMaximumSize(QSize(55, 16777215));
        label_camera->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        layout_instances->addWidget(label_camera, 0, 0, 1, 1);

        watch_image = new QPushButton(gridLayoutWidget_4);
        watch_image->setObjectName(QStringLiteral("watch_image"));
        watch_image->setMaximumSize(QSize(16777215, 16777215));

        layout_instances->addWidget(watch_image, 6, 1, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        layout_instances->addItem(verticalSpacer, 3, 1, 1, 1);

        gridLayoutWidget_5 = new QWidget(centralWidget);
        gridLayoutWidget_5->setObjectName(QStringLiteral("gridLayoutWidget_5"));
        gridLayoutWidget_5->setGeometry(QRect(10, 535, 781, 121));
        gridLayout_3 = new QGridLayout(gridLayoutWidget_5);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        gridLayout_3->setContentsMargins(0, 0, 0, 0);
        delay = new QComboBox(gridLayoutWidget_5);
        delay->setObjectName(QStringLiteral("delay"));
        delay->setMaximumSize(QSize(16777215, 16777215));

        gridLayout_3->addWidget(delay, 0, 8, 1, 1);

        stream = new QPushButton(gridLayoutWidget_5);
        stream->setObjectName(QStringLiteral("stream"));
        stream->setMaximumSize(QSize(80, 16777215));

        gridLayout_3->addWidget(stream, 0, 0, 1, 1);

        save_region = new QPushButton(gridLayoutWidget_5);
        save_region->setObjectName(QStringLiteral("save_region"));
        save_region->setMaximumSize(QSize(110, 16777215));

        gridLayout_3->addWidget(save_region, 0, 3, 1, 1);

        picture = new QPushButton(gridLayoutWidget_5);
        picture->setObjectName(QStringLiteral("picture"));
        picture->setMaximumSize(QSize(70, 16777215));
        picture->setCheckable(true);

        gridLayout_3->addWidget(picture, 0, 1, 1, 1);

        code_label = new QLabel(gridLayoutWidget_5);
        code_label->setObjectName(QStringLiteral("code_label"));
        code_label->setMaximumSize(QSize(40, 16777215));
        code_label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_3->addWidget(code_label, 0, 5, 1, 1);

        codename = new QLineEdit(gridLayoutWidget_5);
        codename->setObjectName(QStringLiteral("codename"));
        codename->setMaximumSize(QSize(16777215, 16777215));

        gridLayout_3->addWidget(codename, 0, 6, 1, 1);

        delay_label = new QLabel(gridLayoutWidget_5);
        delay_label->setObjectName(QStringLiteral("delay_label"));
        delay_label->setMaximumSize(QSize(16777215, 16777215));
        delay_label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_3->addWidget(delay_label, 0, 7, 1, 1);

        has_images = new QCheckBox(gridLayoutWidget_5);
        has_images->setObjectName(QStringLiteral("has_images"));
        has_images->setMaximumSize(QSize(75, 16777215));
        has_images->setChecked(true);

        gridLayout_3->addWidget(has_images, 1, 0, 1, 1);

        mat_progress = new QProgressBar(gridLayoutWidget_5);
        mat_progress->setObjectName(QStringLiteral("mat_progress"));
        mat_progress->setEnabled(true);
        mat_progress->setMinimumSize(QSize(0, 0));
        mat_progress->setMaximumSize(QSize(80, 16777215));
        mat_progress->setValue(24);

        gridLayout_3->addWidget(mat_progress, 0, 2, 1, 1);

        clear_region = new QPushButton(gridLayoutWidget_5);
        clear_region->setObjectName(QStringLiteral("clear_region"));
        clear_region->setMaximumSize(QSize(60, 16777215));

        gridLayout_3->addWidget(clear_region, 0, 4, 1, 1);

        time_from = new QDateTimeEdit(gridLayoutWidget_5);
        time_from->setObjectName(QStringLiteral("time_from"));
        time_from->setMaximumSize(QSize(90, 16777215));
        time_from->setTime(QTime(7, 0, 0));
        time_from->setMaximumDateTime(QDateTime(QDate(2000, 1, 1), QTime(12, 0, 0)));
        time_from->setMinimumDateTime(QDateTime(QDate(2000, 1, 1), QTime(7, 0, 0)));
        time_from->setMaximumTime(QTime(12, 0, 0));
        time_from->setMinimumTime(QTime(7, 0, 0));
        time_from->setTimeSpec(Qt::LocalTime);

        gridLayout_3->addWidget(time_from, 1, 2, 1, 1);

        time_to = new QDateTimeEdit(gridLayoutWidget_5);
        time_to->setObjectName(QStringLiteral("time_to"));
        time_to->setTime(QTime(21, 0, 0));
        time_to->setMaximumDateTime(QDateTime(QDate(2000, 2, 1), QTime(23, 59, 59)));
        time_to->setMinimumDateTime(QDateTime(QDate(2000, 1, 1), QTime(12, 0, 0)));

        gridLayout_3->addWidget(time_to, 1, 3, 1, 1);

        label_name = new QLabel(gridLayoutWidget_5);
        label_name->setObjectName(QStringLiteral("label_name"));
        label_name->setMaximumSize(QSize(16777215, 16777215));
        label_name->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_3->addWidget(label_name, 1, 4, 1, 1);

        rec_name = new QLineEdit(gridLayoutWidget_5);
        rec_name->setObjectName(QStringLiteral("rec_name"));

        gridLayout_3->addWidget(rec_name, 1, 5, 1, 2);

        startup = new QCheckBox(gridLayoutWidget_5);
        startup->setObjectName(QStringLiteral("startup"));
        startup->setMaximumSize(QSize(16777215, 16777215));

        gridLayout_3->addWidget(startup, 1, 7, 1, 1);

        rec_new = new QPushButton(gridLayoutWidget_5);
        rec_new->setObjectName(QStringLiteral("rec_new"));

        gridLayout_3->addWidget(rec_new, 1, 8, 1, 1);

        between = new QCheckBox(gridLayoutWidget_5);
        between->setObjectName(QStringLiteral("between"));
        between->setEnabled(true);
        between->setCheckable(true);
        between->setChecked(false);

        gridLayout_3->addWidget(between, 1, 1, 1, 1);

        gridLayoutWidget_3 = new QWidget(centralWidget);
        gridLayoutWidget_3->setObjectName(QStringLiteral("gridLayoutWidget_3"));
        gridLayoutWidget_3->setGeometry(QRect(1000, 6, 431, 611));
        process_quene = new QGridLayout(gridLayoutWidget_3);
        process_quene->setSpacing(6);
        process_quene->setContentsMargins(11, 11, 11, 11);
        process_quene->setObjectName(QStringLiteral("process_quene"));
        process_quene->setContentsMargins(0, 0, 0, 0);
        process_list = new QTreeWidget(gridLayoutWidget_3);
        process_list->setObjectName(QStringLiteral("process_list"));

        process_quene->addWidget(process_list, 2, 0, 1, 5);

        label_process = new QLabel(gridLayoutWidget_3);
        label_process->setObjectName(QStringLiteral("label_process"));
        label_process->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        process_quene->addWidget(label_process, 0, 0, 1, 1);

        run = new QPushButton(gridLayoutWidget_3);
        run->setObjectName(QStringLiteral("run"));
        run->setMaximumSize(QSize(60, 16777215));

        process_quene->addWidget(run, 0, 4, 1, 1);

        process = new QComboBox(gridLayoutWidget_3);
        process->setObjectName(QStringLiteral("process"));

        process_quene->addWidget(process, 0, 1, 1, 3);

        time_process = new QDateTimeEdit(gridLayoutWidget_3);
        time_process->setObjectName(QStringLiteral("time_process"));
        time_process->setMaximumSize(QSize(16777215, 16777215));

        process_quene->addWidget(time_process, 1, 1, 1, 2);

        edit_process = new QPushButton(gridLayoutWidget_3);
        edit_process->setObjectName(QStringLiteral("edit_process"));
        edit_process->setMaximumSize(QSize(120, 16777215));

        process_quene->addWidget(edit_process, 3, 0, 1, 1);

        quene = new QPushButton(gridLayoutWidget_3);
        quene->setObjectName(QStringLiteral("quene"));
        quene->setMaximumSize(QSize(16777215, 16777215));

        process_quene->addWidget(quene, 1, 3, 1, 2);

        label_process_schedule = new QLabel(gridLayoutWidget_3);
        label_process_schedule->setObjectName(QStringLiteral("label_process_schedule"));
        label_process_schedule->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        process_quene->addWidget(label_process_schedule, 1, 0, 1, 1);

        end_process = new QPushButton(gridLayoutWidget_3);
        end_process->setObjectName(QStringLiteral("end_process"));
        end_process->setMaximumSize(QSize(120, 16777215));

        process_quene->addWidget(end_process, 3, 3, 1, 2);

        line_3 = new QFrame(centralWidget);
        line_3->setObjectName(QStringLiteral("line_3"));
        line_3->setGeometry(QRect(10, 640, 1001, 20));
        line_3->setFrameShape(QFrame::HLine);
        line_3->setFrameShadow(QFrame::Sunken);
        gridLayoutWidget = new QWidget(centralWidget);
        gridLayoutWidget->setObjectName(QStringLiteral("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(800, 535, 191, 121));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        start_recognition = new QPushButton(gridLayoutWidget);
        start_recognition->setObjectName(QStringLiteral("start_recognition"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(start_recognition->sizePolicy().hasHeightForWidth());
        start_recognition->setSizePolicy(sizePolicy1);
        start_recognition->setMaximumSize(QSize(16777215, 60));
        QFont font;
        font.setFamily(QStringLiteral(".Helvetica Neue DeskInterface"));
        font.setPointSize(20);
        font.setBold(true);
        font.setWeight(75);
        start_recognition->setFont(font);
        start_recognition->setLayoutDirection(Qt::RightToLeft);
        start_recognition->setAutoFillBackground(false);
        start_recognition->setIconSize(QSize(40, 40));
        start_recognition->setCheckable(true);

        gridLayout->addWidget(start_recognition, 1, 0, 1, 2);

        save_rec = new QPushButton(gridLayoutWidget);
        save_rec->setObjectName(QStringLiteral("save_rec"));
        sizePolicy.setHeightForWidth(save_rec->sizePolicy().hasHeightForWidth());
        save_rec->setSizePolicy(sizePolicy);
        save_rec->setMinimumSize(QSize(100, 0));
        save_rec->setMaximumSize(QSize(16777215, 16777215));

        gridLayout->addWidget(save_rec, 0, 0, 1, 2);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 999, 22));
        menuMotion_Detection = new QMenu(menuBar);
        menuMotion_Detection->setObjectName(QStringLiteral("menuMotion_Detection"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);
        QWidget::setTabOrder(ips_combo, search_button);
        QWidget::setTabOrder(search_button, engage_button);
        QWidget::setTabOrder(engage_button, mapdrive);
        QWidget::setTabOrder(mapdrive, start_recognition);
        QWidget::setTabOrder(start_recognition, network_ip);

        menuBar->addAction(menuMotion_Detection->menuAction());
        menuMotion_Detection->addSeparator();
        menuMotion_Detection->addSeparator();

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Motion Detect", 0));
        network_ip->setText(QApplication::translate("MainWindow", "192.168.1", 0));
        label_255->setText(QApplication::translate("MainWindow", ".255", 0));
        search_button->setText(QApplication::translate("MainWindow", "Lookup", 0));
        engage_button->setText(QApplication::translate("MainWindow", "Engage", 0));
        mapdrive->setText(QApplication::translate("MainWindow", "Drive", 0));
        label->setText(QApplication::translate("MainWindow", "<html><head/><body><p><span style=\" color:#7f7f7f;\">DigiMotion 1.0.2</span></p></body></html>", 0));
        label_speed->setText(QApplication::translate("MainWindow", "Speed", 0));
        label_up_since->setText(QApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-size:11pt;\">Up Since:</span></p></body></html>", 0));
        status_label->setText(QApplication::translate("MainWindow", "Disconnected", 0));
        output->setText(QString());
        qt_drawing_output->setText(QString());
        label_xy->setText(QString());
        remote_time->setText(QString());
        amount_detected->setText(QString());
        label_mat_piture->setText(QString());
        label_rec_started->setText(QApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-size:11pt;\">Rec Started:</span></p></body></html>", 0));
        label_terminal_status->setText(QApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-size:11pt;\">Terminal Status:</span></p></body></html>", 0));
        label_terminal_time->setText(QApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-size:11pt;\">Terminal Time:</span></p></body></html>", 0));
        instance_started_2->setText(QString());
        remote_terminal_time->setText(QString());
        computer_time->setText(QString());
        instance_started->setText(QString());
        label_computer_time->setText(QApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-size:11pt;\">Computer Time:</span></p></body></html>", 0));
        set_time->setText(QApplication::translate("MainWindow", "Synch Time", 0));
        synched->setText(QString());
        QTreeWidgetItem *___qtreewidgetitem = remote_directory->headerItem();
        ___qtreewidgetitem->setText(2, QApplication::translate("MainWindow", "End", 0));
        ___qtreewidgetitem->setText(1, QApplication::translate("MainWindow", "Start", 0));
        ___qtreewidgetitem->setText(0, QApplication::translate("MainWindow", "Instance", 0));
        label_rec_job->setText(QApplication::translate("MainWindow", "Rec. job", 0));
        expand->setText(QApplication::translate("MainWindow", "Exp", 0));
        watch_video->setText(QApplication::translate("MainWindow", "Video", 0));
        collapse->setText(QApplication::translate("MainWindow", "Coll", 0));
        getxml->setText(QApplication::translate("MainWindow", "XML", 0));
        refresh->setText(QApplication::translate("MainWindow", "Refresh", 0));
        label_camera->setText(QApplication::translate("MainWindow", "Cameras", 0));
        watch_image->setText(QApplication::translate("MainWindow", "Image", 0));
        stream->setText(QApplication::translate("MainWindow", "Stream", 0));
        save_region->setText(QApplication::translate("MainWindow", "Save Region", 0));
        picture->setText(QApplication::translate("MainWindow", "Picture", 0));
        code_label->setText(QApplication::translate("MainWindow", "Code:", 0));
        delay_label->setText(QApplication::translate("MainWindow", "Delay:", 0));
        has_images->setText(QApplication::translate("MainWindow", "Images", 0));
        clear_region->setText(QApplication::translate("MainWindow", "Clear", 0));
        time_from->setDisplayFormat(QApplication::translate("MainWindow", "h:mm AP", 0));
        time_to->setDisplayFormat(QApplication::translate("MainWindow", "h:mm AP", 0));
        label_name->setText(QApplication::translate("MainWindow", "Name:", 0));
        startup->setText(QApplication::translate("MainWindow", "Startup", 0));
        rec_new->setText(QApplication::translate("MainWindow", "New Rec", 0));
        between->setText(QApplication::translate("MainWindow", "Between", 0));
        QTreeWidgetItem *___qtreewidgetitem1 = process_list->headerItem();
        ___qtreewidgetitem1->setText(1, QApplication::translate("MainWindow", "Status", 0));
        ___qtreewidgetitem1->setText(0, QApplication::translate("MainWindow", "Process", 0));
        label_process->setText(QApplication::translate("MainWindow", " Process:", 0));
        run->setText(QApplication::translate("MainWindow", "Run", 0));
        time_process->setDisplayFormat(QApplication::translate("MainWindow", "hh:mm A", 0));
        edit_process->setText(QApplication::translate("MainWindow", "Edit Process", 0));
        quene->setText(QApplication::translate("MainWindow", "Quene", 0));
        label_process_schedule->setText(QApplication::translate("MainWindow", "Schedule:", 0));
        end_process->setText(QApplication::translate("MainWindow", "End Proces", 0));
        start_recognition->setText(QApplication::translate("MainWindow", "START ", 0));
        save_rec->setText(QApplication::translate("MainWindow", "Save Rec", 0));
        menuMotion_Detection->setTitle(QApplication::translate("MainWindow", "Motion Detection", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
