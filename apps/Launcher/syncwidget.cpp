/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2016                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#include "syncwidget.h"

#include "infowidget.h"

#include <openspace/openspace.h>

#include <ghoul/ghoul.h>
#include <ghoul/filesystem/filesystem.h>
#include <ghoul/filesystem/file.h>
#include <ghoul/misc/dictionary.h>
#include <ghoul/lua/ghoul_lua.h>

#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>

#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/alert_types.hpp>

#include <future>

#include <fstream>
#include <fmt/format.h>

namespace {
    const std::string _loggerCat = "SyncWidget";

    const std::string _configurationFile = "Launcher.config";

    const int DownloadApplicationVersion = 1;

    const int nColumns = 3;

    const bool OverwriteFiles = false;
    const bool CleanInfoWidgets = true;
}

SyncWidget::SyncWidget(QWidget* parent, Qt::WindowFlags f) 
    : QWidget(parent, f)
    , _sceneLayout(nullptr)
    , _session(new libtorrent::session)
    , _threadPool(
        4,
        [](){}, [](){},
        ghoul::thread::ThreadPriorityClass::Idle,
        ghoul::thread::ThreadPriorityLevel::Lowest
    )
{
    setObjectName("SyncWidget");
    setFixedSize(500, 500);

    QBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    {
        QGroupBox* sceneBox = new QGroupBox;
        _sceneLayout = new QGridLayout;
        sceneBox->setLayout(_sceneLayout);
        layout->addWidget(sceneBox);
    }
    {
        QWidget* separatorLine = new QWidget;
        separatorLine->setFixedHeight(2);
        separatorLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        separatorLine->setStyleSheet(QString("background-color: #c0c0c0;"));
        layout->addWidget(separatorLine);
    }
    {
        QGroupBox* settingsBox = new QGroupBox;
        _useTorrents = new QCheckBox("Use Torrents");
        _useTorrents->setChecked(true);
        QBoxLayout* settingsLayout = new QVBoxLayout;
        settingsLayout->addWidget(_useTorrents);

        settingsBox->setLayout(settingsLayout);
        layout->addWidget(settingsBox);
    }
    {
        QWidget* separatorLine = new QWidget;
        separatorLine->setFixedHeight(2);
        separatorLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        separatorLine->setStyleSheet(QString("background-color: #c0c0c0;"));
        layout->addWidget(separatorLine);
    }
    {
        QWidget* container = new QWidget;
        QBoxLayout* l = new QHBoxLayout;
        container->setLayout(l);

        QLabel* status = new QLabel("Status:");
        l->addWidget(status);

        _statusInformation = new QLabel;
        l->addWidget(_statusInformation);

        _statusNumbering = new QLabel;
        l->addWidget(_statusNumbering);

        layout->addWidget(container);
    }
    {
        QPushButton* syncButton = new QPushButton("Synchronize Data");
        syncButton->setObjectName("SyncButton");
        QObject::connect(
            syncButton, SIGNAL(clicked(bool)),
            this, SLOT(syncButtonPressed())
        );

        layout->addWidget(syncButton);
    }

    {
        _widgetsArea = new QScrollArea;
        _widgetsArea->setWidgetResizable(true);

        QWidget* w = new QWidget;
        w->setObjectName("DownloadArea");
        _widgetsArea->setWidget(w);

        _downloadLayout = new QVBoxLayout(w);
        _downloadLayout->setMargin(0);
        _downloadLayout->setSpacing(0);
        _downloadLayout->addStretch(100);

        layout->addWidget(_widgetsArea);
    }
    QPushButton* close = new QPushButton("Close");
    layout->addWidget(close, Qt::AlignRight);
    QObject::connect(
        close, SIGNAL(clicked(bool)),
        this, SLOT(close())
    );

    setLayout(layout);

    ghoul::initialize();
    openspace::DownloadManager::initialize();

    // Make use of the rest of the request urls
    std::vector<std::string> requestUrls = {
        "http://data.openspaceproject.com/request"
    };
    _downloadManager = std::make_unique<openspace::DownloadManager>(
        requestUrls, DownloadApplicationVersion);

    libtorrent::error_code ec;
    _session->listen_on(std::make_pair(20280, 20290), ec);

    libtorrent::session_settings settings = _session->settings();
    settings.user_agent =
        "OpenSpace/" +
        std::to_string(openspace::OPENSPACE_VERSION_MAJOR) + "." +
        std::to_string(openspace::OPENSPACE_VERSION_MINOR) + "." +
        std::to_string(openspace::OPENSPACE_VERSION_PATCH);
    settings.allow_multiple_connections_per_ip = true;
    settings.ignore_limits_on_local_network = true;
    settings.connection_speed = 20;
    settings.active_downloads = -1;
    settings.active_seeds = -1;
    settings.active_limit = 30;
    settings.dht_announce_interval = 60;

    if (ec) {
        LFATAL("Failed to open socket: " << ec.message());
        return;
    }
    _session->start_upnp();

    std::ifstream file(_configurationFile);
    if (!file.fail()) {
        union {
            uint32_t value;
            std::array<char, 4> data;
        } size;

        file.read(size.data.data(), sizeof(uint32_t));
        std::vector<char> buffer(size.value);
        file.read(buffer.data(), size.value);
        file.close();

        libtorrent::entry e = libtorrent::bdecode(buffer.begin(), buffer.end());
        _session->start_dht(e);
    }
    else 
        _session->start_dht();

    _session->add_dht_router({ "router.utorrent.com", 6881 });
    _session->add_dht_router({ "dht.transmissionbt.com", 6881 });
    _session->add_dht_router({ "router.bittorrent.com", 6881 });
    _session->add_dht_router({ "router.bitcomet.com", 6881 });

    QTimer* timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(handleTimer()));
    timer->start(100);
}

SyncWidget::~SyncWidget() {
    libtorrent::entry dht = _session->dht_state();

    std::vector<char> buffer;
    libtorrent::bencode(std::back_inserter(buffer), dht);

    std::ofstream f(_configurationFile);

    union {
        uint32_t value;
        std::array<char, 4> data;
    } size;
    size.value = buffer.size();
    f.write(size.data.data(), sizeof(uint32_t));
    f.write(buffer.data(), buffer.size());

    ghoul::deinitialize();
}

void SyncWidget::closeEvent(QCloseEvent* event) {
    std::vector<libtorrent::torrent_handle> handles = _session->get_torrents();
    for (libtorrent::torrent_handle& h : handles) {
        h.flush_cache();
        _session->remove_torrent(h);
    }
}

void SyncWidget::setSceneFiles(QMap<QString, QString> sceneFiles) {
    _sceneFiles = std::move(sceneFiles);
    QStringList keys = _sceneFiles.keys();
    for (int i = 0; i < keys.size(); ++i) {
        const QString& sceneName = keys[i];

        QCheckBox* checkbox = new QCheckBox(sceneName);
        checkbox->setChecked(true);
        //checkbox->setChecked(sceneName == "download");

        _sceneLayout->addWidget(checkbox, i / nColumns, i % nColumns);
    }
}

void SyncWidget::clear() {
    using libtorrent::torrent_handle;
    for (QMap<torrent_handle, InfoWidget*>::iterator i = _torrentInfoWidgetMap.begin();
         i != _torrentInfoWidgetMap.end();
         ++i)
    {
        delete i.value();
    }

    //int nChildren = _downloadLayout->count();
    //for (int i = 0; i < nChildren; ++i) {
    //    delete _downloadLayout->itemAt(i);
    //}

    _updateInformation.clear();
    _finishedInformation.clear();

    _torrentInfoWidgetMap.clear();
    _session->abort();
}

void SyncWidget::syncButtonPressed() {
    using DlManager = openspace::DownloadManager;

    auto downloadFile = [this](std::string url, std::string destination, InfoWidget* w) {
        if (w) {
            return DlManager::download(
                std::move(url),
                std::move(destination),
                0,
                [this, w](DlManager::File& f, size_t currentSize, size_t totalSize) {
                    std::lock_guard<std::mutex> lock(_updateInformationMutex);
                    _updateInformation[w] = {
                        f.errorMessage,
                        currentSize,
                        totalSize
                    };
                },
                [this, w](DlManager::File& f) {
                    std::lock_guard<std::mutex> lock(_updateInformationMutex);
                    _finishedInformation.push_back(w);
                }
            );
        }
        else {
            return DlManager::download(
                std::move(url),
                std::move(destination)
            );
        }
    };

    clear();

    std::vector<std::string> scenes;
    QStringList list = selectedScenes();
    std::transform(
        list.begin(),
        list.end(),
        std::back_inserter(scenes),
        [](const QString& scene) { return scene.toStdString(); }
    );

    setStatus("Loading scenes");
    DownloadCollection::Collection collection = DownloadCollection::crawlScenes(scenes);

    std::vector<DlManager::FileTask> result;
    std::vector<InfoWidget*> newWidgets;

    // We need this only if there are torrent files and we don't want to use torrents
    // this is not in the inner loop as it does a synchronous downloading
    std::string torrentBaseUrl;
    if (!collection.torrentFiles.empty() && !_useTorrents->isChecked()) {
        torrentBaseUrl = _downloadManager->directTorrentDownloadBaseUrl();
    }

    setEnabled(false);
    newWidgets.reserve(
        collection.torrentFiles.size() +
        collection.directFiles.size() +
        collection.fileRequests.size()
    );

    // We are dealing with torrent files first as they might get converted into direct
    // downloads if the user specifies that they don't want to use the torrent library
    setStatus("Loading torrent files");
    LDEBUG("Torrent Files");
    for (const DownloadCollection::TorrentFile& tf : collection.torrentFiles) {
        LDEBUG(tf.file + " -> " + tf.destination);
        setNumbering(&tf - collection.torrentFiles.data(), collection.torrentFiles.size());

        ghoul::filesystem::Directory d = FileSys.currentDirectory();

        FileSys.createDirectory(tf.destination, ghoul::filesystem::FileSystem::Recursive::Yes);
        FileSys.setCurrentDirectory(tf.destination);

        std::string fullFile = absPath(tf.file);
        std::string fullDestination = absPath(tf.destination);

        if (!FileSys.fileExists(fullFile)) {
            LERROR(fmt::format("Torrent file '{}' did not exist", fullFile));
            continue;
        }

        if (_useTorrents->isChecked()) {
            libtorrent::error_code ec;
            libtorrent::add_torrent_params p;

            p.save_path = fullDestination;

            p.ti = new libtorrent::torrent_info(fullFile, ec);
            p.name = tf.file;
            p.storage_mode = libtorrent::storage_mode_allocate;
            p.auto_managed = true;
            if (ec) {
                LERROR(fullFile << ": " << ec.message());
                continue;
            }
            libtorrent::torrent_handle h = _session->add_torrent(p, ec);
            if (ec) {
                LERROR(fullFile << ": " << ec.message());
                continue;
            }

            if (_torrentInfoWidgetMap.find(h) == _torrentInfoWidgetMap.end()) {
                InfoWidget* w = new InfoWidget(
                    QString::fromStdString(fullFile),
                    h.status().total_wanted
                );
                
                newWidgets.push_back(w);
                //_downloadLayout->insertWidget(_downloadLayout->count() - 1, w);
                _torrentInfoWidgetMap[h] = w;
            }
        }
        else {
            libtorrent::error_code ec;
            libtorrent::torrent_info info(fullFile, ec);

            int nFiles = info.num_files();
            for (int i = 0; i < nFiles; ++i) {
                std::string file = info.files().at(i).path;
                std::replace(file.begin(), file.end(), '\\', '/');
                std::string url = torrentBaseUrl + '/' + file;

                DownloadCollection::DirectFile df {
                    tf.module,
                    std::move(url),
                    absPath(file)
                };

                collection.directFiles.push_back(std::move(df));
            }
        }
        FileSys.setCurrentDirectory(d);
        qApp->processEvents();
    }

    setStatus("Loading direct file downloads");
    LDEBUG("Direct Files");
    for (const DownloadCollection::DirectFile& df : collection.directFiles) {
        LDEBUG(df.url + " -> " + df.destination);
        setNumbering(&df - collection.directFiles.data(), collection.directFiles.size());

        //InfoWidget* w = new InfoWidget(QString::fromStdString(df.destination));
        //newWidgets.push_back(w);
        //_downloadLayout->insertWidget(_downloadLayout->count() - 1, w);

        result.push_back(downloadFile(df.url, df.destination, nullptr));

        qApp->processEvents();
    }

    setStatus("Loading file requests");
    LDEBUG("File Requests");
    for (const DownloadCollection::FileRequest& fr : collection.fileRequests) {
        LDEBUG(fmt::format("{}({}) -> {}", fr.identifier, fr.identifier, fr.destination));
        setNumbering(&fr - collection.fileRequests.data(), collection.fileRequests.size());

        std::vector<std::string> urls = _downloadManager->requestFiles(
            fr.identifier,
            fr.version
        );

        for (const std::string& url : urls) {
            std::string file = url.substr(url.find_last_of('/') + 1);

            InfoWidget* w = new InfoWidget(QString::fromStdString(file));
            newWidgets.push_back(w);
            //_downloadLayout->insertWidget(_downloadLayout->count() - 1, w);

            result.push_back(downloadFile(
                url,
                FileSys.pathByAppendingComponent(fr.destination, file),
                w
            ));
            qApp->processEvents();
        }
    }

    setStatus("Adding widgets");
    _widgetsArea->setUpdatesEnabled(false);
    int i = 0;
    for (InfoWidget* w : newWidgets) {
        //setNumbering(&w - newWidgets.data(), newWidgets.size());
        setNumbering(++i, newWidgets.size());
        
        _downloadLayout->addWidget(w);
        //_downloadLayout->insertWidget(_downloadLayout->count() - 1, w);
    }
    qApp->processEvents();
    _widgetsArea->setUpdatesEnabled(true);
    qApp->processEvents();

    setStatus("Starting downloads");
    for (openspace::DownloadManager::FileTask& t : result) {
        setNumbering(&t - result.data(), result.size());
        std::thread(std::move(t)).detach();

        //std::this_thread::sleep_for(std::chrono::microseconds(50));

        qApp->processEvents();
        //t();
    }
    //for (openspace::DownloadManager::FileTask& t : result) {
    //    _threadPool.queue(std::move(t));
    //}

    _statusInformation->setText("Downloading");
    _statusNumbering->setText("");
    setEnabled(true);


}

void SyncWidget::setStatus(QString message) {
    _statusInformation->setText(message);
    qApp->processEvents();
}

void SyncWidget::setNumbering(int i, int n) {
    _statusNumbering->setText(QString::number(i) + '/' + QString::number(n));
    qApp->processEvents();
}

QStringList SyncWidget::selectedScenes() const {
    QStringList result;
    int nChildren = _sceneLayout->count();
    for (int i = 0; i < nChildren; ++i) {
        QWidget* w = _sceneLayout->itemAt(i)->widget();
        QCheckBox* c = static_cast<QCheckBox*>(w);
        if (c->isChecked()) {
            QString t = c->text();
            result.append(_sceneFiles[t]);
        }
    }
    std::string scenes;
    for (QString s : result)
        scenes += s.toStdString() + "; ";
    LDEBUG("Downloading scenes: " << scenes);
    return result;
}

void SyncWidget::handleTimer() {
    using namespace libtorrent;
    
    std::map<InfoWidget*, UpdateInformation> updateInformation;
    std::vector<InfoWidget*> finishedInformation;
    {
        std::lock_guard<std::mutex> lock(_updateInformationMutex);

        updateInformation = _updateInformation;
        _updateInformation.clear();

        finishedInformation = _finishedInformation;
        _finishedInformation.clear();
    }

    for (const std::pair<InfoWidget*, UpdateInformation> p : updateInformation) {
        p.first->update(p.second.currentSize, p.second.totalSize, p.second.errorMessage);
    }

    //setUpdatesEnabled(false);
    for (InfoWidget* w : finishedInformation) {
        _downloadLayout->removeWidget(w);
        delete w;
    }
    //setUpdatesEnabled(true);

    std::vector<torrent_handle> handles = _session->get_torrents();
    for (torrent_handle h : handles) {
        torrent_status s = h.status();
        InfoWidget* w = _torrentInfoWidgetMap[h];

        if (w)
            w->update(s);

        if (CleanInfoWidgets && (s.state == torrent_status::finished || s.state == torrent_status::seeding)) {
            _torrentInfoWidgetMap.remove(h);
            delete w;
        }
    }

    // Only close every torrent if all torrents are finished
    bool allSeeding = true;
    for (torrent_handle h : handles) {
        torrent_status s = h.status();
        allSeeding &= (s.state == torrent_status::seeding);
    }

    if (allSeeding) {
        for (torrent_handle h : handles)
            _session->remove_torrent(h);
    }
}
