
#include <iostream>
#include <cstddef>
#include <tuple>
#include <tinyexpr.h>
#include <vector>
#include <eigen3/Eigen/Dense>
#include <map>
#include <functional>
#include <type_traits>

using namespace Eigen;
using namespace std;

namespace std
{
template <>
struct less<Vector3d>
{
  bool operator()(const Vector3d &a, const Vector3d &b) const
  {
    assert(a.size() == b.size());
    for (size_t i = 0; i < a.size(); ++i)
    {
      if (a(i) < b(i))
        return true;
      if (a(i) > b(i))
        return false;
    }
    return false;
  }
};

template <>
struct less<std::tuple<Vector3d>>
{
  bool operator()(const std::tuple<Vector3d> &aa, const std::tuple<Vector3d> &bb) const
  {
    Vector3d a = get<0>(aa);
    Vector3d b = get<0>(bb);
    assert(a.size() == b.size());
    for (size_t i = 0; i < a.size(); ++i)
    {
      if (a(i) < b(i))
        return true;
      if (a(i) > b(i))
        return false;
    }
    return false;
  }
};
} // namespace std

namespace memo
{

namespace detail
{
// This gets the base function type from pretty much anything it can:
// C function types, member function types, monomorphic function objects.
template <typename T, typename Enable = void>
struct function_signature;

template <typename T>
struct function_signature<T, typename std::enable_if<
                                 std::is_class<T>::value>::type> : function_signature<decltype(&T::operator())>
{
};

template <typename T, typename Ret, typename... Args>
struct function_signature<Ret (T::*)(Args...)>
{
  using type = Ret(Args...);
};

template <typename T, typename Ret, typename... Args>
struct function_signature<Ret (T::*)(Args...) const>
{
  using type = Ret(Args...);
};

template <typename Ret, typename... Args>
struct function_signature<Ret(Args...)>
{
  using type = Ret(Args...);
};

template <typename Ret, typename... Args>
struct function_signature<Ret (&)(Args...)>
{
  using type = Ret(Args...);
};

template <typename Ret, typename... Args>
struct function_signature<Ret (*)(Args...)>
{
  using type = Ret(Args...);
};

template <typename T>
struct remove_const_ref : std::remove_const<
                              typename std::remove_reference<T>::type>
{
};

template <typename T, typename U>
struct is_same_base_type : std::is_same<
                               typename remove_const_ref<T>::type,
                               typename remove_const_ref<U>::type>
{
};
} // namespace detail

template <typename T, typename Function>
class memoizer;

template <typename T, typename Ret, typename... Args>
class memoizer<T, Ret(Args...)>
{
public:
  using function_type = Ret(Args...);
  using tuple_type = std::tuple<typename std::decay_t<Args>...>;
  using return_type = Ret;
  using return_reference = const return_type &;
  // This lets us do lookups in our map without converting our value to
  // key_type, which saves us a copy. There's a proposal to make this work for
  // std::unordered_map too.
  using map_type = std::map<tuple_type, return_type>;

  memoizer() {}
  memoizer(const T &f) : f_(f) {}

  template <typename... CallArgs>
  return_reference operator()(CallArgs &&... args)
  {
    // This is a roundabout way of requiring that call is called with arguments
    // of the same basic type as the function (i.e. it will make use of
    // automatic conversions for similar types). This ensures that we always
    // call the *same* function for polymorphic function objects.
    return call<CallArgs...>(std::forward<CallArgs>(args)...);
  }

private:
  template <typename T2, typename... Args2>
  struct can_pass_memoizer
  {
    static const bool value = std::is_invocable_v<T2, memoizer &, Args2...>;
  };

  template <typename... CallArgs>
  return_reference call(CallArgs &&... args)
  {
    const auto args_tuple = tuple_type(std::forward<CallArgs>(args)...);
    auto i = memo_.find(args_tuple);
    if (i != memo_.end())
    {
      return i->second;
    }
    else
    {
      auto result = call_function(std::forward<CallArgs>(args)...);
      auto ins = memo_.emplace(
                          std::move(args_tuple),
                          std::move(result))
                     .first;
      return ins->second;
    }
  }

  template <typename... CallArgs>
  typename std::enable_if_t<
      can_pass_memoizer<T, CallArgs...>::value,
      return_type>
  call_function(CallArgs &&... args)
  {
    return f_(*this, std::forward<Args>(args)...);
  }

  template <typename... CallArgs>
  typename std::enable_if_t<
      !can_pass_memoizer<T, CallArgs...>::value,
      return_type>
  call_function(CallArgs &&... args)
  {
    return f_(std::forward<Args>(args)...);
  }

  map_type memo_;
  T f_;
};

template <typename Function, typename T>
inline auto memoize(T &&t)
{
  return memoizer<T, Function>(std::forward<T>(t));
}

template <typename T>
inline auto memoize(T &&t)
{
  return memoizer<T, typename detail::function_signature<T>::type>(
      std::forward<T>(t));
}

} // namespace memo

int func(int a, int b)
{
  return a + b;
}

struct func_obj
{
  int operator()(int a, int b)
  {
    return a + b;
  }
};

struct poly_func_obj
{
  int operator()(double, double)
  {
    return 0;
  }
  int operator()(int a, int b)
  {
    return a + b;
  }
};

using namespace memo;
int main()
{
  // auto memo = memoize(func);
  // cout << memo(1, 2) << endl;

  // auto fib = memoize<size_t(size_t)>([](auto &fib, size_t n) -> size_t {
  //   switch (n)
  //   {
  //   case 0:
  //     return 0;
  //   case 1:
  //     return 1;
  //   default:
  //     return fib(n - 1) + fib(n - 2);
  //   }
  // });
  // fib(10);
  auto eigen = memoize([](const Vector3d &v) {
    return v;
  });
  Vector3d v;
  v << 1, 2, 3;
  cout << eigen(v);

  return 0;
}