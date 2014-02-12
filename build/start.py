#!/usr/bin/python

import sys
import os
import copy
import time
import math
import signal
import multiprocessing
import subprocess
import argparse
from pyrap.tables import table
import xml.etree.ElementTree as ET

from pyrap.tables import table

EM_START_PORT = 4100
MON_START_PORT = 4200
SERVER_XML   = '/tmp/' + os.getenv("USER") + '-server.xml'
EMULATOR_XML = '/tmp/' + os.getenv("USER") + '-emulator-%d.xml'
PIPELINE_XML = '/tmp/' + os.getenv("USER") + '-pipeline-%d.xml'

processes = []

def ms_data(MS):
  msdata = []
  for ms in MS:
    try:
      t = table(ms + "/SPECTRAL_WINDOW")
      msdata.append((ms, t[0]["CHAN_FREQ"][0], t[0]["CHAN_WIDTH"][0], t[0]["NUM_CHAN"]))
    except RuntimeError:
      sys.stderr.write("Error: `%s' is an invalid MS\n" % (ms))
      sys.exit(1)

  return msdata

def parse_subbands(subbands, maxchan):
  bands = subbands.split(',')

  n = 1000
  for band in bands:
    if len(band) == 1:
      return 1

    channels = band.split('-')
    channels = map(int, channels)
    assert(len(channels) == 2)
    assert(channels[0] < maxchan)
    assert(channels[1] < maxchan)
    assert(channels[1] >= channels[0])
    n = min(n, (channels[1] - channels[0]) + 1)

  return n

def sighandler(signum, frame):
  for p in processes:
    p.kill()
  sys.exit(0)

if __name__ == "__main__":
  signal.signal(signal.SIGTERM, sighandler)
  signal.signal(signal.SIGINT, sighandler)

  argparser = argparse.ArgumentParser(description=\
    'Starts an imaging pipeline. Press <Ctrl+C> to stop')
  argparser.add_argument('--threads', type=int, default=1, 
    help='max number of threads to spawn')
  argparser.add_argument('--tpldir', type=str,
    help='template directory, should host: {server,emulator,pipeline}Config.xml')
  argparser.add_argument('--output', type=str,
    help='output directory, where to store pipeline output, has to be a new directory')
  argparser.add_argument('--subbands', type=str, default="1-63",
    help='string of subbands with channels c in {0,...,63}, e.g. "1-8,5-15,16-63"')
  argparser.add_argument('MS', metavar='MS', nargs='+',
    help='atleast one measurementset to read from')
  argparser.add_argument('--dryrun', action='store_true', default=False,
    help='do not start pipeline, just show various output and generate xml')

  cmd_args = argparser.parse_args()

  server   = cmd_args.tpldir + '/serverConfig.xml'
  emulator = cmd_args.tpldir + '/emulatorConfig.xml'
  pipeline = cmd_args.tpldir + '/pipelineConfig.xml'

  if not cmd_args.dryrun:
    os.makedirs(cmd_args.output)
  if (not os.path.isfile(server)):
    sys.exit("Error: `%s' doesn't exist" % (server))
  if (not os.path.isfile(emulator)):
    sys.exit("Error: `%s' doesn't exist" % (emulator))
  if (not os.path.isfile(pipeline)):
    sys.exit("Error: `%s' doesn't exist" % (pipeline))

  # 1. Extract frequencies, channel width and number of channels
  msdata = ms_data(cmd_args.MS)

  # 2. Create the server xml and start it
  tree = ET.parse(server)
  root = tree.getroot()
  i = 0
  for data in msdata:
    streamnode = root[0][1][i]
    root[0][1].append(copy.deepcopy(streamnode))
    streamnode.set('name', 'Stream%d' % (i+1))
    streamnode[1].set('subbands', cmd_args.subbands)
    streamnode[1].set('numChannels', str(data[3]))
    streamnode[1].set('frequency', str(data[1]))
    streamnode[1].set('width', str(data[2]))
    streamnode[2].set('port', str(EM_START_PORT+i))
    i += 1
  root[0][1].remove(root[0][1][i])

  with open(SERVER_XML, 'w') as f:
    f.write('<?xml version="1.0" encoding="UTF-8"?>\n')
    f.write('<!DOCTYPE pelican>\n\n')
    tree.write(f, 'utf-8')
    f.close()

  if cmd_args.dryrun:
    print "Generated %s" % (SERVER_XML)
  else:
    processes.append(subprocess.Popen(['./aartfaac-server', SERVER_XML]))
    time.sleep(2)

  # 3. Compute number of pipelines to spawn, create the pipeline xml files and
  #    start the pipeline(s)
  threads = max(1, cmd_args.threads - 1) # server thread
  maxchan = max(msdata, key=lambda x: x[3])[3]
  num_threads = min(threads, parse_subbands(cmd_args.subbands, maxchan))
  num_pipelines = int(max(1, math.floor(threads/float(num_threads))))

  if cmd_args.dryrun:
    print "Starting %d pipelines using %d thread(s) each, %d total" % \
          (num_pipelines, num_threads, num_pipelines*num_threads)

  for i in range(num_pipelines):
    tree = ET.parse(pipeline)
    root = tree.getroot()
    node = root.find("pipeline")
    node.set("monport", str(MON_START_PORT+i))
    node.set("threads", str(num_threads))
    node = root[0].find("output").find("streamers").find("CasaImageStorage")
    node.set('active', 'true')
    node.find("output").set('path', cmd_args.output)
    filename = PIPELINE_XML % (i)
    with open(filename, 'w') as f:
      f.write('<?xml version="1.0" encoding="UTF-8"?>\n')
      f.write('<!DOCTYPE pelican>\n\n')
      tree.write(f, 'utf-8')
      f.close()
    if cmd_args.dryrun:
      print "Generated %s" % (filename)
    else:
      processes.append(subprocess.Popen(['./aartfaac-pipeline', filename]))
    
  if not cmd_args.dryrun:
    time.sleep(5)

  # 4. Setup the netcat processes
  for i in range(num_pipelines):
    filename = cmd_args.output + "/pipeline-%d.log" % (i) 
    if cmd_args.dryrun:
      print "Writing monitoring stats for pipeline %d to `%s'" % (i, filename)
    else:
      f = open(filename, "w")
      processes.append(subprocess.Popen(['nc', 'localhost', '%d' % (MON_START_PORT+i)], stdout=f))
  

  # 5. Setup xml for the emulators and start them
  i = 0
  for ms in cmd_args.MS:
    tree = ET.parse(emulator)
    root = tree.getroot()
    root[0][0].set('port', str(EM_START_PORT+i))
    root[0][1].set('name', ms)
    filename = EMULATOR_XML % (i)
    with open(filename, 'w') as f:
      f.write('<?xml version="1.0" encoding="UTF-8"?>\n')
      f.write('<!DOCTYPE pelican>\n\n')
      tree.write(f, 'utf-8')
      f.close()
    if cmd_args.dryrun:
      print "Generated %s" % (filename)
    else:
      processes.append(subprocess.Popen(['./aartfaac-emulator', filename]))
    i += 1

  # 6. Loop forever
  while not cmd_args.dryrun:
    time.sleep(1)
