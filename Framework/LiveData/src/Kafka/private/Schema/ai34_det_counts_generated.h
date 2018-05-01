// clang-format off
// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_AI34DETCOUNTS_H_
#define FLATBUFFERS_GENERATED_AI34DETCOUNTS_H_

#include "flatbuffers/flatbuffers.h"

struct PulseImage;

struct PulseImage FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_PULSE_TIME = 4,
    VT_DETECTOR_ID = 6,
    VT_DETECTION_COUNT = 8
  };
  uint64_t pulse_time() const {
    return GetField<uint64_t>(VT_PULSE_TIME, 0);
  }
  const flatbuffers::Vector<uint32_t> *detector_id() const {
    return GetPointer<const flatbuffers::Vector<uint32_t> *>(VT_DETECTOR_ID);
  }
  const flatbuffers::Vector<uint32_t> *detection_count() const {
    return GetPointer<const flatbuffers::Vector<uint32_t> *>(VT_DETECTION_COUNT);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint64_t>(verifier, VT_PULSE_TIME) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_DETECTOR_ID) &&
           verifier.Verify(detector_id()) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_DETECTION_COUNT) &&
           verifier.Verify(detection_count()) &&
           verifier.EndTable();
  }
};

struct PulseImageBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_pulse_time(uint64_t pulse_time) {
    fbb_.AddElement<uint64_t>(PulseImage::VT_PULSE_TIME, pulse_time, 0);
  }
  void add_detector_id(flatbuffers::Offset<flatbuffers::Vector<uint32_t>> detector_id) {
    fbb_.AddOffset(PulseImage::VT_DETECTOR_ID, detector_id);
  }
  void add_detection_count(flatbuffers::Offset<flatbuffers::Vector<uint32_t>> detection_count) {
    fbb_.AddOffset(PulseImage::VT_DETECTION_COUNT, detection_count);
  }
  PulseImageBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  PulseImageBuilder &operator=(const PulseImageBuilder &);
  flatbuffers::Offset<PulseImage> Finish() {
    const auto end = fbb_.EndTable(start_, 3);
    auto o = flatbuffers::Offset<PulseImage>(end);
    return o;
  }
};

inline flatbuffers::Offset<PulseImage> CreatePulseImage(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint64_t pulse_time = 0,
    flatbuffers::Offset<flatbuffers::Vector<uint32_t>> detector_id = 0,
    flatbuffers::Offset<flatbuffers::Vector<uint32_t>> detection_count = 0) {
  PulseImageBuilder builder_(_fbb);
  builder_.add_pulse_time(pulse_time);
  builder_.add_detection_count(detection_count);
  builder_.add_detector_id(detector_id);
  return builder_.Finish();
}

inline flatbuffers::Offset<PulseImage> CreatePulseImageDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint64_t pulse_time = 0,
    const std::vector<uint32_t> *detector_id = nullptr,
    const std::vector<uint32_t> *detection_count = nullptr) {
  return CreatePulseImage(
      _fbb,
      pulse_time,
      detector_id ? _fbb.CreateVector<uint32_t>(*detector_id) : 0,
      detection_count ? _fbb.CreateVector<uint32_t>(*detection_count) : 0);
}

inline const PulseImage *GetPulseImage(const void *buf) {
  return flatbuffers::GetRoot<PulseImage>(buf);
}

inline const char *PulseImageIdentifier() {
  return "ai34";
}

inline bool PulseImageBufferHasIdentifier(const void *buf) {
  return flatbuffers::BufferHasIdentifier(
      buf, PulseImageIdentifier());
}

inline bool VerifyPulseImageBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<PulseImage>(PulseImageIdentifier());
}

inline void FinishPulseImageBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<PulseImage> root) {
  fbb.Finish(root, PulseImageIdentifier());
}

#endif  // FLATBUFFERS_GENERATED_AI34DETCOUNTS_H_
// clang-format on
