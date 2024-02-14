#pragma once

#include <cstddef>
#include <cstdio>
namespace smath {

template <class T> struct VectorBase {};

template <class T, std::size_t N> struct Vector : public VectorBase<T> {

  union {
    T data[N];
  };
};

template <class T> struct Vector2 : public Vector<T, 2> {
  union {
    T data[2];
    T x, y;
  };
};

template <class T> struct Vector3 : public Vector<T, 3> {
  union {
    T data[3];
    T x, y, z;
  };
};

template <class T> struct Vector4 : public Vector<T, 4> {
  union {
    T data[4];
    T x, y, z, w;
  };
};

} // namespace smath

// using namespace smath;
// int main() {
//   printf("%lu\n", sizeof(VectorBase<int>));
//   printf("%lu\n", sizeof(Vector<int, 1>));
//   printf("%lu\n", sizeof(Vector2<int>));
//   printf("%lu\n", sizeof(Vector3<int>));
//   printf("%lu\n", sizeof(Vector4<int>));
//
//   Vector4<int> v;
//   v.x = 1;
//   v.y = 2;
//   v.z = 3;
//   v.w = 4;
//
//   auto *b = (Vector2<int> *)(&v);
//
//   printf("%d\n", v.x);
//   printf("%d\n", b->data[0]);
//
//   // printf("%p\n", v.data);
//   // printf("%p\n", b->data);
//
//   // for (int i = 0; i < 2; ++i) {
//   //   b->data[i];
//   //   printf("%d--", b->data[i]);
//   // }
//   // printf("\n");
// }
