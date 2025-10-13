#include <cstddef>
#include <vector>

struct ReservVisitor {  // выделение памяти
    size_t capacity;
    ReservVisitor(size_t c) : capacity(c) {}
    template <class T>
    void operator()(std::vector<T>* ptr) const
    {
        if (ptr) {
            ptr->clear();
            ptr->reserve(capacity);
        }
    }
    void operator()(std::nullptr_t) const
    {
        // TODO:добавить логи
    }
};
struct ResizeVisitor {  // выделение памяти
    size_t capacity;
    ResizeVisitor(size_t c) : capacity(c) {}
    template <class T>
    void operator()(std::vector<T>* ptr) const
    {
        if (ptr) ptr->resize(capacity);
    }
    void operator()(std::nullptr_t) const
    {
        // TODO:добавить логи
    }
};
struct PushValueVisitor {
    double value;
    PushValueVisitor(double v) : value(v) {}
    template <class T>
    void operator()(std::vector<T>* ptr)
    {
        if (ptr) {
            ptr->push_back(value);
        }
    }
    void operator()(std::nullptr_t) {}
};
struct WriteValueVisitor {
    double value;
    size_t index;
    WriteValueVisitor(double v, size_t ind) : value(v), index(ind) {}
    template <class T>
    void operator()(std::vector<T>* ptr)
    {
        if (ptr) (*ptr)[index] = static_cast<T>(value);
    }
    void operator()(std::nullptr_t) {}
};
struct GetPointerVisitor {
    size_t index;
    GetPointerVisitor(size_t ind) : index(ind) {}
    template <class T>
    void* operator()(std::vector<T>* ptr) const
    {
        if (ptr != nullptr && index < ptr->size()) {  // Проверка на nullptr и границы
            return ptr->data() + index;               // Или &(*ptr)[index]
        }
        return nullptr;
    }
    void* operator()(std::nullptr_t) { return nullptr; }
};
struct GetTypeSizeVisitor {
    template <class T>
    size_t operator()(std::vector<T>* ptr) const
    {
        return (ptr != nullptr) ? sizeof(T) : 0;
    }
    size_t operator()(std::nullptr_t) { return 0; }
};