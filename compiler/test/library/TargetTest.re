open Kore;

let suite =
  "Library.Target"
  >::: [
    "of_string()"
    >: (
      () =>
        [
          (None, Target.of_string("foo")),
          (Some(Target.Knot), Target.of_string("knot")),
          (
            Some(Target.JavaScript(ES6)),
            Target.of_string("javascript-es6"),
          ),
          (
            Some(Target.JavaScript(Common)),
            Target.of_string("javascript-common"),
          ),
        ]
        |> Assert.(test_many(opt_target))
    ),
    "extension_of()"
    >: (
      () =>
        [
          (".kn", Target.extension_of(Knot)),
          (".js", Target.extension_of(JavaScript(ES6))),
          (".js", Target.extension_of(JavaScript(Common))),
        ]
        |> Assert.(test_many(string))
    ),
  ];
