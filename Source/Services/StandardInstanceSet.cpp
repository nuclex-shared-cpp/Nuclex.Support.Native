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

#include "Nuclex/Support/Errors/CyclicDependencyError.h"
#include "Nuclex/Support/ScopeGuard.h"

#include <algorithm> // for std::find()
#include <stdexcept> // for std::logic_error
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

namespace Nuclex::Support::Services {

  // ------------------------------------------------------------------------------------------- //

  StandardInstanceSet::ResolutionContext::ResolutionContext(
    StandardInstanceSet &services,
    const std::type_index &outerServiceType
  ) :
    services(services),
    resolutionStack() {
    // This ensures a cyclic dependency is also detected if a service depends on itself
    // even if indirectly through another dependency
    this->resolutionStack.emplace_back(outerServiceType);
  }

  // ------------------------------------------------------------------------------------------- //

  StandardInstanceSet::ResolutionContext::~ResolutionContext() = default;

  // ------------------------------------------------------------------------------------------- //

  std::shared_ptr<ServiceScope> StandardInstanceSet::ResolutionContext::CreateScope() {
    // This would make no sense. Any service scope created inside of a service factory
    // would have to be gone by the time service resolution finishes (unless you involve
    // global variables or state passed through lambdas).
    #if 0
    return std::make_shared<StandardServiceScope>(
      StandardInstanceSet::Create(
        this->services->Bindings, this->services->Bindings->ScopedServices
      ),
      this->services
    );
    #endif

    throw std::logic_error(
      reinterpret_cast<const char *>(u8"Cannot create scopes from a service factory")
    );
  }

  // ------------------------------------------------------------------------------------------- //

  std::any StandardInstanceSet::ResolutionContext::TryGetService(
    const std::type_info &serviceType
  ) {
    // TODO: Implement StandardInstanceSet::ResolutionContext::TryGetService() method
    throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
  }

  // ------------------------------------------------------------------------------------------- //

  std::any StandardInstanceSet::ResolutionContext::GetService(
    const std::type_info &serviceType
  ) {
    const std::type_index serviceIndex(serviceType);

    // Detect circular dependencies
    {
      std::vector<std::type_index>::const_iterator duplicate = std::find(
        this->resolutionStack.begin(), this->resolutionStack.end(), serviceIndex
      );
      if(duplicate != this->resolutionStack.end()) [[unlikely]] {
        throw Errors::CyclicDependencyError(u8"Service dependency cycle detected");
      }
    }

    // Put the next service on the stack and try to resolve it. This guarantees that,
    // should a dependency cycle involving this service type happen, the cyclic
    // dependency error is detected.
    this->resolutionStack.emplace_back(serviceIndex);
    {
      ON_SCOPE_EXIT { this->resolutionStack.pop_back(); };

      // TODO: Implement StandardInstanceSet::ResolutionContext::TryGetService() method
      throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::function<std::any()> StandardInstanceSet::ResolutionContext::GetServiceFactory(
    const std::type_info &serviceType
  ) const {
    (void)serviceType;
    // TODO: Implement StandardInstanceSet::ResolutionContext::GetServiceFactory() method
    throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::any> StandardInstanceSet::ResolutionContext::GetServices(
    const std::type_info &serviceType
  ) {
    (void)serviceType;
    // TODO: Implement StandardInstanceSet::ResolutionContext::GetServices() method
    throw std::runtime_error(reinterpret_cast<const char *>(u8"Not implemented yet"));
  }

  // ------------------------------------------------------------------------------------------- //

  std::any StandardInstanceSet::ResolutionContext::FetchOrActivateSingletonService(
    const StandardBindingSet::TypeIndexBindingMultiMap::const_iterator &serviceIterator
  ) {
    std::size_t uniqueServiceIndex = serviceIterator->second.UniqueServiceIndex;

    // Check, without locking, if the instance has already been created. If so,
    // there's not need to enter the mutex since we're not modifying our state.
    bool isAlreadyCreated = this->services.PresenceFlags[uniqueServiceIndex].load(
      std::memory_order::consume
    );
    if(isAlreadyCreated) [[likely]] {
      return this->services.Instances[uniqueServiceIndex];
    }

    // This is the service resolution context, meaning that the service provider already
    // needed to look up the first service and this is a sub-dependency. When this code
    // runs, the root service provider is currently holding the mutex lock. So we do not
    // need double-checked locking here and are allowed to modify the instances array.

    // If an existing instance was provided, just put it in place without worrying about it
    if(serviceIterator->second.ProvidedInstance.has_value()) [[unlikely]] {
      this->services.Instances[uniqueServiceIndex] = serviceIterator->second.ProvidedInstance;
    } else {
      new(this->services.Instances + uniqueServiceIndex) std::any(
        std::move(serviceIterator->second.Factory(*this))
      );
    }

    this->services.PresenceFlags[uniqueServiceIndex].store(true, std::memory_order::release);

    return this->services.Instances[uniqueServiceIndex];
  }

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
    emptyAny(),
    ChangeMutex(),
    PresenceFlags(nullptr),
    Instances(nullptr) {
    std::uintptr_t address = reinterpret_cast<std::uintptr_t>(this);

    // Figure out where the presence flags start. The ::Create() factory method will have
    // used a custom shared_ptr allocator that allocated enough memory for the presence flags
    // and std::any array to trail the memory we have allocated plus alignment padding.
    {
      address += sizeof(Nuclex::Support::Services::StandardInstanceSet);

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
      //   the constructor call could theoretically be omitted...
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

  const std::any &StandardInstanceSet::TryFetchOrCreateSingletonServiceInstance(
    const std::type_index &serviceTypeIndex
  ) {
    // TODO: This should look up the *last* service of the type in the multimap
    StandardBindingSet::TypeIndexBindingMultiMap::const_iterator serviceIterator = (
      this->Bindings->SingletonServices.find(serviceTypeIndex)
    );
    if(serviceIterator == this->Bindings->SingletonServices.end()) {
      return this->emptyAny;
    }

    std::size_t uniqueServiceIndex = serviceIterator->second.UniqueServiceIndex;

    // Check, without locking, if the instance has already been created. If so,
    // there's no need to enter the mutex since we're not modifying our state.
    bool isAlreadyCreated = this->PresenceFlags[uniqueServiceIndex].load(
      std::memory_order::consume
    );
    if(!isAlreadyCreated) [[unlikely]] {
      std::unique_lock<std::mutex> changeMutexLockScope(this->ChangeMutex);

      // Before entering the mutex, no instance of the service had been created. However,
      // another thread could have been faster, so check again from inside the mutex where
      // only one thread can enter at a time. This ensures the service is only constructed
      // once and no other threads are in danger of handling the 'std::any' simultaneously.
      isAlreadyCreated = this->PresenceFlags[uniqueServiceIndex].load(
        std::memory_order::consume
      );
      if(!isAlreadyCreated) [[likely]] {
        if(serviceIterator->second.ProvidedInstance.has_value()) [[unlikely]] {
          new(this->Instances + uniqueServiceIndex) std::any(
            serviceIterator->second.CloneFactory(serviceIterator->second.ProvidedInstance)
          );
        } else {
          ResolutionContext nestedServiceProvider(*this, serviceTypeIndex);
          new(this->Instances + uniqueServiceIndex) std::any(
            std::move(serviceIterator->second.Factory(nestedServiceProvider))
          );
        }

        this->PresenceFlags[uniqueServiceIndex].store(true, std::memory_order::release);
      } // second check inside mutex lock
    } // first opportunistic check without mutex lock

    return this->Instances[uniqueServiceIndex];
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::Support::Services
