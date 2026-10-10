// Copyright © 2026 Shiomachi Software. All rights reserved.
/**
 * @file Nvm.h
 * @brief 不揮発性メモリ (EEPROM) 設定管理ヘッダ
 */
#pragma once

#include "Common.h"
#include "Can.h"

// =============================================================================
// 不揮発設定 (EEPROM) 定義・定数
// =============================================================================
#define EEPROM_MAGIC                0x52344342  // "R4CB" (設定データ識別用マジック番号)
#define EEPROM_ADDR_CAN_CONFIG      0           // CAN設定データ格納先 EEPROM アドレスオフセット

// =============================================================================
// 共有変数宣言
// =============================================================================
extern ST_CAN_CONFIG g_stCanConfig; // CAN通信設定データ (RAM上の共有実体)

// =============================================================================
// 不揮発設定制御関数
// =============================================================================
void NVM_Init(void);
void NVM_GetCanConfig(ST_CAN_CONFIG* pstOut);
void NVM_SaveCanConfig(const ST_CAN_CONFIG* pstCfg);
void NVM_ResetDefault(void);
