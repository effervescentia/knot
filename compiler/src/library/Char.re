include Input.Make({
  type t = (Uchar.t, Cursor.t);
  type f = Uchar.t;
  type s = Cursor.t;

  let fst = fst;
  let snd = snd;

  let make = (f, s) => (f, s);
});
