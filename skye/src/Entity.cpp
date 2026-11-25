#include "skye/Entity.hpp"

namespace skye {

EntityID EntityRegistry::CreateEntity() {
  return entityIDs_.CreateID();
}

void EntityRegistry::DestroyEntity( EntityID entityID ) {
  entityIDs_.DestroyID( entityID );
}

}
