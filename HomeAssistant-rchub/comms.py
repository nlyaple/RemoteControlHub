"""Nelsony Hub Low Level Communication functions."""
from __future__ import annotations

# from enum import Enum
import logging

# import re
from typing import Any, Callable, List, Optional

# from xmlrpc.client import Boolean
from .actions import Action

from aiohttp import ClientSession, ClientTimeout
from aiohttp.client_exceptions import ServerDisconnectedError, ClientOSError

_LOGGER = logging.getLogger(__name__)


class Comms:
    """Communication API."""

    def __init__(
        self,
        host: str,
        token: str,
        *,
        session: Optional[ClientSession] = None,
        timeout: Optional[ClientTimeout] = None,
    ) -> None:
        """Initialize Hub with provided host and token."""
        self._host = host
        self._api_kwargs = {"headers": {"x-api-key": token}}
        if timeout:
            self._api_kwargs["timeout"] = timeout
        self._session = session

    async def version(self) -> dict:
        """Return the version of the Hub reported by API."""
        _LOGGER.info("Hub version call")
        return 1.0

    async def devices(self) -> List[str]:
        """Return the list of available device IDs reported by API."""
        json = await self.__get("/deviceList")
        _LOGGER.info("Nelsony hub reported devices: %s", json)
        return json

    async def device(self, device_id: str) -> dict:
        """Return main device metadata reported by API."""
        _LOGGER.info("Hub device call")
        return []  # await self.__get(f"/deviceList")

    async def action(self, device_id: str, action: Action) -> None:
        """Execute given action for a given device."""
        _LOGGER.info("Hub action command: %s", action.name)
        path = "/devaction"
        payload = device_id + "~" + action.name

        async def post(session: ClientSession) -> None:
            async with session.post(
                f"http://{self._host}{path}", **self._api_kwargs, data=payload
            ) as response:
                response.raise_for_status()

        await self.__call(post)

    async def __get(self, path) -> dict:
        async def get(session: ClientSession) -> dict:
            async with session.get(
                f"http://{self._host}{path}", **self._api_kwargs
            ) as response:
                response.raise_for_status()
                return await response.json()

        return await self.__call(get)

    async def __call(self, handler: Callable[[ClientSession], Any]):
        if not self._session:
            async with ClientSession() as request_session:
                return await handler(request_session)
        else:
            try:
                return await handler(self._session)
            except (ClientOSError, ServerDisconnectedError):
                # Hub has a short connection close time
                # so we need to retry if we idled for a bit
                return await handler(self._session)
