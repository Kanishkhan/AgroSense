#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// ==== Config ====
#define SENSOR_TASK_QUOTA   2048
#define COMM_TASK_QUOTA     2048
#define STACK_SIZE          2048

// ==== Task Handles ====
TaskHandle_t sensorTaskHandle;
TaskHandle_t commTaskHandle;
TaskHandle_t heapMonitorHandle;

// ==== Memory Tracking ====
size_t sensorMemUsage = 0;
size_t commMemUsage = 0;
SemaphoreHandle_t memMutex;

// ==== Quota-Aware Malloc/Free ====
void* pvPortMallocWithQuota(size_t size, TaskHandle_t taskHandle) {
  xSemaphoreTake(memMutex, portMAX_DELAY);

  void* ptr = NULL;
  if (taskHandle == sensorTaskHandle) {
    if ((sensorMemUsage + size) <= SENSOR_TASK_QUOTA) {
      ptr = pvPortMalloc(size);
      if (ptr) sensorMemUsage += size;
    } else {
      Serial.printf("[SensorTask] ❌ Quota exceeded! Requested: %d, Used: %d/%d\n",
                    size, sensorMemUsage, SENSOR_TASK_QUOTA);
    }
  } else if (taskHandle == commTaskHandle) {
    if ((commMemUsage + size) <= COMM_TASK_QUOTA) {
      ptr = pvPortMalloc(size);
      if (ptr) commMemUsage += size;
    } else {
      Serial.printf("[CommTask] ❌ Quota exceeded! Requested: %d, Used: %d/%d\n",
                    size, commMemUsage, COMM_TASK_QUOTA);
    }
  }

  xSemaphoreGive(memMutex);
  return ptr;
}

void vPortFreeWithTracking(void* ptr, size_t size, TaskHandle_t taskHandle) {
  if (!ptr) return;
  xSemaphoreTake(memMutex, portMAX_DELAY);
  vPortFree(ptr);

  if (taskHandle == sensorTaskHandle) {
    sensorMemUsage -= size;
  } else if (taskHandle == commTaskHandle) {
    commMemUsage -= size;
  }

  xSemaphoreGive(memMutex);
}

// ==== Optional: Simulate Stack Growth ====
void stressStack(int depth) {
  char buffer[128];
  if (depth > 0) {
    stressStack(depth - 1);
  }
}

// ==== Sensor Task ====
void SensorTask(void* pvParams) {
  TaskHandle_t self = xTaskGetCurrentTaskHandle();

  while (1) {
    size_t allocSize = 256 + (rand() % 1024);  // 256–1280 bytes
    Serial.printf("[SensorTask] 🌱 Allocating %d bytes...\n", allocSize);

    void* data = pvPortMallocWithQuota(allocSize, self);
    if (data != NULL) {
      Serial.printf("[SensorTask] ✅ Allocated | Usage: %d / %d\n",
                    sensorMemUsage, SENSOR_TASK_QUOTA);
      stressStack(4 + (rand() % 4)); // Simulate stack use
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      vPortFreeWithTracking(data, allocSize, self);
      Serial.println("[SensorTask] 🧹 Freed memory.");
    }

    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

// ==== Communication Task ====
void CommTask(void* pvParams) {
  TaskHandle_t self = xTaskGetCurrentTaskHandle();

  while (1) {
    size_t allocSize = 512 + (rand() % 2048); // 512–2560 bytes
    Serial.printf("[CommTask] 📡 Allocating %d bytes...\n", allocSize);

    void* packet = pvPortMallocWithQuota(allocSize, self);
    if (packet != NULL) {
      Serial.printf("[CommTask] ✅ Allocated | Usage: %d / %d\n",
                    commMemUsage, COMM_TASK_QUOTA);
      stressStack(6 + (rand() % 3)); // Simulate stack use
      vTaskDelay(800 / portTICK_PERIOD_MS);
      vPortFreeWithTracking(packet, allocSize, self);
      Serial.println("[CommTask] 🧹 Freed memory.");
    }

    vTaskDelay(2500 / portTICK_PERIOD_MS);
  }
}

// ==== Heap Monitor Task ====
void HeapMonitorTask(void* pvParams) {
  while (1) {
    size_t freeHeap = xPortGetFreeHeapSize();
    size_t minHeap = xPortGetMinimumEverFreeHeapSize();
    Serial.printf("[HeapMonitor] 📉 Free Heap: %d bytes | Min Ever: %d bytes\n", freeHeap, minHeap);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
  }
}

// ==== Stack Monitor Task ====
void StackMonitorTask(void* pvParams) {
  while (1) {
    UBaseType_t sensorStack = uxTaskGetStackHighWaterMark(sensorTaskHandle);
    UBaseType_t commStack = uxTaskGetStackHighWaterMark(commTaskHandle);
    UBaseType_t monitorStack = uxTaskGetStackHighWaterMark(heapMonitorHandle);
    Serial.printf("[StackMonitor] 🧠 Stack - Sensor: %d | Comm: %d | HeapMon: %d (words)\n",
                  sensorStack, commStack, monitorStack);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

// ==== Setup ====
void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("🌾 Smart Irrigation Node Starting...");

  memMutex = xSemaphoreCreateMutex();

  xTaskCreate(SensorTask, "SensorTask", STACK_SIZE, NULL, 1, &sensorTaskHandle);
  xTaskCreate(CommTask, "CommTask", STACK_SIZE, NULL, 1, &commTaskHandle);
  xTaskCreate(HeapMonitorTask, "HeapMonitor", STACK_SIZE, NULL, 1, &heapMonitorHandle);
  xTaskCreate(StackMonitorTask, "StackMonitor", STACK_SIZE, NULL, 1, NULL);
}

void loop() {
  // Not used
}
