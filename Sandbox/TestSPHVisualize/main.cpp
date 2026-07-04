#include "Common/Enums/IOEnums.h"
#include "Common/Enums/RenderEnums.h"
#include "Common/Interfaces/IOFactory.h"
#include "Common/Interfaces/IView.h"
#include "Common/Interfaces/LayerFactory.h"
#include "Common/Structures/IOStructures.h"
#include <QBlendEquation>
#include <QBlendEquationArguments>
#include <QByteArray>
#include <QCamera>
#include <QCameraSelector>
#include <QComputeCommand>
#include <QCuboidMesh>
#include <QDispatchCompute>
#include <QFirstPersonCameraController>
#include <QGraphicsApiFilter>
#include <QGuiApplication>
#include <QLoggingCategory>
#include <QMaterial>
#include <QMemoryBarrier>
#include <QNoDepthMask>
#include <QOrbitCameraController>
#include <QParameter>
#include <QPlaneMesh>
#include <QPointSize>
#include <QRenderPass>
#include <QRenderPassFilter>
#include <QShaderImage>
#include <QTechnique>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QNode>
#include <Qt3DExtras/QDiffuseSpecularMaterial>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DRender/QAbstractTextureImage>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QMemoryBarrier>
#include <Qt3DRender/QRenderSurfaceSelector>
#include <Qt3DRender/QRenderTarget>
#include <Qt3DRender/QRenderTargetOutput>
#include <Qt3DRender/QRenderTargetSelector>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QTextureImageData>
#include <Qt3DRender/QTextureImageDataGenerator>
#include <Qt3DRender/QViewport>
#include <qabstractcameracontroller.h>
#include <qabstracttexture.h>
#include <qbuffer.h>
#include <qcuboidmesh.h>
#include <qeffect.h>
#include <qentity.h>
#include <qfilterkey.h>
#include <qfirstpersoncameracontroller.h>
#include <qgraphicsapifilter.h>
#include <qimage.h>
#include <qmaterial.h>
#include <qnamespace.h>
#include <qorbitcameracontroller.h>
#include <qparameter.h>
#include <qphongmaterial.h>
#include <qplanemesh.h>
#include <qrenderpass.h>
#include <qshaderimage.h>
#include <qshaderprogram.h>
#include <qspheremesh.h>
#include <qtechnique.h>
#include <qtexture.h>
#include <qtransform.h>
#include <qurl.h>
#include <qvariant.h>
#include <qvectornd.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <locale.h>

// ============================================================
// main
// ============================================================
int main(int argc, char* argv[]) {
    QLoggingCategory::setFilterRules(QStringLiteral("qt.qt3d.render.shaders.debug=true"));
    // Добавьте это вместе с вашим правилом фильтрации
    QLoggingCategory::setFilterRules(QStringLiteral("qt.qt3d.*.debug=true\n"
                                                    "qt.qt3d.render.framegraph.debug=true"));
    setlocale(LC_ALL, "rus");

    // qputenv("QT3D_RENDERER", "opengl");
    QGuiApplication app(argc, argv);

    // создаем основное окно
    Qt3DExtras::Qt3DWindow view;
    view.resize(1024, 768);

    // корневая сущность
    Qt3DCore::QEntity* rootEntity = new Qt3DCore::QEntity();

    // задаем поизицию камеры
    Qt3DRender::QCamera* camera = view.camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    camera->setPosition(QVector3D(0, 0, 10.0f));
    camera->setViewCenter(QVector3D(0, 0, 0));
    camera->setUpVector(QVector3D(0, 1, 0));

    // настраиваем поворот камеры
    Qt3DExtras::QOrbitCameraController* cameraController =
        new Qt3DExtras::QOrbitCameraController(rootEntity);
    cameraController->setUpVector(QVector3D(0, 1, 0));
    cameraController->setCamera(camera);

    // INFO: Настройки камеры для удобного управления
    cameraController->setInverseTilt(true);
    cameraController->setInversePan(true);
    cameraController->setZoomTranslateViewCenter(false);
    // ------

    // указываем используемые директории
    const QString shaderDir = "D:/NIR/NIR_6_semestr/QSpace/Sandbox/TestSPHVisualize";
    const QString dataPath  = "D:/NIR/NIR_6_semestr/QSpace/Sandbox/TEST_DATA/TF250_v2/G_    0.bin";


    // ================ Читаем Файлы с частицами ================
    auto reader = QSpace::IO::IOFactory::createReader(QSpace::IO::FileFormat::BIN);
    if (!reader) {
        qCritical() << "Failed to create reader";
        return 1;
    }
    reader->setPolicy(QSpace::IO::FilePolicy::ForceStandart);

    auto scheme = QSpace::IO::SchemeFactory::createSheme_v2(QSpace::Visualize::EntityType::Gas,
                                                            QSpace::IO::FileFormat::BIN);

    auto readResult = reader->read(dataPath, scheme);
    if (!readResult.isSuccess()) {
        qCritical() << "Failed to read data";
        return 1;
    }
    // ========================================================


    // ================ Устанавливаем камеру в центр системы частиц ================

    vtkPolyData* polyData = vtkPolyData::SafeDownCast(readResult.data);
    if (!polyData || !polyData->GetPoints() || polyData->GetPoints()->GetNumberOfPoints() <= 0) {
        qCritical() << "Invalid polyData or empty points";
        return 1;
    }
    // ========================================================


    // ================ Устанавливаем камеру в центр системы частиц ================

    float  radius = 1.0f;
    double bounds[6];
    // Вычисляем центр и радиус облака точек по bounds VTK

    polyData->GetPoints()->GetBounds(bounds);

    QVector3D center = QVector3D(static_cast<float>((bounds[0] + bounds[1]) * 0.5),
                                 static_cast<float>((bounds[2] + bounds[3]) * 0.5),
                                 static_cast<float>((bounds[4] + bounds[5]) * 0.5));

    const float dx = static_cast<float>(bounds[1] - bounds[0]);

    const float dy = static_cast<float>(bounds[3] - bounds[2]);
    const float dz = static_cast<float>(bounds[5] - bounds[4]);

    double outRadius = 0.5f * std::sqrt(dx * dx + dy * dy + dz * dz);
    if (outRadius < 1e-6f)
        outRadius = 1.0f; // защита от вырожденного случая

    qDebug() << "Bounds center:" << center << "  radius:" << radius;

    // Камера смотрит на центр, отступает на 2 радиуса по Z
    camera->setViewCenter(center);
    camera->setPosition(center + QVector3D(0.0f, 0.0f, radius * 4.0f));
    camera->setUpVector(QVector3D(0.0f, 1.0f, 0.0f));

    Qt3DCore::QTransform* cubeTransform = new Qt3DCore::QTransform();
    cubeTransform->setTranslation(center);
    // ========================================================

    // INFO: ================ Настраиваем отображение частиц ================
    // >---- КОНВЕЙР ОТРИСОВКИ ----<
    // 1. Проецирование размытых частиц на сетку (текстуру с точностью FLOAT)
    // 2. Суммирование всех значений в ячейках сетки
    // 3. Сопоставление с цветовой палитрой
    //
    // >------ Описание используемых механизмов ------<

    //===================================================================================
    // 1) Нам нужна промежуточная текстура высокой точности, куда видеокарта будет суммировать
    //    значения частиц.
    //===================================================================================

    //    Для этого используется следующая последовательность настроек:
    //      a) создание текстуры, с высокой точностью
    //      б) создание ShaderImage для этой текстуры (это как Массив памяти)
    //      в) создание вычислительного шейдера
    //      г) создание последовательности настроек: Шейдер -> проход
    //         рендеринга -> Техника (указания видеокарте)  -> эффект для материала -> Материал
    //      д) Фильтр прохода рендеринга, чтобы указать в какой последовательности вызывать шейдеры
    //      е) добавляем текстуру ShaderImage к материалу, через параметр gridImage, чтобе compude
    //         шейдер знал куда писать результат
    //      ж) передать матрицу камеры в материал (чтобы при ее вращении частицы корректно
    //         перерисовывались) и размер сетки
    //      з) преобразовать набор частиц в SSBO буффер
    //      и) создать команду для вызова шейдера
    //      к) создать вычислительную сущность и добавить к ней созданный материал и комманду


    // ========> a)
    auto* gridTexture = new Qt3DRender::QTexture2D();
    gridTexture->setFormat(Qt3DRender::QAbstractTexture::R32F);
    gridTexture->setWidth(1024);
    gridTexture->setHeight(1024);

    gridTexture->setGenerateMipMaps(false);
    gridTexture->setMinificationFilter(Qt3DRender::QAbstractTexture::Linear);
    gridTexture->setMagnificationFilter(Qt3DRender::QAbstractTexture::Linear);
    // ========> б)
    // Оборачиваем текстуру в QShaderImage (для чтения/записи в шейдере) - это как Массив памяти
    // для чтения/записи»
    auto shaderImagePass1 = new Qt3DRender::QShaderImage();
    shaderImagePass1->setTexture(gridTexture);                        // задаем текстуру
    shaderImagePass1->setAccess(Qt3DRender::QShaderImage::ReadWrite); // разращаем чтение и запись
    shaderImagePass1->setFormat(Qt3DRender::QShaderImage::R32F);

    // ===================================================================================
    // Внедрение PASS 0: Очистка текстуры
    // ===================================================================================

    auto* computeShaderPass0 = new Qt3DRender::QShaderProgram();
    computeShaderPass0->setComputeShaderCode(
        Qt3DRender::QShaderProgram::loadSource(QUrl::fromLocalFile(shaderDir + "/clear.comp")));

    auto* renderPass0 = new Qt3DRender::QRenderPass();
    renderPass0->setShaderProgram(computeShaderPass0);

    auto* filterKeyPass0 = new Qt3DRender::QFilterKey();
    filterKeyPass0->setName(QStringLiteral("pass0"));
    filterKeyPass0->setValue(QStringLiteral("CLEAR_TEXTURE"));
    renderPass0->addFilterKey(filterKeyPass0);

    auto* techniquePass0 = new Qt3DRender::QTechnique();
    techniquePass0->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::RHI);
    techniquePass0->addRenderPass(renderPass0);

    auto* effectPass0 = new Qt3DRender::QEffect();
    effectPass0->addTechnique(techniquePass0);

    auto* computeMaterialPass0 = new Qt3DRender::QMaterial();
    computeMaterialPass0->setEffect(effectPass0);

    auto* imageParamPass0 = new Qt3DRender::QParameter();
    imageParamPass0->setName(QStringLiteral("gridImage"));
    imageParamPass0->setValue(QVariant::fromValue(shaderImagePass1));
    computeMaterialPass0->addParameter(imageParamPass0);

    auto* computeCommandPass0 = new Qt3DRender::QComputeCommand();
    // 1024 / 16 = 64 рабочих групп по X и Y (при local_size = 16 в GLSL)
    computeCommandPass0->setWorkGroupX(64);
    computeCommandPass0->setWorkGroupY(64);
    computeCommandPass0->setWorkGroupZ(1);

    auto* computeEntityPass0 = new Qt3DCore::QEntity(rootEntity);
    computeEntityPass0->addComponent(computeCommandPass0);
    computeEntityPass0->addComponent(computeMaterialPass0);


    // ========> в)
    // Создаем материал для вычислений и загружаем Compute Шейдер
    auto computeShaderPass1 = new Qt3DRender::QShaderProgram();
    computeShaderPass1->setComputeShaderCode(
        Qt3DRender::QShaderProgram::loadSource(QUrl::fromLocalFile(shaderDir + "/pass1.comp")));


    // ========> г)
    auto renderPass1          = new Qt3DRender::QRenderPass();
    auto techniquePass1       = new Qt3DRender::QTechnique();
    auto effectPass1          = new Qt3DRender::QEffect();
    auto computeMaterialPass1 = new Qt3DRender::QMaterial();

    // Настраиваем технику RHi чтобы qt сам выбирал графическое API (требования к видеокарте)
    techniquePass1->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::RHI);
    techniquePass1->graphicsApiFilter()->setMajorVersion(1);
    techniquePass1->graphicsApiFilter()->setMinorVersion(0);

    // Собираем матрешку: Шейдер -> Проход -> Техника -> Эффект -> Материал
    renderPass1->setShaderProgram(computeShaderPass1);
    techniquePass1->addRenderPass(renderPass1);
    effectPass1->addTechnique(techniquePass1);
    computeMaterialPass1->setEffect(effectPass1);


    // ========> д)
    // Добавляем тег (FilterKey) на этот проход, чтобы FrameGraph мог его вызвать
    auto filterKeyPass1 = new Qt3DRender::QFilterKey();
    filterKeyPass1->setName(QStringLiteral("pass1"));
    filterKeyPass1->setValue(QStringLiteral("COMPUTE_PARTICLES"));
    renderPass1->addFilterKey(filterKeyPass1);


    // ========> е)
    // Передаем параметры в материал (они станут доступны в GLSL), по сути задаем текстуру для
    // материала
    auto imageParamPass1 = new Qt3DRender::QParameter();
    imageParamPass1->setName(QStringLiteral("gridImage"));
    imageParamPass1->setValue(QVariant::fromValue(shaderImagePass1));
    computeMaterialPass1->addParameter(imageParamPass1);

    // ========> ж)
    // Передаем МАТРИЦУ КАМЕРЫ
    auto vpMatrixParam = new Qt3DRender::QParameter();
    vpMatrixParam->setName(QStringLiteral("viewProjectionMatrix")); // имя в GLSL

    auto updateVPMatrix = [camera, vpMatrixParam]() {
        // В Qt 3D полная матрица получается перемножением матрицы проекции на матрицу вида
        QMatrix4x4 viewProj = camera->projectionMatrix() * camera->viewMatrix();
        vpMatrixParam->setValue(QVariant::fromValue(viewProj));
    };
    updateVPMatrix();
    computeMaterialPass1->addParameter(vpMatrixParam);

    QObject::connect(camera,
                     &Qt3DRender::QCamera::viewMatrixChanged,
                     vpMatrixParam,
                     updateVPMatrix);

    // На всякий случай подключаем сигнал изменения ЛИНЗЫ (например, при ресайзе окна)
    QObject::connect(camera,
                     &Qt3DRender::QCamera::projectionMatrixChanged,
                     vpMatrixParam,
                     updateVPMatrix);

    auto gridSizeParam = new Qt3DRender::QParameter();
    gridSizeParam->setName(QStringLiteral("gridSize"));
    gridSizeParam->setValue(QSize(1024, 1024));
    computeMaterialPass1->addParameter(gridSizeParam);


    // ========> з)
    // >---------- Добавляем частицы в буфер SSBO ----------<
    vtkPoints* points      = polyData->GetPoints();
    int        pointsCount = points->GetNumberOfPoints();

    vtkDataArray* densityArray = polyData->GetPointData()->GetArray("Density");

    if (!densityArray) {
        qWarning() << "Array 'Density' not found in point data";
        return 1;
    }

    struct shaderParticle {
        float x, y, z;
        float padding; // выравниваем vec3 (x,y,z) до vec4
        float value;
        float pad1, pad2, pad3; // выраванивание value дло 16 байт
    };

    QByteArray bufferData;
    bufferData.resize(sizeof(shaderParticle) * pointsCount);
    shaderParticle* rawBuffer = reinterpret_cast<shaderParticle*>(bufferData.data());

    for (int i = 0; i < pointsCount; ++i) {
        double p[3];
        points->GetPoint(i, p);
        rawBuffer[i].x       = static_cast<float>(p[0]);
        rawBuffer[i].y       = static_cast<float>(p[1]);
        rawBuffer[i].z       = static_cast<float>(p[2]);
        rawBuffer[i].padding = 0.0f;
        rawBuffer[i].value   = static_cast<float>(densityArray->GetTuple1(i));
        rawBuffer[i].pad1    = 0.0f;
        rawBuffer[i].pad2    = 0.0f;
        rawBuffer[i].pad3    = 0.0f;
    }

    Qt3DCore::QBuffer* particleBuffer = new Qt3DCore::QBuffer();
    particleBuffer->setData(bufferData);

    Qt3DRender::QParameter* bufferParam = new Qt3DRender::QParameter();
    bufferParam->setName(QStringLiteral("ParticleBuffer"));
    bufferParam->setValue(QVariant::fromValue(particleBuffer));
    computeMaterialPass1->addParameter(bufferParam);

    Qt3DRender::QParameter* totalParticleCount = new Qt3DRender::QParameter();
    totalParticleCount->setName(QStringLiteral("totalParticles"));
    totalParticleCount->setValue(pointsCount);
    computeMaterialPass1->addParameter(totalParticleCount);


    // ========> и)
    // Создаем команду вызова шейдера и вешаем на неё наш материал
    auto computeCommandPass1 = new Qt3DRender::QComputeCommand();

    int localSizeX = 256;
    int numGroupsX = (pointsCount + localSizeX - 1) / localSizeX;
    computeCommandPass1->setWorkGroupX(numGroupsX);
    computeCommandPass1->setWorkGroupY(1);
    computeCommandPass1->setWorkGroupZ(1);

    // ========> к)
    auto computeEntityPass1 = new Qt3DCore::QEntity(rootEntity);

    computeEntityPass1->addComponent(computeCommandPass1);
    computeEntityPass1->addComponent(computeMaterialPass1);

    //===================================================================================
    // 2) Второй проход: Вывод сетки на экран, Теперь нам нужно создать геометрию и материал,
    //    которые возьмут текстуру gridTexture (уже заполненную первым проходом) и отобразят её на
    //    экране монитора.
    //===================================================================================
    //
    // Для этого используется следующая последовательность настроек:
    //    а) создать мешь плоскости чтобы на него проецироватьчастицы
    //    б) создать шейдер визуализации
    //    в) создать проход рендеринга и добавить к нему шейдер
    //    г) добавить фильтр к проходу
    //    е) создать технику и добавить к ней проходд рендеринга
    //    ж) ...
    // создать отдельную сущность для отображения на экран


    // ========> a)
    auto planeMeshPass2 = new Qt3DExtras::QPlaneMesh();
    planeMeshPass2->setWidth(2.0f);
    planeMeshPass2->setHeight(2.0f);

    // В Qt 3D QPlaneMesh по умолчанию лежит горизонтально.
    // Поворачиваем её на 90 градусов по оси X, чтобы она встала вертикально перед глазами.
    auto planeTransformPass2 = new Qt3DCore::QTransform();
    planeTransformPass2->setTranslation(center);
    planeTransformPass2->setRotation(
        QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), 90.0f));


    // ========> б)
    // добавляем визуальные шейдеры для сетки

    auto visualShaderPass2 = new Qt3DRender::QShaderProgram();
    visualShaderPass2->setVertexShaderCode(
        Qt3DRender::QShaderProgram::loadSource(QUrl::fromLocalFile(shaderDir + "/pass2.vert")));
    visualShaderPass2->setFragmentShaderCode(
        Qt3DRender::QShaderProgram::loadSource(QUrl::fromLocalFile(shaderDir + "/pass2.frag")));

    // ========> в)
    auto renderPass2 = new Qt3DRender::QRenderPass();
    renderPass2->setShaderProgram(visualShaderPass2);

    // ========> г)
    auto filterKeyPass2 = new Qt3DRender::QFilterKey();
    filterKeyPass2->setName(QStringLiteral("pass2"));
    filterKeyPass2->setValue(QStringLiteral("FORWARD_RENDER_SCENE"));
    renderPass2->addFilterKey(filterKeyPass2);

    // ========> г)
    auto techniquePass2 = new Qt3DRender::QTechnique();
    techniquePass2->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::RHI);
    techniquePass2->addRenderPass(renderPass2);

    auto effectPass2 = new Qt3DRender::QEffect();
    effectPass2->addTechnique(techniquePass2);

    auto visualMaterialPass2 = new Qt3DRender::QMaterial();
    visualMaterialPass2->setEffect(effectPass2);

    auto textureParam = new Qt3DRender::QParameter();
    textureParam->setName(QStringLiteral("gridTexture"));
    textureParam->setValue(QVariant::fromValue(gridTexture));
    visualMaterialPass2->addParameter(textureParam);


    Qt3DCore::QEntity* screenQuadEntityPass2 = new Qt3DCore::QEntity(rootEntity);
    screenQuadEntityPass2->addComponent(planeMeshPass2);
    screenQuadEntityPass2->addComponent(planeTransformPass2);
    screenQuadEntityPass2->addComponent(visualMaterialPass2);


    // ================ НАСТРОЙКА КОНВЕЙЕРА (FRAME GRAPH) ================

    auto* surfaceSelector = new Qt3DRender::QRenderSurfaceSelector();
    surfaceSelector->setSurface(&view);

    // Узел очистки экрана (чистим и цвет, и глубину)
    auto* clearBuffers = new Qt3DRender::QClearBuffers(surfaceSelector);
    clearBuffers->setBuffers(Qt3DRender::QClearBuffers::ColorDepthBuffer);
    clearBuffers->setClearColor(Qt::white);

    // --- ВЕТКА 1: Очистка (Pass 0) ---
    auto* pass0Filter   = new Qt3DRender::QRenderPassFilter(clearBuffers);
    auto* matchKeyPass0 = new Qt3DRender::QFilterKey();
    matchKeyPass0->setName(QStringLiteral("pass0"));
    matchKeyPass0->setValue(QStringLiteral("CLEAR_TEXTURE"));
    pass0Filter->addMatch(matchKeyPass0);

    // --- БАРЬЕР 1: Ждем окончания очистки ---
    auto* barrier1 = new Qt3DRender::QMemoryBarrier(clearBuffers);
    barrier1->setWaitOperations(Qt3DRender::QMemoryBarrier::ShaderImageAccess);

    // --- ВЕТКА 2: Вычисления (Pass 1) ---
    // Важно: Ветка растет ИЗ барьера, чтобы гарантировать строгий порядок!
    auto* computeFilter = new Qt3DRender::QRenderPassFilter(barrier1);
    auto* matchKeyPass1 = new Qt3DRender::QFilterKey();
    matchKeyPass1->setName(QStringLiteral("pass1"));
    matchKeyPass1->setValue(QStringLiteral("COMPUTE_PARTICLES"));
    computeFilter->addMatch(matchKeyPass1);

    auto* dispatch = new Qt3DRender::QDispatchCompute(computeFilter);
    dispatch->setWorkGroupX(numGroupsX); // Должно совпадать с тем, что в computeCommand
    dispatch->setWorkGroupY(1);
    dispatch->setWorkGroupZ(1);

    // --- БАРЬЕР 2: Ждем окончания записи частиц ---
    auto* barrier2 = new Qt3DRender::QMemoryBarrier(clearBuffers);
    barrier2->setWaitOperations(Qt3DRender::QMemoryBarrier::Operations(
        Qt3DRender::QMemoryBarrier::ShaderImageAccess | Qt3DRender::QMemoryBarrier::TextureFetch));

    // --- ВЕТКА 3: Отрисовка геометрии (растет ИЗ второго барьера) ---
    auto* viewport = new Qt3DRender::QViewport(barrier2);
    viewport->setNormalizedRect(QRectF(0.0, 0.0, 1.0, 1.0));

    auto* cameraSelector = new Qt3DRender::QCameraSelector(viewport);
    cameraSelector->setCamera(camera);

    // Отрисовка обычных объектов (Куб)
    auto* standardRenderFilter = new Qt3DRender::QRenderPassFilter(cameraSelector);
    auto* matchKeyStandard     = new Qt3DRender::QFilterKey();
    matchKeyStandard->setName(QStringLiteral("renderingStyle"));
    matchKeyStandard->setValue(QStringLiteral("forward"));
    standardRenderFilter->addMatch(matchKeyStandard);

    // Отрисовка нашей сетки на экран (Pass 2)
    auto* renderFilter  = new Qt3DRender::QRenderPassFilter(cameraSelector);
    auto* matchKeyPass2 = new Qt3DRender::QFilterKey();
    matchKeyPass2->setName(QStringLiteral("pass2"));
    matchKeyPass2->setValue(QStringLiteral("FORWARD_RENDER_SCENE"));
    renderFilter->addMatch(matchKeyPass2);

    view.setActiveFrameGraph(surfaceSelector);
    view.setRootEntity(rootEntity);
    view.show();
    return app.exec();
}