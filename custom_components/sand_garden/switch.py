"""Switch platform for Sand Garden."""
from __future__ import annotations

import logging
from typing import Any

from homeassistant.components.switch import SwitchEntity
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity_platform import AddEntitiesCallback
from homeassistant.helpers.update_coordinator import CoordinatorEntity

from .const import API_MODE, API_RUN, DOMAIN
from .coordinator import SandGardenCoordinator

_LOGGER = logging.getLogger(__name__)


async def async_setup_entry(
    hass: HomeAssistant,
    entry: ConfigEntry,
    async_add_entities: AddEntitiesCallback,
) -> None:
    """Set up Sand Garden switch entities."""
    coordinator: SandGardenCoordinator = hass.data[DOMAIN][entry.entry_id]

    async_add_entities(
        [
            SandGardenAutoModeSwitch(coordinator, entry),
            SandGardenRunningSwitch(coordinator, entry),
        ]
    )


class SandGardenAutoModeSwitch(CoordinatorEntity, SwitchEntity):
    """Representation of Sand Garden auto mode switch."""

    _attr_has_entity_name = True
    _attr_name = "Auto Mode"
    _attr_icon = "mdi:auto-mode"

    def __init__(
        self, coordinator: SandGardenCoordinator, entry: ConfigEntry
    ) -> None:
        """Initialize the switch entity."""
        super().__init__(coordinator)
        self._attr_unique_id = f"{entry.entry_id}_auto_mode"
        self._attr_device_info = {
            "identifiers": {(DOMAIN, entry.entry_id)},
            "name": "Sand Garden",
            "manufacturer": "CrunchLabs",
            "model": "Sand Garden",
            "icon": "mdi:palette",
        }

    @property
    def is_on(self) -> bool | None:
        """Return true if auto mode is enabled."""
        return self.coordinator.data.get("autoMode")

    async def async_turn_on(self, **kwargs: Any) -> None:
        """Turn on auto mode."""
        await self.coordinator.async_send_command(API_MODE, {"value": True})
        # Optimistically update the value
        self.coordinator.data["autoMode"] = True
        self.async_write_ha_state()

    async def async_turn_off(self, **kwargs: Any) -> None:
        """Turn off auto mode."""
        await self.coordinator.async_send_command(API_MODE, {"value": False})
        # Optimistically update the value
        self.coordinator.data["autoMode"] = False
        self.async_write_ha_state()


class SandGardenRunningSwitch(CoordinatorEntity, SwitchEntity):
    """Representation of Sand Garden running state switch."""

    _attr_has_entity_name = True
    _attr_name = "Running"
    _attr_icon = "mdi:play-pause"

    def __init__(
        self, coordinator: SandGardenCoordinator, entry: ConfigEntry
    ) -> None:
        """Initialize the switch entity."""
        super().__init__(coordinator)
        self._attr_unique_id = f"{entry.entry_id}_running"
        self._attr_device_info = {
            "identifiers": {(DOMAIN, entry.entry_id)},
            "name": "Sand Garden",
            "manufacturer": "CrunchLabs",
            "model": "Sand Garden",
            "icon": "mdi:palette",
        }

    @property
    def is_on(self) -> bool | None:
        """Return true if pattern is running."""
        return self.coordinator.data.get("running")

    async def async_turn_on(self, **kwargs: Any) -> None:
        """Start pattern execution."""
        await self.coordinator.async_send_command(API_RUN, {"value": True})
        # Optimistically update the value
        self.coordinator.data["running"] = True
        self.async_write_ha_state()

    async def async_turn_off(self, **kwargs: Any) -> None:
        """Stop pattern execution."""
        await self.coordinator.async_send_command(API_RUN, {"value": False})
        # Optimistically update the value
        self.coordinator.data["running"] = False
        self.async_write_ha_state()
