include Exception;

module LazyStream = Opal.LazyStream;
module Log = Log;
module Emoji = Emoji;

let (%) = (f, g, x) => f(x) |> g;

let some = x => Some(x);
let w_opt = (x, g, f) =>
  switch (f) {
  | Some(y) => g(y)
  | None => x
  };
let iff = (a, x, y) => w_opt(y, _ => x, a);
let is_some = a => iff(a, true, false);

let print_uchar = ch => {
  let buf = Buffer.create(128);
  Buffer.add_utf_8_uchar(buf, ch);

  Buffer.contents(buf);
};

let (|?>) = (f, g) => w_opt(None, g % some, f);
let (|!>) = (f, g) => w_opt((), g, f);
let (|^>) = (f, g) => w_opt(f, g % (_ => f), f);
let (|%>) = (f, g) => w_opt(false, g % (_ => true), f);

let (|-) = (f, g, x) =>
  f(x)
  |> (
    y => {
      g(x);
      y;
    }
  );

let knot_source_file_ext = ".kn";
let knot_types_file_ext = ".kd";
let knot_config_file = ".knot.yaml";

let reserved = [
  "import",
  "from",
  "const",
  "let",
  "func",
  "view",
  "state",
  "style",
  "else",
  "if",
  "as",
  "mut",
  "get",
  "main",
  "true",
  "false",
];

let main_alias = "[main]";
