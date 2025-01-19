"""Bond Device type enumeration."""
from __future__ import annotations

# "1": "3 Button - Up/Stop/Down (433MHz)",
# "2": "3 Button - Open/Stop/Close (433 MHz)",
# "3": "6 Button - Open/Stop/Close/Partial open (433 MHz)",
# "4": "MinkaAire AireControl (303 MHz)",
# "5": "6 Button - Up/Stop/Down/Tilt Up/Down (433 MHz)"


class DeviceType:
    """Bond Device type enumeration."""

    CEILING_FAN = "4"  # "CF"
    MOTORIZED_SHADES = "5"  # "MS"
    FIREPLACE = "0"  # "FP"
    AIR_CONDITIONER = "0"  # "AC"
    GARAGE_DOOR = "0"  # "GD"
    BIDET = "0"  # "BD"
    LIGHT = "4"  # "LT"
    GENERIC_DEVICE = "0"  # "GX"

    BLINDS = "5"
    DRAPES = "3"
    SHADES = "1"
    SHADES_2 = "2"

    @staticmethod
    def is_fan(device_type: str) -> bool:
        """Checks if specified device type is a fan."""
        return device_type == DeviceType.CEILING_FAN

    @staticmethod
    def is_shades(device_type: str) -> bool:
        """Checks if specified device type is shades."""
        return device_type == DeviceType.SHADES or device_type == DeviceType.SHADES_2

    @staticmethod
    def is_drapes(device_type: str) -> bool:
        """Checks if specified device type is drapes."""
        return device_type == DeviceType.DRAPES

    @staticmethod
    def is_blinds(device_type: str) -> bool:
        """Checks if specified device type is blinds."""
        return device_type == DeviceType.BLINDS

    @staticmethod
    def is_fireplace(device_type: str) -> bool:
        """Checks if specified device type is fireplace."""
        return device_type == DeviceType.FIREPLACE

    @staticmethod
    def is_air_conditioner(device_type: str) -> bool:
        """Checks if specified device type is air conditioner."""
        return device_type == DeviceType.AIR_CONDITIONER

    @staticmethod
    def is_garage_door(device_type: str) -> bool:
        """Checks if specified device type is garage door."""
        return device_type == DeviceType.GARAGE_DOOR

    @staticmethod
    def is_bidet(device_type: str) -> bool:
        """Checks if specified device type is bidet."""
        return device_type == DeviceType.BIDET

    @staticmethod
    def is_light(device_type: str) -> bool:
        """Checks if specified device type is light."""
        return device_type == DeviceType.LIGHT

    @staticmethod
    def is_generic(device_type: str) -> bool:
        """Checks if specified device type is generic."""
        return device_type == DeviceType.GENERIC_DEVICE
