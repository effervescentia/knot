/**
 Utilities for working with pairs of values.
 */

type t2('a) = ('a, 'a);
type t3('a) = ('a, 'a, 'a);

/* methods */

let map2 = (f: 'a => 'b, (x, y): ('a, 'a)): ('b, 'b) => (f(x), f(y));

let map_fst2 = (f: 'a => 'c, (x, y): ('a, 'b)): ('c, 'b) => (f(x), y);
let map_snd2 = (f: 'b => 'c, (x, y): ('a, 'b)): ('a, 'c) => (x, f(y));

let map3 = (f: 'a => 'b, (x, y, z): ('a, 'a, 'a)): ('b, 'b, 'b) => (
  f(x),
  f(y),
  f(z),
);
let fst3 = ((x, y, z): ('a, 'b, 'c)): 'a => x;
let snd3 = ((x, y, z): ('a, 'b, 'c)): 'b => y;
let thd3 = ((x, y, z): ('a, 'b, 'c)): 'c => z;

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

let reduce2 = (f: ('a, 'b) => 'c, (x, y): ('a, 'b)): 'c => f(x, y);
let reduce3 = (f: ('a, 'b, 'c) => 'd, (x, y, z): ('a, 'b, 'c)): 'd =>
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
