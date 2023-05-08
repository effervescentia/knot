open Kore;

module Reporter = Executable.Reporter;
module Type = AST.Type;

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

finished with 24 error(s) and 0 warning(s)

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
    \n    • rename x so that there is no conflict with reserved keywords (import, from, main, const, enum, func, view, style, let, nil, true, false, as)

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

12) Invalid KSX Primitive Expression : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  ksx only supports rendering primitive values inline but received string[]
  \n  [code frame not available]
  \n  try one of the following to resolve this issue:
  \n    • convert the value to have a primitive type
        ie. nil, boolean, integer, float, string, element

13) Invalid KSX Tag : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  this ksx tag was expected to be of type view with props (my_attr: boolean) but received integer
  \n  [code frame not available]

14) Invalid KSX Attribute : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  this ksx tag expects the attribute my_attr to be of type integer but received boolean
  \n  [code frame not available]

15) Unexpected KSX Attribute : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  found an unexpected attribute my_attr with type boolean
  \n  [code frame not available]

16) Missing KSX Attributes : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  ksx tag MyTag is missing the attributes (my_bool: boolean, my_int: integer)
  \n  [code frame not available]

17) Invalid Dot Access : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  dot access can only be performed on values with struct types
  expected a value matching the type { my_prop: any } but received integer
  \n  [code frame not available]

18) Invalid Style Binding : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  style binding expects the left-side value to be a view
  and the right-side value to be style but received integer and string
  \n  [code frame not available]

19) Invalid Function Call : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  function calls can only be performed on values with function types
  expected a value matching the type (string, nil) -> any but received integer
  \n  [code frame not available]

20) Untyped Function Argument : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  the function argument my_argument must define a type
  \n  [code frame not available]

21) Default Argument Missing : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  the function argument my_argument must define a default value
  \n  [code frame not available]
  \n  try one of the following to resolve this issue:
  \n    • remove default values from all preceding arguments

22) Invalid Decorator Invocation : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  decorator invocations can only be performed on values with decorator types
  expected a value matching the type (string, nil) on target but received integer
  \n  [code frame not available]

23) Decorator Target Mismatch : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  this decorator can only target a module but found style
  \n  [code frame not available]

24) Must Use Explicit Children : bar/my_namespace.kn:0.0
  (foo/bar/my_namespace.kn:0.0)

  the explicitly defined children attribute of type string must be used over the implicit $children
  \n  [code frame not available]

finished with 24 error(s) and 0 warning(s)
",
          [
            ImportCycle(["a", "b", "c", "d"]),
            UnresolvedModule("my_module"),
            FileNotFound("/path/to/my/file"),
            InvalidModule(__namespace),
            ParseError(ReservedKeyword("x"), __namespace, Range.zero),
            ParseError(
              TypeError(NotFound("my_id")),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(TypeMismatch(Valid(String), Valid(Integer))),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(
                ExternalNotFound(
                  __namespace,
                  Reference.(Export.Named("my_export")),
                ),
              ),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(DuplicateIdentifier("my_export")),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(
                InvalidUnaryOperation(Negative, Type.Valid(String)),
              ),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  LogicalAnd,
                  Type.Valid(String),
                  Type.Valid(Integer),
                ),
              ),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(
                InvalidKSXPrimitiveExpression(
                  Type.Valid(List(Valid(String))),
                ),
              ),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(
                InvalidKSXTag(
                  "MyTag",
                  Type.Valid(Integer),
                  [("my_attr", Type.Valid(Boolean))],
                ),
              ),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(
                InvalidKSXAttribute(
                  "my_attr",
                  Type.Valid(Integer),
                  Type.Valid(Boolean),
                ),
              ),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(
                UnexpectedKSXAttribute("my_attr", Type.Valid(Boolean)),
              ),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(
                MissingKSXAttributes(
                  "MyTag",
                  [
                    ("my_bool", Type.Valid(Boolean)),
                    ("my_int", Type.Valid(Integer)),
                  ],
                ),
              ),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(InvalidDotAccess(Type.Valid(Integer), "my_prop")),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(
                InvalidStyleBinding(
                  Type.Valid(Integer),
                  Type.Valid(String),
                ),
              ),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(
                InvalidFunctionCall(
                  Type.Valid(Integer),
                  [Type.Valid(String), Type.Valid(Nil)],
                ),
              ),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(UntypedFunctionArgument("my_argument")),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(DefaultArgumentMissing("my_argument")),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(
                InvalidDecoratorInvocation(
                  Type.Valid(Integer),
                  [Type.Valid(String), Type.Valid(Nil)],
                ),
              ),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(
                DecoratorTargetMismatch(
                  Type.DecoratorTarget.Module,
                  Type.DecoratorTarget.Style,
                ),
              ),
              __namespace,
              Range.zero,
            ),
            ParseError(
              TypeError(MustUseExplicitChildren(Type.Valid(String))),
              __namespace,
              Range.zero,
            ),
          ]
          |> ~@((ppf, x) => Reporter.report(__resolver, x, ppf)),
        )
    ),
  ];
