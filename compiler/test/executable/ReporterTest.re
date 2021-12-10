open Kore;

module Reporter = Executable.Reporter;

let __namespace = Reference.Namespace.Internal("my_namespace");
let __resolver =
  Resolve.Resolver.{
    root_dir: "foo",
    source_dir: "bar",
    cache: File.Cache.create("fizz"),
  };

let suite =
  "Executable.Reporter"
  >::: [
    "report()"
    >: (
      () =>
        [
          (
            "
╔══════════════════════════════════════════════╗
║                    FAILED                    ║
╚══════════════════════════════════════════════╝

finished with 0 error(s) and 0 warning(s)

finished with 0 error(s) and 0 warning(s)
",
            [] |> Reporter.report(__resolver) |> Pretty.to_string,
          ),
          (
            "
╔══════════════════════════════════════════════╗
║                    FAILED                    ║
╚══════════════════════════════════════════════╝

finished with 1 error(s) and 0 warning(s)

1) Import Cycle Found

  import cycle between the following modules: a -> b -> c -> d

2) Unresolved Module

  could not resolve module: my_module

3) File Not Found

  could not find file with path: /path/to/my/file

4) Invalid Module : bar/my_namespace.kn:0:0
  (foo/bar/my_namespace.kn:0:0)

  failed to parse module: @/my_namespace

5) Reserved Keyword : bar/my_namespace.kn:0:0
  (foo/bar/my_namespace.kn:0:0)

  the reserved keyword x was used as an identifier [code frame not available]

  try one of the following to resolve this issue:

    • check that the identifier x is spelled correctly

    • rename x so that there is no conflict with reserved keywords (import, from, main, const, let, nil, true, false, as)

6) Type Resolution Failed : bar/my_namespace.kn:0:0
  (foo/bar/my_namespace.kn:0:0)

   [code frame not available]

7) Type Resolution Failed : bar/my_namespace.kn:0:0
  (foo/bar/my_namespace.kn:0:0)

   [code frame not available]

8) Types Do Not Match : bar/my_namespace.kn:0:0
  (foo/bar/my_namespace.kn:0:0)

  expected the type string but found the type int instead

   [code frame not available]

9) Type Cannot Be Assigned : bar/my_namespace.kn:0:0
  (foo/bar/my_namespace.kn:0:0)

  expected a type that implements the trait number which is shared by the types int and float but found the type string instead

  [code frame not available]

10) External Not Found : bar/my_namespace.kn:0:0
  (foo/bar/my_namespace.kn:0:0)

  an export with the identifier my_export could not be found in module @/my_namespace

  [code frame not available]

11) Identifier Already Defined : bar/my_namespace.kn:0:0
  (foo/bar/my_namespace.kn:0:0)

  a variable with the same name (my_export) already exists in the local scope or an inherited scope

  [code frame not available]

  try one of the following to resolve this issue:

    • change the name of this variable

finished with 1 error(s) and 0 warning(s)
",
            [
              ImportCycle(["a", "b", "c", "d"]),
              UnresolvedModule("my_module"),
              FileNotFound("/path/to/my/file"),
              InvalidModule(__namespace),
              ParseError(ReservedKeyword("x"), __namespace, Range.zero),
              ParseError(
                TypeError(TypeResolutionFailed),
                __namespace,
                Range.zero,
              ),
              ParseError(
                TypeError(
                  NotFound(Reference.Identifier.of_string("my_id")),
                ),
                __namespace,
                Range.zero,
              ),
              ParseError(
                TypeError(TypeMismatch(Strong(`String), Strong(`Integer))),
                __namespace,
                Range.zero,
              ),
              ParseError(
                TypeError(
                  NotAssignable(Strong(`String), Type.Trait.Number),
                ),
                __namespace,
                Range.zero,
              ),
              ParseError(
                TypeError(
                  ExternalNotFound(
                    __namespace,
                    Reference.(
                      Export.Named(Identifier.of_string("my_export"))
                    ),
                  ),
                ),
                __namespace,
                Range.zero,
              ),
              ParseError(
                TypeError(
                  DuplicateIdentifier(
                    Reference.Identifier.of_string("my_export"),
                  ),
                ),
                __namespace,
                Range.zero,
              ),
            ]
            |> Reporter.report(__resolver)
            |> Pretty.to_string,
          ),
        ]
        |> Assert.(test_many(string))
    ),
  ];
