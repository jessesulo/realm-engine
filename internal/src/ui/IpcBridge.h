#pragma once

#include <Windows.h>
#include <cstdint>

// Named pipe IPC bridge. Pipe name comes from BUILD_PIPE_NAME (BuildSecrets.h)
// and is regenerated per-release by build-prod.mjs.
// Handles mutual HMAC-SHA256 auth (Handshake), tile walkability data from
// bot-client, and periodic state/player pushes.
//
// Pipe-delivered feature state is authoritative for unified controls like auto-aim.

DWORD WINAPI IpcBridgeThread(LPVOID lpParam);

// Signal the IpcBridge thread to stop (call before DetourUninitialization).
void IpcBridge_RequestShutdown();

// Emit a synthetic "predicted hit" event from GhostHit's swept-collision
// check to the proxy, piggy-backing on the signed `hotkeyEvent` channel
// (pluginId="ghostHit", action="<ownerObjId>:<bulletId>"). The proxy
// crafts and sends a real PLAYERHIT packet on the client's behalf —
// keeps server-client hit accounting consistent AND is observed by the
// in-process Auto Nexus plugin's PLAYERHIT handling so Auto Nexus gets the
// pre-damage signal it would otherwise miss. Thread-safe: queues
// internally so callers on the game-update thread don't need to know
// the pipe handle. No-op if the pipe is not authenticated.
void IpcBridge_EmitPredictedHit(int ownerObjId, int bulletId);

// ── Tile walkability ──────────────────────────────────────────────────────────
// Populated from bot-client tileUpdate / noWalkInit packets.
// Returns true (walkable/unknown — optimistic) or false (known impassable tile).
bool IpcBridge_IsTileWalkable(float worldX, float worldY);

// ── Tile diagnostic accessors ─────────────────────────────────────────────────
void IpcBridge_GetTileStats(int* outTileCount, int* outNoWalkTypeCount);

struct IpcTileTypeEntry {
    uint16_t typeId;
    int      count;
    bool     noWalk;
};
int IpcBridge_CopyUniqueTypeEntries(IpcTileTypeEntry* buf, int maxCount);

// ── Auth state ────────────────────────────────────────────────────────────────
const char* IpcBridge_GetUserId();
bool        IpcBridge_IsAuthenticated();

// ── Overlay enable (set remotely by admin via developer mode) ─────────────────
// Returns true only when the admin has enabled developer mode for this session.
bool        IpcBridge_IsOverlayEnabled();

// ── Unified feature state accessors ────────────────────────────────────────────
bool        IpcBridge_GetAutoAimEnabled();
int         IpcBridge_GetAutoAimMode();
void        IpcBridge_SetAutoAimEnabled(bool enabled);
void        IpcBridge_SetAutoAimMode(int mode);
int         IpcBridge_GetAutoDodgeMode();
void        IpcBridge_SetAutoDodgeMode(int mode);
float       IpcBridge_GetAutoDodgeHorizonMs();
void        IpcBridge_SetAutoDodgeHorizonMs(float ms);
float       IpcBridge_GetAutoDodgeHitboxPadding();
void        IpcBridge_SetAutoDodgeHitboxPadding(float paddingTiles);
bool        IpcBridge_GetAutoDodgeWallAvoid();
void        IpcBridge_SetAutoDodgeWallAvoid(bool enabled);
bool        IpcBridge_GetAutoAbilityEnabled();
void        IpcBridge_SetAutoAbilityEnabled(bool enabled);
float       IpcBridge_GetAutoAbilityMpPct();
void        IpcBridge_SetAutoAbilityMpPct(float pctZeroTo100);
int         IpcBridge_GetAutoAbilityWizardMode();
void        IpcBridge_SetAutoAbilityWizardMode(int mode);
float       IpcBridge_GetWalkTargetX();
float       IpcBridge_GetWalkTargetY();
bool        IpcBridge_GetWalkTargetActive();
void        IpcBridge_SetWalkTarget(float worldX, float worldY, bool active);
bool        IpcBridge_GetCameraZoomActive();
float       IpcBridge_GetCameraZoomValue();
void        IpcBridge_SetCameraZoom(bool active, float zoom);
bool        IpcBridge_GetCameraAngleActive();
int         IpcBridge_GetCameraAngleValue();
void        IpcBridge_SetCameraAngle(bool active, int angle);
bool        IpcBridge_GetCameraCenteringActive();
bool        IpcBridge_GetCameraCentered();
void        IpcBridge_SetCameraCentering(bool active, bool centered);
bool        IpcBridge_GetSkinOverrideEnabled();
int         IpcBridge_GetSkinOverrideId();
void        IpcBridge_SetSkinOverride(bool enabled, int skinId);
int32_t     IpcBridge_GetClientDefense();
int32_t     IpcBridge_GetClientClassType();

// Apply latest feature values received via setFeature commands.
// Call from the render thread once per frame.
void        IpcBridge_ApplyFeatureOverrides();
