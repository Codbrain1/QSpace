// pch.h
#pragma once

#ifdef __cplusplus
// Стандартная библиотека (доступна везде)
#include <memory>
#include <string>
#include <vector>


// Qt Core & Widgets (они у вас подключены почти во всех модулях, включая Logger)
#include <QList>
#include <QMap>
#include <QPair>
#include <QSharedPointer>
#include <QString>
#include <QUuid>


// Проверяем: если компилятор «видит» VTK в путях инклудов текущего модуля,
// то добавляем его в PCH. Если это изолированный Logger — просто пропускаем.
#if __has_include(<vtkSmartPointer.h>)
#include <vtkDataSet.h>
#include <vtkObject.h>
#include <vtkSmartPointer.h>

#endif

#endif