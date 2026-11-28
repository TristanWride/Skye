#pragma once

#include "skye/Component.hpp"
#include "skye/EntityID.hpp"
#include "skye/SparseSet.hpp"

#include <array>
#include <cstddef>
#include <iterator>
#include <optional>
#include <tuple>

namespace skye {

namespace detail {

// Maps any component type to ComponentID, to build one ComponentID per type in a pack.
template <class>
using AsComponentID = ComponentID;

}

// Range of ( EntityID, ComponentID... ) tuples, one ComponentID per Ts, for every entity that has all of Ts.
// Walks the smallest of the sets and looks each entity up in the others. Get one from EntityRegistry::View().
// Iterators hold pointers to the registry's sets: adding or removing components or entities while iterating is not
// allowed, and the view must not outlive the registry.
template <class... Ts>
class ComponentView {
public:
  static_assert( sizeof...( Ts ) > 0, "A view needs at least one component type." );

  static constexpr size_t TYPE_COUNT = sizeof...( Ts );

  using value_type = std::tuple<EntityID, detail::AsComponentID<Ts>...>;
  using Sets       = std::array<const SparseSet *, TYPE_COUNT>;

  class Iterator {
  public:
    using value_type       = ComponentView::value_type;
    using reference        = const value_type &;
    using difference_type  = std::ptrdiff_t;
    using iterator_concept = std::forward_iterator_tag;

  private:
    Sets       sets_{};
    size_t     smallest_{ 0 };
    size_t     position_{ 0 };
    value_type current_{};

  public:
    // Past-the-end iterator of no view, as forward iterators require.
    explicit Iterator() = default;

    // Starts at the first entity of the smallest set that has all the components.
    explicit Iterator( const Sets & sets, size_t smallest )
      : sets_{ sets }
      , smallest_{ smallest } {
      SkipToMatch();
    }

    // Returns ( EntityID, ComponentID... ) for the current entity.
    [[nodiscard]] reference operator*() const noexcept {
      return current_;
    }

    // Moves to the next entity that has all the components.
    Iterator & operator++() {
      ++position_;
      SkipToMatch();
      return *this;
    }

    // Moves to the next entity that has all the components, returning the previous position.
    Iterator operator++( int ) {
      Iterator previous = *this;
      ++*this;
      return previous;
    }

    // Returns whether two iterators of the same view are at the same entity.
    [[nodiscard]] bool operator==( const Iterator & other ) const noexcept {
      return position_ == other.position_;
    }

    // Returns whether the iteration is over.
    [[nodiscard]] bool operator==( std::default_sentinel_t ) const noexcept {
      return sets_[smallest_] == nullptr || position_ >= sets_[smallest_]->Size();
    }

  private:
    // Advances position_ to the next entity present in every set and loads its IDs into current_.
    void SkipToMatch() {
      if ( sets_[smallest_] == nullptr ) return;

      const auto entries = sets_[smallest_]->Entries();
      for ( ; position_ < entries.size(); ++position_ )
        if ( TryLoad( entries[position_] ) ) return;
    }

    // Loads the entry's IDs into current_ if every set holds its entity.
    [[nodiscard]] bool TryLoad( const SparseSet::Entry & entry ) {
      auto ids = std::array<ComponentID, TYPE_COUNT>{};
      for ( size_t i = 0; i < TYPE_COUNT; ++i ) {
        if ( i == smallest_ ) {
          ids[i] = entry.componentID;
          continue;
        }

        const std::optional<ComponentID> id = sets_[i]->Find( entry.entity );
        if ( !id ) return false;
        ids[i] = *id;
      }

      current_ = std::apply( [&]( auto... id ) { return value_type{ entry.entity, id... }; }, ids );
      return true;
    }
  };

private:
  Sets   sets_;
  size_t smallest_{ 0 };

public:
  // Views the entities present in all of sets, one set per Ts in order. Every pointer must be non-null.
  explicit ComponentView( const Sets & sets ) noexcept
    : sets_{ sets } {
    for ( size_t i = 1; i < TYPE_COUNT; ++i )
      if ( sets_[i]->Size() < sets_[smallest_]->Size() )
        smallest_ = i;
  }

  // Returns an iterator to the first entity that has all the components.
  [[nodiscard]] Iterator begin() const {
    return Iterator{ sets_, smallest_ };
  }

  // Returns the sentinel that ends the iteration.
  [[nodiscard]] std::default_sentinel_t end() const noexcept {
    return std::default_sentinel;
  }
};

}
