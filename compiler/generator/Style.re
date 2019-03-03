open Core;

let gen_key =
  fun
  | ClassKey(s) => Printf.sprintf(".%s", s)
  | IdKey(s) => Printf.sprintf("#%s", s);

let gen_rule = ((name, value)) =>
  Printf.sprintf(
    "[%s]:%s",
    abandon_ctx(name) |> Reference.generate(Expression.generate),
    abandon_ctx(value) |> Reference.generate(Expression.generate),
  );

let gen_rule_set = ((key, rules)) =>
  Printf.sprintf("['%s']:{%s}", gen_key(key), gen_list(gen_rule, rules));
