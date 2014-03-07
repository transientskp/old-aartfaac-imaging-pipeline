from string import Template
import subprocess
import tempfile
import argparse
import os

EMULATOR_CMD = "aartfaac-emulator"
TEMPLATE_XML = Template("""
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE pelican>

<configuration version="1.0">
    <StreamEmulator     name="${name}">
        <connection     host="${host}" port="${port}" />
        <measurementset name="${ms}" />
        <emulator       packetInterval="${packet_interval}"/>
    </StreamEmulator>
</configuration>
""".strip())

def generate_xml(**kwargs):
    """Returns a temporary file object"""
    xmlfile = tempfile.NamedTemporaryFile()
    xmlfile.write(TEMPLATE_XML.substitute(**kwargs))
    xmlfile.flush()
    return xmlfile

def read_environment(variable_name="AARTFAAC_SERVER_PORT"):
    """Returns (host, port)"""
    # If defined, we expect the environment variable to be set by Docker to
    # something in the format "tcp://hostname:port".
    if variable_name in os.environ:
        return os.environ[variable_name][6:].split(":")
    else:
        return "localhost", "4100"

def get_configuration():
    """Rerturns a populated configuration"""
    default_host, default_port = read_environment()
    parser = argparse.ArgumentParser()
    parser.add_argument("--stream-name", help="Stream name", default="O1")
    parser.add_argument("--server-host", help="Server hostname", default=default_host)
    parser.add_argument("--server-port", help="Server port", default=default_port)
    parser.add_argument("--packet-interval", help="Packet interval (ms)", type=int, default=10)
    parser.add_argument("ms", help="MeasurementSet containing data to send")
    return parser.parse_args()

if __name__ == "__main__":
    config = get_configuration()

    # Sanity check: input data exists
    assert(os.path.exists(config.ms))

    xmlfile = generate_xml(
        name=config.stream_name,
        host=config.server_host,
        port=config.server_port,
        ms=config.ms,
        packet_interval=config.packet_interval
    )
    subprocess.call([EMULATOR_CMD, xmlfile.name])
