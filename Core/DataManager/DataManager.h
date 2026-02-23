#pragma once
#include "Common/Structures/IOStructures.h"
#include "Core/TaskManager/TaskManager.h"
#include "Enums/CommonEnumsIO.h"
#include <qcontainerfwd.h>
#include <qlist.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <quuid.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkSmartPointer.h>

namespace QSpace::Core {
class DataManager : public QObject {
    Q_OBJECT
  public:
    explicit DataManager(TaskManager* tastManager, QObject* parent = nullptr);
    ~DataManager() override = default;
    /**
     * @brief Устанавливает глобальную политику (MMap/Stream) для создаваемых ридеров.
     */
    void setDefaultPolicy(IO::FilePolicy policy);
    /**
     * @brief чтение одного файла
     * создает объект ридера и читает файл в отдельном потоке,
     * Сигнал: dataReady(vtkSmartPointer<vtkDataSet>)
     */
    void importDataAsync(const QString& path, const IO::ReadScheme& scheme);
    /**
     * @brief  чтение пакета независимых файлов, каждый из которых отдельный объект
     * создает для каждого файла отдельный ридер
     * Сигнал: batchFileReady(ReadResult result)
     */
    void importBatchDataAsync(const QList<IO::BatchTask>& tasks);
    /**
     * @brief читает пакет файлов с одинаковой структурой
     * создается один ридер, но все данные независимы
     * Сигнал: batchFileReady(ReadResult result)
     */
    void importBatchIdendicalDataAsync(const QStringList& paths, const IO::ReadScheme& scheme, IO::FileFormat format);
    /**
     * @brief читает данные из нескольких файлов относящихся к одному снимку
     * могут быть созданы разные ридеры для чтения
     * Сигнал: snapshotReady(vtkSmartPointer<vtkMultiBlokDataSet> data)
     */
    // void importSnapshotAsync(const QList<IO::BatchTask>& tasks, QString& snapShotName);

  signals:
    // запущена операция чтения файлов
    void ioStarted(const QUuid& taskId, const QString& description, int total);
    void progressChanged(const QUuid& taskId, int value, int total);
    void ioFinished(const QUuid& taskId, bool succes);

    void fileReady(const QUuid& taskId, IO::ReadResult result);
    // void snapShotReady(vtkSmartPointer<vtkMultiBlockDataSet> data, const QString& name);

    void errorOccured(const QString& mes);

  private:
    TaskPriority   getPriority(qint64 fileSize);
    TaskManager*   m_taskManager;
    IO::FilePolicy m_global_policy = IO::FilePolicy::Auto;
};
} // namespace QSpace::Core