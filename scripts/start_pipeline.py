from string import Template
import subprocess
import tempfile
import argparse
import os

PIPELINE_CMD = "aartfaac-pipeline"
TEMPLATE_XML = Template("""
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE pelican>

<configuration version="1.0">
  <pipeline monport="${monitor_port}" threads="${nthreads}">
    <clients>
      <PelicanServerClient>
        <server host="${server_host}" port="${server_port}"/>
        <data type="StreamBlob" adapter="StreamAdapter"/>
      </PelicanServerClient>
    </clients>

    <adapters>
      <StreamAdapter>
        <!-- No specific settings -->
      </StreamAdapter>
    </adapters>

    <modules>
      <Flagger>
        <deviation multiplier="${flagger_deviation_multiplier}" />
      </Flagger>

      <Calibrator>
        <positrf path="${antenna_positions}" />
      </Calibrator>

      <Imager>
        <positrf path="${antenna_positions}" />
      </Imager>
    </modules>

    <output>
      <streamers>
        <TiffStorage active="${store_tiff}">
          <output path="${output_path}" />
        </TiffStorage>
        <CasaImageStorage active="${store_casa}">
          <output path="${output_path}" />
        </CasaImageStorage>
      </streamers>
      <dataStreams>
        <stream name="post" listeners="TiffStorage"/>
        <stream name="post" listeners="CasaImageStorage"/>
      </dataStreams>
    </output>
  </pipeline>
</configuration>
""".strip())

def generate_xml(**kwargs):
    """Returns a temporary file object"""
    xmlfile = tempfile.NamedTemporaryFile(delete=False)
    xmlfile.write(TEMPLATE_XML.substitute(**kwargs))
    xmlfile.flush()
    return xmlfile

def get_configuration():
    """Rerturns a populated configuration"""
    parser = argparse.ArgumentParser()
    parser.add_argument("--nthreads", help="Number of threads", type=int, default=1)
    parser.add_argument("--monitor-port", help="Port on which to publish monitoring data", default="4200")
    parser.add_argument("--server-host", help="Address of Pelican server", default="127.0.0.1")
    parser.add_argument("--server-port", help="Address of Pelican server", default="2000")
    parser.add_argument("--flagger-deviation-multiplier", help="???", type=int, default=4)
    parser.add_argument("--antenna-positions",
        help="Path to file containing ordered IRTF positions of all antennae",
        default="../data/posITRF.dat" # TODO: Use installed version of this file.
    )
    parser.add_argument("--casa", help="Store CASA images", action="store_true")
    parser.add_argument("--tiff", help="Store TIFF images", action="store_true")
    parser.add_argument("output_path", help="Root directory for image output")
    return parser.parse_args()

if __name__ == "__main__":
    config = get_configuration()

    # Sanity check: we need at least one of CASA and TIFF output
    assert(config.casa or config.tiff)

    # Sanity check: are the antenna positions available?
    print config.antenna_positions
    assert(os.path.exists(config.antenna_positions))

    # Ensure the output directory exists
    if not os.path.exists(config.output_path):
        os.makedirs(output_path)

    xmlfile = generate_xml(
        nthreads=config.nthreads,
        monitor_port=config.monitor_port,
        server_host=config.server_host,
        server_port=config.server_port,
        flagger_deviation_multiplier=config.flagger_deviation_multiplier,
        antenna_positions=os.path.abspath(config.antenna_positions),
        store_tiff=config.tiff,
        store_casa=config.casa,
        output_path=config.output_path
    )
    subprocess.call([PIPELINE_CMD, xmlfile.name])
