#include <QGuiApplication>

// Модули окна и ввода
#include <Qt3DExtras/QFirstPersonCameraController>
#include <Qt3DExtras/Qt3DWindow>


// Модули ядра и рендеринга Qt 3D
#include <QBlendEquation>
#include <QBlendEquationArguments>
#include <QElapsedTimer>
#include <QPerVertexColorMaterial>
#include <QRenderPass>
#include <QSphereGeometry>
#include <QTechnique>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QComponent>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QGeometry>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QGeometryRenderer>

#include <QTimer>
// Материалы, свет и базовые меши
#include "Common/Enums/IOEnums.h"
#include "Common/Enums/RenderEnums.h"
#include "Common/Interfaces/IOFactory.h"
#include "Common/Interfaces/IView.h"
#include "Common/Interfaces/LayerFactory.h"
#include "Common/Structures/IOStructures.h"
#include <QApplication>
#include <QByteArray>
#include <QMainWindow>
#include <QRandomGenerator>
#include <QVector3D>
#include <QWindow>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QComponent> // <-- ВАЖНО
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QGeometry>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QPerVertexColorMaterial> // <-- Новый материал
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QNoDepthMask>
#include <Qt3DRender/QPointLight>
#include <Qt3DRender/QPointSize>
#include <Qt3DRender/QRenderStateSet> // <-- ВАЖНО
#include <Qt3DRender/QRenderStateSet>
#include <qelapsedtimer.h>
#include <vtkDataSet.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

// Структура одной вершины: 3 float на позицию, 3 float на цвет (RGB)
struct PointVertex {
    float x, y, z;
    float r, g, b, a;
};

void getJetColor(float t, float& r, float& g, float& b) {
    r = std::min(4.0f * t - 1.5f, -4.0f * t + 4.5f);
    g = std::min(4.0f * t - 0.5f, -4.0f * t + 3.5f);
    b = std::min(4.0f * t + 0.5f, -4.0f * t + 2.5f);

    r = std::clamp(r, 0.0f, 1.0f);
    g = std::clamp(g, 0.0f, 1.0f);
    b = std::clamp(b, 0.0f, 1.0f);
}

Qt3DCore::QEntity* createPointCloudFromVtk(Qt3DCore::QEntity* rootEntity, vtkDataSet* dataSet) {
    if (!dataSet)
        return nullptr;

    vtkPolyData* polyData = vtkPolyData::SafeDownCast(dataSet);
    if (!polyData)
        return nullptr;

    vtkPoints*    points        = polyData->GetPoints();
    const int     particleCount = points->GetNumberOfPoints();
    vtkDataArray* densityArray  = polyData->GetPointData()->GetArray("Energy");
    double        range[2]      = {0.0, 1.0};
    if (densityArray)
        densityArray->GetRange(range);

    Qt3DCore::QEntity* cloudEntity = new Qt3DCore::QEntity(rootEntity);

    // 1. Подготовка данных
    QByteArray bufferBytes;
    bufferBytes.resize(particleCount * sizeof(PointVertex));
    PointVertex* vertices = reinterpret_cast<PointVertex*>(bufferBytes.data());

    for (int i = 0; i < particleCount; ++i) {
        double p[3];
        points->GetPoint(i, p);
        vertices[i].x = static_cast<float>(p[0]);
        vertices[i].y = static_cast<float>(p[1]);
        vertices[i].z = static_cast<float>(p[2]);

        float t = 0.5f;
        if (densityArray) {
            double val = densityArray->GetTuple1(i);
            t          = static_cast<float>((val - range[0]) / (range[1] - range[0]));
            getJetColor(t, vertices[i].r, vertices[i].g, vertices[i].b);
        }
        vertices[i].r = t;
        vertices[i].g = 0.0f;
        vertices[i].b = 1.0f - t;
        vertices[i].a = 1.0f;
    }

    // 2. Создаем материал ОДИН РАЗ
    Qt3DExtras::QPerVertexColorMaterial* material =
        new Qt3DExtras::QPerVertexColorMaterial(cloudEntity);

    // 3. Настраиваем PointSize и добавляем в ЭТОТ ЖЕ материал
    Qt3DRender::QPointSize* pointSize = new Qt3DRender::QPointSize();
    pointSize->setValue(5.0f); // Увеличили размер
    pointSize->setSizeMode(Qt3DRender::QPointSize::Fixed);


    Qt3DRender::QEffect* effect = material->effect();
    if (!effect->techniques().isEmpty()) {
        Qt3DRender::QTechnique* technique = effect->techniques().first();
        if (!technique->renderPasses().isEmpty()) {
            Qt3DRender::QRenderPass* pass = technique->renderPasses().first();

            // 1. Размер точки
            Qt3DRender::QPointSize* pointSize = new Qt3DRender::QPointSize(pass);
            pointSize->setValue(1.0f);
            pointSize->setSizeMode(Qt3DRender::QPointSize::Fixed);

            // 2. Смешивание (Аддитивное)
            Qt3DRender::QBlendEquation* blendEquation = new Qt3DRender::QBlendEquation(pass);
            blendEquation->setBlendFunction(Qt3DRender::QBlendEquation::Add);

            Qt3DRender::QBlendEquationArguments* blendArguments =
                new Qt3DRender::QBlendEquationArguments(pass);
            blendArguments->setSourceRgb(Qt3DRender::QBlendEquationArguments::SourceAlpha);
            blendArguments->setDestinationRgb(Qt3DRender::QBlendEquationArguments::One);

            // 3. ТЕСТ ГЛУБИНЫ (оставляем, чтобы точки прятались за стенами)
            Qt3DRender::QDepthTest* depthTest = new Qt3DRender::QDepthTest(pass);
            depthTest->setDepthFunction(Qt3DRender::QDepthTest::Less);

            // 4. ОТКЛЮЧЕНИЕ ЗАПИСИ (делает точки прозрачными друг для друга)
            Qt3DRender::QNoDepthMask* noDepthMask = new Qt3DRender::QNoDepthMask(pass);

            // Добавляем ВСЕ состояния в pass
            pass->addRenderState(pointSize);
            pass->addRenderState(blendEquation);
            pass->addRenderState(blendArguments);
            pass->addRenderState(depthTest);
            pass->addRenderState(noDepthMask);
        }
    }

    // 4. Настраиваем геометрию (Renderer)
    Qt3DRender::QGeometryRenderer* renderer     = new Qt3DRender::QGeometryRenderer(cloudEntity);
    Qt3DCore::QGeometry*           geometry     = new Qt3DCore::QGeometry(renderer);
    Qt3DCore::QBuffer*             vertexBuffer = new Qt3DCore::QBuffer(geometry);
    vertexBuffer->setData(bufferBytes);

    const int             stride       = sizeof(PointVertex);
    Qt3DCore::QAttribute* posAttribute = new Qt3DCore::QAttribute(geometry);
    posAttribute->setName(Qt3DCore::QAttribute::defaultPositionAttributeName());
    posAttribute->setVertexBaseType(Qt3DCore::QAttribute::Float);
    posAttribute->setVertexSize(3);
    posAttribute->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
    posAttribute->setBuffer(vertexBuffer);
    posAttribute->setByteStride(stride);
    posAttribute->setCount(particleCount);

    Qt3DCore::QAttribute* colorAttribute = new Qt3DCore::QAttribute(geometry);
    colorAttribute->setName(Qt3DCore::QAttribute::defaultColorAttributeName());
    colorAttribute->setVertexBaseType(Qt3DCore::QAttribute::Float);
    colorAttribute->setVertexSize(4);
    colorAttribute->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
    colorAttribute->setBuffer(vertexBuffer);
    colorAttribute->setByteStride(stride);
    colorAttribute->setByteOffset(3 * sizeof(float));
    colorAttribute->setCount(particleCount);

    geometry->addAttribute(posAttribute);
    geometry->addAttribute(colorAttribute);
    renderer->setGeometry(geometry);
    renderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Points);

    // 5. Добавляем ОДИН настроенный материал и рендерер
    cloudEntity->addComponent(renderer);
    cloudEntity->addComponent(material);

    return cloudEntity;
}

int main(int argc, char* argv[]) {
    qputenv("QT3D_RENDERER", "opengl");
    QGuiApplication app(argc, argv);

    // 3. Создаем окно Qt 3D
    Qt3DExtras::Qt3DWindow view;
    view.setTitle(QStringLiteral("Qt 3D Particle Cloud Test"));
    view.resize(1024, 768);

    // --- ИСПРАВЛЕНИЕ 1: Делаем фон черным ---
    view.defaultFrameGraph()->setClearColor(Qt::black);

    // 4. Создаем корневую сущность сцены
    Qt3DCore::QEntity* rootEntity = new Qt3DCore::QEntity();

    // 5. Настраиваем освещение (оставляем как есть)
    Qt3DCore::QEntity*       lightEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QPointLight* light       = new Qt3DRender::QPointLight(lightEntity);
    light->setColor(Qt::white);
    light->setIntensity(1.2f);
    lightEntity->addComponent(light);

    Qt3DCore::QTransform* lightTransform = new Qt3DCore::QTransform(lightEntity);
    lightTransform->setTranslation(QVector3D(20.0f, 30.0f, 20.0f));
    lightEntity->addComponent(lightTransform);

    // 6. Настраиваем камеру
    Qt3DRender::QCamera* camera = view.camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);

    // 7. Контроллер камеры
    Qt3DExtras::QFirstPersonCameraController* camController =
        new Qt3DExtras::QFirstPersonCameraController(rootEntity);
    camController->setCamera(camera);


    // --- ЛОГИКА ЧТЕНИЯ ФАЙЛА ---
    auto reader = QSpace::IO::IOFactory::createReader(QSpace::IO::FileFormat::BIN);
    reader->setPolicy(QSpace::IO::FilePolicy::ForceStandart);
    auto    scheme = QSpace::IO::SchemeFactory::createSheme_v2(QSpace::Visualize::EntityType::Gas,
                                                               QSpace::IO::FileFormat::BIN);
    QString path   = "D:/NIR/NIR_6_semestr/QSpace/Sandbox/TEST_DATA/TF250_v2/G_    0.bin";

    if (!reader) {
        qCritical() << "Exception create reader!";
        return 0;
    }

    QElapsedTimer timer;
    timer.start();
    auto   readResult = reader->read(path, scheme);
    qint64 timestamp  = timer.elapsed();

    if (readResult.isSuccess()) {
        qInfo() << "read file: " << path.toStdString()
                << QString(" is Success (Time = %1 ms)\n").arg(timestamp);
    } else {
        qCritical() << "read file failed!";
        return 0;
    }


    if (readResult.isSuccess()) {
        vtkDataSet* data = readResult.data;

        if (data) {
            // Создаем таймер для замера общего времени инициализации
            QElapsedTimer* renderTimer = new QElapsedTimer();
            renderTimer->start();

            // 1. Измеряем только C++ подготовку (заполнение QByteArray на CPU)
            createPointCloudFromVtk(rootEntity, data);
            qint64 cpuTime = renderTimer->elapsed();
            qInfo() << "[CPU] Reading data in CPU:" << cpuTime << "мс";

            // Устанавливаем сцену в окно
            view.setRootEntity(rootEntity);
            view.show();

            // 2. Ловим момент старта цикла событий (когда движок забрал данные)
            QTimer::singleShot(0, [renderTimer]() {
                qint64 totalTime = renderTimer->elapsed();
                qInfo() << "[Qt3D / GPU] initialize rendering conveyor:" << totalTime << "ms";
                delete renderTimer; // чистим память таймера
            });
        }
    } else {
        qCritical() << "Не удалось прочитать файл!";
    }

    view.setRootEntity(rootEntity);
    view.show();
    return app.exec();
}
