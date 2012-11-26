#!/usr/bin/env python

# Quick and dirty script to plot the array correlation matrix for the first
# timestep in a MeasurementSet to help with AARTFAAC imaging pipeline
# debugging.
#
# Requires numpy, pyrap, matplotlib.

import sys
import numpy
from matplotlib import pyplot
from pyrap.tables import table

# Read the name of the MS to plot from the command line
t = table(sys.argv[1])

# Figure out the number of baselines
t_ant = t.sort('unique ANTENNA1')
n_ant = len(t_ant.getcol("ANTENNA1"))
n_bl = n_ant*(n_ant+1)/2 # including autocorrelations

# We fetch only the number of rows equivalent to the first timestep in the MS,
# assuming it's ordered by time.
data = numpy.sqrt(numpy.abs(t.getcolslice("DATA", [0,0], [0,0], nrow=n_bl)))

# Ugly loop over the antenna values to build a square correlation matrix for
# passing to matplotlib, which wants a square array for plotting. Probably
# some smart use of numpy could do this much more elegantly.
to_plot = numpy.zeros((n_ant,n_ant))
for i, antpair in enumerate(zip(t.getcol("ANTENNA1")[:n_bl], t.getcol("ANTENNA2")[:n_bl])):
    to_plot[antpair[0], antpair[1]] = data[i]
    to_plot[antpair[1], antpair[0]] = data[i]

# Save to file acm.png
fig = pyplot.figure()
pyplot.imshow(to_plot)
fig.savefig("acm.png")
