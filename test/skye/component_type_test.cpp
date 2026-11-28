#include "skye/Component.hpp"

#include <gtest/gtest.h>

#include <string>

namespace {

struct Position {
  float x = 0.0f;
  float y = 0.0f;
};

struct Velocity {
  float dx = 0.0f;
  float dy = 0.0f;
};

}

TEST( ComponentTypeTest, DistinctTypesGetDistinctIDs ) {
  EXPECT_NE( skye::ComponentTypeOf<Position>(), skye::ComponentTypeOf<Velocity>() );
  EXPECT_NE( skye::ComponentTypeOf<Position>(), skye::ComponentTypeOf<std::string>() );
  EXPECT_NE( skye::ComponentTypeOf<Velocity>(), skye::ComponentTypeOf<std::string>() );
}

TEST( ComponentTypeTest, SameTypeGetsSameID ) {
  const skye::ComponentTypeID first = skye::ComponentTypeOf<Position>();
  EXPECT_EQ( skye::ComponentTypeOf<Position>(), first );
  EXPECT_EQ( skye::ComponentTypeOf<Position>(), first );
}

TEST( ComponentTypeTest, QualifiersAreIgnored ) {
  EXPECT_EQ( skye::ComponentTypeOf<const Position>(), skye::ComponentTypeOf<Position>() );
  EXPECT_EQ( skye::ComponentTypeOf<Position &>(), skye::ComponentTypeOf<Position>() );
  EXPECT_EQ( skye::ComponentTypeOf<const Position &>(), skye::ComponentTypeOf<Position>() );
}
