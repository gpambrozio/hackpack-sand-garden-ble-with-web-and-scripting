"""Light platform for Sand Garden."""
from __future__ import annotations

import logging
from typing import Any

from homeassistant.components.light import (
    ATTR_BRIGHTNESS,
    ATTR_RGB_COLOR,
    ColorMode,
    LightEntity,
)
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity_platform import AddEntitiesCallback
from homeassistant.helpers.update_coordinator import CoordinatorEntity

from .const import API_LED_BRIGHTNESS, API_LED_COLOR, DOMAIN
from .coordinator import SandGardenCoordinator

_LOGGER = logging.getLogger(__name__)


async def async_setup_entry(
    hass: HomeAssistant,
    entry: ConfigEntry,
    async_add_entities: AddEntitiesCallback,
) -> None:
    """Set up Sand Garden light entities."""
    coordinator: SandGardenCoordinator = hass.data[DOMAIN][entry.entry_id]

    async_add_entities([SandGardenLight(coordinator, entry)])


class SandGardenLight(CoordinatorEntity, LightEntity):
    """Representation of Sand Garden LED strip."""

    _attr_has_entity_name = True
    _attr_name = "LED Strip"
    _attr_color_mode = ColorMode.RGB
    _attr_supported_color_modes = {ColorMode.RGB}

    def __init__(
        self, coordinator: SandGardenCoordinator, entry: ConfigEntry
    ) -> None:
        """Initialize the light entity."""
        super().__init__(coordinator)
        self._attr_unique_id = f"{entry.entry_id}_led_strip"
        self._attr_device_info = {
            "identifiers": {(DOMAIN, entry.entry_id)},
            "name": "Sand Garden",
            "manufacturer": "CrunchLabs",
            "model": "Sand Garden",
        }

    @property
    def is_on(self) -> bool:
        """Return true if light is on."""
        # Consider light "on" if brightness > 0
        brightness = self.coordinator.data.get("ledBrightness", 0)
        return brightness > 0

    @property
    def brightness(self) -> int | None:
        """Return the brightness of the light (0-255)."""
        return self.coordinator.data.get("ledBrightness")

    @property
    def rgb_color(self) -> tuple[int, int, int] | None:
        """Return the RGB color value."""
        r = self.coordinator.data.get("ledColorR")
        g = self.coordinator.data.get("ledColorG")
        b = self.coordinator.data.get("ledColorB")
        if r is not None and g is not None and b is not None:
            return (r, g, b)
        return None

    async def async_turn_on(self, **kwargs: Any) -> None:
        """Turn on the light."""
        # Handle brightness
        if ATTR_BRIGHTNESS in kwargs:
            brightness = kwargs[ATTR_BRIGHTNESS]
            await self.coordinator.async_send_command(
                API_LED_BRIGHTNESS, {"value": brightness}
            )
            self.coordinator.data["ledBrightness"] = brightness

        # Handle color
        if ATTR_RGB_COLOR in kwargs:
            r, g, b = kwargs[ATTR_RGB_COLOR]
            await self.coordinator.async_send_command(
                API_LED_COLOR, {"r": r, "g": g, "b": b}
            )
            self.coordinator.data["ledColorR"] = r
            self.coordinator.data["ledColorG"] = g
            self.coordinator.data["ledColorB"] = b

        # If just turning on without parameters, set to full brightness
        if not kwargs:
            await self.coordinator.async_send_command(
                API_LED_BRIGHTNESS, {"value": 255}
            )
            self.coordinator.data["ledBrightness"] = 255

        self.async_write_ha_state()

    async def async_turn_off(self, **kwargs: Any) -> None:
        """Turn off the light."""
        await self.coordinator.async_send_command(API_LED_BRIGHTNESS, {"value": 0})
        self.coordinator.data["ledBrightness"] = 0
        self.async_write_ha_state()
