#include "services/settings_service.h"

void SettingsService::saveRegion(MemoryRegion t_region, uint8_t t_programIndex, uint8_t t_potIndex) {
  RegionInfo info = m_handler.calculateRegionInfo(t_region, t_programIndex, t_potIndex);

  uint8_t buffer[info.m_length];
  m_handler.serializeRegion(t_region, m_logicalState, buffer, t_programIndex, t_potIndex);
  m_eeprom.write(info.m_address, buffer, info.m_length);
}

void SettingsService::loadRegion(MemoryRegion t_region, uint8_t t_programIndex, uint8_t t_potIndex) {
  RegionInfo info = m_handler.calculateRegionInfo(t_region, t_programIndex, t_potIndex);

  uint8_t buffer[info.m_length];
  m_eeprom.read(info.m_address, buffer, info.m_length);
  m_handler.deserializeRegion(t_region, m_logicalState, buffer, t_programIndex, t_potIndex);
}

void SettingsService::init() {
  m_eeprom.init();

  loadRegion(MemoryRegion::kLogicalState);
}

void SettingsService::handleEvent(const Event& t_event) {
  if (t_event.m_action == EventAction::kSave) {
    bool isPresetMode = m_logicalState.m_programMode == ProgramMode::kPreset;

    switch (t_event.m_subject) {
      case EventSubject::kBypass:
        m_dirtyFlags |= static_cast<uint8_t>(DirtyBits::kBypass);
        m_lastEditTime = t_event.m_timestamp;
        break;

      case EventSubject::kProgramMode:
        m_dirtyFlags |= static_cast<uint8_t>(DirtyBits::kProgramMode);
        m_lastEditTime = t_event.m_timestamp;
        break;

      case EventSubject::kProgram:
          m_dirtyFlags |= static_cast<uint8_t>(DirtyBits::kProgram);
          m_lastEditTime = t_event.m_timestamp;
        break;

      case EventSubject::kTap:
        if (isPresetMode) {
          m_logicalState.m_presetDirty = true;
        }
        else {
          m_dirtyFlags |= static_cast<uint8_t>(DirtyBits::kTap);
          m_lastEditTime = t_event.m_timestamp;
        }
        break;

      case EventSubject::kTempo:
        if (isPresetMode) {
          m_logicalState.m_presetDirty = true;
        }
        else {
          m_dirtyFlags |= static_cast<uint8_t>(DirtyBits::kTempo);
          m_lastEditTime = t_event.m_timestamp;
        }
        break;

      case EventSubject::kExpr:
        if (isPresetMode) {
          m_logicalState.m_presetDirty = true;
        }
        else {
          m_dirtyFlags |= static_cast<uint8_t>(DirtyBits::kExpr);
          m_lastEditTime = t_event.m_timestamp;
        }
        break;

      case EventSubject::kPot:
        if (isPresetMode) {
          m_logicalState.m_presetDirty = true;
        }
        else {
          m_dirtyFlags |= static_cast<uint8_t>(DirtyBits::kPot);

          switch (static_cast<PotId>(t_event.m_id)) {
            case PotId::kPot0:
              m_dirtyPotFlags |= static_cast<uint8_t>(DirtyPotBits::kPot0);
              break;

            case PotId::kPot1:
              m_dirtyPotFlags |= static_cast<uint8_t>(DirtyPotBits::kPot1);
              break;

            case PotId::kPot2:
              m_dirtyPotFlags |= static_cast<uint8_t>(DirtyPotBits::kPot2);
              break;

            case PotId::kMixPot:
              m_dirtyPotFlags |= static_cast<uint8_t>(DirtyPotBits::kMixPot);
              break;

            default:
              break;
          }

          m_lastEditTime = t_event.m_timestamp;
        }
        break;

      case EventSubject::kGeneral:
        saveRegion(MemoryRegion::kLogicalState);
        break;

      default:
        break;
    }
  }

  if (t_event.m_action == EventAction::kLoad) {
    switch (t_event.m_subject) {
      case EventSubject::kGeneral:
        loadRegion(MemoryRegion::kLogicalState);
        break;

      default:
        break;
    }
  }
}

void SettingsService::update() {
  if (m_dirtyFlags) {
    uint32_t now = m_clock.now();

    if ((now - m_lastEditTime) >= SettingsServiceConstants::c_editTimeout) {
      if (m_dirtyFlags & static_cast<uint8_t>(DirtyBits::kBypass)) {
        saveRegion(MemoryRegion::kBypass);
        m_dirtyFlags &= ~static_cast<uint8_t>(DirtyBits::kBypass);
      }

      if (m_dirtyFlags & static_cast<uint8_t>(DirtyBits::kProgramMode)) {
        saveRegion(MemoryRegion::kProgramMode);
        m_dirtyFlags &= ~static_cast<uint8_t>(DirtyBits::kProgramMode);
      }

      if (m_dirtyFlags & static_cast<uint8_t>(DirtyBits::kProgram)) {
        saveRegion(MemoryRegion::kCurrentProgram);
        m_dirtyFlags &= ~static_cast<uint8_t>(DirtyBits::kProgram);
      }

      if (m_dirtyFlags & static_cast<uint8_t>(DirtyBits::kTap)) {
        saveRegion(MemoryRegion::kTap);
        m_dirtyFlags &= ~static_cast<uint8_t>(DirtyBits::kTap);
      }

      if (m_dirtyFlags & static_cast<uint8_t>(DirtyBits::kTempo)) {
        saveRegion(MemoryRegion::kTempo);
        m_dirtyFlags &= ~static_cast<uint8_t>(DirtyBits::kTempo);
      }

      if (m_dirtyFlags & static_cast<uint8_t>(DirtyBits::kExpr)) {
        saveRegion(MemoryRegion::kExpr, m_logicalState.m_currentProgram);
        m_dirtyFlags &= ~static_cast<uint8_t>(DirtyBits::kExpr);
      }

      if (m_dirtyFlags & static_cast<uint8_t>(DirtyBits::kPot)) {
        if (m_dirtyPotFlags & static_cast<uint8_t>(DirtyPotBits::kPot0)) {
          saveRegion(MemoryRegion::kPot, m_logicalState.m_currentProgram, static_cast<uint8_t>(PotId::kPot0));
          m_dirtyPotFlags &= ~static_cast<uint8_t>(DirtyPotBits::kPot0);
        }

        if (m_dirtyPotFlags & static_cast<uint8_t>(DirtyPotBits::kPot1)) {
          saveRegion(MemoryRegion::kPot, m_logicalState.m_currentProgram, static_cast<uint8_t>(PotId::kPot1));
          m_dirtyPotFlags &= ~static_cast<uint8_t>(DirtyPotBits::kPot1);
        }

        if (m_dirtyPotFlags & static_cast<uint8_t>(DirtyPotBits::kPot2)) {
          saveRegion(MemoryRegion::kPot, m_logicalState.m_currentProgram, static_cast<uint8_t>(PotId::kPot2));
          m_dirtyPotFlags &= ~static_cast<uint8_t>(DirtyPotBits::kPot2);
        }

        if (m_dirtyPotFlags & static_cast<uint8_t>(DirtyPotBits::kMixPot)) {
          saveRegion(MemoryRegion::kPot, m_logicalState.m_currentProgram, static_cast<uint8_t>(PotId::kMixPot));
          m_dirtyPotFlags &= ~static_cast<uint8_t>(DirtyPotBits::kMixPot);
        }

        m_dirtyFlags &= ~static_cast<uint8_t>(DirtyBits::kPot);
      }
    }
  }
}

bool SettingsService::interestedIn(const Event& t_event) const {
  if (t_event.m_domain == EventDomain::kMemory) {
    if (t_event.m_subject != EventSubject::kPreset
        && t_event.m_subject != EventSubject::kPresetBank) {
      return true;
    }
  }

  return false;
}