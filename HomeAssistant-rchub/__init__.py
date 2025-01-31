"""The Hub integration."""
from asyncio import TimeoutError as AsyncIOTimeoutError
from http import HTTPStatus
import logging
from typing import Any

from aiohttp import ClientError, ClientResponseError, ClientTimeout

from .comms import Comms


from homeassistant.config_entries import ConfigEntry
from homeassistant.const import (
    CONF_ACCESS_TOKEN,
    CONF_HOST,
    EVENT_HOMEASSISTANT_STOP,
    Platform,
)
from homeassistant.core import HomeAssistant, callback
from homeassistant.exceptions import ConfigEntryNotReady
from homeassistant.helpers import device_registry as dr
from homeassistant.helpers.aiohttp_client import async_get_clientsession
from homeassistant.helpers.entity import SLOW_UPDATE_WARNING

from .const import BRIDGE_MAKE, DOMAIN
from .models import BondData
from .utils import BondHub

PLATFORMS = [
    Platform.BUTTON,
    Platform.COVER,
    Platform.FAN,
    Platform.LIGHT,
    Platform.SWITCH,
]
_API_TIMEOUT = SLOW_UPDATE_WARNING - 1

_LOGGER = logging.getLogger(__name__)


async def async_setup_entry(hass: HomeAssistant, entry: ConfigEntry) -> bool:
    """Set up Hub from a config entry."""
    host = entry.data[CONF_HOST]
    token = entry.data[CONF_ACCESS_TOKEN]
    config_entry_id = entry.entry_id

    comms = Comms(
        host=host,
        token=token,
        timeout=ClientTimeout(total=_API_TIMEOUT),
        session=async_get_clientsession(hass),
    )
    hub = BondHub(comms, host)
    try:
        await hub.setup()
    except ClientResponseError as ex:
        if ex.status == HTTPStatus.UNAUTHORIZED:
            _LOGGER.error("Bond token no longer valid: %s", ex)
            return False
        raise ConfigEntryNotReady from ex
    except (ClientError, AsyncIOTimeoutError, OSError) as error:
        raise ConfigEntryNotReady from error

    hass.data.setdefault(DOMAIN, {})[entry.entry_id] = BondData(hub)

    if not entry.unique_id:
        hass.config_entries.async_update_entry(entry, unique_id="55366")

    device_registry = dr.async_get(hass)
    device_registry.async_get_or_create(
        config_entry_id=config_entry_id,
        identifiers={(DOMAIN, "55366")},
        manufacturer=BRIDGE_MAKE,
        name="Hub",
        model="Model 1",
        sw_version="1.0.0",
        hw_version="1.0.0",
        suggested_area="home",
        configuration_url=f"http://{host}",
    )

    _async_remove_old_device_identifiers(config_entry_id, device_registry, hub)

    await hass.config_entries.async_forward_entry_setups(entry, PLATFORMS)
    # await hass.config_entries.async_setup_platforms(entry, PLATFORMS)

    return True


async def async_unload_entry(hass: HomeAssistant, entry: ConfigEntry) -> bool:
    """Unload a config entry."""
    if unload_ok := await hass.config_entries.async_unload_platforms(entry, PLATFORMS):
        hass.data[DOMAIN].pop(entry.entry_id)
    return unload_ok


@callback
def _async_remove_old_device_identifiers(
    config_entry_id: str, device_registry: dr.DeviceRegistry, hub: BondHub
) -> None:
    """Remove the non-unique device registry entries."""
    for device in hub.devices:
        dev = device_registry.async_get_device(identifiers={(DOMAIN, device.device_id)})
        if dev is None:
            continue
        if config_entry_id in dev.config_entries:
            device_registry.async_remove_device(dev.id)


async def async_remove_config_entry_device(
    hass: HomeAssistant, config_entry: ConfigEntry, device_entry: dr.DeviceEntry
) -> bool:
    """Remove bond config entry from a device."""
    data: BondData = hass.data[DOMAIN][config_entry.entry_id]
    hub = data.hub
    for identifier in device_entry.identifiers:
        if identifier[0] != DOMAIN or len(identifier) != 3:
            continue
        bond_id: str = identifier[1]
        # Bond still uses the 3 arg tuple before
        # the identifiers were typed
        device_id: str = identifier[2]  # type: ignore[misc]
        # If device_id is no longer present on
        # the hub, we allow removal.
        if hub.bond_id != bond_id or not any(
            device_id == device.device_id for device in hub.devices
        ):
            return True
    return False
