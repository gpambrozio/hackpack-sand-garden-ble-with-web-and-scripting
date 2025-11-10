"""Button platform for Sand Garden."""
from __future__ import annotations

import logging

from homeassistant.components.button import ButtonEntity
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity_platform import AddEntitiesCallback

from .const import API_COMMAND, API_RESET, DOMAIN
from .coordinator import SandGardenCoordinator
from .entity import SandGardenEntity

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


class SandGardenHomeButton(SandGardenEntity, ButtonEntity):
    """Representation of Sand Garden home command button."""

    _attr_name = "Home"
    _attr_icon = "mdi:home"

    def __init__(
        self, coordinator: SandGardenCoordinator, entry: ConfigEntry
    ) -> None:
        """Initialize the button entity."""
        super().__init__(coordinator, entry)
        self._attr_unique_id = f"{entry.entry_id}_home"

    async def async_press(self) -> None:
        """Handle the button press."""
        await self.coordinator.async_send_command(API_COMMAND, {"command": "HOME"})
        _LOGGER.info("Home command sent to Sand Garden")


class SandGardenResetButton(SandGardenEntity, ButtonEntity):
    """Representation of Sand Garden reset button."""

    _attr_name = "Reset"
    _attr_icon = "mdi:restart"

    def __init__(
        self, coordinator: SandGardenCoordinator, entry: ConfigEntry
    ) -> None:
        """Initialize the button entity."""
        super().__init__(coordinator, entry)
        self._attr_unique_id = f"{entry.entry_id}_reset"

    async def async_press(self) -> None:
        """Handle the button press."""
        await self.coordinator.async_send_command(API_RESET)
        _LOGGER.info("Reset command sent to Sand Garden - device will restart")
