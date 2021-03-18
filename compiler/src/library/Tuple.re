/**
 Utilities for working with pairs of values.
 */

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
