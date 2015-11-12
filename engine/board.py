# -*- coding: utf-8 -*-
# This file is part of the Aditi Project

__author__ = 'Jesús Arroyo Torrens'
__email__ = 'jesus.arroyo@bq.com'
__copyright__ = 'Copyright (c) 2015 Mundo Reader S.L.'
__license__ = 'GPLv2'

import math
import serial


class Board:
    def __init__(self, serial_name='/dev/ttyUSB0', baud_rate=115200):
        self.serial_name = serial_name
        self.baud_rate = baud_rate
        self.serial_port = None

    def __del__(self):
        self.stop()
        self.disconnect()

    def connect(self):
        print ">>> Connecting board {0} {1}".format(self.serial_name, self.baud_rate)
        try:
            self.serial_port = serial.Serial(self.serial_name, self.baud_rate, timeout=1)
            if self.serial_port.isOpen():
                self.serial_port.flushInput()
                self.serial_port.flushOutput()
                print ">>> Done"
        except serial.SerialException:
            print "Error opening the port {0}\n".format(self.serial_name)
            self.serial_port = None

    def disconnect(self):
        if self.serial_port.isOpen():
            print ">>> Disconnecting board {0}".format(self.serial_name)
            try:
                if self.serial_port is not None:
                    self.stop()
                    self.serial_port.close()
                    del self.serial_port
            except serial.SerialException:
                print "Error closing the port {0}\n".format(self.serial_name)
                print ">>> Done"

    def start(self):
        self.write('s')

    def stop(self):
        self.write('q')

    def read(self):
        if self.serial_port.isOpen() and self.serial_port.inWaiting():
            data = self.serial_port.readline()
            if data is not None:
                ldata = data.replace('\x00', '').split(',')
                if len(ldata) == 3:
                    theta, phi, r = map(float, ldata)
                    if theta < 180:
                        theta = math.radians(90 - theta)
                        phi = math.radians(phi)
                        x = r * math.sin(theta) * math.cos(phi)
                        y = r * math.sin(theta) * math.sin(phi)
                        z = r * math.cos(theta)
                        return x, y, z

    def write(self, cmd):
        if self.serial_port.isOpen():
            self.serial_port.write(cmd)
