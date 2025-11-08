"""Select platform for Sand Garden."""
from __future__ import annotations

import logging
from typing import Any

from homeassistant.components.select import SelectEntity
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity_platform import AddEntitiesCallback
from homeassistant.helpers.update_coordinator import CoordinatorEntity

from .const import API_LED_EFFECT, API_PATTERN, DOMAIN, LED_EFFECTS, PATTERNS
from .coordinator import SandGardenCoordinator

_LOGGER = logging.getLogger(__name__)


async def async_setup_entry(
    hass: HomeAssistant,
    entry: ConfigEntry,
    async_add_entities: AddEntitiesCallback,
) -> None:
    """Set up Sand Garden select entities."""
    coordinator: SandGardenCoordinator = hass.data[DOMAIN][entry.entry_id]

    async_add_entities(
        [
            SandGardenPatternSelect(coordinator, entry),
            SandGardenLEDEffectSelect(coordinator, entry),
        ]
    )


class SandGardenPatternSelect(CoordinatorEntity, SelectEntity):
    """Representation of Sand Garden pattern selector."""

    _attr_has_entity_name = True
    _attr_name = "Pattern"
    _attr_icon = "mdi:drawing"

    def __init__(
        self, coordinator: SandGardenCoordinator, entry: ConfigEntry
    ) -> None:
        """Initialize the select entity."""
        super().__init__(coordinator)
        self._attr_unique_id = f"{entry.entry_id}_pattern"
        self._attr_options = list(PATTERNS.values())
        self._attr_device_info = {
            "identifiers": {(DOMAIN, entry.entry_id)},
            "name": "Sand Garden",
            "manufacturer": "CrunchLabs",
            "model": "Sand Garden",
        }

    @property
    def current_option(self) -> str | None:
        """Return the current pattern."""
        pattern_id = self.coordinator.data.get("pattern")
        if pattern_id is not None:
            return PATTERNS.get(pattern_id)
        return None

    async def async_select_option(self, option: str) -> None:
        """Change the selected pattern."""
        # Find pattern ID by name
        pattern_id = next(
            (pid for pid, name in PATTERNS.items() if name == option), None
        )
        if pattern_id is not None:
            await self.coordinator.async_send_command(API_PATTERN, {"value": pattern_id})
            # Optimistically update the value
            self.coordinator.data["pattern"] = pattern_id
            self.async_write_ha_state()


class SandGardenLEDEffectSelect(CoordinatorEntity, SelectEntity):
    """Representation of Sand Garden LED effect selector."""

    _attr_has_entity_name = True
    _attr_name = "LED Effect"
    _attr_icon = "mdi:led-strip-variant"

    def __init__(
        self, coordinator: SandGardenCoordinator, entry: ConfigEntry
    ) -> None:
        """Initialize the select entity."""
        super().__init__(coordinator)
        self._attr_unique_id = f"{entry.entry_id}_led_effect"
        self._attr_options = list(LED_EFFECTS.values())
        self._attr_device_info = {
            "identifiers": {(DOMAIN, entry.entry_id)},
            "name": "Sand Garden",
            "manufacturer": "CrunchLabs",
            "model": "Sand Garden",
        }

    @property
    def current_option(self) -> str | None:
        """Return the current LED effect."""
        effect_id = self.coordinator.data.get("ledEffect")
        if effect_id is not None:
            return LED_EFFECTS.get(effect_id)
        return None

    async def async_select_option(self, option: str) -> None:
        """Change the selected LED effect."""
        # Find effect ID by name
        effect_id = next(
            (eid for eid, name in LED_EFFECTS.items() if name == option), None
        )
        if effect_id is not None:
            await self.coordinator.async_send_command(API_LED_EFFECT, {"value": effect_id})
            # Optimistically update the value
            self.coordinator.data["ledEffect"] = effect_id
            self.async_write_ha_state()
