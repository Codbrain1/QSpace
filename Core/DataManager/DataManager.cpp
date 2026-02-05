#include "DataManager.h"
#include "Core/TaskManager/TaskManager.h"
#include "Enums/CommonEnumsIO.h"
#include "Interfaces/IOFactory.h"
#include "Structures/CommonStructuresIO.h"
#include <memory>
#include <qcontainerfwd.h>
#include <qfileinfo.h>
#include <qfuturewatcher.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <quuid.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkSmartPointer.h>
// TODO: добавить возможность отменить чтение через std::function<bool()> isCanceled = []{ return false; }
namespace QSpace::Core {
DataManager::DataManager(TaskManager* taskManager, QObject* parent) : QObject(parent), m_taskManager(taskManager) {
}
void DataManager::importDataAsync(const QString& path, const IO::ReadScheme& scheme) {
    qint64 fileSize = QFileInfo(path).size();
    auto   priority = getPriority(fileSize);
    QUuid  taskId   = QUuid::createUuid();
    emit   ioStarted(taskId, QString("Import File: " + QFileInfo(path).fileName()), 1);
    auto*  watcher = new QFutureWatcher<IO::ReadResult>(this);
    connect(watcher, &QFutureWatcher<IO::ReadResult>::finished, this, [this, watcher, taskId]() {
        IO::ReadResult result = watcher->result();
        if (result.isSuccess()) {
            emit dataReady(result.data);
        } else {
            emit errorOccured(result.errMessage);
        }
        emit ioFinished(taskId, result.isSuccess());
        watcher->deleteLater();
    });

    watcher->setFuture(m_taskManager->runIO(priority, [path, scheme, policy = m_global_policy]() -> IO::ReadResult {
        auto reader = IO::IOFactory::createReader(path);
        if (!reader) {
            return IO::ReadResult{nullptr, "Unsupported file format", IO::ReadStatus::InvalidFormat};
        }
        reader->setPolicy(policy);
        return reader->read(path, scheme);
    }));
}
void DataManager::importBatchDataAsync(const QList<IO::BatchTask>& tasks) {
    QUuid taskId = QUuid::createUuid();
    emit  ioStarted(taskId, QString("Import Batch"), tasks.size());
    auto* watcher = new QFutureWatcher<IO::ReadResult>(this);
    connect(watcher, &QFutureWatcher<IO::ReadResult>::resultReadyAt, this, [this, watcher, taskId](int index) {
        IO::ReadResult result = watcher->resultAt(index);
        if (result.isSuccess())
            emit batchFileReady(result);
        emit progressChanged(taskId, watcher->progressValue(), watcher->progressMaximum());
    });
    connect(watcher, &QFutureWatcher<IO::ReadResult>::finished, this, [this, watcher, taskId]() {
        emit ioFinished(taskId, true);
        watcher->deleteLater();
    });
    watcher->setFuture(
        m_taskManager->mapIO(tasks, [policy = m_global_policy](const IO::BatchTask& t) -> IO::ReadResult {
            auto reader = IO::IOFactory::createReader(t.path);
            if (!reader)
                return {nullptr, "Unsupported file format", IO::ReadStatus::InvalidFormat};
            reader->setPolicy(policy);
            return reader->read(t.path, t.scheme);
        }));
}

void DataManager::importBatchIdendicalDataAsync(const QStringList&    paths,
                                                const IO::ReadScheme& scheme,
                                                IO::FileFormat        format) {
    if (paths.isEmpty()) {
        emit errorOccured(QString("Paths list is empty"));
        return;
    }
    auto reader = IO::IOFactory::createReader(format);
    if (!reader) {
        emit errorOccured(QString("Unsupported file format"));
        return;
    }
    reader->setPolicy(m_global_policy);
    QUuid taskId = QUuid::createUuid();
    emit  ioStarted(taskId, "Import Batch", paths.size());

    auto* readerPtr = reader.release();
    auto* watcher   = new QFutureWatcher<IO::ReadResult>(this);
    connect(watcher, &QFutureWatcher<IO::ReadResult>::resultReadyAt, this, [this, watcher, taskId](int index) {
        auto result = watcher->resultAt(index);
        if (result.isSuccess()) {
            emit batchFileReady(result);
        }
        emit progressChanged(taskId, watcher->progressValue(), watcher->progressMaximum());
    });
    connect(watcher, &QFutureWatcher<IO::ReadResult>::finished, this, [this, watcher, taskId, readerPtr]() {
        bool overAllSuccess = (watcher->progressValue() > 0);
        delete readerPtr;
        emit ioFinished(taskId, overAllSuccess);
        watcher->deleteLater();
    });
    watcher->setFuture(m_taskManager->mapIO(paths, [readerPtr, scheme](const QString& path) -> IO::ReadResult {
        return readerPtr->read(path, scheme);
    }));
}
TaskPriority DataManager::getPriority(qint64 fileSize) {
    if (fileSize < 64 * 1024 * 1024) {
        return TaskPriority::Urgent;
    } else {
        return TaskPriority::Bulk;
    }
}
} // namespace QSpace::Core