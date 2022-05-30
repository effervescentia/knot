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
    "report() - no errors or warnings"
    >: (
      () =>
        Assert.string(
          "
╔══════════════════════════════════════════════╗
║                    FAILED                    ║
╚══════════════════════════════════════════════╝

finished with 0 error(s) and 0 warning(s)

finished with 0 error(s) and 0 warning(s)
",
          [] |> ~@((ppf, x) => Reporter.report(__resolver, x, ppf)),
        )
    ),
    "report() - all errors"
    >: (
      () =>
        Assert.string(
          "
╔══════════════════════════════════════════════╗
║                    FAILED                    ║
╚══════════════════════════════════════════════╝

finished with 17 error(s) and 0 warning(s)

1) Import Cycle Found

  import cycle found between modules a -> b -> c -> d

2) Unresolved Module

  could not resolve module my_module

3) File Not Found

  could not find file with path /path/to/my/file

4) Invalid Module : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  failed to parse module @/my_namespace

5) Reserved Keyword : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  the reserved keyword x was used as an identifier
  \n  [code frame not available]
  \n  try one of the following to resolve this issue:
  \n    • check that the identifier x is spelled correctly
    \n    • rename x so that there is no conflict with reserved keywords (import, from, main, const, let, nil, true, false, as)

6) Identifier Not Found : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  unable to resolve an identifier my_id in the local scope or any inherited scope
  \n  [code frame not available]
  \n  try one of the following to resolve this issue:
  \n    • check that the identifier my_id is spelled correctly
    \n    • define the value yourself
    \n      const my_id = …;
      // or
      let my_id = …;
    \n    • import the value from another module
    \n      import { my_id } from \"…\";

7) Types Do Not Match : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  expected the type string but received integer
  \n  [code frame not available]

8) External Not Found : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  an export with the identifier my_export could not be found in module @/my_namespace
  \n  [code frame not available]

9) Identifier Already Defined : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  a variable with the same name (my_export) already exists in the local scope or an inherited scope
  \n  [code frame not available]
  \n  try one of the following to resolve this issue:
  \n    • change the name of this variable

10) Invalid Unary Operation : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  the NEGATE (-) unary operator expects an expression of type float or integer but received string
  \n  [code frame not available]

11) Invalid Binary Operation : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  the AND (&&) binary operator expects both arguments to be of type boolean but received string and integer
  \n  [code frame not available]

12) Invalid JSX Primitive Expression : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  jsx only supports rendering primitive values inline but received string[]
  \n  [code frame not available]
  \n  try one of the following to resolve this issue:
  \n    • convert the value to have a primitive type
        ie. nil, boolean, integer, float, string, element

13) Invalid JSX Class Expression : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  jsx classes can only be controlled with arguments of type boolean but received integer
  \n  [code frame not available]

14) Invalid Dot Access : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  dot access can only be performed on values with struct types
  expected a value matching the type { my_prop: any } but received integer
  \n  [code frame not available]

15) Invalid Function Call : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  function calls can only be performed on values with function types
  expected a value matching the type (string, nil) -> any but received integer
  \n  [code frame not available]

16) Untyped Function Argument : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  the function argument my_argument must define a type
  \n  [code frame not available]

17) Default Argument Missing : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  the function argument my_argument must define a default value
  \n  [code frame not available]
  \n  try one of the following to resolve this issue:
  \n    • remove default values from all preceding arguments

finished with 17 error(s) and 0 warning(s)
",
          [
            ImportCycle(["a", "b", "c", "d"]),
            UnresolvedModule("my_module"),
            FileNotFound("/path/to/my/file"),
            InvalidModule(__namespace),
            ParseError(ReservedKeyword("x"), __namespace, Range.zero),
            ParseError(
              TypeError(NotFound(Reference.Identifier.of_string("my_id"))),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(TypeMismatch(Valid(`String), Valid(`Integer))),
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
            ParseError(
              TypeError(
                InvalidUnaryOperation(Negative, Type.Valid(`String)),
              ),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  LogicalAnd,
                  Type.Valid(`String),
                  Type.Valid(`Integer),
                ),
              ),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(
                InvalidJSXPrimitiveExpression(
                  Type.Valid(`List(Valid(`String))),
                ),
              ),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(InvalidJSXClassExpression(Type.Valid(`Integer))),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(InvalidDotAccess(Type.Valid(`Integer), "my_prop")),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(
                InvalidFunctionCall(
                  Type.Valid(`Integer),
                  [Type.Valid(`String), Type.Valid(`Nil)],
                ),
              ),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(
                UntypedFunctionArgument(
                  Reference.Identifier.of_string("my_argument"),
                ),
              ),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(
                DefaultArgumentMissing(
                  Reference.Identifier.of_string("my_argument"),
                ),
              ),
              __namespace,
              Range.zero,
            ),
          ]
          |> ~@((ppf, x) => Reporter.report(__resolver, x, ppf)),
        )
    ),
  ];
