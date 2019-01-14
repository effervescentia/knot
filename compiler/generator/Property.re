let gen_key = (~prefix="", name) =>
  Str.string_match(Str.regexp("^[_a-zA-Z][_a-zA-Z0-9]*$"), name, 0) ?
    Printf.sprintf("%s%s", prefix, name) : Printf.sprintf("['%s']", name);

let gen_access = gen_key(~prefix=".");