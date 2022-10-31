/**
 Utilities for working with pairs and trios of values.
 */
module Fmt = Pretty.Formatters;

/* static */

let with_fst2 = (x: 'a, y: 'b): ('a, 'b) => (x, y);
let with_snd2 = (x: 'a, y: 'b): ('b, 'a) => (y, x);

/* getters */

let fst3 = ((x, _, _): ('a, 'b, 'c)): 'a => x;
let snd3 = ((_, y, _): ('a, 'b, 'c)): 'b => y;
let thd3 = ((_, _, z): ('a, 'b, 'c)): 'c => z;

let fst4 = ((w, _, _, _): ('a, 'b, 'c, 'd)): 'a => w;
let snd4 = ((_, x, _, _): ('a, 'b, 'c, 'd)): 'b => x;
let thd4 = ((_, _, y, _): ('a, 'b, 'c, 'd)): 'c => y;
let fth4 = ((_, _, _, z): ('a, 'b, 'c, 'd)): 'd => z;

/* methods */

let map2 = (f: 'a => 'b, (x, y): ('a, 'a)): ('b, 'b) => (f(x), f(y));
let map3 = (f: 'a => 'b, (x, y, z): ('a, 'a, 'a)): ('b, 'b, 'b) => (
  f(x),
  f(y),
  f(z),
);
let map4 = (f: 'a => 'b, (w, x, y, z): ('a, 'a, 'a, 'a)): ('b, 'b, 'b, 'b) => (
  f(w),
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
let map_each4 =
    (
      f: 'a => 'e,
      g: 'b => 'f,
      h: 'c => 'g,
      i: 'd => 'h,
      (w, x, y, z): ('a, 'b, 'c, 'd),
    )
    : ('e, 'f, 'g, 'h) => (
  f(w),
  g(x),
  h(y),
  i(z),
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

let map_fst4 =
    (f: 'a => 'e, (w, x, y, z): ('a, 'b, 'c, 'd)): ('e, 'b, 'c, 'd) => (
  f(w),
  x,
  y,
  z,
);
let map_snd4 =
    (f: 'b => 'e, (w, x, y, z): ('a, 'b, 'c, 'd)): ('a, 'e, 'c, 'd) => (
  w,
  f(x),
  y,
  z,
);
let map_thd4 =
    (f: 'c => 'e, (w, x, y, z): ('a, 'b, 'c, 'd)): ('a, 'b, 'e, 'd) => (
  w,
  x,
  f(y),
  z,
);
let map_fth4 =
    (f: 'd => 'e, (w, x, y, z): ('a, 'b, 'c, 'd)): ('a, 'b, 'c, 'e) => (
  w,
  x,
  y,
  f(z),
);

let join2 = (f: ('a, 'b) => 'c, (x, y): ('a, 'b)): 'c => f(x, y);
let join3 = (f: ('a, 'b, 'c) => 'd, (x, y, z): ('a, 'b, 'c)): 'd =>
  f(x, y, z);
let join4 = (f: ('a, 'b, 'c, 'd) => 'e, (w, x, y, z): ('a, 'b, 'c, 'd)): 'e =>
  f(w, x, y, z);

let split2 = (f0: 'a => 'b, f1: 'a => 'c, x: 'a): ('b, 'c) => (
  f0(x),
  f1(x),
);
let split3 = (f0: 'a => 'b, f1: 'a => 'c, f2: 'a => 'd, x: 'a): ('b, 'c, 'd) => (
  f0(x),
  f1(x),
  f2(x),
);
let split4 =
    (f0: 'a => 'b, f1: 'a => 'c, f2: 'a => 'd, f3: 'a => 'e, x: 'a)
    : ('b, 'c, 'd, 'e) => (
  f0(x),
  f1(x),
  f2(x),
  f3(x),
);

let fold2 = (f: (('a, 'b)) => 'c, x: 'a, y: 'b): 'c => f((x, y));
let fold3 = (f: (('a, 'b, 'c)) => 'd, x: 'a, y: 'b, z: 'c): 'd =>
  f((x, y, z));
let fold4 = (f: (('a, 'b, 'c, 'd)) => 'e, w: 'a, x: 'b, y: 'c, z: 'd): 'e =>
  f((w, x, y, z));

/* pretty printing */

let pp2 = (ppa, ppb): Fmt.t(('a, 'b)) =>
  (ppf, (a, b)) => Fmt.pf(ppf, "(%a, %a)", ppa, a, ppb, b);

let pp3 = (ppa, ppb, ppc): Fmt.t(('a, 'b, 'c)) =>
  (ppf, (a, b, c)) => Fmt.pf(ppf, "(%a, %a, %a)", ppa, a, ppb, b, ppc, c);

let pp4 = (ppa, ppb, ppc, ppd): Fmt.t(('a, 'b, 'c, 'd)) =>
  (ppf, (a, b, c, d)) =>
    Fmt.pf(ppf, "(%a, %a, %a, %a)", ppa, a, ppb, b, ppc, c, ppd, d);
