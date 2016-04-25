/*
 * Copyright (c) 2013, The University of Oxford
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the University of Oxford nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <pelican/comms/PelicanProtocol.h>
#include <pelican/utility/Config.h>
#include "DirectDataClient.h"

#include "StreamChunker.h"
#include "StreamAdapter.h"

#include <iostream>

DirectDataClient::DirectDataClient(const pelican::ConfigNode &node,
                                   const pelican::DataTypes &types, const pelican::Config *config)
  : DirectStreamDataClient(node, types, config)
{
  pelican::Config::TreeAddress address;
  address << pelican::Config::NodeId("configuration", "")
          << pelican::Config::NodeId("pipeline", "")
          << pelican::Config::NodeId("chunkers", "");

  pelican::ConfigNode chunker = config->get(address);
  QList<pelican::ConfigNode> streams = chunker.getNodes("StreamChunker");

  try
  {
    for (int i = 0; i < streams.count(); i++)
      addStreamChunker("StreamChunker", streams.at(i).name());
  }
  catch (const QString &err)
  {
    std::cerr << "ERROR: " << err.toStdString() << std::endl;
  }
}

DirectDataClient::~DirectDataClient()
{
}

