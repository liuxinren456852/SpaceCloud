/*
 * Copyright (c) 2018 Ally of Intelligence Technology Co., Ltd. All rights reserved.
 *
 * Created by WuKun on 2/15/19.
 * Contact with:wk707060335@gmail.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http: *www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
#include <stdio.h>
#include <utility>
#include <unordered_map>
#include <functional>

#include <QtGui/QIcon>
#include <QtGui/QKeyEvent>
#include <QtCore/QSettings>
#include <QtWidgets/QMenu>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QApplication>

#include <boost/log/trivial.hpp>

#include "OSGWidget.h"
#include "MainWindow.h"
#include "ItemInfos.h"
#include "OctreeWidget.h"
#include "FileLoadWidget.h"

//using namespace core;
using std::placeholders::_1;

const QString config_dir_name_str = "./config";

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
{
    this->setWindowTitle("SpaceCloud");

    osgwidget_ = new OSGWidget(this);
    this->setCentralWidget(osgwidget_);
    osgwidget_->init();

    initUI();
}

void MainWindow::initUI() {
    createMenu();
    createToolBar();
    createDockWidget();
}

void MainWindow::open() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open OSG Model"),
                                                    "./model",
                                                    tr("OSG Models (*.osg *.osgt *.osgb)"));
    if (fileName.isEmpty()) return;

    QFileInfo f(fileName);
    osgwidget_->readDataFromFile(f);
}

void MainWindow::createMenu() {
    open_model_action_ = new QAction(tr("Open"), this);
    open_model_action_->setIcon(QIcon(":/images/file_open.png"));
    connect(open_model_action_, &QAction::triggered, this, &MainWindow::loadFileActionTriggered);

    read_config_action_ = new QAction(tr("Read Config"), this);
    read_config_action_->setIcon(QIcon(":/images/setup.png"));
    connect(read_config_action_, &QAction::triggered, this, &MainWindow::readConfig);

    octree_action_ = new QAction(tr("Build Octree"), this);
    octree_action_->setIcon(QIcon(":/images/tree.png"));
    connect(octree_action_, &QAction::triggered, this, &MainWindow::buildOctree);
}

void MainWindow::createToolBar() {
    QToolBar *toolBar = addToolBar(tr("Tools"));

    toolBar->addAction(open_model_action_);
    toolBar->addSeparator();

    toolBar->addAction(read_config_action_);
    toolBar->addSeparator();

    toolBar->addAction(octree_action_);
    toolBar->addSeparator();
}

void MainWindow::createDockWidget() {
    tree_widget_ = new QTreeWidget(this);
    tree_widget_->setColumnCount(1);
    tree_widget_->setHeaderHidden(true);
    //tree_widget_->setColumnWidth(0, 100);
    //tree_widget_->setStyleSheet("QTreeWidget::item {height:25px;");

    // index: 0
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(tree_widget_, QStringList(tr("Layer")));
        item->setIcon(0, QIcon(":/images/subjectgeographic.png"));
        item->setExpanded(true);

        std::vector<ItemInfos> city_items = {
                {tr("ShenShan"), ItemInfos::DataType::Others, {""}, osgEarth::Viewpoint("Guangzhou", 114.978, 22.843, 0,
                                                                                        0,
                                                                                        -90, 1000), false},
                {tr("Boston"),   ItemInfos::DataType::Others, {""}, osgEarth::Viewpoint("Boston", -71.07, 42.34, 0, 0,
                                                                                        -90, 1000), false}
        };

        for (const auto &info : city_items) {
            QTreeWidgetItem *child_item = new QTreeWidgetItem(item, QStringList(info.name));
            child_item->setExpanded(true);

            addVariantToTreeWidgetItem(info, child_item);
        }
    }

    // 1
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(tree_widget_, QStringList(tr("User Data")));
        item->setIcon(0, QIcon(":/images/draw.png"));
        item->setExpanded(true);
    }

    // 2
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(tree_widget_, QStringList(tr("City Marker")));
        item->setIcon(0, QIcon(":/images/geographical-position.png"));
        item->setExpanded(true);

        std::vector<ItemInfos> city_items = {
                {tr("ShenZhen"),  ItemInfos::DataType::Others, {""}, osgEarth::Viewpoint("ShenZhen", 114.06, 22.55, 0, 0,
                                                                                        -90, 1000), false},
                {tr("BeiJing"),   ItemInfos::DataType::Others, {""}, osgEarth::Viewpoint("BeiJing", 116.30, 39.90, 0, 0,
                                                                                        -90, 1000),        false},
                {tr("Guangzhou"), ItemInfos::DataType::Others, {""}, osgEarth::Viewpoint("Guangzhou", 114.978, 22.843,
                                                                                         0, 0,
                                                                                         -90, 1000),       false},
                {tr("Test"),      ItemInfos::DataType::Others, {""}, osgEarth::Viewpoint("Test", -122.0856545755255,
                                                                                         37.42243077405461,
                                                                                         0, 0, -90, 1000), false}
        };

        for (const auto &info : city_items) {
            QTreeWidgetItem *child_item = new QTreeWidgetItem(item, QStringList(info.name));
            child_item->setExpanded(true);

            addVariantToTreeWidgetItem(info, child_item);
        }
    }

    dock_widget_ = new QDockWidget(tr("Scene"), this);
    dock_widget_->setFixedWidth(200);
    dock_widget_->setFeatures(QDockWidget::AllDockWidgetFeatures);
    dock_widget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock_widget_->setWidget(tree_widget_);
    dock_widget_->setFocusPolicy(Qt::NoFocus);
    this->addDockWidget(Qt::LeftDockWidgetArea, dock_widget_);

    //QTreeWidget connect
    connect(tree_widget_, &QTreeWidget::itemPressed, this, &MainWindow::TreeWidgetClicked);
    connect(tree_widget_, &QTreeWidget::itemDoubleClicked, this, &MainWindow::TreeWidgetDoubleClicked);
}

void MainWindow::addVariantToTreeWidgetItem(const ItemInfos &info, QTreeWidgetItem *item) {
    QVariant item_var;
    item_var.setValue(info);
    item->setData(0, Qt::UserRole, item_var);
}


void MainWindow::keyPressEvent(QKeyEvent *event) {
//    QMainWindow::keyPressEvent(event);
    BOOST_LOG_TRIVIAL(trace) << "MainWindow keyPressEvent " << event->text().toStdString();
    osgwidget_->keyPressEvent(event);
//    QWidget::keyPressEvent(event);
}

void MainWindow::TreeWidgetClicked(QTreeWidgetItem *item, int column) {
    if (!(QApplication::mouseButtons() & Qt::RightButton)) return;

    // valid check
    QVariant item_var = item->data(column, Qt::UserRole);
    if (item_var.isNull()) return;

    ItemInfos infos = item_var.value<ItemInfos>();
    BOOST_LOG_TRIVIAL(trace) << "TreeWidgetClicked at " << infos;

    // menu exec
    QMenu *menu = new QMenu(this);

    QAction *remove_action = new QAction(QIcon(":/images/delete.png"), tr("Remove"), this);
    remove_action->setStatusTip(tr("remove this from the scene"));
    menu->addAction(remove_action);

    QAction *cancel_action = new QAction(QIcon(":/images/cancel.png"), tr("Cancel"), this);
    menu->addAction(cancel_action);

    connect(remove_action, &QAction::triggered, [=]() {
        // treewidget
        item->parent()->removeChild(item);

        // memory
        all_items_.removeOne(infos);

        // scene
        osgwidget_->removeModelFromScene(infos);

        // file
        QString config_file_path = config_dir_name_str + "/" + infos.name + ".conf";
        QFile f(config_file_path);
        BOOST_LOG_TRIVIAL(trace) << "remove config: " << f.fileName().toStdString();
        f.remove();
    });

    connect(cancel_action, &QAction::triggered, menu, &QMenu::close);

    menu->exec(QCursor::pos());
}

void MainWindow::TreeWidgetDoubleClicked(QTreeWidgetItem *item, int column) {

    // valid check
    QVariant item_var = item->data(column, Qt::UserRole);
    if (item_var.isNull()) return;

    ItemInfos infos = item_var.value<ItemInfos>();
    BOOST_LOG_TRIVIAL(trace) << "TreeWidgetDoubleClicked at " << infos;

    osgwidget_->flyToViewPoint(infos.localtion);
}

void MainWindow::loadFileActionTriggered() {

    auto fileLoadWidget = new FileLoadWidget;
    connect(fileLoadWidget, &FileLoadWidget::loadItem, this, &MainWindow::loadItem);

    fileLoadWidget->exec();
}

void MainWindow::loadItem(ItemInfos infos) {
    QStringList file_paths = infos.file_path;
    QFileInfo file_info(file_paths.first());

    std::unordered_map<std::string, std::function<void(const ItemInfos &)>> hash_map;
    hash_map["osg"] = std::bind(&MainWindow::loadModel, this, _1);
    hash_map["osgb"] = std::bind(&MainWindow::loadModel, this, _1);
    hash_map["earth"] = std::bind(&MainWindow::loadLayer, this, _1);
    hash_map["kml"] = std::bind(&MainWindow::loadLayer, this, _1);

    std::string file_ext = file_info.suffix().toStdString();
    auto func = hash_map[file_ext];
    func(infos);
}

void MainWindow::loadLayer(const ItemInfos &infos) {
    BOOST_LOG_TRIVIAL(trace) << "MainWindow load layer: " << infos;

    QList<QTreeWidgetItem *> child_list = tree_widget_->findItems(tr("Layer"),
                                                                  Qt::MatchContains | Qt::MatchRecursive, 0);
    assert(child_list.size() == 1);

    QTreeWidgetItem *child_item = new QTreeWidgetItem(child_list.first(), QStringList(infos.name));
    addVariantToTreeWidgetItem(infos, child_item);
    all_items_.push_back(infos);

    osgwidget_->loadLayerToScene(infos);
}

void MainWindow::loadModel(const ItemInfos &infos) {
    BOOST_LOG_TRIVIAL(trace) << "MainWindow load model: " << infos;

    QList<QTreeWidgetItem *> child_list = tree_widget_->findItems(tr("User Data"),
                                                                  Qt::MatchContains | Qt::MatchRecursive, 0);
    assert(child_list.size() == 1);

    QTreeWidgetItem *child_item = new QTreeWidgetItem(child_list.first(), QStringList(infos.name));
    addVariantToTreeWidgetItem(infos, child_item);
    all_items_.push_back(infos);

    osgwidget_->loadModelToScene(infos);
}

void MainWindow::loadKML(const ItemInfos &infos) {
    BOOST_LOG_TRIVIAL(trace) << "MainWindow load kml: " << infos;

    QList<QTreeWidgetItem *> child_list = tree_widget_->findItems(tr("Layer"),
                                                                  Qt::MatchContains | Qt::MatchRecursive, 0);
    assert(child_list.size() == 1);

    QTreeWidgetItem *child_item = new QTreeWidgetItem(child_list.first(), QStringList(infos.name));
    addVariantToTreeWidgetItem(infos, child_item);
    all_items_.push_back(infos);

    osgwidget_->loadKMLToScene(infos);
}

void MainWindow::readConfig() {
    QDir config_dir("./config");
    QStringList nameFilters;
    nameFilters << "*.conf";
    QFileInfoList file_infos = config_dir.entryInfoList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);

    for (const auto &file_info : file_infos) {
        BOOST_LOG_TRIVIAL(trace) << "load config:" << file_info.filePath().toStdString();
        QFile f(file_info.filePath());
        f.open(QFile::ReadOnly);
        QDataStream s(&f);
        ItemInfos infos;
        s >> infos;
        f.close();

        if (infos.persistence) loadItem(infos);
    }
}

void MainWindow::saveConfig() {
    QDir config_dir;
    if (!config_dir.exists(config_dir_name_str)) config_dir.mkdir(config_dir_name_str);

    config_dir.cd(config_dir_name_str);
    BOOST_LOG_TRIVIAL(trace) << "save config to: " << config_dir.absolutePath().toStdString();
    for (const ItemInfos &infos : all_items_) {
        QString file_path = config_dir.path() + "/" + infos.name + ".conf";
        QFile f(file_path);
        f.open(QFile::WriteOnly);
        QDataStream s(&f);
        s << infos;
        f.close();

        BOOST_LOG_TRIVIAL(trace) << "write to: " << file_path.toStdString();
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    saveConfig();
    event->accept();
}

void MainWindow::buildOctree() {
    auto octree_widget = new OctreeWidget;

    octree_widget->exec();
}
