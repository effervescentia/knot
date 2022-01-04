open Kore;

let suite =
  "Library.Target"
  >::: [
    "of_string() - not recognized"
    >: (() => Assert.opt_target(None, Target.of_string("foo"))),
    "of_string() - knot"
    >: (
      () => Assert.opt_target(Some(Target.Knot), Target.of_string("knot"))
    ),
    "of_string() - javascript es6"
    >: (
      () =>
        Assert.opt_target(
          Some(Target.JavaScript(ES6)),
          Target.of_string("javascript-es6"),
        )
    ),
    "of_string() - javascript commonjs"
    >: (
      () =>
        Assert.opt_target(
          Some(Target.JavaScript(Common)),
          Target.of_string("javascript-common"),
        )
    ),
    "extension_of() - knot"
    >: (() => Assert.string(".kn", Target.extension_of(Knot))),
    "extension_of() - javascript es6"
    >: (() => Assert.string(".js", Target.extension_of(JavaScript(ES6)))),
    "extension_of() - javascript commonjs"
    >: (() => Assert.string(".js", Target.extension_of(JavaScript(Common)))),
    "pp() - knot"
    >: (() => Assert.string(Target.knot, Target.Knot |> ~@Target.pp)),
    "pp() - javscript es6"
    >: (
      () =>
        Assert.string(
          Target.javascript_es6,
          Target.JavaScript(ES6) |> ~@Target.pp,
        )
    ),
    "pp() - javscript commonjs"
    >: (
      () =>
        Assert.string(
          Target.javascript_common,
          Target.JavaScript(Common) |> ~@Target.pp,
        )
    ),
  ];
