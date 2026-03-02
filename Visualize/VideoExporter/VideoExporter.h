#pragma once

#include "Visualize/Renderer.h"
#include <QObject>
#include <QString>
#include <vtkOggTheoraWriter.h>
#include <vtkSmartPointer.h>
#include <vtkWindowToImageFilter.h>

namespace QSpace::Visualize {

class VideoExporter : public QObject {
    Q_OBJECT
  public:
    explicit VideoExporter(Renderer* renderer, QObject* parent = nullptr);
    ~VideoExporter();
    /**
     * @brief инициализирует создание видеофайла
     */
    bool startExport(const QString& filepath, int fps = 30);
    void captureFrame();
    void finishExport();

  private:
    Renderer*                               m_renderer;
    vtkSmartPointer<vtkWindowToImageFilter> m_windowToImage;
    vtkSmartPointer<vtkOggTheoraWriter>     m_writer;
    bool                                    m_isRecording = false;
};

} // namespace QSpace::Visualize