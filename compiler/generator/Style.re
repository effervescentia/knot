open Core;

let gen_key =
  fun
  | ClassKey(s) => Printf.sprintf(".%s", s)
  | IdKey(s) => Printf.sprintf("#%s", s);

let gen_rule = (((name, _), (value, _))) =>
  Printf.sprintf(
    "[%s]:%s",
    Reference.generate(Expression.generate, name),
    Reference.generate(Expression.generate, value),
  );

let gen_rule_set = ((key, rules)) =>
  Printf.sprintf("['%s']:{%s}", gen_key(key), gen_list(gen_rule, rules));
