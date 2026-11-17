// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mfem.hpp>
#include <umpire/ResourceManager.hpp>
#include "utils/communication.hpp"
#include "utils/device.hpp"
int main(int argc, char **argv)
{
  using namespace palace;
  Mpi::Init(argc, argv);
  const int id = utils::GetDeviceId(Mpi::World(), utils::GetDeviceCount());
  utils::ConfigureMemory("cuda", id);
  mfem::Device device("cuda", id);
  {
    mfem::Vector live(4096);
    live.UseDevice(true);
    live = 7.0;
    auto allocator = umpire::ResourceManager::getInstance().getAllocator(utils::UMPIRE_DEVICE_POOL_NAME);
    const auto before = allocator.getActualSize();
    void *scratch = allocator.allocate(1024ULL * 1024 * 1024);
    const auto peak = allocator.getActualSize();
    allocator.deallocate(scratch);
    const auto after = allocator.getActualSize();
    const auto *values = live.HostRead();
    for (int i = 0; i < live.Size(); i++) MFEM_VERIFY(values[i] == 7.0, "Live allocation corrupted");
    const char *flag = std::getenv("PALACE_GPU_POOL_RELEASE_UNUSED");
    const bool enabled = flag && std::strcmp(flag, "1") == 0;
    if (enabled)
    {
      MFEM_VERIFY(after < 256ULL * 1024 * 1024 && after < peak, "Unused cache not released");
    }
    else
    {
      MFEM_VERIFY(after >= 1024ULL * 1024 * 1024, "Default pool policy unexpectedly changed");
    }
    if (Mpi::Root(Mpi::World()))
      std::cout << "POOL_JSON {\"enabled\":" << enabled << ",\"beforeBytes\":" << before
                << ",\"peakBytes\":" << peak << ",\"afterBytes\":" << after << "}\n";
  }
  Mpi::Finalize();
}
