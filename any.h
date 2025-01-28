#pragma once

#include <concepts>
#include <type_traits>
#include <typeinfo>
#include <utility>
template <class T>
concept NotAny = !std::same_as<std::remove_cvref_t<T>, class Any>;

class Any {
   public:
    Any() = default;

    // NOLINTBEGIN(cppcoreguidelines-owning-memory)
    // NOLINTBEGIN(google-explicit-constructor,hicpp-explicit-conversions)
    template <NotAny T>
    Any(T&& value)
        : storage_(
              new Derived<std::remove_cv_t<std::remove_reference_t<T>>>(std::forward<T>(value))) {
    }

    // NOLINTEND(google-explicit-constructor,hicpp-explicit-conversions)
    // NOLINTEND(cppcoreguidelines-owning-memory)

    Any(const Any& other)
        : storage_((other.storage_ != nullptr) ? other.storage_->GetCopy() : nullptr) {
    }

    Any(Any&& other) noexcept : storage_(std::exchange(other.storage_, nullptr)) {
    }

    Any& operator=(const Any& other) {
        if (this != &other) {
            Base* new_storage = (other.storage_ != nullptr) ? other.storage_->GetCopy() : nullptr;
            Clear();  // Освобождаем текущую память
            storage_ = new_storage;
        }
        return *this;
    }

    Any& operator=(Any&& other) noexcept {
        if (this != &other) {
            Clear();
            storage_ = std::exchange(other.storage_, nullptr);
        }
        return *this;
    }

    ~Any() {
        Clear();
    }

    [[nodiscard]] bool Empty() const {
        return storage_ == nullptr;
    }

    void Clear() {
        delete storage_;
        storage_ = nullptr;
    }

    void Swap(Any& other) {
        std::swap(storage_, other.storage_);
    }

    template <class T>
    [[nodiscard]] const T& GetValue() const {
        if (storage_ == nullptr) {
            throw std::bad_cast();
        }
        const auto* ret = dynamic_cast<Derived<T>*>(storage_);
        if (ret == nullptr) {
            throw std::bad_cast();
        }
        return ret->Get();
    }

   private:
    class Base {
       public:
        virtual ~Base() = default;
        [[nodiscard]] virtual Base* GetCopy() const = 0;
        Base() = default;
        Base(const Base& other) = delete;
        Base& operator=(const Base& other) = delete;
        Base(Base&& other) = delete;
        Base& operator=(Base&& other) = delete;
    };

    template <typename T>
    class Derived final : public Base {
       public:
        explicit Derived(T value) : value_(std::forward<T>(value)) {
        }

        [[nodiscard]] const T& Get() const {
            return value_;
        }

        [[nodiscard]] Base* GetCopy() const override {
            return new Derived(value_);  // NOLINT(cppcoreguidelines-owning-memory)
        }

       private:
        T value_;
    };

    Base* storage_ = nullptr;
};
