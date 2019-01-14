open Core;

let gen_key =
  (
    fun
    | ClassKey(s) => Printf.sprintf(".%s", s)
    | IdKey(s) => Printf.sprintf("#%s", s)
  )
  % gen_string;

let gen_rule = ((name, value)) =>
  Printf.sprintf(
    "[%s]:%s",
    Expression.gen_reference(name),
    Expression.gen_reference(value),
  );

let gen_rule_set = ((key, rules)) =>
  Printf.sprintf("[%s]:{%s}", gen_key(key), gen_list(gen_rule, rules));