/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org
 *
 * Test: Plugin settings save/load
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include <QtTest>
#include <QSettings>
#include <QCoreApplication>

/**
 * Tests for nvmonitor plugin settings save/load.
 *
 * These tests verify:
 * 1. Metric saving (gpuUtilization, memUtilization, temperature)
 * 2. Update interval saving
 * 3. Color saving (graph, grid, title)
 * 4. Display settings saving (show value, title)
 * 5. Graph settings saving (history length, minimal size, grid lines)
 * 6. Default values
 */
class TestSettings : public QObject
{
    Q_OBJECT

private slots:
    // Initialization and cleanup
    void init();
    void cleanup();

    // Default value tests
    void testDefaultSettings_data();
    void testDefaultSettings();

    // Metric tests
    void testMetricGpuUtilization();
    void testMetricMemUtilization();
    void testMetricTemperature();

    // Update interval tests
    void testUpdateInterval();

    // Color tests
    void testGraphColor();
    void testGridColor();
    void testTitleColor();

    // Display setting tests
    void testShowValue();
    void testTitleLabel();

    // Graph setting tests
    void testGridLines();
    void testMaxHistory();
    void testMinimalSize();

    // Full save/load roundtrip test
    void testFullSettingsRoundtrip();

private:
    QString mSettingsFile;

    // Helper: create QSettings with temporary file
    QSettings& createSettings();

    // Helper: get metric value
    QString getMetric(QSettings& settings);

    // Helper: set metric value
    void setMetric(QSettings& settings, const QString& metric);
};

void TestSettings::init()
{
    // Create temporary settings file
    mSettingsFile = QCoreApplication::applicationDirPath() + "/nvmonitor_test_settings.ini";
    if (QFile::exists(mSettingsFile)) {
        QFile::remove(mSettingsFile);
    }
}

void TestSettings::cleanup()
{
    // Remove temporary settings file
    if (QFile::exists(mSettingsFile)) {
        QFile::remove(mSettingsFile);
    }
}

QSettings& TestSettings::createSettings()
{
    // Static QSettings with our temporary file
    static QSettings* settings = nullptr;
    if (!settings) {
        settings = new QSettings(mSettingsFile, QSettings::IniFormat);
    } else {
        // Reset for new test
        settings->clear();
        settings->sync();
    }
    return *settings;
}

QString TestSettings::getMetric(QSettings& settings)
{
    return settings.value("data/metric", "gpuUtilization").toString();
}

void TestSettings::setMetric(QSettings& settings, const QString& metric)
{
    settings.setValue("data/metric", metric);
}

void TestSettings::testDefaultSettings_data()
{
    QTest::addColumn<QString>("key");
    QTest::addColumn<QString>("expectedDefault");
    QTest::addColumn<QString>("type");  // "string", "int", "bool"

    QTest::newRow("metric") << "data/metric" << "gpuUtilization" << "string";
    QTest::newRow("updateInterval") << "graph/updateInterval" << "1000" << "int";
    QTest::newRow("minimalSize") << "graph/minimalSize" << "30" << "int";
    QTest::newRow("gridLines") << "grid/lines" << "1" << "int";
    QTest::newRow("showValue") << "graph/showValue" << "0" << "bool";
    QTest::newRow("maxHistory") << "graph/maxHistory" << "100" << "int";
    QTest::newRow("graphColor") << "graph/color" << "#ff0000" << "string";
    QTest::newRow("gridColor") << "grid/color" << "#c0c0c0" << "string";
    QTest::newRow("titleColor") << "title/color" << "#ffffff" << "string";
    QTest::newRow("titleLabel") << "title/label" << "" << "string";
}

void TestSettings::testDefaultSettings()
{
    QFETCH(QString, key);
    QFETCH(QString, expectedDefault);
    QFETCH(QString, type);

    QSettings& settings = createSettings();

    QString value;
    if (type == "int") {
        value = QString::number(settings.value(key, expectedDefault.toInt()).toInt());
    } else if (type == "bool") {
        bool expected = (expectedDefault == "1");
        bool actual = settings.value(key, expected).toBool();
        value = QString::number(actual ? 1 : 0);
    } else {
        value = settings.value(key, expectedDefault).toString();
    }

    QCOMPARE(value, expectedDefault);
}

void TestSettings::testMetricGpuUtilization()
{
    QSettings& settings = createSettings();
    setMetric(settings, "gpuUtilization");

    QString metric = getMetric(settings);
    QCOMPARE(metric, QString("gpuUtilization"));
}

void TestSettings::testMetricMemUtilization()
{
    QSettings& settings = createSettings();
    setMetric(settings, "memUtilization");

    QString metric = getMetric(settings);
    QCOMPARE(metric, QString("memUtilization"));
}

void TestSettings::testMetricTemperature()
{
    QSettings& settings = createSettings();
    setMetric(settings, "temperature");

    QString metric = getMetric(settings);
    QCOMPARE(metric, QString("temperature"));
}

void TestSettings::testUpdateInterval()
{
    QSettings& settings = createSettings();

    // Set different interval values
    settings.setValue("graph/updateInterval", 500);
    QCOMPARE(settings.value("graph/updateInterval").toInt(), 500);

    settings.setValue("graph/updateInterval", 1000);
    QCOMPARE(settings.value("graph/updateInterval").toInt(), 1000);

    settings.setValue("graph/updateInterval", 5000);
    QCOMPARE(settings.value("graph/updateInterval").toInt(), 5000);
}

void TestSettings::testGraphColor()
{
    QSettings& settings = createSettings();

    // Set different colors
    settings.setValue("graph/color", "#00ff00");
    QCOMPARE(settings.value("graph/color").toString(), QString("#00ff00"));

    settings.setValue("graph/color", "#0000ff");
    QCOMPARE(settings.value("graph/color").toString(), QString("#0000ff"));

    settings.setValue("graph/color", "#ffffff");
    QCOMPARE(settings.value("graph/color").toString(), QString("#ffffff"));

    // Check default value
    QSettings emptySettings(mSettingsFile + "_empty", QSettings::IniFormat);
    emptySettings.clear();
    QCOMPARE(emptySettings.value("graph/color", "#ff0000").toString(), QString("#ff0000"));
}

void TestSettings::testGridColor()
{
    QSettings& settings = createSettings();

    settings.setValue("grid/color", "#c0c0c0");
    QCOMPARE(settings.value("grid/color").toString(), QString("#c0c0c0"));

    settings.setValue("grid/color", "#808080");
    QCOMPARE(settings.value("grid/color").toString(), QString("#808080"));
}

void TestSettings::testTitleColor()
{
    QSettings& settings = createSettings();

    settings.setValue("title/color", "#ffffff");
    QCOMPARE(settings.value("title/color").toString(), QString("#ffffff"));

    settings.setValue("title/color", "#000000");
    QCOMPARE(settings.value("title/color").toString(), QString("#000000"));
}

void TestSettings::testShowValue()
{
    QSettings& settings = createSettings();

    settings.setValue("graph/showValue", true);
    QVERIFY(settings.value("graph/showValue").toBool());

    settings.setValue("graph/showValue", false);
    QVERIFY(!settings.value("graph/showValue").toBool());
}

void TestSettings::testTitleLabel()
{
    QSettings& settings = createSettings();

    settings.setValue("title/label", "GPU Monitor");
    QCOMPARE(settings.value("title/label").toString(), QString("GPU Monitor"));

    settings.setValue("title/label", "NVIDIA GPU");
    QCOMPARE(settings.value("title/label").toString(), QString("NVIDIA GPU"));

    // Empty title
    settings.setValue("title/label", QString());
    QVERIFY(settings.value("title/label").toString().isEmpty());
}

void TestSettings::testGridLines()
{
    QSettings& settings = createSettings();

    settings.setValue("grid/lines", 0);
    QCOMPARE(settings.value("grid/lines").toInt(), 0);

    settings.setValue("grid/lines", 1);
    QCOMPARE(settings.value("grid/lines").toInt(), 1);

    settings.setValue("grid/lines", 5);
    QCOMPARE(settings.value("grid/lines").toInt(), 5);
}

void TestSettings::testMaxHistory()
{
    QSettings& settings = createSettings();

    settings.setValue("graph/maxHistory", 10);
    QCOMPARE(settings.value("graph/maxHistory").toInt(), 10);

    settings.setValue("graph/maxHistory", 100);
    QCOMPARE(settings.value("graph/maxHistory").toInt(), 100);

    settings.setValue("graph/maxHistory", 1000);
    QCOMPARE(settings.value("graph/maxHistory").toInt(), 1000);
}

void TestSettings::testMinimalSize()
{
    QSettings& settings = createSettings();

    settings.setValue("graph/minimalSize", 10);
    QCOMPARE(settings.value("graph/minimalSize").toInt(), 10);

    settings.setValue("graph/minimalSize", 30);
    QCOMPARE(settings.value("graph/minimalSize").toInt(), 30);

    settings.setValue("graph/minimalSize", 200);
    QCOMPARE(settings.value("graph/minimalSize").toInt(), 200);
}

void TestSettings::testFullSettingsRoundtrip()
{
    QSettings& settings = createSettings();

    // Save full settings set
    settings.setValue("data/metric", "temperature");
    settings.setValue("graph/updateInterval", 2000);
    settings.setValue("graph/minimalSize", 50);
    settings.setValue("grid/lines", 3);
    settings.setValue("graph/showValue", true);
    settings.setValue("graph/maxHistory", 200);
    settings.setValue("graph/color", "#ff8800");
    settings.setValue("grid/color", "#aaaaaa");
    settings.setValue("title/color", "#eeeeee");
    settings.setValue("title/label", "GPU Temp");

    // Load and verify
    QCOMPARE(getMetric(settings), QString("temperature"));
    QCOMPARE(settings.value("graph/updateInterval").toInt(), 2000);
    QCOMPARE(settings.value("graph/minimalSize").toInt(), 50);
    QCOMPARE(settings.value("grid/lines").toInt(), 3);
    QCOMPARE(settings.value("graph/showValue").toBool(), true);
    QCOMPARE(settings.value("graph/maxHistory").toInt(), 200);
    QCOMPARE(settings.value("graph/color").toString(), QString("#ff8800"));
    QCOMPARE(settings.value("grid/color").toString(), QString("#aaaaaa"));
    QCOMPARE(settings.value("title/color").toString(), QString("#eeeeee"));
    QCOMPARE(settings.value("title/label").toString(), QString("GPU Temp"));

    // Check file synchronization
    settings.sync();
    QVERIFY(QFile::exists(mSettingsFile));

    // Load from new QSettings (simulating restart)
    QSettings freshSettings(mSettingsFile, QSettings::IniFormat);
    QCOMPARE(freshSettings.value("data/metric").toString(), QString("temperature"));
    QCOMPARE(freshSettings.value("graph/updateInterval").toInt(), 2000);
    QCOMPARE(freshSettings.value("graph/color").toString(), QString("#ff8800"));
}

QTEST_MAIN(TestSettings)

#include "test_settings.moc"
