"""Button platform for Sand Garden."""
from __future__ import annotations

import logging

from homeassistant.components.button import ButtonEntity
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity_platform import AddEntitiesCallback
from homeassistant.helpers.update_coordinator import CoordinatorEntity

from .const import API_COMMAND, API_RESET, DOMAIN
from .coordinator import SandGardenCoordinator

_LOGGER = logging.getLogger(__name__)


async def async_setup_entry(
    hass: HomeAssistant,
    entry: ConfigEntry,
    async_add_entities: AddEntitiesCallback,
) -> None:
    """Set up Sand Garden button entities."""
    coordinator: SandGardenCoordinator = hass.data[DOMAIN][entry.entry_id]

    async_add_entities([
        SandGardenHomeButton(coordinator, entry),
        SandGardenResetButton(coordinator, entry),
    ])


class SandGardenHomeButton(CoordinatorEntity, ButtonEntity):
    """Representation of Sand Garden home command button."""

    _attr_has_entity_name = True
    _attr_name = "Home"
    _attr_icon = "mdi:home"

    def __init__(
        self, coordinator: SandGardenCoordinator, entry: ConfigEntry
    ) -> None:
        """Initialize the button entity."""
        super().__init__(coordinator)
        self._attr_unique_id = f"{entry.entry_id}_home"
        self._attr_device_info = {
            "identifiers": {(DOMAIN, entry.entry_id)},
            "name": "Sand Garden",
            "manufacturer": "CrunchLabs",
            "model": "Sand Garden",
        }

    async def async_press(self) -> None:
        """Handle the button press."""
        await self.coordinator.async_send_command(API_COMMAND, {"command": "HOME"})
        _LOGGER.info("Home command sent to Sand Garden")


class SandGardenResetButton(CoordinatorEntity, ButtonEntity):
    """Representation of Sand Garden reset button."""

    _attr_has_entity_name = True
    _attr_name = "Reset"
    _attr_icon = "mdi:restart"

    def __init__(
        self, coordinator: SandGardenCoordinator, entry: ConfigEntry
    ) -> None:
        """Initialize the button entity."""
        super().__init__(coordinator)
        self._attr_unique_id = f"{entry.entry_id}_reset"
        self._attr_device_info = {
            "identifiers": {(DOMAIN, entry.entry_id)},
            "name": "Sand Garden",
            "manufacturer": "CrunchLabs",
            "model": "Sand Garden",
        }

    async def async_press(self) -> None:
        """Handle the button press."""
        await self.coordinator.async_send_command(API_RESET)
        _LOGGER.info("Reset command sent to Sand Garden - device will restart")
