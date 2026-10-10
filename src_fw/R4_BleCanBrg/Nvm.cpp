// Copyright © 2026 Shiomachi Software. All rights reserved.
/**
 * @file Nvm.cpp
 * @brief 不揮発性メモリ (EEPROM) 設定管理実装
 */
#include "Nvm.h"
#include <EEPROM.h>

ST_CAN_CONFIG g_stCanConfig; // CAN通信設定データ (RAM上の実体)

// =============================================================================
// 不揮発設定の読み込み
// =============================================================================

/**
 * @brief 不揮発設定(EEPROM)の初期化を行う。
 *        マジック番号またはチェックサムが不正な場合は工場出荷時設定に初期化する。
 * @param なし
 * @return なし
 */
void NVM_Init(void) {
    EEPROM.get(EEPROM_ADDR_CAN_CONFIG, g_stCanConfig);
    uint16_t expected = CMN_CalcChecksum(&g_stCanConfig, sizeof(g_stCanConfig) - sizeof(uint16_t)); // チェックサム期待値
    if (g_stCanConfig.magic != EEPROM_MAGIC || g_stCanConfig.checksum != expected) {
        NVM_ResetDefault();
    }
}

// =============================================================================
// 工場出荷時設定へ初期化 (スレッドセーフ)
// =============================================================================

/**
 * @brief CAN設定を工場出荷時のデフォルト値に初期化し、EEPROMに保存する。
 * @param なし
 * @return なし
 */
void NVM_ResetDefault(void) {
    ST_CAN_CONFIG stLocalCfg; // デフォルト設定データ一時領域
    memset(&stLocalCfg, 0, sizeof(stLocalCfg));
    stLocalCfg.magic = EEPROM_MAGIC;
    stLocalCfg.bitRate = CAN_DEFAULT_BITRATE;
    stLocalCfg.sendId = CAN_DEFAULT_SEND_ID;
    stLocalCfg.idType = CAN_DEFAULT_ID_TYPE;
    stLocalCfg.checksum = CMN_CalcChecksum(&stLocalCfg, sizeof(ST_CAN_CONFIG) - sizeof(uint16_t));

    taskENTER_CRITICAL();
    memcpy(&g_stCanConfig, &stLocalCfg, sizeof(ST_CAN_CONFIG));
    taskEXIT_CRITICAL();

    EEPROM.put(EEPROM_ADDR_CAN_CONFIG, stLocalCfg);
}

// =============================================================================
// 現在の設定を安全に取得 (スレッドセーフ)
// =============================================================================

/**
 * @brief 現在のCAN設定をスレッドセーフに取得する。
 * @param[out] pstOut 取得した設定の格納先バッファへのポインタ
 * @return なし
 */
void NVM_GetCanConfig(ST_CAN_CONFIG* pstOut) {
    if (pstOut == NULL) return;
    taskENTER_CRITICAL();
    memcpy(pstOut, &g_stCanConfig, sizeof(ST_CAN_CONFIG));
    taskEXIT_CRITICAL();
}

// =============================================================================
// 不揮発設定の保存 (スレッドセーフ)
// =============================================================================

/**
 * @brief CAN設定をスレッドセーフにRAMおよびEEPROMへ保存する。
 * @param[in] pstCfg 保存するCAN設定へのポインタ
 * @return なし
 */
void NVM_SaveCanConfig(const ST_CAN_CONFIG* pstCfg) {
    if (pstCfg == NULL) return;
    ST_CAN_CONFIG stLocalCfg; // 保存用設定データ一時領域
    memcpy(&stLocalCfg, pstCfg, sizeof(ST_CAN_CONFIG));
    stLocalCfg.magic = EEPROM_MAGIC;
    stLocalCfg.checksum = CMN_CalcChecksum(&stLocalCfg, sizeof(ST_CAN_CONFIG) - sizeof(uint16_t));

    taskENTER_CRITICAL();
    memcpy(&g_stCanConfig, &stLocalCfg, sizeof(ST_CAN_CONFIG));
    taskEXIT_CRITICAL();

    EEPROM.put(EEPROM_ADDR_CAN_CONFIG, stLocalCfg);
}
