#ifndef IDLIB_HEAP_H_
#define IDLIB_HEAP_H_

template <typename T>
struct MyAlloc : std::allocator<T> {
  template <class U>
  struct rebind {
    typedef MyAlloc<U> other;
  };

  using value_type = T;

  MyAlloc() = default;

  template <typename U>
  MyAlloc(const MyAlloc<U>&) {}

  T* allocate(const size_t size);

  void deallocate(T* ptr, size_t size);

  static int i;
};

template <typename T>
T* MyAlloc<T>::allocate(const size_t size) {
  idLib::Printf("Alloc request size %d\n", size);

#ifdef DEBUG
  return DBG_NEW T[size];
#else
  return static_cast<T*>(::operator new(size * sizeof(T)));
#endif
}

template <typename T>
void MyAlloc<T>::deallocate(T* ptr, size_t size) {
  idLib::Printf("Dealloc size ", size);

  delete[] ptr;
}

#endif