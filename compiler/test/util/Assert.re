module type Target = {
  type t;

  let test: (t, t) => unit;
};

let test_many = test => List.iter(((exp, act)) => test(exp, act));
let test_all = (test, exp) => List.iter(act => test(exp, act));

module Make = (T: Target) => {
  include T;

  let test_many = List.iter(((i, o)) => test(o, i));

  let test_all = o => List.iter(i => test(o, i));
};

let true_ = Alcotest.(check(bool, "boolean matches", true));
let false_ = Alcotest.(check(bool, "boolean matches", false));
let string = Alcotest.(check(string, "string matches"));

let string_list = Alcotest.(check(list(string), "string list matches"));

let string_pair_list =
  Alcotest.(check(list(pair(string, string)), "string pair list matches"));
