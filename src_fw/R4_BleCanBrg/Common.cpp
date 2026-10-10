// Copyright © 2026 Shiomachi Software. All rights reserved.
/**
 * @file Common.cpp
 * @brief 共通変数・共通ユーティリティ実装
 */
#include "Common.h"
#include "Can.h"
#include "bsp_api.h"
#include <WDT.h>

// =============================================================================
// 定数定義
// =============================================================================
#define WDT_MASK_RESET_VAL          0         // WDTチェックインマスクリセット値

// =============================================================================
// 静的変数
// =============================================================================
static volatile uint8_t f_wdtCheckInMask = 0; // タスク生存チェックインビットマスク

// =============================================================================
// FreeRTOS 共有リソース実体定義
// =============================================================================
QueueHandle_t g_xQueueBleToCan = NULL; // BLE -> CAN 転送用FreeRTOSキュー
QueueHandle_t g_xQueueCanToBle = NULL; // CAN -> BLE 転送用FreeRTOSキュー

volatile uint32_t g_fwErrorBits = 0; // FWエラービットフラグ

// =============================================================================
// 初期化関数
// =============================================================================

/**
 * @brief 共通リソース(FreeRTOSキュー等)の初期化を行う。
 * @param なし
 * @return なし
 */
void CMN_Init(void) {
    // FreeRTOS キュー生成 (各16要素)
    g_xQueueBleToCan = xQueueCreate(CAN_QUEUE_SIZE, sizeof(ST_CAN_FRAME));
    g_xQueueCanToBle = xQueueCreate(CAN_QUEUE_SIZE, sizeof(ST_CAN_FRAME));
}

/**
 * @brief WDTの初期化と再起動理由判定を行う。
 *        前回の再起動がWDTタイムアウトであった場合はエラービットをセットし、WDTハードウェアを開始する。
 * @param なし
 * @return なし
 */
void CMN_WdtInit(void) {
    // 前回起動がWDTタイムアウトによるリセットかを判定 (RSTSR1.WDTRF または IWDTRF)
    if (R_SYSTEM->RSTSR1 & RSTSR1_WDT_RESET_MASK) {
        CMN_SetErrorBit(CMN_ERR_BIT_WDT_RESET);
        R_SYSTEM->PRCR = PRCR_KEY_UNLOCK_RSTSR; // PRC3 (リセット機能レジスタ書き込み保護) 解除
        R_SYSTEM->RSTSR1 = 0;                  // リセットステータスフラグをクリア
        R_SYSTEM->PRCR = PRCR_KEY_LOCK;         // 再度プロテクト
    }

    f_wdtCheckInMask = WDT_MASK_RESET_VAL;
    WDT.begin(WDT_TIMEOUT_MS);
}

// =============================================================================
// ウォッチドッグタイマ (WDT) 制御
// =============================================================================

/**
 * @brief 各タスクの生存をチェックイン(報告)する。
 * @param[in] taskId チェックインするタスクID (WDT_TASK_ID_*)
 * @return なし
 */
void CMN_WdtCheckIn(uint8_t taskId) {
    taskENTER_CRITICAL();
    f_wdtCheckInMask |= (uint8_t)(1 << taskId);
    taskEXIT_CRITICAL();
}

/**
 * @brief 全タスクが生存しているか検査し、健全であればWDTをリフレッシュ(キック)する。
 *        FreeRTOSタイマーから定期的に呼び出される。
 * @param なし
 * @return なし
 */
void CMN_WdtRefreshIfAllHealthy(void) {
    taskENTER_CRITICAL();
    bool bAllHealthy = (f_wdtCheckInMask == WDT_ALL_TASKS_MASK); // 全タスク健全判定フラグ
    if (bAllHealthy) {
        f_wdtCheckInMask = WDT_MASK_RESET_VAL; // 全タスク生存を確認できたためマスクをリセット
    }
    taskEXIT_CRITICAL();

    if (bAllHealthy) {
        WDT.refresh();
    }
}

// =============================================================================
// エラービット操作 (クリティカルセクションによるアトミック操作)
// =============================================================================

/**
 * @brief FWエラービットをスレッドセーフ(アトミック)にセットする。
 * @param[in] bit セットするエラービットフラグ (CMN_ERR_BIT_*)
 * @return なし
 */
void CMN_SetErrorBit(uint32_t bit) {
    taskENTER_CRITICAL();
    g_fwErrorBits |= bit;
    taskEXIT_CRITICAL();
}

/**
 * @brief 現在のFWエラービットをスレッドセーフに取得する。
 * @param なし
 * @return 現在設定されているFWエラービット
 */
uint32_t CMN_GetErrorBits(void) {
    uint32_t bits; // エラービット一時保持用変数
    taskENTER_CRITICAL();
    bits = g_fwErrorBits;
    taskEXIT_CRITICAL();
    return bits;
}

/**
 * @brief FWエラービットをスレッドセーフに全クリアする。
 * @param なし
 * @return なし
 */
void CMN_ClearErrorBits(void) {
    taskENTER_CRITICAL();
    g_fwErrorBits = 0;
    taskEXIT_CRITICAL();
}

// =============================================================================
// システム・ユーティリティ関数
// =============================================================================

/**
 * @brief マイコンをソフトウェアリセット(再起動)する。
 * @param なし
 * @return なし
 */
void CMN_ResetMcu(void) {
    NVIC_SystemReset();
}

/**
 * @brief 確実に1tick以上休止させる安全なDelay関数。
 * @param[in] ms 遅延時間 (ms)
 * @return なし
 */
void CMN_SafeDelayMs(uint32_t ms) {
    TickType_t ticks = pdMS_TO_TICKS(ms); // 遅延tick数
    vTaskDelay(ticks > 0 ? ticks : CMN_MIN_DELAY_TICKS);
}

/**
 * @brief 指定バッファの単純加算チェックサムを計算する。
 * @param[in] pBuf チェックサム計算対象バッファへのポインタ
 * @param[in] size バッファサイズ(バイト単位)
 * @return 計算された16ビットチェックサム値
 */
uint16_t CMN_CalcChecksum(const void* pBuf, size_t size) {
    const uint8_t* p = (const uint8_t*)pBuf; // バッファ読み込み用ポインタ
    uint16_t sum = 0; // チェックサム加算用変数
    for (size_t i = 0; i < size; i++) { // チェックサム加算ループカウンタ
        sum += p[i];
    }
    return sum;
}
