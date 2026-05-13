/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org
 *
 * Test: NVIDIA NVML library loading and GPU data retrieval
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include <QtTest>
#include <QCoreApplication>
#include <dlfcn.h>

/**
 * Tests for the NvmlGpu module — NVML library loading and GPU data retrieval.
 *
 * These tests verify:
 * 1. Dynamic loading of libnvidia-ml.so
 * 2. Symbol loading (dlsym)
 * 3. NVML initialization (nvmlInit)
 * 4. Device count retrieval
 * 5. GPU data retrieval (utilization, temperature, memory)
 *
 * Tests are automatically skipped if NVIDIA GPU with NVML is not available.
 */
class TestNvml : public QObject
{
    Q_OBJECT

private slots:
    // Test: check NVML library availability
    void testNvmlLibraryAvailable_data();
    void testNvmlLibraryAvailable();

    // Test: NVML symbol loading
    void testNvmlSymbolsLoaded_data();
    void testNvmlSymbolsLoaded();

    // Test: NVML initialization
    void testNvmlInit_data();
    void testNvmlInit();

    // Test: GPU device count
    void testNvmlDeviceCount_data();
    void testNvmlDeviceCount();

    // Test: GPU data retrieval
    void testNvmlGetDeviceData_data();
    void testNvmlGetDeviceData();

    // Test: GPU name retrieval
    void testNvmlGetDeviceName_data();
    void testNvmlGetDeviceName();

    // Test: NVML shutdown
    void testNvmlShutdown_data();
    void testNvmlShutdown();

private:
    // Helper: check if NVML library is available
    bool isNvmlAvailable();

    // Helper: load library and symbols
    bool loadNvml(void*& dlHandle, void**& symbols);

    // Structure to hold loaded symbols
    struct NvmlSymbols {
        void* nvmlInit;
        void* nvmlShutdown;
        void* nvmlDeviceGetCount;
        void* nvmlDeviceGetHandleByIndex;
        void* nvmlDeviceGetName;
        void* nvmlDeviceGetUtilizationRates;
        void* nvmlDeviceGetTemperature;
        void* nvmlDeviceGetMemoryInfo;
    };
};

bool TestNvml::isNvmlAvailable()
{
    void* handle = dlopen("libnvidia-ml.so", RTLD_LAZY);
    if (!handle) {
        handle = dlopen("libnvidia-ml.so.1", RTLD_LAZY);
    }
    if (handle) {
        dlclose(handle);
        return true;
    }
    return false;
}

void TestNvml::testNvmlLibraryAvailable_data()
{
    QTest::addColumn<bool>("available");

    bool available = isNvmlAvailable();
    QTest::newRow("nvml_available") << available;
}

void TestNvml::testNvmlLibraryAvailable()
{
    QFETCH(bool, available);

    if (!available) {
        QSKIP("NVIDIA NVML library (libnvidia-ml.so) not found. "
              "Install NVIDIA drivers to run this test.", SkipAll);
    }

    // Library is available, continue testing
    QVERIFY(true);
}

void TestNvml::testNvmlSymbolsLoaded_data()
{
    QTest::addColumn<bool>("available");
    QTest::newRow("nvml_available") << isNvmlAvailable();
}

void TestNvml::testNvmlSymbolsLoaded()
{
    bool available = isNvmlAvailable();
    if (!available) {
        QSKIP("NVIDIA NVML library not found.", SkipAll);
    }

    void* dlHandle = dlopen("libnvidia-ml.so", RTLD_LAZY);
    if (!dlHandle) {
        dlHandle = dlopen("libnvidia-ml.so.1", RTLD_LAZY);
    }
    QVERIFY2(dlHandle != nullptr, "Failed to load NVML library");

    // Check that key symbols are loaded
    QVERIFY(dlsym(dlHandle, "nvmlInit") != nullptr);
    QVERIFY(dlsym(dlHandle, "nvmlShutdown") != nullptr);
    QVERIFY(dlsym(dlHandle, "nvmlDeviceGetCount") != nullptr);
    QVERIFY(dlsym(dlHandle, "nvmlDeviceGetHandleByIndex") != nullptr);
    QVERIFY(dlsym(dlHandle, "nvmlDeviceGetName") != nullptr);
    QVERIFY(dlsym(dlHandle, "nvmlDeviceGetUtilizationRates") != nullptr);
    QVERIFY(dlsym(dlHandle, "nvmlDeviceGetTemperature") != nullptr);
    QVERIFY(dlsym(dlHandle, "nvmlDeviceGetMemoryInfo") != nullptr);

    dlclose(dlHandle);
}

void TestNvml::testNvmlInit_data()
{
    QTest::addColumn<bool>("available");
    QTest::newRow("nvml_available") << isNvmlAvailable();
}

void TestNvml::testNvmlInit()
{
    bool available = isNvmlAvailable();
    if (!available) {
        QSKIP("NVIDIA NVML library not found.", SkipAll);
    }

    void* dlHandle = dlopen("libnvidia-ml.so", RTLD_LAZY);
    if (!dlHandle) {
        dlHandle = dlopen("libnvidia-ml.so.1", RTLD_LAZY);
    }
    QVERIFY2(dlHandle != nullptr, "Failed to load NVML library");

    typedef int (*nvmlInitFn)();
    auto nvmlInit = reinterpret_cast<nvmlInitFn>(dlsym(dlHandle, "nvmlInit"));
    QVERIFY(nvmlInit != nullptr);

    // nvmlInit returns 0 on success
    int result = nvmlInit();
    QCOMPARE(result, 0);

    // Must call shutdown
    typedef int (*nvmlShutdownFn)();
    auto nvmlShutdown = reinterpret_cast<nvmlShutdownFn>(dlsym(dlHandle, "nvmlShutdown"));
    QVERIFY(nvmlShutdown != nullptr);
    nvmlShutdown();

    dlclose(dlHandle);
}

void TestNvml::testNvmlDeviceCount_data()
{
    QTest::addColumn<bool>("available");
    QTest::newRow("nvml_available") << isNvmlAvailable();
}

void TestNvml::testNvmlDeviceCount()
{
    bool available = isNvmlAvailable();
    if (!available) {
        QSKIP("NVIDIA NVML library not found.", SkipAll);
    }

    void* dlHandle = dlopen("libnvidia-ml.so", RTLD_LAZY);
    if (!dlHandle) {
        dlHandle = dlopen("libnvidia-ml.so.1", RTLD_LAZY);
    }
    QVERIFY2(dlHandle != nullptr, "Failed to load NVML library");

    typedef int (*nvmlInitFn)();
    typedef int (*nvmlDeviceGetCountFn)(unsigned int*);
    typedef int (*nvmlShutdownFn)();

    auto nvmlInit = reinterpret_cast<nvmlInitFn>(dlsym(dlHandle, "nvmlInit"));
    auto nvmlDeviceGetCount = reinterpret_cast<nvmlDeviceGetCountFn>(dlsym(dlHandle, "nvmlDeviceGetCount"));
    auto nvmlShutdown = reinterpret_cast<nvmlShutdownFn>(dlsym(dlHandle, "nvmlShutdown"));

    QVERIFY(nvmlInit != nullptr);
    QVERIFY(nvmlDeviceGetCount != nullptr);
    QVERIFY(nvmlShutdown != nullptr);

    nvmlInit();

    unsigned int deviceCount = 0;
    int result = nvmlDeviceGetCount(&deviceCount);
    QCOMPARE(result, 0);

    // There should be at least one device
    QVERIFY(deviceCount > 0);

    nvmlShutdown();
    dlclose(dlHandle);
}

void TestNvml::testNvmlGetDeviceData_data()
{
    QTest::addColumn<bool>("available");
    QTest::newRow("nvml_available") << isNvmlAvailable();
}

void TestNvml::testNvmlGetDeviceData()
{
    bool available = isNvmlAvailable();
    if (!available) {
        QSKIP("NVIDIA NVML library not found.", SkipAll);
    }

    void* dlHandle = dlopen("libnvidia-ml.so", RTLD_LAZY);
    if (!dlHandle) {
        dlHandle = dlopen("libnvidia-ml.so.1", RTLD_LAZY);
    }
    QVERIFY2(dlHandle != nullptr, "Failed to load NVML library");

    typedef int (*nvmlInitFn)();
    typedef int (*nvmlShutdownFn)();
    typedef int (*nvmlDeviceGetCountFn)(unsigned int*);
    typedef int (*nvmlDeviceGetHandleByIndexFn)(unsigned int, void**);
    typedef int (*nvmlDeviceGetNameFn)(void*, char*, unsigned int);
    typedef int (*nvmlDeviceGetUtilizationRatesFn)(void*, void*);
    typedef int (*nvmlDeviceGetTemperatureFn)(void*, unsigned int, unsigned int*);
    typedef int (*nvmlDeviceGetMemoryInfoFn)(void*, void*);

    auto nvmlInit = reinterpret_cast<nvmlInitFn>(dlsym(dlHandle, "nvmlInit"));
    auto nvmlShutdown = reinterpret_cast<nvmlShutdownFn>(dlsym(dlHandle, "nvmlShutdown"));
    auto nvmlDeviceGetCount = reinterpret_cast<nvmlDeviceGetCountFn>(dlsym(dlHandle, "nvmlDeviceGetCount"));
    auto nvmlDeviceGetHandleByIndex = reinterpret_cast<nvmlDeviceGetHandleByIndexFn>(dlsym(dlHandle, "nvmlDeviceGetHandleByIndex"));
    auto nvmlDeviceGetName = reinterpret_cast<nvmlDeviceGetNameFn>(dlsym(dlHandle, "nvmlDeviceGetName"));
    auto nvmlDeviceGetUtilizationRates = reinterpret_cast<nvmlDeviceGetUtilizationRatesFn>(dlsym(dlHandle, "nvmlDeviceGetUtilizationRates"));
    auto nvmlDeviceGetTemperature = reinterpret_cast<nvmlDeviceGetTemperatureFn>(dlsym(dlHandle, "nvmlDeviceGetTemperature"));
    auto nvmlDeviceGetMemoryInfo = reinterpret_cast<nvmlDeviceGetMemoryInfoFn>(dlsym(dlHandle, "nvmlDeviceGetMemoryInfo"));

    QVERIFY(nvmlInit != nullptr);
    QVERIFY(nvmlDeviceGetCount != nullptr);
    QVERIFY(nvmlDeviceGetHandleByIndex != nullptr);
    QVERIFY(nvmlDeviceGetName != nullptr);
    QVERIFY(nvmlDeviceGetUtilizationRates != nullptr);
    QVERIFY(nvmlDeviceGetTemperature != nullptr);
    QVERIFY(nvmlDeviceGetMemoryInfo != nullptr);

    nvmlInit();

    unsigned int deviceCount = 0;
    nvmlDeviceGetCount(&deviceCount);

    // Test first device (index 0)
    void* device = nullptr;
    int result = nvmlDeviceGetHandleByIndex(0, &device);
    QCOMPARE(result, 0);
    QVERIFY(device != nullptr);

    // Get device name
    char name[64] = {0};
    result = nvmlDeviceGetName(device, name, sizeof(name));
    QCOMPARE(result, 0);
    QVERIFY(strlen(name) > 0);
    QVERIFY(!QString(name).isEmpty());

    // Get utilization rates
    struct { unsigned int gpu, memory; } util;
    result = nvmlDeviceGetUtilizationRates(device, &util);
    QCOMPARE(result, 0);
    QVERIFY(util.gpu <= 100);
    QVERIFY(util.memory <= 100);

    // Get temperature
    unsigned int temp = 0;
    result = nvmlDeviceGetTemperature(device, 0, &temp);
    QCOMPARE(result, 0);
    QVERIFY(temp > 0); // Temperature should be greater than 0

    // Get memory info
    struct { unsigned long long total, free, used; } mem;
    result = nvmlDeviceGetMemoryInfo(device, &mem);
    QCOMPARE(result, 0);
    QVERIFY(mem.total > 0);
    QVERIFY(mem.used <= mem.total);

    nvmlShutdown();
    dlclose(dlHandle);
}

void TestNvml::testNvmlGetDeviceName_data()
{
    QTest::addColumn<bool>("available");
    QTest::newRow("nvml_available") << isNvmlAvailable();
}

void TestNvml::testNvmlGetDeviceName()
{
    bool available = isNvmlAvailable();
    if (!available) {
        QSKIP("NVIDIA NVML library not found.", SkipAll);
    }

    void* dlHandle = dlopen("libnvidia-ml.so", RTLD_LAZY);
    if (!dlHandle) {
        dlHandle = dlopen("libnvidia-ml.so.1", RTLD_LAZY);
    }
    QVERIFY2(dlHandle != nullptr, "Failed to load NVML library");

    typedef int (*nvmlInitFn)();
    typedef int (*nvmlShutdownFn)();
    typedef int (*nvmlDeviceGetHandleByIndexFn)(unsigned int, void**);
    typedef int (*nvmlDeviceGetNameFn)(void*, char*, unsigned int);

    auto nvmlInit = reinterpret_cast<nvmlInitFn>(dlsym(dlHandle, "nvmlInit"));
    auto nvmlShutdown = reinterpret_cast<nvmlShutdownFn>(dlsym(dlHandle, "nvmlShutdown"));
    auto nvmlDeviceGetHandleByIndex = reinterpret_cast<nvmlDeviceGetHandleByIndexFn>(dlsym(dlHandle, "nvmlDeviceGetHandleByIndex"));
    auto nvmlDeviceGetName = reinterpret_cast<nvmlDeviceGetNameFn>(dlsym(dlHandle, "nvmlDeviceGetName"));

    nvmlInit();

    void* device = nullptr;
    int result = nvmlDeviceGetHandleByIndex(0, &device);
    QCOMPARE(result, 0);

    char name[64] = {0};
    result = nvmlDeviceGetName(device, name, sizeof(name));
    QCOMPARE(result, 0);

    // Name should be non-empty and contain the GPU name
    QVERIFY(strlen(name) > 0);
    QString gpuName = QString(name);
    QVERIFY(!gpuName.isEmpty());
    qDebug() << "GPU Name:" << gpuName;

    nvmlShutdown();
    dlclose(dlHandle);
}

void TestNvml::testNvmlShutdown_data()
{
    QTest::addColumn<bool>("available");
    QTest::newRow("nvml_available") << isNvmlAvailable();
}

void TestNvml::testNvmlShutdown()
{
    bool available = isNvmlAvailable();
    if (!available) {
        QSKIP("NVIDIA NVML library not found.", SkipAll);
    }

    void* dlHandle = dlopen("libnvidia-ml.so", RTLD_LAZY);
    if (!dlHandle) {
        dlHandle = dlopen("libnvidia-ml.so.1", RTLD_LAZY);
    }
    QVERIFY2(dlHandle != nullptr, "Failed to load NVML library");

    typedef int (*nvmlInitFn)();
    typedef int (*nvmlShutdownFn)();

    auto nvmlInit = reinterpret_cast<nvmlInitFn>(dlsym(dlHandle, "nvmlInit"));
    auto nvmlShutdown = reinterpret_cast<nvmlShutdownFn>(dlsym(dlHandle, "nvmlShutdown"));

    QVERIFY(nvmlInit != nullptr);
    QVERIFY(nvmlShutdown != nullptr);

    nvmlInit();

    // Shutdown should complete successfully
    int result = nvmlShutdown();
    QCOMPARE(result, 0);

    dlclose(dlHandle);
}

QTEST_MAIN(TestNvml)

#include "test_nvml.moc"
