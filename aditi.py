# -*- coding: utf-8 -*-
# This file is part of the Aditi Project

__author__ = 'Jesús Arroyo Torrens'
__email__ = 'jesus.arroyo@bq.com'
__copyright__ = 'Copyright (c) 2015 Mundo Reader S.L.'
__license__ = 'GPLv2'

import json
import threading

from engine import board
from rainbow import register, publish, run

running = False
scanner = board.Board('/dev/ttyUSB0', 115200)


@register('connect')
def connect():
    scanner.connect()
    return scanner.serial_port.isOpen()


@register('start')
def start():
    global running
    if not running:
        running = True
        threading.Thread(target=_read).start()
    scanner.start()


@register('stop')
def stop():
    global running
    running = False
    scanner.stop()


def _read():
    global running
    while running:
        point = scanner.read()
        if point is not None:
            x, y, z = point
            publish('point.cloud', json.dumps({"x": x, "y": y, "z": z}))


run(host='0.0.0.0')
