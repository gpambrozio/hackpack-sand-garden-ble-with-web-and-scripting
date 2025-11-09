"""Select platform for Sand Garden."""
from __future__ import annotations

import logging
from typing import Any

from homeassistant.components.select import SelectEntity
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity_platform import AddEntitiesCallback
from homeassistant.helpers.update_coordinator import CoordinatorEntity

from .const import API_PATTERN, DOMAIN, PATTERNS
from .coordinator import SandGardenCoordinator

_LOGGER = logging.getLogger(__name__)


async def async_setup_entry(
    hass: HomeAssistant,
    entry: ConfigEntry,
    async_add_entities: AddEntitiesCallback,
) -> None:
    """Set up Sand Garden select entities."""
    coordinator: SandGardenCoordinator = hass.data[DOMAIN][entry.entry_id]

    async_add_entities([SandGardenPatternSelect(coordinator, entry)])


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
