#include "Common/Logger/Logger.h"
#include <QTest>
#include <qcontainerfwd.h>
#include <qdebug.h>
#include <qdir.h>
#include <qiodevicebase.h>
#include <qlogging.h>
#include <qobject.h>
#include <qtestcase.h>
#include <qtmetamacros.h>

class TestLoggerInit : public QObject {
  Q_OBJECT
private slots:

  void initTestCase() {}
  void testFileCreation() {
    QString testLogName = "test_output.log";
    QFile::remove(testLogName);
    QSpace::Common::Logger::init(testLogName);
    qInfo(LogCommon) << "Test Message";
    QVERIFY(QFile::exists(testLogName));
    QFile file(testLogName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString file_text = file.readAll();
    QVERIFY(file_text.contains("[INF]"));
    QVERIFY(file_text.contains("[QSpace.Common]"));
    QVERIFY(file_text.contains("Test Message"));
  }
  void cleanupTestCase() { QFile::remove("test_output.log"); }
};

QTEST_GUILESS_MAIN(TestLoggerInit)
#include "TestLoggerInit.moc"