type t('a) = {
  name: string,
  arguments: list(Argument.t),
  resolve: (option(Config.t), Config.global_t) => 'a,
};

let create =
    (
      name: string,
      arguments: list(Argument.t),
      resolve: (option(Config.t), Config.global_t) => 'a,
    ) => {
  name,
  arguments,
  resolve,
};

let map = (f, {resolve} as cmd: t('a)): t('b) => {
  ...cmd,
  resolve: (static, global) => f(resolve(static, global)),
};
