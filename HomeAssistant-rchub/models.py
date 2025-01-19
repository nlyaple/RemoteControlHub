"""The bond integration models."""
from __future__ import annotations

from dataclasses import dataclass

from .utils import BondHub


@dataclass
class BondData:
    """Data for the hub integration."""

    hub: BondHub
