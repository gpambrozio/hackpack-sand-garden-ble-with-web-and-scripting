"""Number platform for Sand Garden."""
from __future__ import annotations

import logging
from typing import Any

from homeassistant.components.number import NumberEntity, NumberMode
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity_platform import AddEntitiesCallback
from homeassistant.helpers.update_coordinator import CoordinatorEntity

from .const import API_SPEED, DOMAIN, SPEED_MAX, SPEED_MIN, SPEED_STEP
from .coordinator import SandGardenCoordinator

_LOGGER = logging.getLogger(__name__)


async def async_setup_entry(
    hass: HomeAssistant,
    entry: ConfigEntry,
    async_add_entities: AddEntitiesCallback,
) -> None:
    """Set up Sand Garden number entities."""
    coordinator: SandGardenCoordinator = hass.data[DOMAIN][entry.entry_id]

    async_add_entities([SandGardenSpeedNumber(coordinator, entry)])


class SandGardenSpeedNumber(CoordinatorEntity, NumberEntity):
    """Representation of Sand Garden speed control."""

    _attr_has_entity_name = True
    _attr_name = "Speed Multiplier"
    _attr_icon = "mdi:speedometer"
    _attr_native_min_value = SPEED_MIN
    _attr_native_max_value = SPEED_MAX
    _attr_native_step = SPEED_STEP
    _attr_mode = NumberMode.SLIDER

    def __init__(
        self, coordinator: SandGardenCoordinator, entry: ConfigEntry
    ) -> None:
        """Initialize the number entity."""
        super().__init__(coordinator)
        self._attr_unique_id = f"{entry.entry_id}_speed"
        self._attr_device_info = {
            "identifiers": {(DOMAIN, entry.entry_id)},
            "name": "Sand Garden",
            "manufacturer": "CrunchLabs",
            "model": "Sand Garden",
            "icon": "mdi:palette",
        }

    @property
    def native_value(self) -> float | None:
        """Return the current speed multiplier."""
        return self.coordinator.data.get("speedMultiplier")

    async def async_set_native_value(self, value: float) -> None:
        """Set the speed multiplier."""
        await self.coordinator.async_send_command(API_SPEED, {"value": value})
        # Optimistically update the value
        self.coordinator.data["speedMultiplier"] = value
        self.async_write_ha_state()
