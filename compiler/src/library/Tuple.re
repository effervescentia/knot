/**
 Utilities for working with pairs of values.
 */

let map2 = (f: 'a => 'b, (x, y): ('a, 'a)): ('b, 'b) => (f(x), f(y));

let map_fst2 = (f: 'a => 'c, (x, y): ('a, 'b)): ('c, 'b) => (f(x), y);
let map_snd2 = (f: 'b => 'c, (x, y): ('a, 'b)): ('a, 'c) => (x, f(y));
