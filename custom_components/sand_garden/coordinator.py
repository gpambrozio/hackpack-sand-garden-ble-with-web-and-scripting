"""Data update coordinator for Sand Garden."""
from __future__ import annotations

import asyncio
import logging
from datetime import timedelta
from typing import Any

import aiohttp

from homeassistant.core import HomeAssistant
from homeassistant.helpers.aiohttp_client import async_get_clientsession
from homeassistant.helpers.update_coordinator import DataUpdateCoordinator, UpdateFailed

from .const import API_EVENTS, API_STATE, DOMAIN, SCAN_INTERVAL

_LOGGER = logging.getLogger(__name__)


class SandGardenCoordinator(DataUpdateCoordinator):
    """Class to manage fetching Sand Garden data."""

    def __init__(self, hass: HomeAssistant, host: str) -> None:
        """Initialize."""
        self.host = host
        self.base_url = f"http://{host}"
        self._session = async_get_clientsession(hass)
        self._sse_task: asyncio.Task | None = None
        self._stop_sse = False

        super().__init__(
            hass,
            _LOGGER,
            name=DOMAIN,
            update_interval=timedelta(seconds=SCAN_INTERVAL),
        )

    async def _async_update_data(self) -> dict[str, Any]:
        """Fetch data from API endpoint.

        This is called periodically as a fallback and for initial setup.
        Real-time updates come from SSE.
        """
        try:
            url = f"{self.base_url}{API_STATE}"
            async with self._session.get(
                url, timeout=aiohttp.ClientTimeout(total=10)
            ) as response:
                if response.status != 200:
                    raise UpdateFailed(f"HTTP {response.status}")
                data = await response.json()
                _LOGGER.debug("Fetched state: %s", data)
                return data
        except aiohttp.ClientError as err:
            raise UpdateFailed(f"Error communicating with API: {err}") from err

    async def async_config_entry_first_refresh(self) -> None:
        """Refresh data for the first time and start SSE listener."""
        await super().async_config_entry_first_refresh()
        # Start SSE listener after initial refresh
        if self._sse_task is None or self._sse_task.done():
            self._stop_sse = False
            self._sse_task = asyncio.create_task(self._listen_sse())

    async def async_shutdown(self) -> None:
        """Shutdown the coordinator."""
        self._stop_sse = True
        if self._sse_task and not self._sse_task.done():
            self._sse_task.cancel()
            try:
                await self._sse_task
            except asyncio.CancelledError:
                pass

    async def _listen_sse(self) -> None:
        """Listen to Server-Sent Events for real-time updates."""
        url = f"{self.base_url}{API_EVENTS}"
        _LOGGER.info("Starting SSE listener for %s", url)

        while not self._stop_sse:
            try:
                async with self._session.get(
                    url,
                    timeout=aiohttp.ClientTimeout(total=None, sock_read=300),
                ) as response:
                    if response.status != 200:
                        _LOGGER.warning("SSE connection failed with HTTP %s", response.status)
                        await asyncio.sleep(10)
                        continue

                    _LOGGER.info("SSE connection established")
                    async for line in response.content:
                        if self._stop_sse:
                            break

                        line = line.decode("utf-8").strip()
                        if not line:
                            continue

                        # Parse SSE format: "event: <type>" and "data: <json>"
                        if line.startswith("event:"):
                            continue  # We'll process on data line

                        if line.startswith("data:"):
                            data_str = line[5:].strip()
                            try:
                                # Try to parse as JSON
                                import json
                                data = json.loads(data_str)

                                # Update coordinator data with new values
                                if isinstance(data, dict):
                                    # Merge new data with existing data
                                    if self.data:
                                        self.data.update(data)
                                    else:
                                        self.data = data

                                    # Notify listeners
                                    self.async_set_updated_data(self.data)
                                    _LOGGER.debug("SSE update: %s", data)
                            except json.JSONDecodeError:
                                _LOGGER.debug("SSE non-JSON data: %s", data_str)
                                pass

            except asyncio.CancelledError:
                _LOGGER.debug("SSE listener cancelled")
                break
            except aiohttp.ClientError as err:
                if not self._stop_sse:
                    _LOGGER.warning("SSE connection error: %s, retrying in 10s", err)
                    await asyncio.sleep(10)
            except Exception as err:
                if not self._stop_sse:
                    _LOGGER.exception("Unexpected error in SSE listener: %s", err)
                    await asyncio.sleep(10)

        _LOGGER.info("SSE listener stopped")

    async def async_send_command(self, endpoint: str, data: dict[str, Any] | None = None) -> None:
        """Send a command to the device."""
        url = f"{self.base_url}{endpoint}"
        try:
            async with self._session.post(
                url,
                json=data,
                timeout=aiohttp.ClientTimeout(total=10),
            ) as response:
                if response.status != 200:
                    _LOGGER.error("Command failed with HTTP %s: %s", response.status, await response.text())
                    raise UpdateFailed(f"HTTP {response.status}")
                _LOGGER.debug("Sent command to %s: %s", endpoint, data)
        except aiohttp.ClientError as err:
            raise UpdateFailed(f"Error sending command: {err}") from err
