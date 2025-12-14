#pragma region Apache License 2.0
/*
Nuclex Native Framework
Copyright (C) 2002-2024 Markus Ewald / Nuclex Development Labs

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#pragma endregion // Apache License 2.0

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "./StandardInstanceSet.h"

#include <array> // for std::array
#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Custom alloctor that allocates a standard instance set</summary>
  /// <typeparam name="TInstanceSet">
  ///   Type that will be allocated, expected to derive from InstanceSet
  /// </typeparam>
  /// <remarks>
  ///   Normally, a non-templated implementation of this allocator would seem to suffice,
  ///   but <code>std::allocate_shared()</code> implementations will very likely
  ///   (via the type-changing copy constructor) allocate a type inherited from our
  ///   <see cref="InstanceSet" /> that packages the reference counter of
  ///   the <code>std::shared_ptr</code> together with the instance.
  /// </remarks>
  template<class TInstanceSet>
  class InstanceSetAllocator {

    /// <summary>Type of element the allocator is for, required by standard</summary>
    public: typedef TInstanceSet value_type;

    /// <summary>Initializes a new allocator using the specified appended list size</summary>
    /// <param name="instanceCount">Number of instances to allocate extra space for</param>
    public: InstanceSetAllocator(std::size_t instanceCount) :
      instanceCount(instanceCount) {}

    /// <summary>
    ///   Creates this allocator as a clone of an allocator for a different type
    /// </summary>
    /// <typeparam name="TOther">Type the existing allocator is allocating for</typeparam>
    /// <param name="other">Existing allocator whose attributes will be copied</param>
    public: template<class TOther> InstanceSetAllocator(
      const InstanceSetAllocator<TOther> &other
    ) : instanceCount(other.instanceCount) {}

    /// <summary>Allocates memory for the specified number of elements (must be 1)</summary>
    /// <param name="count">Number of elements to allocate memory for (must be 1)</param>
    /// <returns>The allocated (but not initialized) memory for the requested type</returns>
    public: TInstanceSet *allocate(std::size_t count) {
      NUCLEX_SUPPORT_NDEBUG_UNUSED(count);
      assert(count == 1);

      // The base footprint is the size of TInstanceSet, which (at the time this is
      // called) will be the StandardInstanceSet owned by or wrapped in a class that
      // includes the control block for the std::shared_ptr<>
      std::size_t requiredByteCount = sizeof(TInstanceSet);

      // On top of that, we allocate space for the std::atomic<bool>s that track which
      // services have been created and the service instances in an std::any array.
      requiredByteCount += sizeof(std::atomic<bool>[2]) * this->instanceCount / 2;
      requiredByteCount += alignof(std::atomic<bool>) - 1; // padding space for worst case misalignment
      requiredByteCount += sizeof(std::any[2]) * this->instanceCount / 2;
      requiredByteCount += alignof(std::any) - 1; // padding space for worst case misalignment

      return reinterpret_cast<TInstanceSet *>(new std::byte[requiredByteCount]);
    }

    /// <summary>Frees memory for the specified element (count must be 1)</summary>
    /// <param name="instance">Instance for which memory will be freed</param>
    /// <param name="count">Number of instances that will be freed (must be 1)</param>
    public: void deallocate(TInstanceSet *instance, std::size_t count) {
      NUCLEX_SUPPORT_NDEBUG_UNUSED(count);
      assert(count == 1);

      delete[] reinterpret_cast<std::byte *>(instance);
    }

    /// <summary>Number of service insances for which extra space will be allocated</summary>
    public: std::size_t instanceCount;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::Support::Services2 {

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<StandardInstanceSet> StandardInstanceSet::Create(
    const std::shared_ptr<const StandardBindingSet> &bindings,
    const StandardBindingSet::TypeIndexBindingMultiMap &ownBindings
  ) {
    std::shared_ptr<StandardInstanceSet> serviceBindings = (
      std::allocate_shared<StandardInstanceSet>(
        InstanceSetAllocator<StandardInstanceSet>(ownBindings.size()), bindings, ownBindings
      )
    );

    return serviceBindings;
  }

  // ------------------------------------------------------------------------------------------- //

  StandardInstanceSet::StandardInstanceSet(
    const std::shared_ptr<const StandardBindingSet> &bindings,
    const StandardBindingSet::TypeIndexBindingMultiMap &ownBindings
  ) :
    Bindings(bindings),
    OwnBindings(ownBindings),
    ChangeMutex(),
    PresenceFlags(nullptr),
    Instances(nullptr) {
    std::uintptr_t address = reinterpret_cast<std::uintptr_t>(this);

    // Figure out where the presence flags start. The ::Create() factory method will have
    // used a custom shared_ptr allocator that allocated enough memory for the presence flags
    // and std::any array to trail the memory we have allocated plus alignment padding.
    {
      address += sizeof(Nuclex::Support::Services2::StandardInstanceSet);

      std::uintptr_t misalignment = address % alignof(std::atomic<bool>);
      if(misalignment != 0) {
        address += alignof(std::atomic<bool>) - misalignment;
      }

      this->PresenceFlags = reinterpret_cast<std::atomic<bool> *>(address);
    }

    // CHECK: If any of the constructors throw, we'd have to destruct again
    //   But 'new std::atomic<bool>' is unlkely to throw, especially since we don't allocate

    // Initialize the presence flags
    {
      std::size_t instanceCount = ownBindings.size();

      for(std::size_t index = 0; index < instanceCount; ++index) {
        new(reinterpret_cast<std::byte *>(this->PresenceFlags + index))
        std::atomic<bool>(false);
      }
      // CHECK: If would be cool if we could just memset / std::fill_n() to zero
      //   If there was any guarantee that std::atomic<bool> is just an actual bool,
      //   the constructor call could theoretically be ommitted...
    }

    // Behind the presence flags, the array of instances stored as std::any values follow.
    {
      address += sizeof(std::atomic_bool[2]) * ownBindings.size() / 2;
    
      std::uintptr_t misalignment = address % alignof(std::any);
      if(misalignment != 0) {
        address += alignof(std::any) - misalignment;
      }

      this->Instances = reinterpret_cast<std::any *>(address);
    }

    // Note: we do not initialize the std::any instances. While we could have an array
    // of std::any instances containing nothing, why bother? We only initialize those
    // that we actually fill with service instances.
  }

  // ------------------------------------------------------------------------------------------- //

  StandardInstanceSet::~StandardInstanceSet() {
    std::size_t instanceCount = this->OwnBindings.size();
    while(0 < instanceCount) {
      --instanceCount;
      if(this->PresenceFlags[instanceCount].load(std::memory_order::consume)) {
        this->Instances[instanceCount].~any();
      }
      this->PresenceFlags[instanceCount].~atomic();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  const std::any &StandardInstanceSet::CreateOrFetchServiceInstance(
    ServiceProvider &serviceProvider,
    const StandardBindingSet::TypeIndexBindingMultiMap::const_iterator &service
  ) {
    std::size_t uniqueServiceIndex = service->second.UniqueServiceIndex;

    // Check, without locking, if the instance has already been created. If so,
    // there's not need to enter the mutex since we're not modifying our state.
    bool isAlreadyCreated = this->PresenceFlags[uniqueServiceIndex].load(
      std::memory_order::consume
    );
    if(!isAlreadyCreated) [[unlikely]] {
      std::unique_lock<std::mutex> changeMutexLocklScope(this->ChangeMutex);

      // Before entering the mutex, no instance of the service has been created. However,
      // another thread could have been faster, so check again from inside the mutex were
      // only one thread can enter at a time. This ensures the service is only constructed
      // once and not modified while other threads are in the process of fetching it.
      isAlreadyCreated = this->PresenceFlags[uniqueServiceIndex].load(
        std::memory_order::consume
      );
      if(!isAlreadyCreated) [[likely]] {
        if(service->second.ProvidedInstance.has_value()) [[unlikely]] {
          this->Instances[uniqueServiceIndex] = service->second.CloneFactory(
            service->second.ProvidedInstance
          );
        } else {
          this->Instances[uniqueServiceIndex] = service->second.Factory(serviceProvider);
        }

        this->PresenceFlags[uniqueServiceIndex].store(true, std::memory_order::release);
      } // second check inside mutex lock
    } // first opportunistic check without mutex lock

    return this->Instances[uniqueServiceIndex];
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services2
