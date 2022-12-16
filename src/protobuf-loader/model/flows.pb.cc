// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: flows.proto

#include "flows.pb.h"

#include <algorithm>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>

PROTOBUF_PRAGMA_INIT_SEG

namespace _pb = ::PROTOBUF_NAMESPACE_ID;
namespace _pbi = _pb::internal;

namespace ns3_proto {
PROTOBUF_CONSTEXPR Flow::Flow(
    ::_pbi::ConstantInitialized): _impl_{
    /*decltype(_impl_._has_bits_)*/{}
  , /*decltype(_impl_._cached_size_)*/{}
  , /*decltype(_impl_.srcnode_)*/0u
  , /*decltype(_impl_.srcport_)*/0u
  , /*decltype(_impl_.dstnode_)*/0u
  , /*decltype(_impl_.dstport_)*/0u
  , /*decltype(_impl_.size_)*/0u
  , /*decltype(_impl_.arrivetime_)*/0u
  , /*decltype(_impl_.priority_)*/0u} {}
struct FlowDefaultTypeInternal {
  PROTOBUF_CONSTEXPR FlowDefaultTypeInternal()
      : _instance(::_pbi::ConstantInitialized{}) {}
  ~FlowDefaultTypeInternal() {}
  union {
    Flow _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 FlowDefaultTypeInternal _Flow_default_instance_;
PROTOBUF_CONSTEXPR Flows::Flows(
    ::_pbi::ConstantInitialized): _impl_{
    /*decltype(_impl_.flows_)*/{}
  , /*decltype(_impl_._cached_size_)*/{}} {}
struct FlowsDefaultTypeInternal {
  PROTOBUF_CONSTEXPR FlowsDefaultTypeInternal()
      : _instance(::_pbi::ConstantInitialized{}) {}
  ~FlowsDefaultTypeInternal() {}
  union {
    Flows _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 FlowsDefaultTypeInternal _Flows_default_instance_;
}  // namespace ns3_proto
static ::_pb::Metadata file_level_metadata_flows_2eproto[2];
static constexpr ::_pb::EnumDescriptor const** file_level_enum_descriptors_flows_2eproto = nullptr;
static constexpr ::_pb::ServiceDescriptor const** file_level_service_descriptors_flows_2eproto = nullptr;

const uint32_t TableStruct_flows_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  PROTOBUF_FIELD_OFFSET(::ns3_proto::Flow, _impl_._has_bits_),
  PROTOBUF_FIELD_OFFSET(::ns3_proto::Flow, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  PROTOBUF_FIELD_OFFSET(::ns3_proto::Flow, _impl_.srcnode_),
  PROTOBUF_FIELD_OFFSET(::ns3_proto::Flow, _impl_.srcport_),
  PROTOBUF_FIELD_OFFSET(::ns3_proto::Flow, _impl_.dstnode_),
  PROTOBUF_FIELD_OFFSET(::ns3_proto::Flow, _impl_.dstport_),
  PROTOBUF_FIELD_OFFSET(::ns3_proto::Flow, _impl_.size_),
  PROTOBUF_FIELD_OFFSET(::ns3_proto::Flow, _impl_.arrivetime_),
  PROTOBUF_FIELD_OFFSET(::ns3_proto::Flow, _impl_.priority_),
  ~0u,
  ~0u,
  ~0u,
  ~0u,
  ~0u,
  ~0u,
  0,
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::ns3_proto::Flows, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  PROTOBUF_FIELD_OFFSET(::ns3_proto::Flows, _impl_.flows_),
};
static const ::_pbi::MigrationSchema schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  { 0, 13, -1, sizeof(::ns3_proto::Flow)},
  { 20, -1, -1, sizeof(::ns3_proto::Flows)},
};

static const ::_pb::Message* const file_default_instances[] = {
  &::ns3_proto::_Flow_default_instance_._instance,
  &::ns3_proto::_Flows_default_instance_._instance,
};

const char descriptor_table_protodef_flows_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =
  "\n\013flows.proto\022\tns3_proto\"\220\001\n\004Flow\022\017\n\007src"
  "Node\030\001 \001(\r\022\017\n\007srcPort\030\002 \001(\r\022\017\n\007dstNode\030\003"
  " \001(\r\022\017\n\007dstPort\030\004 \001(\r\022\014\n\004size\030\005 \001(\r\022\022\n\na"
  "rriveTime\030\006 \001(\r\022\025\n\010priority\030\007 \001(\rH\000\210\001\001B\013"
  "\n\t_priority\"\'\n\005Flows\022\036\n\005flows\030\001 \003(\0132\017.ns"
  "3_proto.Flowb\006proto3"
  ;
static ::_pbi::once_flag descriptor_table_flows_2eproto_once;
const ::_pbi::DescriptorTable descriptor_table_flows_2eproto = {
    false, false, 220, descriptor_table_protodef_flows_2eproto,
    "flows.proto",
    &descriptor_table_flows_2eproto_once, nullptr, 0, 2,
    schemas, file_default_instances, TableStruct_flows_2eproto::offsets,
    file_level_metadata_flows_2eproto, file_level_enum_descriptors_flows_2eproto,
    file_level_service_descriptors_flows_2eproto,
};
PROTOBUF_ATTRIBUTE_WEAK const ::_pbi::DescriptorTable* descriptor_table_flows_2eproto_getter() {
  return &descriptor_table_flows_2eproto;
}

// Force running AddDescriptors() at dynamic initialization time.
PROTOBUF_ATTRIBUTE_INIT_PRIORITY2 static ::_pbi::AddDescriptorsRunner dynamic_init_dummy_flows_2eproto(&descriptor_table_flows_2eproto);
namespace ns3_proto {

// ===================================================================

class Flow::_Internal {
 public:
  using HasBits = decltype(std::declval<Flow>()._impl_._has_bits_);
  static void set_has_priority(HasBits* has_bits) {
    (*has_bits)[0] |= 1u;
  }
};

Flow::Flow(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor(arena, is_message_owned);
  // @@protoc_insertion_point(arena_constructor:ns3_proto.Flow)
}
Flow::Flow(const Flow& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  Flow* const _this = this; (void)_this;
  new (&_impl_) Impl_{
      decltype(_impl_._has_bits_){from._impl_._has_bits_}
    , /*decltype(_impl_._cached_size_)*/{}
    , decltype(_impl_.srcnode_){}
    , decltype(_impl_.srcport_){}
    , decltype(_impl_.dstnode_){}
    , decltype(_impl_.dstport_){}
    , decltype(_impl_.size_){}
    , decltype(_impl_.arrivetime_){}
    , decltype(_impl_.priority_){}};

  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  ::memcpy(&_impl_.srcnode_, &from._impl_.srcnode_,
    static_cast<size_t>(reinterpret_cast<char*>(&_impl_.priority_) -
    reinterpret_cast<char*>(&_impl_.srcnode_)) + sizeof(_impl_.priority_));
  // @@protoc_insertion_point(copy_constructor:ns3_proto.Flow)
}

inline void Flow::SharedCtor(
    ::_pb::Arena* arena, bool is_message_owned) {
  (void)arena;
  (void)is_message_owned;
  new (&_impl_) Impl_{
      decltype(_impl_._has_bits_){}
    , /*decltype(_impl_._cached_size_)*/{}
    , decltype(_impl_.srcnode_){0u}
    , decltype(_impl_.srcport_){0u}
    , decltype(_impl_.dstnode_){0u}
    , decltype(_impl_.dstport_){0u}
    , decltype(_impl_.size_){0u}
    , decltype(_impl_.arrivetime_){0u}
    , decltype(_impl_.priority_){0u}
  };
}

Flow::~Flow() {
  // @@protoc_insertion_point(destructor:ns3_proto.Flow)
  if (auto *arena = _internal_metadata_.DeleteReturnArena<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>()) {
  (void)arena;
    return;
  }
  SharedDtor();
}

inline void Flow::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
}

void Flow::SetCachedSize(int size) const {
  _impl_._cached_size_.Set(size);
}

void Flow::Clear() {
// @@protoc_insertion_point(message_clear_start:ns3_proto.Flow)
  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  ::memset(&_impl_.srcnode_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&_impl_.arrivetime_) -
      reinterpret_cast<char*>(&_impl_.srcnode_)) + sizeof(_impl_.arrivetime_));
  _impl_.priority_ = 0u;
  _impl_._has_bits_.Clear();
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* Flow::_InternalParse(const char* ptr, ::_pbi::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  _Internal::HasBits has_bits{};
  while (!ctx->Done(&ptr)) {
    uint32_t tag;
    ptr = ::_pbi::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // uint32 srcNode = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 8)) {
          _impl_.srcnode_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint32(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // uint32 srcPort = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 16)) {
          _impl_.srcport_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint32(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // uint32 dstNode = 3;
      case 3:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 24)) {
          _impl_.dstnode_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint32(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // uint32 dstPort = 4;
      case 4:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 32)) {
          _impl_.dstport_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint32(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // uint32 size = 5;
      case 5:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 40)) {
          _impl_.size_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint32(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // uint32 arriveTime = 6;
      case 6:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 48)) {
          _impl_.arrivetime_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint32(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // optional uint32 priority = 7;
      case 7:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 56)) {
          _Internal::set_has_priority(&has_bits);
          _impl_.priority_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint32(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      default:
        goto handle_unusual;
    }  // switch
  handle_unusual:
    if ((tag == 0) || ((tag & 7) == 4)) {
      CHK_(ptr);
      ctx->SetLastTag(tag);
      goto message_done;
    }
    ptr = UnknownFieldParse(
        tag,
        _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
        ptr, ctx);
    CHK_(ptr != nullptr);
  }  // while
message_done:
  _impl_._has_bits_.Or(has_bits);
  return ptr;
failure:
  ptr = nullptr;
  goto message_done;
#undef CHK_
}

uint8_t* Flow::_InternalSerialize(
    uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:ns3_proto.Flow)
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  // uint32 srcNode = 1;
  if (this->_internal_srcnode() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteUInt32ToArray(1, this->_internal_srcnode(), target);
  }

  // uint32 srcPort = 2;
  if (this->_internal_srcport() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteUInt32ToArray(2, this->_internal_srcport(), target);
  }

  // uint32 dstNode = 3;
  if (this->_internal_dstnode() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteUInt32ToArray(3, this->_internal_dstnode(), target);
  }

  // uint32 dstPort = 4;
  if (this->_internal_dstport() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteUInt32ToArray(4, this->_internal_dstport(), target);
  }

  // uint32 size = 5;
  if (this->_internal_size() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteUInt32ToArray(5, this->_internal_size(), target);
  }

  // uint32 arriveTime = 6;
  if (this->_internal_arrivetime() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteUInt32ToArray(6, this->_internal_arrivetime(), target);
  }

  // optional uint32 priority = 7;
  if (_internal_has_priority()) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteUInt32ToArray(7, this->_internal_priority(), target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:ns3_proto.Flow)
  return target;
}

size_t Flow::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:ns3_proto.Flow)
  size_t total_size = 0;

  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // uint32 srcNode = 1;
  if (this->_internal_srcnode() != 0) {
    total_size += ::_pbi::WireFormatLite::UInt32SizePlusOne(this->_internal_srcnode());
  }

  // uint32 srcPort = 2;
  if (this->_internal_srcport() != 0) {
    total_size += ::_pbi::WireFormatLite::UInt32SizePlusOne(this->_internal_srcport());
  }

  // uint32 dstNode = 3;
  if (this->_internal_dstnode() != 0) {
    total_size += ::_pbi::WireFormatLite::UInt32SizePlusOne(this->_internal_dstnode());
  }

  // uint32 dstPort = 4;
  if (this->_internal_dstport() != 0) {
    total_size += ::_pbi::WireFormatLite::UInt32SizePlusOne(this->_internal_dstport());
  }

  // uint32 size = 5;
  if (this->_internal_size() != 0) {
    total_size += ::_pbi::WireFormatLite::UInt32SizePlusOne(this->_internal_size());
  }

  // uint32 arriveTime = 6;
  if (this->_internal_arrivetime() != 0) {
    total_size += ::_pbi::WireFormatLite::UInt32SizePlusOne(this->_internal_arrivetime());
  }

  // optional uint32 priority = 7;
  cached_has_bits = _impl_._has_bits_[0];
  if (cached_has_bits & 0x00000001u) {
    total_size += ::_pbi::WireFormatLite::UInt32SizePlusOne(this->_internal_priority());
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData Flow::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSourceCheck,
    Flow::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*Flow::GetClassData() const { return &_class_data_; }


void Flow::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg) {
  auto* const _this = static_cast<Flow*>(&to_msg);
  auto& from = static_cast<const Flow&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:ns3_proto.Flow)
  GOOGLE_DCHECK_NE(&from, _this);
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  if (from._internal_srcnode() != 0) {
    _this->_internal_set_srcnode(from._internal_srcnode());
  }
  if (from._internal_srcport() != 0) {
    _this->_internal_set_srcport(from._internal_srcport());
  }
  if (from._internal_dstnode() != 0) {
    _this->_internal_set_dstnode(from._internal_dstnode());
  }
  if (from._internal_dstport() != 0) {
    _this->_internal_set_dstport(from._internal_dstport());
  }
  if (from._internal_size() != 0) {
    _this->_internal_set_size(from._internal_size());
  }
  if (from._internal_arrivetime() != 0) {
    _this->_internal_set_arrivetime(from._internal_arrivetime());
  }
  if (from._internal_has_priority()) {
    _this->_internal_set_priority(from._internal_priority());
  }
  _this->_internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void Flow::CopyFrom(const Flow& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:ns3_proto.Flow)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool Flow::IsInitialized() const {
  return true;
}

void Flow::InternalSwap(Flow* other) {
  using std::swap;
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  swap(_impl_._has_bits_[0], other->_impl_._has_bits_[0]);
  ::PROTOBUF_NAMESPACE_ID::internal::memswap<
      PROTOBUF_FIELD_OFFSET(Flow, _impl_.priority_)
      + sizeof(Flow::_impl_.priority_)
      - PROTOBUF_FIELD_OFFSET(Flow, _impl_.srcnode_)>(
          reinterpret_cast<char*>(&_impl_.srcnode_),
          reinterpret_cast<char*>(&other->_impl_.srcnode_));
}

::PROTOBUF_NAMESPACE_ID::Metadata Flow::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_flows_2eproto_getter, &descriptor_table_flows_2eproto_once,
      file_level_metadata_flows_2eproto[0]);
}

// ===================================================================

class Flows::_Internal {
 public:
};

Flows::Flows(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor(arena, is_message_owned);
  // @@protoc_insertion_point(arena_constructor:ns3_proto.Flows)
}
Flows::Flows(const Flows& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  Flows* const _this = this; (void)_this;
  new (&_impl_) Impl_{
      decltype(_impl_.flows_){from._impl_.flows_}
    , /*decltype(_impl_._cached_size_)*/{}};

  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  // @@protoc_insertion_point(copy_constructor:ns3_proto.Flows)
}

inline void Flows::SharedCtor(
    ::_pb::Arena* arena, bool is_message_owned) {
  (void)arena;
  (void)is_message_owned;
  new (&_impl_) Impl_{
      decltype(_impl_.flows_){arena}
    , /*decltype(_impl_._cached_size_)*/{}
  };
}

Flows::~Flows() {
  // @@protoc_insertion_point(destructor:ns3_proto.Flows)
  if (auto *arena = _internal_metadata_.DeleteReturnArena<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>()) {
  (void)arena;
    return;
  }
  SharedDtor();
}

inline void Flows::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  _impl_.flows_.~RepeatedPtrField();
}

void Flows::SetCachedSize(int size) const {
  _impl_._cached_size_.Set(size);
}

void Flows::Clear() {
// @@protoc_insertion_point(message_clear_start:ns3_proto.Flows)
  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.flows_.Clear();
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* Flows::_InternalParse(const char* ptr, ::_pbi::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    uint32_t tag;
    ptr = ::_pbi::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // repeated .ns3_proto.Flow flows = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 10)) {
          ptr -= 1;
          do {
            ptr += 1;
            ptr = ctx->ParseMessage(_internal_add_flows(), ptr);
            CHK_(ptr);
            if (!ctx->DataAvailable(ptr)) break;
          } while (::PROTOBUF_NAMESPACE_ID::internal::ExpectTag<10>(ptr));
        } else
          goto handle_unusual;
        continue;
      default:
        goto handle_unusual;
    }  // switch
  handle_unusual:
    if ((tag == 0) || ((tag & 7) == 4)) {
      CHK_(ptr);
      ctx->SetLastTag(tag);
      goto message_done;
    }
    ptr = UnknownFieldParse(
        tag,
        _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
        ptr, ctx);
    CHK_(ptr != nullptr);
  }  // while
message_done:
  return ptr;
failure:
  ptr = nullptr;
  goto message_done;
#undef CHK_
}

uint8_t* Flows::_InternalSerialize(
    uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:ns3_proto.Flows)
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  // repeated .ns3_proto.Flow flows = 1;
  for (unsigned i = 0,
      n = static_cast<unsigned>(this->_internal_flows_size()); i < n; i++) {
    const auto& repfield = this->_internal_flows(i);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::
        InternalWriteMessage(1, repfield, repfield.GetCachedSize(), target, stream);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:ns3_proto.Flows)
  return target;
}

size_t Flows::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:ns3_proto.Flows)
  size_t total_size = 0;

  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // repeated .ns3_proto.Flow flows = 1;
  total_size += 1UL * this->_internal_flows_size();
  for (const auto& msg : this->_impl_.flows_) {
    total_size +=
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::MessageSize(msg);
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData Flows::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSourceCheck,
    Flows::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*Flows::GetClassData() const { return &_class_data_; }


void Flows::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg) {
  auto* const _this = static_cast<Flows*>(&to_msg);
  auto& from = static_cast<const Flows&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:ns3_proto.Flows)
  GOOGLE_DCHECK_NE(&from, _this);
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  _this->_impl_.flows_.MergeFrom(from._impl_.flows_);
  _this->_internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void Flows::CopyFrom(const Flows& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:ns3_proto.Flows)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool Flows::IsInitialized() const {
  return true;
}

void Flows::InternalSwap(Flows* other) {
  using std::swap;
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  _impl_.flows_.InternalSwap(&other->_impl_.flows_);
}

::PROTOBUF_NAMESPACE_ID::Metadata Flows::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_flows_2eproto_getter, &descriptor_table_flows_2eproto_once,
      file_level_metadata_flows_2eproto[1]);
}

// @@protoc_insertion_point(namespace_scope)
}  // namespace ns3_proto
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_NOINLINE ::ns3_proto::Flow*
Arena::CreateMaybeMessage< ::ns3_proto::Flow >(Arena* arena) {
  return Arena::CreateMessageInternal< ::ns3_proto::Flow >(arena);
}
template<> PROTOBUF_NOINLINE ::ns3_proto::Flows*
Arena::CreateMaybeMessage< ::ns3_proto::Flows >(Arena* arena) {
  return Arena::CreateMessageInternal< ::ns3_proto::Flows >(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>