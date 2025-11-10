"""Light platform for Sand Garden."""
from __future__ import annotations

import logging
from typing import Any

from homeassistant.components.light import (
    ATTR_BRIGHTNESS,
    ATTR_EFFECT,
    ATTR_RGB_COLOR,
    ColorMode,
    LightEntity,
    LightEntityFeature,
)
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity_platform import AddEntitiesCallback

from .const import API_LED_BRIGHTNESS, API_LED_COLOR, API_LED_EFFECT, LED_EFFECTS
from .coordinator import SandGardenCoordinator
from .entity import SandGardenEntity

_LOGGER = logging.getLogger(__name__)


async def async_setup_entry(
    hass: HomeAssistant,
    entry: ConfigEntry,
    async_add_entities: AddEntitiesCallback,
) -> None:
    """Set up Sand Garden light entities."""
    coordinator: SandGardenCoordinator = hass.data[DOMAIN][entry.entry_id]

    async_add_entities([SandGardenLight(coordinator, entry)])


class SandGardenLight(SandGardenEntity, LightEntity):
    """Representation of Sand Garden LED strip."""

    _attr_name = "LED Strip"
    _attr_color_mode = ColorMode.RGB
    _attr_supported_color_modes = {ColorMode.RGB}
    _attr_supported_features = LightEntityFeature.EFFECT
    _attr_effect_list = list(LED_EFFECTS.values())

    def __init__(
        self, coordinator: SandGardenCoordinator, entry: ConfigEntry
    ) -> None:
        """Initialize the light entity."""
        super().__init__(coordinator, entry)
        self._attr_unique_id = f"{entry.entry_id}_led_strip"

    @property
    def is_on(self) -> bool:
        """Return true if light is on."""
        # LEDs are always "on" unless effect is "Off" (effect 13)
        # This ensures color and brightness controls are always visible
        effect = self.coordinator.data.get("ledEffect", 0)
        return effect != 13  # Effect 13 is "Off"

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

    @property
    def effect(self) -> str | None:
        """Return the current LED effect."""
        effect_id = self.coordinator.data.get("ledEffect")
        if effect_id is not None:
            return LED_EFFECTS.get(effect_id)
        return None

    async def async_turn_on(self, **kwargs: Any) -> None:
        """Turn on the light."""
        # Handle effect
        if ATTR_EFFECT in kwargs:
            effect_name = kwargs[ATTR_EFFECT]
            # Find effect ID by name
            effect_id = next(
                (eid for eid, name in LED_EFFECTS.items() if name == effect_name), None
            )
            if effect_id is not None:
                await self.coordinator.async_send_command(
                    API_LED_EFFECT, {"value": effect_id}
                )
                self.coordinator.data["ledEffect"] = effect_id

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

        # If just turning on without parameters, turn on with default effect
        if not kwargs:
            # If currently "Off", switch to "Rainbow" effect
            current_effect = self.coordinator.data.get("ledEffect", 0)
            if current_effect == 13:  # Currently "Off"
                await self.coordinator.async_send_command(
                    API_LED_EFFECT, {"value": 0}  # Rainbow
                )
                self.coordinator.data["ledEffect"] = 0

        self.async_write_ha_state()

    async def async_turn_off(self, **kwargs: Any) -> None:
        """Turn off the light."""
        # Set effect to "Off" (13)
        await self.coordinator.async_send_command(API_LED_EFFECT, {"value": 13})
        self.coordinator.data["ledEffect"] = 13
        self.async_write_ha_state()
