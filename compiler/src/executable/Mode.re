open Kore;

type t('a) = {
  name: string,
  opts: list(Opt.t),
  resolve: (option(static_t), global_t) => 'a,
};

let create =
    (
      name: string,
      opts: list(Opt.t),
      resolve: (option(static_t), global_t) => 'a,
    ) => {
  name,
  opts,
  resolve,
};

let map = (f, {resolve} as mode: t('a)): t('b) => {
  ...mode,
  resolve: (static, global) => f(resolve(static, global)),
};
