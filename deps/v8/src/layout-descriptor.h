// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_LAYOUT_DESCRIPTOR_H_
#define V8_LAYOUT_DESCRIPTOR_H_

#include <iosfwd>

#include "src/objects.h"

namespace v8 {
namespace internal {

// LayoutDescriptor is a bit vector defining which fields contain non-tagged
// values. It could either be a fixed typed array (slow form) or a Smi
// if the length fits (fast form).
// Each bit in the layout represents a FIELD. The bits are referenced by
// field_index which is a field number. If the bit is set then the corresponding
// field contains a non-tagged value and therefore must be skipped by GC.
// Otherwise the field is considered tagged. If the queried bit lays "outside"
// of the descriptor then the field is also considered tagged.
// Once a layout descriptor is created it is allowed only to append properties
// to it.
class LayoutDescriptor : public FixedTypedArray<Uint32ArrayTraits> {
 public:
  V8_INLINE bool IsTagged(int field_index);

  // Queries the contiguous region of fields that are either tagged or not.
  // Returns true if the given field is tagged or false otherwise and writes
  // the length of the contiguous region to |out_sequence_length|.
  // If the sequence is longer than |max_sequence_length| then
  // |out_sequence_length| is set to |max_sequence_length|.
  bool IsTagged(int field_index, int max_sequence_length,
                int* out_sequence_length);

  // Returns true if this is a layout of the object having only tagged fields.
  V8_INLINE bool IsFastPointerLayout();
  V8_INLINE static bool IsFastPointerLayout(Object* layout_descriptor);

  // Returns true if the layout descriptor is in non-Smi form.
  V8_INLINE bool IsSlowLayout();

  V8_INLINE static LayoutDescriptor* cast(Object* object);
  V8_INLINE static const LayoutDescriptor* cast(const Object* object);

  V8_INLINE static LayoutDescriptor* cast_gc_safe(Object* object);

  // Builds layout descriptor optimized for given |map| by |num_descriptors|
  // elements of given descriptors array. The |map|'s descriptors could be
  // different.
  static Handle<LayoutDescriptor> New(Handle<Map> map,
                                      Handle<DescriptorArray> descriptors,
                                      int num_descriptors);

  // Modifies |map|'s layout descriptor or creates a new one if necessary by
  // appending property with |details| to it.
  static Handle<LayoutDescriptor> ShareAppend(Handle<Map> map,
                                              PropertyDetails details);

  // Creates new layout descriptor by appending property with |details| to
  // |map|'s layout descriptor and if it is still fast then returns it.
  // Otherwise the |full_layout_descriptor| is returned.
  static Handle<LayoutDescriptor> AppendIfFastOrUseFull(
      Handle<Map> map, PropertyDetails details,
      Handle<LayoutDescriptor> full_layout_descriptor);

  // Layout descriptor that corresponds to an object all fields of which are
  // tagged (FastPointerLayout).
  V8_INLINE static LayoutDescriptor* FastPointerLayout();

  // Check that this layout descriptor corresponds to given map.
  bool IsConsistentWithMap(Map* map);

#ifdef OBJECT_PRINT
  // For our gdb macros, we should perhaps change these in the future.
  void Print();

  void Print(std::ostream& os);  // NOLINT
#endif

  // Capacity of layout descriptors in bits.
  V8_INLINE int capacity();

  static Handle<LayoutDescriptor> NewForTesting(Isolate* isolate, int length);
  LayoutDescriptor* SetTaggedForTesting(int field_index, bool tagged);

 private:
  static const int kNumberOfBits = 32;

  V8_INLINE static Handle<LayoutDescriptor> New(Isolate* isolate, int length);
  V8_INLINE static LayoutDescriptor* FromSmi(Smi* smi);

  V8_INLINE static bool InobjectUnboxedField(int inobject_properties,
                                             PropertyDetails details);

  static Handle<LayoutDescriptor> EnsureCapacity(
      Isolate* isolate, Handle<LayoutDescriptor> layout_descriptor,
      int new_capacity);

  // Returns false if requested field_index is out of bounds.
  V8_INLINE bool GetIndexes(int field_index, int* layout_word_index,
                            int* layout_bit_index);

  V8_INLINE MUST_USE_RESULT LayoutDescriptor* SetRawData(int field_index) {
    return SetTagged(field_index, false);
  }

  V8_INLINE MUST_USE_RESULT LayoutDescriptor* SetTagged(int field_index,
                                                        bool tagged);
};


// LayoutDescriptorHelper is a helper class for querying layout descriptor
// about whether the field at given offset is tagged or not.
class LayoutDescriptorHelper {
 public:
  inline explicit LayoutDescriptorHelper(Map* map);

  bool all_fields_tagged() { return all_fields_tagged_; }
  inline bool IsTagged(int offset_in_bytes);

  // Queries the contiguous region of fields that are either tagged or not.
  // Returns true if fields starting at |offset_in_bytes| are tagged or false
  // otherwise and writes the offset of the end of the contiguous region to
  // |out_end_of_contiguous_region_offset|. The |end_offset| value is the
  // upper bound for |out_end_of_contiguous_region_offset|.
  bool IsTagged(int offset_in_bytes, int end_offset,
                int* out_end_of_contiguous_region_offset);

 private:
  bool all_fields_tagged_;
  int header_size_;
  LayoutDescriptor* layout_descriptor_;
};
}
}  // namespace v8::internal

#endif  // V8_LAYOUT_DESCRIPTOR_H_
