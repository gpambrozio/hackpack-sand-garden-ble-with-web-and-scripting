"""Base entity for Sand Garden integration."""
from __future__ import annotations

from homeassistant.config_entries import ConfigEntry
from homeassistant.helpers.update_coordinator import CoordinatorEntity

from .const import DOMAIN
from .coordinator import SandGardenCoordinator


class SandGardenEntity(CoordinatorEntity[SandGardenCoordinator]):
    """Base entity for Sand Garden devices."""

    _attr_has_entity_name = True

    def __init__(
        self, coordinator: SandGardenCoordinator, entry: ConfigEntry
    ) -> None:
        """Initialize the entity."""
        super().__init__(coordinator)
        self._attr_device_info = {
            "identifiers": {(DOMAIN, entry.entry_id)},
            "name": "Sand Garden",
            "manufacturer": "CrunchLabs",
            "model": "Sand Garden",
        }
