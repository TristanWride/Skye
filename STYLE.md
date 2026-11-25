# Skye C++ Style

`.clang-format` enforces layout. Where this document and clang-format disagree, clang-format wins.

## Naming

| Thing | Rule | Example |
|---|---|---|
| Functions (free and member) | PascalCase | `CreateEntity()` |
| Types, aliases, enums | PascalCase | `EntityRegistry`, `EntityID` |
| Enum values | PascalCase | `EventKind::KeyDown` |
| Locals, parameters | camelCase | `entityID` |
| Class data members | camelCase + `_` | `firstFreeID_` |
| POD struct fields | camelCase | `Entry::isActive` |
| Constants, `constexpr` | SCREAMING_CASE | `MAX_ENTITIES` |
| Namespaces | lowercase | `skye::util` |
| Template parameters | `T` if one; descriptive if several | `template <class Key, class Value>` |
| Acronyms | All caps | `ID`, `GPU` |
| Library files | PascalCase | `Entity.hpp`, `Entity.cpp` |
| Example and test files | snake_case | `entity_registry_test.cpp` |

Exception: standard-library-compatible member aliases keep their std names (`size_type`, `value_type`).

## Layout

- Indent 2 spaces. No tabs.
- No hard line limit.
- Opening braces on the same line.
- Prefer tabular layout: align consecutive assignments, declarations and trailing comments.
- Spaces inside non-empty parentheses: `Fail( "x", id )`, `if ( x )`. None in empty ones: `Foo()`.
- Spaces inside braced initialisers: `{ 0 }`.
- `*` and `&` spaced on both sides: `char * x`, `const T & y`.
- `const` before the type.

### Braces on control statements

- Single-statement body: no braces, at any nesting depth, including `else`.
- Body on its own indented line. A short, simple `if` with no `else` may stay on one line.
- A single statement that wraps onto several lines gets braces.

```cpp
if ( done ) return;

for ( auto & entry : entries_ )
  if ( entry.isActive )
    Update( entry );
  else
    Reset( entry );

if ( entityID >= MAX_ENTITIES ) {
  util::Fail( "Invalid entity ID: {}. Must be less than {}.",
              entityID, MAX_ENTITIES );
}
```

### Constructor initialiser lists

- Leading commas, one member per line.

```cpp
EntityRegistry::EntityRegistry()
  : entityStorage_{ std::make_unique<Entry[]>( MAX_ENTITIES ) }
  , entities_{ entityStorage_.get(), MAX_ENTITIES } {
  ...
}
```

## Classes

Order by kind, then by access (`public`, `protected`, `private`) within each kind:

1. Types and type aliases, then static constants and static data
2. Data members
3. Methods: factory functions, constructors and assignment, destructor, then all others

Every group starts with its own access specifier, even if it repeats the one above.

```cpp
class EntityRegistry {
public:
  using size_type = uint64_t;
  static constexpr size_type MAX_ENTITIES = 1 << 18;

private:
  EntityID firstFreeID_{ 0 };

public:
  explicit EntityRegistry();
  [[nodiscard]] EntityID CreateEntity();
};
```

## Files

- Headers use `#pragma once`.
- Never rely on transitive includes. Every file directly includes the header for each name it uses. Only exception: a `.cpp` may rely on its own header's includes (`Entity.cpp` on `Entity.hpp`).
- Include order: own header, project, third-party, std. Blank line between groups.
- Quotes for project headers (`"skye/Entity.hpp"`); angle brackets for everything else.
- Definitions go in `.cpp`. Templates and `constexpr` must be in headers. Other inline definitions are permitted but rare.

## Language

- Fixed-width integers without `std::`: `uint32_t`.
- `auto` when the type is obvious from the right-hand side.
- Initialise as `auto name = Class{ ... };`. Data members: `Type name_{ ... };`.
- `using namespace` only for literal namespaces (`std::chrono_literals`). Never at global scope in a header.
- No raw owning pointers. `std::unique_ptr` owns; raw pointers and references borrow. Wrap C handles (SDL) in `std::unique_ptr` with a custom deleter.

### Attributes and specifiers

| Specifier | Use on |
|---|---|
| `[[nodiscard]]` | Pure functions, and any function where discarding the result is a bug |
| `noexcept` | Functions that cannot throw for any input, preconditions met or not |
| `explicit` | Every constructor and conversion operator, except intentional implicit conversions (commented) |
| `[[noreturn]]` | Every function that never returns |

### Errors

- Terminating errors throw, via `util::Fail`.
- Use `std::optional` / `std::expected` only when the caller must react to the error at runtime.

## Comments

- `//` only.
- One-line summary on every public function.
- Inside bodies, comment *why*, never *what*.
