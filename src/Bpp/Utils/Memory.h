//
// File: Memory.h
// Authors:
//   Francois Gindraud (2017)
// Created: 2017-07-07
// Last modified: 2017-07-07
//

/*
  Copyright or © or Copr. Bio++ Development Team, (November 17, 2004)

  This software is a computer program whose purpose is to provide classes
  for numerical calculus.

  This software is governed by the CeCILL license under French law and
  abiding by the rules of distribution of free software. You can use,
  modify and/ or redistribute the software under the terms of the CeCILL
  license as circulated by CEA, CNRS and INRIA at the following URL
  "http://www.cecill.info".

  As a counterpart to the access to the source code and rights to copy,
  modify and redistribute granted by the license, users are provided only
  with a limited warranty and the software's author, the holder of the
  economic rights, and the successive licensors have only limited
  liability.

  In this respect, the user's attention is drawn to the risks associated
  with loading, using, modifying and/or developing or reproducing the
  software by the user in light of its specific status of free software,
  that may mean that it is complicated to manipulate, and that also
  therefore means that it is reserved for developers and experienced
  professionals having in-depth computer knowledge. Users are therefore
  encouraged to load and test the software's suitability as regards their
  requirements in conditions enabling the security of their systems and/or
  data to be ensured and, more generally, to use and operate it in the
  same conditions as regards security.

  The fact that you are presently reading this means that you have had
  knowledge of the CeCILL license and that you accept its terms.
*/

#ifndef BPP_UTILS_MEMORY_H
#define BPP_UTILS_MEMORY_H

#include "../Clonable.h"

#include <memory> // This file is guaranteed to include <memory>
#include <type_traits>

namespace bpp
{
  /// Missing std::make_unique() in C++11
  template<typename T, typename... Args>
  std::unique_ptr<T> makeUnique(Args&&... args)
  {
    return std::unique_ptr<T>{new T(std::forward<Args>(args)...)};
  }

  /** NonNull pointer annotation.
   * This annotation has no effect, and just returns the Ptr type itself.
   * It is used to indicate in code that a raw pointer value is supposed to be non null.
   */
  template<typename Ptr>
  using NonNull = Ptr;

  /** CopyUniquePtr default policy.
   * This template class describes how CopyUniquePtr should manipulate its object.
   * This include how to delete it : use std::default_delete<T>.
   * And how to copy : use Clonable clone().
   * Other policy can be used to specialize CopyUniquePtr.
   */
  template<typename T>
  struct DefaultPtrPolicy : public std::default_delete<T>
  {
    static_assert(std::is_base_of<Clonable, T>::value, "DefaultPolicy requires type to derive from bpp::Clonable");

    /// Use Clonable's clone() for copy
    T* clone(T* ptr) const
    {
      if (ptr != nullptr)
        return ptr->clone();
      else
        return nullptr;
    }
  };

  /** unique_ptr that calls clone from Clonable on copy.
   * This is a std::unique_ptr (same API, inherits from std::unique_ptr).
   * The constructor API is not exactly equivalent to unique_ptr, which requires ugly TMP with SFINAE.
   * TODO deprecate use of Clone and only use this as upgrade for legacy code ?
   *
   * CopyUniquePtr behavior is configured by its Policy parameter.
   * The policy describes how to copy and delete the object, and must provide :
   * \code{.cpp}
   * T* clone (T* ptr); // Clone, must handle nullptr
   * void operator () (T* ptr); // Delete object (same as unique_ptr Deleter)
   * \endcode
   * Note that the Policy is stored as the internal unique_ptr deleter.
   */
  template<typename T, typename Policy = DefaultPtrPolicy<T>>
  class CopyUniquePtr : public std::unique_ptr<T, Policy>
  {
  private:
    using UP = std::unique_ptr<T, Policy>;

  public:
    // Defaults
    constexpr CopyUniquePtr() = default;
    CopyUniquePtr(CopyUniquePtr&&) = default;
    CopyUniquePtr& operator=(CopyUniquePtr&&) = default;
    ~CopyUniquePtr() = default;

    // Generate copy constructor and assignement using clone
    CopyUniquePtr(const CopyUniquePtr& other)
      : UP(other.get_deleter().clone(other.get()), other.get_deleter())
    {
    }
    CopyUniquePtr& operator=(const CopyUniquePtr& other)
    {
      UP::operator=(UP(other.get_deleter().clone(other.get()), other.get_deleter()));
      return *this;
    }

    // Forward other constructors
    constexpr CopyUniquePtr(std::nullptr_t) noexcept
      : CopyUniquePtr()
    {
    }
    explicit CopyUniquePtr(typename UP::pointer p) noexcept
      : UP(p)
    {
    }
    CopyUniquePtr(typename UP::pointer p, const Policy& d) noexcept(std::is_nothrow_copy_constructible<Policy>::value)
      : UP(p, d)
    {
    }
    CopyUniquePtr(typename UP::pointer p, Policy&& d) noexcept(std::is_nothrow_move_constructible<Policy>::value)
      : UP(p, std::move(d))
    {
    }
    template<typename U, typename E>
    CopyUniquePtr(std::unique_ptr<U, E>&& other) noexcept
      : UP(std::move(other))
    {
    }

    // Forward other assignement
    CopyUniquePtr& operator=(std::nullptr_t) noexcept
    {
      UP::operator=(nullptr);
      return *this;
    }
    template<typename U, typename E>
    CopyUniquePtr& operator=(std::unique_ptr<U, E>&& other) noexcept
    {
      UP::operator=(std::move(other));
      return *this;
    }

    constexpr operator bool() const noexcept { return UP::operator bool(); }
  };

  /** Conditional ownership policy for CopyUniquePtr.
   * Allow a CopyUniquePtr to be a weird mix of unique_ptr and unsafe shared_ptr depending on ownsPointer.
   * Used for Parameter constraints.
   * TODO tag as deprecated to remove this weird semantic ?
   */
  template<typename T>
  struct ConditionalOwnershipPolicy : private DefaultPtrPolicy<T>
  {
    /// Determines if the pointer is owned and should be managed (copy / delete).
    bool ownsPointer;

    /// Create a policy (defaults to owning the pointer).
    constexpr ConditionalOwnershipPolicy(bool ownsPointer_ = true) noexcept
      : DefaultPtrPolicy<T>()
      , ownsPointer(ownsPointer_)
    {
    }

    /// Copy
    T* clone(T* ptr) const
    {
      if (ownsPointer)
        return DefaultPtrPolicy<T>::clone(ptr);
      else
        return ptr; // Just share the pointer value
    }

    /// Deletion
    void operator()(T* ptr) const
    {
      if (ownsPointer)
        DefaultPtrPolicy<T>::operator()(ptr);
    }
  };
}

#endif // BPP_UTILS_MEMORY_H
