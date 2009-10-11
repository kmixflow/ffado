#
# Copyright (C) 2009 by Arnold Krille
#
# This file is part of FFADO
# FFADO = Free Firewire (pro-)audio drivers for linux
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) version 3 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

from PyQt4 import QtGui, QtCore, Qt
import dbus, numpy

from ffado.widgets.ntompanner import N2MPanner

import logging
log = logging.getLogger("matrixmixer")

class MixerNode(QtGui.QFrame):
    def __init__(self, inputs, outputs, parent):
        """inputs  = list of input-channel numbers
           outputs = list of output-channel numbers
           """
        QtGui.QFrame.__init__(self, parent)
        self.setLineWidth(2)
        self.setFrameStyle(QtGui.QFrame.Panel|QtGui.QFrame.Raised)

        self.inputs = []
        self.outputs = []

        self.fader = QtGui.QDial(self)
        self.fader.setRange(0,pow(2,16)-1)
        self.connect(self.fader, QtCore.SIGNAL("valueChanged(int)"), self.valuesChanged)

        self.layout = QtGui.QGridLayout(self)
        self.setLayout(self.layout)

        self.layout.addWidget(self.fader, 0, 0)

        self.addInputs(inputs)
        self.addOutputs(outputs)

    def valuesChanged(self):
        #log.debug("MixerNode.valuesChanged")
        fader = self.fader.value()
        values = numpy.ones((1, len(self.inputs)))
        if len(self.outputs) > 1:
            values = self.panner.values
            if values.size == 0:
                return
            values = numpy.minimum(values, 2)
            values = 1 - numpy.power(values/2, 2)
        #print values
        #print numpy.exp(-values)
        values = values * fader
        #print values
        ret = []
        for i in range(len(self.inputs)):
            for j in range(len(self.outputs)):
                ret.append( (self.inputs[i], self.outputs[j], values[i,j]) )
        #print ret
        self.emit(QtCore.SIGNAL("valueChanged"), ret)

    def addInputs(self, inputs, add=True):
        if not isinstance(inputs, list):
            inputs = [inputs]
        if add:
            self.inputs += inputs
        else:
            for item in inputs:
                self.inputs.remove(item)
        self.checkWidgets()
    def removeInputs(self, inputs):
        self.addInputs(inputs, False)

    def addOutputs(self, outputs, add=True):
        if not isinstance(outputs, list):
            outputs = [outputs]
        if add:
            self.outputs += outputs
        else:
            for item in outputs:
                self.outputs.remove(item)
        self.checkWidgets()
    def removeOutputs(self, outputs):
        self.addOutputs(outputs, False)

    def checkWidgets(self):
        if not hasattr(self, "panner") and len(self.outputs)>1:
            self.panner = N2MPanner(self)
            self.layout.addWidget(self.panner, 1, 0)
            self.connect(self.panner, QtCore.SIGNAL("valuesChanged"), self.valuesChanged)
        if hasattr(self, "panner"):
            self.panner.setVisible(len(self.outputs)>1)
            self.panner.setNumberOfSources(len(self.inputs))
            self.panner.setNumberOfSinks(len(self.outputs))
        if len(self.inputs) and len(self.outputs):
            valuematrix = []
            for row in self.outputs:
                valuematrix.append( [] )
                for col in self.inputs:
                    valuematrix[self.outputs.index(row)] = self.parent().interface.getValue(row, col)
            self.fader.setValue(max(valuematrix))


class MixerChannel(QtGui.QWidget):
    def __init__(self, number, parent=None, name=""):
        QtGui.QWidget.__init__(self, parent)
        layout = QtGui.QGridLayout(self)
        self.number = number
        if name is not "":
            name = " (%s)" % name
        self.lbl = QtGui.QLabel("Ch. %i%s" % (self.number, name), self)
        layout.addWidget(self.lbl, 0, 0, 1, 2)

        self.btnHide = QtGui.QToolButton(self)
        self.btnHide.setText("Hide")
        self.btnHide.setCheckable(True)
        self.connect(self.btnHide, QtCore.SIGNAL("clicked(bool)"), self.hideChannel)
        layout.addWidget(self.btnHide, 1, 0)

        self.btnCouple = QtGui.QToolButton(self)
        self.btnCouple.setText("Stereo")
        self.btnCouple.setCheckable(True)
        self.connect(self.btnCouple, QtCore.SIGNAL("toggled(bool)"), self.coupleWithNext)
        layout.addWidget(self.btnCouple, 1, 1)

    def hideChannel(self, hide):
        self.btnCouple.setHidden(hide)
        self.emit(QtCore.SIGNAL("hide"), self.number, hide)

    def coupleWithNext(self, couple):
        self.emit(QtCore.SIGNAL("couple"), self.number, couple)

    def canCouple(self, cancouple):
        #self.btnCouple.setEnabled(cancouple)
        self.btnCouple.setHidden(not cancouple)

class MatrixMixer(QtGui.QWidget):
    def __init__(self, servername, basepath, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self.bus = dbus.SessionBus()
        self.dev = self.bus.get_object(servername, basepath)
        self.interface = dbus.Interface(self.dev, dbus_interface="org.ffado.Control.Element.MatrixMixer")

        palette = self.palette()
        palette.setColor(QtGui.QPalette.Window, palette.color(QtGui.QPalette.Window).darker());
        self.setPalette(palette)

        rows = self.interface.getColCount()
        cols = self.interface.getRowCount()
        log.debug("Mixer has %i rows and %i columns" % (rows, cols))

        layout = QtGui.QGridLayout(self)
        self.setLayout(layout)

        self.rowHeaders = []
        self.columnHeaders = []
        self.items = []

        # Add row/column headers
        for i in range(cols):
            ch = MixerChannel(i, self, self.interface.getColName(i))
            self.connect(ch, QtCore.SIGNAL("couple"), self.coupleColumn)
            self.connect(ch, QtCore.SIGNAL("hide"), self.hideColumn)
            ch.canCouple(i+1!=cols)
            layout.addWidget(ch, 0, i+1)
            self.columnHeaders.append( ch )
        for i in range(rows):
            ch = MixerChannel(i, self, self.interface.getRowName(i))
            self.connect(ch, QtCore.SIGNAL("couple"), self.coupleRow)
            self.connect(ch, QtCore.SIGNAL("hide"), self.hideRow)
            ch.canCouple(i+1!=rows)
            layout.addWidget(ch, i+1, 0)
            self.rowHeaders.append( ch )

        # Add node-widgets
        for i in range(rows):
            self.items.append([])
            for j in range(cols):
                node = MixerNode(j, i, self)
                self.connect(node, QtCore.SIGNAL("valueChanged"), self.valueChanged)
                layout.addWidget(node, i+1, j+1)
                self.items[i].append(node)

        self.hiddenRows = []
        self.hiddenCols = []
        self.coupledRows = []
        self.coupledCols = []


    def checkVisibilities(self):
        for x in range(len(self.items)):
            for y in range(len(self.items[x])):
                self.items[x][y].setHidden( (x in self.hiddenRows) | (x in self.coupledRows) | (y in self.hiddenCols) | (y in self.coupledCols) )

    def coupleColumn(self, column, couple):
        if column+1 < len(self.columnHeaders):
            self.columnHeaders[column+1].setHidden(couple)
        if column > 0:
            self.columnHeaders[column-1].canCouple(not couple)
        if couple:
            self.coupledCols.append(column+1)
        else:
            self.coupledCols.remove(column+1)
        for row in self.items:
            row[column].addInputs(column+1,couple)
            row[column+1].addInputs(column+1,not couple)
        self.checkVisibilities()

    def coupleRow(self, row, couple):
        if row+1 < len(self.rowHeaders):
            self.rowHeaders[row+1].setHidden(couple)
        if row > 0:
            self.rowHeaders[row-1].canCouple(not couple)
        if couple:
            self.coupledRows.append(row+1)
        else:
            self.coupledRows.remove(row+1)
        for col in self.items[row]:
            col.addOutputs(row+1,couple)
        for col in self.items[row+1]:
            col.addOutputs(row+1,not couple)
        self.checkVisibilities()

    def hideColumn(self, column, hide):
        if hide:
            self.hiddenCols.append(column)
        else:
            self.hiddenCols.remove(column)
        self.checkVisibilities()
    def hideRow(self, row, hide):
        if hide:
            self.hiddenRows.append(row)
        else:
            self.hiddenRows.remove(row)
        self.checkVisibilities()

    def valueChanged(self, n):
        #log.debug("MatrixNode.valueChanged( %s )" % str(n))
        for tmp in n:
            self.interface.setValue(tmp[1], tmp[0], tmp[2])


#
# vim: et ts=4 sw=4
