"""Reusable utilities for the Hub component."""
from __future__ import annotations

import logging
from typing import Any, cast

# from aiohttp import ClientResponseError

from .actions import Action, DeviceActions
from .comms import Comms

# from homeassistant.util.async_ import gather_with_concurrency

# from .const import BRIDGE_MAKE

MAX_REQUESTS = 6

_LOGGER = logging.getLogger(__name__)


class BondDevice:
    """Helper device class to hold ID and attributes together."""

    def __init__(
        self,
        device_id: str,
        device_type: int,
        state: dict[str, Any],
    ) -> None:
        """Create a helper device from ID and attributes returned by API."""
        self.device_id = device_id
        self.device_type = device_type
        self.state = state
        self._supported_actions = DeviceActions.devTypes[int(device_type)]

    def __repr__(self) -> str:
        """Return readable representation of a hub device."""
        return {
            "device_id": self.device_id,
            "device_type": self.device_type,
        }.__repr__()

    @property
    def name(self) -> str:
        """Get the name of this device."""
        return cast(str, self.device_id)  # _attrs["name"])

    @property
    def type(self) -> str:
        """Get the type of this device."""
        return cast(str, self.device_type)  # _attrs["type"])

    @property
    def location(self) -> str | None:
        """Get the location of this device."""
        return self.device_id  # _attrs.get("location")

    @property
    def template(self) -> str | None:
        """Return this model template."""
        return ""  # self._attrs.get("template")

    @property
    def branding_profile(self) -> str | None:
        """Return this branding profile."""
        return ""  # self.props.get("branding_profile")

    @property
    def trust_state(self) -> bool:
        """Check if Trust State is turned on."""
        return False  # self.props.get("trust_state", False)

    def has_action(self, action: str) -> bool:
        """Check to see if the device supports an actions."""
        return action in self._supported_actions

    def _has_any_action(self, actions: set[str]) -> bool:
        """Check to see if the device supports any of the actions."""
        # return bool(self._supported_actions.intersection(actions))
        found = actions in self._supported_actions
        return found

    def supports_speed(self) -> bool:
        """Return True if this device supports any of the speed related commands."""
        # return self._has_any_action({Action.SET_SPEED})
        return self.has_action(Action.SPEED_1)

    def supports_direction(self) -> bool:
        """Return True if this device supports any of the direction related commands."""
        # return self._has_any_action({Action.DIRECTION})
        return self.has_action(Action.DIRECTION)

    def supports_set_position(self) -> bool:
        """Return True if this device supports setting the position."""
        # return self._has_any_action({Action.POSITION})
        return self.has_action(Action.POSITION)

    def supports_open(self) -> bool:
        """Return True if this device supports opening."""
        return self.has_action(Action.OPEN)

    def supports_close(self) -> bool:
        """Return True if this device supports closing."""
        return self.has_action(Action.CLOSE)

    def supports_up(self) -> bool:
        """Return True if this device supports up (open)."""
        return self.has_action(Action.OPEN)

    def supports_down(self) -> bool:
        """Return True if this device supports down (close)."""
        return self.has_action(Action.CLOSE)

    def supports_tilt_open(self) -> bool:
        """Return True if this device supports tilt Up."""
        # return self._has_any_action({Action.TILT_UP})
        return self.has_action(Action.TILT_UP)

    def supports_tilt_close(self) -> bool:
        """Return True if this device supports tilt Down."""
        # return self._has_any_action({Action.TILT_DOWN})
        return self.has_action(Action.TILT_DOWN)

    def supports_tilt_up(self) -> bool:
        """Return True if this device supports tilt Up."""
        return self.has_action(Action.TILT_UP)

    def supports_tilt_down(self) -> bool:
        """Return True if this device supports tilt Down."""
        return self.has_action(Action.TILT_DOWN)

    def supports_hold(self) -> bool:
        """Return True if this device supports hold aka stop."""
        # return self._has_any_action({Action.HOLD})
        return self.has_action(Action.HOLD)

    def supports_light(self) -> bool:
        """Return True if this device supports any of the light related commands."""
        # return self._has_any_action({Action.LIGHT_ON, Action.LIGHT_OFF})
        return self.has_action(Action.LIGHT_ON) or self.has_action(Action.LIGHT_OFF)

    def supports_up_light(self) -> bool:
        """Return true if the device has an up light."""
        # return self._has_any_action({Action.TURN_UP_LIGHT_ON, Action.TURN_UP_LIGHT_OFF})
        # return self.has_action(Action.TURN_UP_LIGHT_ON) or self.has_action(
        #    Action.TURN_UP_LIGHT_OFF
        # )
        return False

    def supports_down_light(self) -> bool:
        """Return true if the device has a down light."""
        # return self._has_any_action(
        #    {Action.TURN_DOWN_LIGHT_ON, Action.TURN_DOWN_LIGHT_OFF}
        # )
        # return self.has_action(Action.TURN_DOWN_LIGHT_ON) or self.has_action(
        #    Action.TURN_DOWN_LIGHT_OFF
        # )
        return False

    def supports_set_brightness(self) -> bool:
        """Return True if this device supports setting a light brightness."""
        # return self._has_any_action({Action.SET_BRIGHTNESS})
        return self.has_action(Action.SET_BRIGHTNESS)


class BondHub:
    """Hub device itself."""

    def __init__(self, bond: Comms, host: str) -> None:
        """Initialize Bond Hub."""
        self.bond: Comms = bond
        self.host = host
        self._bridge: dict[str, Any] = {}
        self._version: dict[str, Any] = {}
        self._devices: list[BondDevice] = []

    async def setup(self, max_devices: int | None = None) -> None:
        """Read hub version information."""
        self._version = 1.0  # await self.bond.version()
        _LOGGER.debug("Bond reported the following version info: %s", self._version)
        # Fetch all available devices using Bond API.
        device_ids = await self.bond.devices()
        self._devices = []
        # setup_device_ids = []
        # for idx, device_id in enumerate(device_ids):
        #    if max_devices is not None and idx >= max_devices:
        #        break
        #    setup_device_ids.append(device_id)
        # for device_id in setup_device_ids:
        device_states = {}
        for device_id, device_type in device_ids.items():
            self._devices.append(
                BondDevice(
                    device_id,
                    device_type,
                    device_states,
                )
            )

        _LOGGER.debug("Discovered Bond devices: %s", self._devices)
        # try:
        # Smart by bond devices do not have a bridge api call
        #    self._bridge = await self.bond.bridge()
        # except ClientResponseError:
        #    self._bridge = {}
        self._bridge = "Hub"
        _LOGGER.debug("Bond reported the following bridge info: %s", self._bridge)

    @property
    def bond_id(self) -> str | None:
        """Return unique Bond ID for this hub."""
        # Old firmwares are missing the bondid
        return "55366"  # self._version.get("bondid")

    @property
    def target(self) -> str | None:
        """Return this hub target."""
        return "Hub"  # self._version.get("target")

    @property
    def model(self) -> str | None:
        """Return this hub model."""
        return "bridge_pro"  # self._version.get("model")

    @property
    def make(self) -> str:
        """Return this hub make."""
        return "Nelsony"  # self._version.get("make", BRIDGE_MAKE)

    # @property
    # def name(self) -> str:
    #    """Get the name of this bridge."""
    #    if not self.is_bridge and self._devices:
    #        return self._devices[0].name
    #    return "Hub1"  # cast(str, self._bridge["name"])

    # @property
    # def location(self) -> str | None:
    #    """Get the location of this bridge."""
    #    if not self.is_bridge and self._devices:
    #        return self._devices[0].location
    #    return "Remote"  # self._bridge.get("location")

    @property
    def fw_ver(self) -> str | None:
        """Return this hub firmware version."""
        return "1.0.0"  # self._version.get("fw_ver")

    @property
    def mcu_ver(self) -> str | None:
        """Return this hub hardware version."""
        return "1.0.0"  # self._version.get("mcu_ver")

    @property
    def devices(self) -> list[BondDevice]:
        """Return a list of all devices controlled by this hub."""
        return self._devices

    @property
    def is_bridge(self) -> bool:
        """Return if the Bond is a Bond Bridge."""
        # bondid = self._version["bondid"]
        return True  # bool(BondType.is_bridge_from_serial(bondid))
