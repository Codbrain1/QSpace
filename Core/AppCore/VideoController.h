#pragma once
#include <QObject>
#include <QStringList>
#include <quuid.h>
#include <memory>

namespace QSpace::Core {
class DataManager;
class ViewManager;
class LayerManager;
class VideoExportManager;
} // namespace QSpace::Core

namespace QSpace::Core::Controllers {
// ---------------------------------------------------------
// @SECTION: Экспорт видео
// ---------------------------------------------------------
class VideoController : public QObject {
    Q_OBJECT
  public:
    explicit VideoController(Core::DataManager*  dataManager,
                             Core::ViewManager*  viewManager,
                             Core::LayerManager* layerManager,
                             QObject*            parent = nullptr);
    /**
     * @brief  startVideoExport() --- инициализирует VideoExportManager для создания видео анимации
     * @param baseNodeId --- уникальный идентификатор слоя настройки которого исползуются для
     * анимации
     * @param files --- список путей к файлам для анимирования (все файла должны быть семантически
     * совместимы)
     * @param outputPath --- путь к выходному файлу с видео
     * @param stride --- шаг для пропуска файлов
     */
    void startVideoExport(const QUuid&       baseNodeId,
                          const QStringList& files,
                          const QString&     outputPath,
                          int                stride = 1,
                          int                fps    = 30);
    /**
     * @brief отменяет рендеринг видео
     */
    void cancelVideoExport();

  signals:
    /**
     * @brief сообщает об обновлении процесса создания видео
     * @param  currentFrame --- номер читаемого файла
     * @param  totalFrames --- общее число файлов
     */
    void exportProgressUpdated(int currentFrame, int totalFrames);
    /**
     * @brief сигнал об окончании создания видео
     */
    void exportFinished(bool success);

  private:
    Core::DataManager*  m_dataManager;
    Core::ViewManager*  m_viewManager;
    Core::LayerManager* m_layerManager;

    // Хранится прямо в контроллере на время экспорта
    std::shared_ptr<Core::VideoExportManager> m_videoExportManager;
};

} // namespace QSpace::Core::Controllers