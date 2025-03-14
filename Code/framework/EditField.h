#ifndef FRAMEWORK_EDIT_FIELD_H_
#define FRAMEWORK_EDIT_FIELD_H_

class idEditField {
 public:
  idEditField();
  ~idEditField();
  idEditField(const idEditField&) = default;
  idEditField& operator=(const idEditField&) = default;
  idEditField(idEditField&&) = default;
  idEditField& operator=(idEditField&&) = default;

  void Clear() noexcept;
  void CharEvent(int c);
  void KeyDownEvent(int key);
  const char* GetBuffer() noexcept;
  const std::string& GetBufferString() noexcept { return buffer; }

 private:
  std::string buffer;
};

#endif