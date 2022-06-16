/**
 Utilities for working with pairs and trios of values.
 */
module Fmt = Pretty.Formatters;

type t2('a, 'b) = ('a, 'b);
type t3('a, 'b, 'c) = ('a, 'b, 'c);

/* static */

let with_fst2 = (x: 'a, y: 'b): t2('a, 'b) => (x, y);
let with_snd2 = (x: 'a, y: 'b): t2('b, 'a) => (y, x);

/* getters */

let fst3 = ((x, y, z): ('a, 'b, 'c)): 'a => x;
let snd3 = ((x, y, z): ('a, 'b, 'c)): 'b => y;
let thd3 = ((x, y, z): ('a, 'b, 'c)): 'c => z;

/* methods */

let map2 = (f: 'a => 'b, (x, y): ('a, 'a)): ('b, 'b) => (f(x), f(y));
let map3 = (f: 'a => 'b, (x, y, z): ('a, 'a, 'a)): ('b, 'b, 'b) => (
  f(x),
  f(y),
  f(z),
);

let map_each2 = (f: 'a => 'c, g: 'b => 'd, (x, y): ('a, 'b)): ('c, 'd) => (
  f(x),
  g(y),
);
let map_each3 =
    (f: 'a => 'd, g: 'b => 'e, h: 'c => 'f, (x, y, z): ('a, 'b, 'c))
    : ('d, 'e, 'f) => (
  f(x),
  g(y),
  h(z),
);

let map_fst2 = (f: 'a => 'c, (x, y): ('a, 'b)): ('c, 'b) => (f(x), y);
let map_snd2 = (f: 'b => 'c, (x, y): ('a, 'b)): ('a, 'c) => (x, f(y));

let map_fst3 = (f: 'a => 'd, (x, y, z): ('a, 'b, 'c)): ('d, 'b, 'c) => (
  f(x),
  y,
  z,
);
let map_snd3 = (f: 'b => 'd, (x, y, z): ('a, 'b, 'c)): ('a, 'd, 'c) => (
  x,
  f(y),
  z,
);
let map_thd3 = (f: 'c => 'd, (x, y, z): ('a, 'b, 'c)): ('a, 'b, 'd) => (
  x,
  y,
  f(z),
);

let join2 = (f: ('a, 'b) => 'c, (x, y): ('a, 'b)): 'c => f(x, y);
let join3 = (f: ('a, 'b, 'c) => 'd, (x, y, z): ('a, 'b, 'c)): 'd =>
  f(x, y, z);

let split2 = (f0: 'a => 'b, f1: 'a => 'c, x: 'a): ('b, 'c) => (
  f0(x),
  f1(x),
);
let split3 = (f0: 'a => 'b, f1: 'a => 'c, f2: 'a => 'd, x: 'a): ('b, 'c, 'd) => (
  f0(x),
  f1(x),
  f2(x),
);

let fold2 = (f: (('a, 'b)) => 'c, x: 'a, y: 'b): 'c => f((x, y));
let fold3 = (f: (('a, 'b, 'c)) => 'd, x: 'a, y: 'b, z: 'c): 'd =>
  f((x, y, z));

/* pretty printing */

let pp2 = (ppa, ppb): Fmt.t(t2('a, 'b)) =>
  (ppf, (a, b)) => Fmt.pf(ppf, "(%a, %a)", ppa, a, ppb, b);

let pp3 = (ppa, ppb, ppc): Fmt.t(t3('a, 'b, 'c)) =>
  (ppf, (a, b, c)) => Fmt.pf(ppf, "(%a, %a, %a)", ppa, a, ppb, b, ppc, c);
