
#include <iostream>
#include <cstddef>
#include <tuple>
#include <tinyexpr.h>
#include <vector>
#include <eigen3/Eigen/Dense>
#include <map>

using namespace Eigen;
using namespace std;

double funnyThing(int a, const double &b)
{
  return 3.141 * a * b;
}

// template <typename Callable, typename F>
// class MemoizeG;

// template <typename ReturnType, typename... ArgTypes, typename F>
// class MemoizeG<ReturnType(ArgTypes...), F>
// {
// public:
//   MemoizeG(F &&f) : _f(std::forward<F>(f)) {}
//   ReturnType operator()(ArgTypes... args)
//   {
//     const auto argsAsTuple = std::make_tuple(args...);
//     auto it = _m.find(argsAsTuple);
//     if (it == _m.end())
//     {
//       const auto &&res = _f(args...);
//       _m.emplace(argsAsTuple, res);
//       std::cout << "computed: ...->" << std::endl;
//       return res;
//     }
//     else
//     {
//       std::cout << "memoized: ...->" << std::endl;
//       return it->second;
//     }
//   }

// private:
//   F _f;
//   mutable std::map<std::tuple<ArgTypes...>, ReturnType> _m;
// };

// template <typename ReturnType, typename... ArgTypes, typename F>
// MemoizeG<ReturnType(ArgTypes...), F> MakeMemoizeG(F &&f)
// {
//   return MemoizeG<decltype(f), F>(f);
// }

struct wrap
{
};

template <class Sig, class F, template <class...> class Hash = std::hash>
struct memoizer;
template <class R, class... Args, class F, template <class...> class Hash>
struct memoizer<R(Args...), F, Hash>
{
  using base_type = F;

private:
  F base;
  std::map<std::tuple<std::decay_t<Args>...>, R> cache;

public:
  template <class... Ts>
  R operator()(Ts &&... ts) const
  {
    auto args = std::make_tuple(ts...);
    auto it = cache.find(args);
    if (it != cache.end())
      return it->second;

    auto &&retval = base(*this, std::forward<Ts>(ts)...);

    cache.emplace(std::move(args), retval);

    return decltype(retval)(retval);
  }
  template <class... Ts>
  R operator()(Ts &&... ts)
  {
    auto args = std::tie(ts...);
    auto it = cache.find(args);
    if (it != cache.end())
      return it->second;

    auto &&retval = base(*this, std::forward<Ts>(ts)...);

    cache.emplace(std::move(args), retval);

    return decltype(retval)(retval);
  }

  memoizer(memoizer const &) = default;
  memoizer(memoizer &&) = default;
  memoizer &operator=(memoizer const &) = default;
  memoizer &operator=(memoizer &&) = default;
  memoizer() = delete;
  template <typename L>
  memoizer(wrap, L &&f) : base(std::forward<L>(f))
  {
  }
};

template <class Sig, class F>
memoizer<Sig, std::decay_t<F>> memoize(F &&f) { return {wrap{}, std::forward<F>(f)}; }

int main()
{
  // auto memoizedFunnyThing = MemoizeG<double(int, double)>(funnyThing);
  // cout << memoizedFunnyThing(1, 1.1) << endl;
  // memoizedFunnyThing(2, 1.1);
  // memoizedFunnyThing(3, 1.1);
  // memoizedFunnyThing(1, 1.1);

  // auto convenientMemoizedFunnyThing = MakeMemoizeG(funnyThing);

  // convenientMemoizedFunnyThing(1, 1.1);
  // convenientMemoizedFunnyThing(1, 1.1);

  auto func = [](VectorXd &a) {
    return a.norm();
  };
  auto memo_lambda = memoizer(func);
  // auto memo_lambda = MakeMemoizeG(func);
  return 0;
}