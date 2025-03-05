#ifndef DATAQUEUE_H
#define DATAQUEUE_H

template <int maxItems, int maxBuffer>
class idDataQueue {
 public:
  idDataQueue() { dataLength = 0; }
  bool Append(int sequence, const std::byte* b1, int b1Len,
              const std::byte* b2 = nullptr, int b2Len = 0);
  void RemoveOlderThan(int sequence);

  int GetDataLength() const { return dataLength; }

  int Num() const { return items.size(); }
  int ItemSequence(int i) const { return items[i].sequence; }
  int ItemLength(int i) const { return items[i].length; }
  const std::byte* ItemData(int i) const { return &data[items[i].dataOffset]; }

  void Clear() {
    dataLength = 0;
    items.clear();
    std::fill(data.begin(), data.end(), std::byte(0));
  }

 private:
  struct msgItem_t {
    int sequence;
    int length;
    int dataOffset;
  };
  std::deque<msgItem_t> items;
  int dataLength;
  std::array<std::byte, maxBuffer> data;
};

/*
========================
idDataQueue::RemoveOlderThan
========================
*/
template <int maxItems, int maxBuffer>
void idDataQueue<maxItems, maxBuffer>::RemoveOlderThan(int sequence) {
  int length = 0;
  while (items.size() > 0 && items[0].sequence < sequence) {
    length += items[0].length;
    items.pop_front();
  }
  if (length >= dataLength) {
    idassert(items.size() == 0);
    idassert(dataLength == length);
    dataLength = 0;
  } else if (length > 0) {
    memmove(data.data(), data.data() + length, dataLength - length);
    dataLength -= length;
  }
  length = 0;
  for (int i = 0; i < items.size(); i++) {
    items[i].dataOffset = length;
    length += items[i].length;
  }
  idassert(length == dataLength);
}

/*
========================
idDataQueue::Append
========================
*/
template <int maxItems, int maxBuffer>
bool idDataQueue<maxItems, maxBuffer>::Append(int sequence, const std::byte* b1,
                                              int b1Len, const std::byte* b2,
                                              int b2Len) {
  if (items.size() == maxItems) {
    return false;
  }
  if (dataLength + b1Len + b2Len >= maxBuffer) {
    return false;
  }
  items.push_back({});
  msgItem_t& item = items.back();
  item.length = b1Len + b2Len;
  item.sequence = sequence;
  item.dataOffset = dataLength;
  memcpy(data.data() + dataLength, b1, b1Len);
  dataLength += b1Len;
  memcpy(data.data() + dataLength, b2, b2Len);
  dataLength += b2Len;
  return true;
}

#endif