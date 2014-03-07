from string import Template
from itertools import chain
import subprocess
import tempfile
import argparse
import os

SERVER_CMD = "aartfaac-server"
TEMPLATE_XML = Template("""
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE pelican>

<configuration version="1.0">
  <server>
    <buffers>
      <StreamBlob>
        <buffer maxSize="${buffer_max_size}" maxChunkSize="${buffer_max_chunk_size}"/>
      </StreamBlob>
    </buffers>

    <chunkers>
      <StreamChunker name="${stream_chunker_name}">
        <data type="StreamBlob"/>
        <stream subbands="${subbands}"
                numChannels="${num_channels}"
                frequency="${start_frequency}"
                width="${channel_width}" />
        <connection host="${input_host}"
                    port="${input_port}"
                    timeout="${input_timeout}" />
      </StreamChunker>
    </chunkers>
  </server>
</configuration>
""".strip())

def generate_xml(save_xml, **kwargs):
    """Returns a temporary file object"""
    xmlfile = tempfile.NamedTemporaryFile(delete=(not save_xml))
    xmlfile.write(
        TEMPLATE_XML.substitute(**kwargs)
    )
    xmlfile.flush()
    if save_xml:
        print "Saved XML file as %s" % (xmlfile.name,)
    return xmlfile

def get_configuration():
    """Rerturns a populated configuration"""
    parser = argparse.ArgumentParser()
    parser.add_argument("--buffer-max-size", help="???", default=4294967296)
    parser.add_argument("--buffer-max-chunk-size", help="???", default=4294967296)
    parser.add_argument("--stream-chunker-name", help="StreamChunker name", default="Stream1")
    parser.add_argument("--input-host", help="Address to listen for incoming data", default="0.0.0.0")
    parser.add_argument("--input-port", help="Port to listen for incoming data", default="4100")
    parser.add_argument("--input-timeout", help="???", default="4100")
    parser.add_argument("--save-xml", help="Don't delete XML file", action="store_true")
    parser.add_argument("num_channels", help="Number of channels", type=int)
    parser.add_argument("start_frequency", help="Frequency of first channel (Hz)", type=float)
    parser.add_argument("channel_width", help="Width of each channel (Hz)", type=float)
    parser.add_argument("subband", help="Define a subband: <first channel>-<last channel> (inclusive)", type=lambda x: x.split('-'), nargs="*")
    return parser.parse_args()

if __name__ == "__main__":
    config = get_configuration()

    # Sanity check on subband definitions: no channels less than 0
    # or greater than the maximum.
    assert min(int(x) for x in chain(*config.subband)) >= 0
    assert max(int(x) for x in chain(*config.subband)) < int(config.num_channels)

    xmlfile = generate_xml(
        config.save_xml,
        buffer_max_size=config.buffer_max_size,
        buffer_max_chunk_size=config.buffer_max_chunk_size,
        stream_chunker_name=config.stream_chunker_name,
        subbands=",".join("-".join(x) for x in config.subband),
        num_channels=config.num_channels,
        start_frequency=config.start_frequency,
        channel_width=config.channel_width,
        input_host=config.input_host,
        input_port=config.input_port,
        input_timeout=config.input_timeout
    )
    subprocess.call([SERVER_CMD, xmlfile.name])
