// Copyright © 2026 Shiomachi Software. All rights reserved.
/**
 * @file Cdc.h
 * @brief USB CDC (仮想COMポート) 通信制御ヘッダ
 */
#pragma once

#include "Common.h"

// =============================================================================
// USB CDC 定義・定数
// =============================================================================

// USB CDC シリアル通信設定
#define CDC_SERIAL_BAUDRATE         115200      // USB CDC シリアル通信ボーレート (bps)
#define CDC_SERIAL_BUFFER_EXTRA     32          // シリアル受信バッファ予備サイズ (バイト)
#define CDC_POLL_DELAY_MS           20          // シリアル未受信時の待機休止時間 (ms)

// CDC タスク設定 (FreeRTOS)
#define CDC_TASK_PRIORITY           1           // USB CDCタスク優先度 (低)
#define CDC_TASK_STACK_SIZE         200         // CDCタスクスタックサイズ (words)

// =============================================================================
// USB CDC シリアル通信制御 (JigAppプロトコル)
// =============================================================================
void CDC_Init(void);
void CDC_Task(void* pvParameters);
