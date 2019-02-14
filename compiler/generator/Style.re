open Core;

let gen_key =
  fun
  | ClassKey(s) => Printf.sprintf(".%s", s)
  | IdKey(s) => Printf.sprintf("#%s", s);

let gen_rule = ((name, value)) =>
  Printf.sprintf(
    "[%s]:%s",
    unwrap(name) |> Reference.generate(Expression.generate),
    unwrap(value) |> Reference.generate(Expression.generate),
  );

let gen_rule_set = ((key, rules)) =>
  Printf.sprintf(
    "['%s']:{%s}",
    unwrap(key) |> gen_key,
    gen_list(unwrap % gen_rule, rules),
  );