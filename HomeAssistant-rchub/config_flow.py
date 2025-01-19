"""Config flow for Hub integration."""
from __future__ import annotations

# import contextlib

from http import HTTPStatus
import logging
from typing import Any

from aiohttp import ClientConnectionError, ClientResponseError

import voluptuous as vol

from homeassistant import config_entries, exceptions
from homeassistant.components import zeroconf

# from homeassistant.config_entries import ConfigEntryState
from homeassistant.const import CONF_ACCESS_TOKEN, CONF_HOST, CONF_NAME
from homeassistant.core import HomeAssistant
from homeassistant.data_entry_flow import AbortFlow, FlowResult
from homeassistant.helpers.aiohttp_client import async_get_clientsession

from .comms import Comms
from .const import DOMAIN
from .utils import BondHub

_LOGGER = logging.getLogger(__name__)

USER_SCHEMA = vol.Schema(
    {vol.Required(CONF_HOST): str, vol.Required(CONF_ACCESS_TOKEN): str}
)
DISCOVERY_SCHEMA = vol.Schema({vol.Required(CONF_ACCESS_TOKEN): str})
TOKEN_SCHEMA = vol.Schema({})


async def _validate_input(hass: HomeAssistant, data: dict[str, Any]) -> tuple[str, str]:
    """Validate the user input allows us to connect."""

    hub_comms = Comms(
        data[CONF_HOST], data[CONF_ACCESS_TOKEN], session=async_get_clientsession(hass)
    )
    try:
        hub_obj = BondHub(hub_comms, data[CONF_HOST])
        await hub_obj.setup(max_devices=1)
    except ClientConnectionError as error:
        raise InputValidationError("cannot_connect") from error
    except ClientResponseError as error:
        if error.status == HTTPStatus.UNAUTHORIZED:
            raise InputValidationError("invalid_auth") from error
        raise InputValidationError("unknown") from error
    except Exception as error:
        _LOGGER.exception("Unexpected exception")
        raise InputValidationError("unknown") from error
    return 55366, "hub"


class ConfigFlow(config_entries.ConfigFlow, domain=DOMAIN):
    """Handle a config flow for Hub."""

    VERSION = 1

    def __init__(self) -> None:
        """Initialize config flow."""
        self._discovered: dict[str, str] = {}

    async def async_step_zeroconf(
        self, discovery_info: zeroconf.ZeroconfServiceInfo
    ) -> FlowResult:
        """Handle a flow initialized by zeroconf discovery."""
        name: str = discovery_info.name
        host: str = discovery_info.host
        hub_id = name.partition(".")[0]
        _LOGGER.warning("Found Hub device: %s on host %s", hub_id, host)
        await self.async_set_unique_id(hub_id)
        hass = self.hass
        for entry in self._async_current_entries():
            if entry.unique_id != hub_id:
                continue
            updates = {CONF_HOST: host}
            new_data = {**entry.data, **updates}
            changed = new_data != dict(entry.data)
            if changed:
                hass.config_entries.async_update_entry(entry, data=new_data)
                entry_id = entry.entry_id
                hass.async_create_task(hass.config_entries.async_reload(entry_id))
            raise AbortFlow("already_configured")

        self._discovered = {CONF_HOST: host, CONF_NAME: hub_id}
        self._discovered[CONF_NAME] = "Hub1"

        self.context.update(
            {
                "title_placeholders": {
                    CONF_HOST: self._discovered[CONF_HOST],
                    CONF_NAME: self._discovered[CONF_NAME],
                }
            }
        )

        return await self.async_step_confirm()

    async def async_step_confirm(
        self, user_input: dict[str, Any] | None = None
    ) -> FlowResult:
        """Handle confirmation flow for discovered bond hub."""
        errors = {}
        if user_input is not None:
            if CONF_ACCESS_TOKEN in self._discovered:
                return self.async_create_entry(
                    title=self._discovered[CONF_NAME],
                    data={
                        CONF_ACCESS_TOKEN: self._discovered[CONF_ACCESS_TOKEN],
                        CONF_HOST: self._discovered[CONF_HOST],
                    },
                )

            data = {
                CONF_ACCESS_TOKEN: user_input[CONF_ACCESS_TOKEN],
                CONF_HOST: self._discovered[CONF_HOST],
            }
            try:
                _, hub_name = await _validate_input(self.hass, data)
            except InputValidationError as error:
                errors["base"] = error.base
            else:
                return self.async_create_entry(
                    title=hub_name,
                    data=data,
                )

        if CONF_ACCESS_TOKEN in self._discovered:
            data_schema = TOKEN_SCHEMA
        else:
            data_schema = DISCOVERY_SCHEMA

        return self.async_show_form(
            step_id="confirm",
            data_schema=data_schema,
            errors=errors,
            description_placeholders=self._discovered,
        )

    async def async_step_user(
        self, user_input: dict[str, Any] | None = None
    ) -> FlowResult:
        """Handle a flow initialized by the user."""
        errors = {}
        if user_input is not None:
            try:
                bond_id, hub_name = await _validate_input(self.hass, user_input)
            except InputValidationError as error:
                errors["base"] = error.base
            else:
                await self.async_set_unique_id(bond_id)
                self._abort_if_unique_id_configured()
                return self.async_create_entry(title=hub_name, data=user_input)

        return self.async_show_form(
            step_id="user", data_schema=USER_SCHEMA, errors=errors
        )


class InputValidationError(exceptions.HomeAssistantError):
    """Error to indicate we cannot proceed due to invalid input."""

    def __init__(self, base: str) -> None:
        """Initialize with error base."""
        super().__init__()
        self.base = base
