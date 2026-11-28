#include "skye/Entity.hpp"

#include "skye/Util.hpp"

#include <utility>

namespace skye {

EntityID EntityRegistry::CreateEntity() {
  return entityIDs_.CreateID();
}

void EntityRegistry::DestroyEntity( EntityID entityID ) {
  // Fails here, with the allocator's description of the problem, before any component is released.
  if ( !entityIDs_.IsAlive( entityID ) )
    entityIDs_.DestroyID( entityID );

  for ( const auto & type : componentTypes_ ) {
    if ( !type ) continue;

    const std::optional<ComponentID> componentID = type->set.Find( entityID );
    if ( !componentID ) continue;

    type->release( entityID, *componentID );
    type->set.Remove( entityID );
  }

  entityIDs_.DestroyID( entityID );
}

bool EntityRegistry::IsAlive( EntityID entityID ) const noexcept {
  return entityIDs_.IsAlive( entityID );
}

void EntityRegistry::RegisterComponentType( ComponentTypeID type, ReleaseCallback release ) {
  if ( IsComponentTypeRegistered( type ) )
    util::Fail( "Component type {:d} is already registered.", type );

  if ( !release )
    util::Fail( "Component type {:d} cannot be registered without a release callback.", type );

  if ( type >= componentTypes_.size() )
    componentTypes_.resize( type + 1 );

  componentTypes_[type] = std::make_unique<ComponentType>( ComponentType{ .set = SparseSet{}, .release = std::move( release ) } );
}

void EntityRegistry::UnregisterComponentType( ComponentTypeID type ) {
  if ( !IsComponentTypeRegistered( type ) )
    util::Fail( "Component type {:d} is not registered.", type );

  componentTypes_[type].reset();
}

bool EntityRegistry::IsComponentTypeRegistered( ComponentTypeID type ) const noexcept {
  return type < componentTypes_.size() && componentTypes_[type] != nullptr;
}

void EntityRegistry::AddComponent( ComponentTypeID type, EntityID entityID, ComponentID componentID ) {
  SparseSet & set = SetOf( type );
  if ( !entityIDs_.IsAlive( entityID ) )
    util::Fail( "Entity {:d} is not alive and cannot be given a component of type {:d}.", entityID, type );

  set.Insert( entityID, componentID );
}

SparseSet & EntityRegistry::SetOf( ComponentTypeID type ) {
  return const_cast<SparseSet &>( std::as_const( *this ).SetOf( type ) );
}

const SparseSet & EntityRegistry::SetOf( ComponentTypeID type ) const {
  if ( !IsComponentTypeRegistered( type ) )
    util::Fail( "Component type {:d} is not registered.", type );

  return componentTypes_[type]->set;
}

}
