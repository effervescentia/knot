module type Source = {
  type t;
  type f;
  type s;

  let fst: t => f;
  let snd: t => s;

  let create: (f, s) => t;
};

module Make = (S: Source) => {
  type t = S.t;

  let value = (x: S.t) => S.fst(x);
  let context = (x: S.t) => S.snd(x);

  let create = (v: S.f, c: S.s) => S.create(v, c);
};
