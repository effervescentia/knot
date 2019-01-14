let gen_key = (~prefix="", name) =>
  Str.string_match(Str.regexp_string("^[_a-zA-z][_a-zA-Z0-9]*$"), name, 0) ?
    Printf.sprintf("%s%s", prefix, name) : Printf.sprintf("['%s']", name);

let gen_access = gen_key(~prefix=".");