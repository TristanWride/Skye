#include "skye/SparseSet.hpp"

#include "skye/Util.hpp"

namespace skye {

void SparseSet::Insert( EntityID entity, ComponentID componentID ) {
  Position & slot = SlotFor( entity );
  if ( slot != NO_POSITION ) {
    util::Fail( "Entity {:d} cannot be inserted: its index is already held by entity {:d}.",
                entity, dense_[slot].entity );
  }

  slot = static_cast<Position>( dense_.size() );
  dense_.push_back( Entry{ .entity = entity, .componentID = componentID } );
}

void SparseSet::Remove( EntityID entity ) {
  const Position position = PositionOf( entity );
  if ( position == NO_POSITION || dense_[position].entity != entity )
    util::Fail( "Entity {:d} cannot be removed: it is not in the set.", entity );

  const Entry last       = dense_.back();
  SlotFor( last.entity ) = position;
  SlotFor( entity )      = NO_POSITION;
  dense_[position]       = last;
  dense_.pop_back();
}

bool SparseSet::Contains( EntityID entity ) const noexcept {
  return Find( entity ).has_value();
}

std::optional<ComponentID> SparseSet::Find( EntityID entity ) const noexcept {
  const Position position = PositionOf( entity );
  if ( position == NO_POSITION || dense_[position].entity != entity ) return std::nullopt;

  return dense_[position].componentID;
}

SparseSet::size_type SparseSet::Size() const noexcept {
  return dense_.size();
}

bool SparseSet::Empty() const noexcept {
  return dense_.empty();
}

SparseSet::size_type SparseSet::AllocatedPages() const noexcept {
  auto count = size_type{ 0 };
  for ( const auto & page : pages_ )
    if ( page ) ++count;
  return count;
}

std::span<const SparseSet::Entry> SparseSet::Entries() const noexcept {
  return dense_;
}

SparseSet::const_iterator SparseSet::begin() const noexcept {
  return dense_.begin();
}

SparseSet::const_iterator SparseSet::end() const noexcept {
  return dense_.end();
}

SparseSet::Position SparseSet::PositionOf( EntityID entity ) const noexcept {
  const EntityID  index = EntityIDs::IndexOf( entity );
  const size_type page  = index / PAGE_SIZE;
  if ( page >= pages_.size() || !pages_[page] ) return NO_POSITION;

  return ( *pages_[page] )[index % PAGE_SIZE];
}

SparseSet::Position & SparseSet::SlotFor( EntityID entity ) {
  const EntityID  index = EntityIDs::IndexOf( entity );
  const size_type page  = index / PAGE_SIZE;
  if ( page >= pages_.size() )
    pages_.resize( page + 1 );

  if ( !pages_[page] ) {
    pages_[page] = std::make_unique<Page>();
    pages_[page]->fill( NO_POSITION );
  }

  return ( *pages_[page] )[index % PAGE_SIZE];
}

}
