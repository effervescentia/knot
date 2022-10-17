open Kore;

module Build = Executable.Build;

let __semantic_cwd = fixture("every_semantic_error");
let __syntax_cwd = fixture("every_syntax_error");
let __program_cwd = fixture("every_program_error");
let __mock_library_file = fixture("mock_library.kd");
let __semantic_namespace = Reference.Namespace.of_internal("semantic");
let __syntax_namespace = Reference.Namespace.of_internal("syntax");

let suite =
  "Build | Every Error"
  >::: [
    "semantic errors"
    >: (
      () => {
        let errors = ref([]);
        let argv = [|"knotc", "build"|];
        let (global, config) = process_build_cmd(__semantic_cwd, argv);

        Build.run(
          ~report=_ => (@)(errors^) % (:=)(errors),
          global,
          {...config, ambient: __mock_library_file},
        );

        Assert.compile_errors(
          [
            ParseError(
              TypeError(NotFound("not_found")),
              __semantic_namespace,
              Range.create((1, 28), (1, 36)),
            ),
            ParseError(
              TypeError(InvalidUnaryOperation(Not, Valid(`String))),
              __semantic_namespace,
              Range.create((8, 31), (8, 38)),
            ),
            ParseError(
              TypeError(InvalidUnaryOperation(Positive, Valid(`Boolean))),
              __semantic_namespace,
              Range.create((9, 36), (9, 39)),
            ),
            ParseError(
              TypeError(InvalidUnaryOperation(Negative, Valid(`Nil))),
              __semantic_namespace,
              Range.create((10, 35), (10, 37)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  LogicalAnd,
                  Valid(`String),
                  Valid(`Nil),
                ),
              ),
              __semantic_namespace,
              Range.create((12, 30), (12, 44)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  LogicalOr,
                  Valid(`Element),
                  Valid(`Integer),
                ),
              ),
              __semantic_namespace,
              Range.create((13, 30), (13, 42)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  LessOrEqual,
                  Valid(`Boolean),
                  Valid(`String),
                ),
              ),
              __semantic_namespace,
              Range.create((14, 30), (14, 46)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  LessThan,
                  Valid(`Nil),
                  Valid(`Boolean),
                ),
              ),
              __semantic_namespace,
              Range.create((15, 29), (15, 38)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  GreaterOrEqual,
                  Valid(
                    `Function((
                      [Valid(`String), Valid(`Boolean)],
                      Valid(`Nil),
                    )),
                  ),
                  Valid(`Element),
                ),
              ),
              __semantic_namespace,
              Range.create((16, 30), (16, 57)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  GreaterThan,
                  Valid(`String),
                  Valid(`Boolean),
                ),
              ),
              __semantic_namespace,
              Range.create((17, 29), (17, 54)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  Equal,
                  Valid(`Integer),
                  Valid(`Float),
                ),
              ),
              __semantic_namespace,
              Range.create((18, 35), (18, 47)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  Unequal,
                  Valid(`Nil),
                  Valid(`String),
                ),
              ),
              __semantic_namespace,
              Range.create((19, 37), (19, 45)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(Add, Valid(`String), Valid(`String)),
              ),
              __semantic_namespace,
              Range.create((20, 30), (20, 47)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  Subtract,
                  Valid(`Boolean),
                  Valid(`Boolean),
                ),
              ),
              __semantic_namespace,
              Range.create((21, 35), (21, 46)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(Divide, Valid(`Nil), Valid(`Nil)),
              ),
              __semantic_namespace,
              Range.create((22, 33), (22, 40)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  Multiply,
                  Valid(`String),
                  Valid(`Integer),
                ),
              ),
              __semantic_namespace,
              Range.create((23, 35), (23, 47)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  Exponent,
                  Valid(`Boolean),
                  Valid(`Nil),
                ),
              ),
              __semantic_namespace,
              Range.create((24, 35), (24, 44)),
            ),
            ParseError(
              TypeError(
                InvalidJSXPrimitiveExpression(
                  Valid(
                    `Function((
                      [Valid(`String), Valid(`Boolean)],
                      Valid(`Nil),
                    )),
                  ),
                ),
              ),
              __semantic_namespace,
              Range.create((26, 41), (26, 57)),
            ),
            ParseError(
              TypeError(InvalidJSXClassExpression(Valid(`String))),
              __semantic_namespace,
              Range.create((28, 56), (28, 63)),
            ),
            ParseError(
              TypeError(InvalidJSXTag("NotView", Valid(`Nil), [])),
              __semantic_namespace,
              Range.create((31, 35), (31, 41)),
            ),
            ParseError(
              TypeError(
                UnexpectedJSXAttribute("unexpected_attr", Valid(`Boolean)),
              ),
              __semantic_namespace,
              Range.create((35, 48), (35, 67)),
            ),
            ParseError(
              TypeError(
                InvalidJSXAttribute(
                  "first_attr",
                  Valid(`String),
                  Valid(`Boolean),
                ),
              ),
              __semantic_namespace,
              Range.create((37, 45), (37, 59)),
            ),
            ParseError(
              TypeError(
                MissingJSXAttributes(
                  "TwoAttrsView",
                  [
                    ("second_attr", Valid(`Boolean)),
                    ("first_attr", Valid(`String)),
                  ],
                ),
              ),
              __semantic_namespace,
              Range.create((39, 33), (39, 44)),
            ),
            ParseError(
              TypeError(InvalidDotAccess(Valid(`Boolean), "property")),
              __semantic_namespace,
              Range.create((41, 44), (41, 51)),
            ),
            ParseError(
              TypeError(
                InvalidFunctionCall(
                  Valid(`String),
                  [Valid(`Integer), Valid(`Float)],
                ),
              ),
              __semantic_namespace,
              Range.create((43, 46), (43, 58)),
            ),
            ParseError(
              TypeError(UntypedFunctionArgument("second_arg")),
              __semantic_namespace,
              Range.create((45, 47), (45, 56)),
            ),
            ParseError(
              TypeError(UnknownStyleRule("foo")),
              __semantic_namespace,
              Range.create((48, 3), (48, 5)),
            ),
            ParseError(
              TypeError(
                InvalidStyleRule(
                  "mockColor",
                  Valid(`Enumerated([("Named", [Valid(`String)])])),
                  Valid(`Integer),
                ),
              ),
              __semantic_namespace,
              Range.create((51, 28), (53, 1)),
            ),
            ParseError(
              TypeError(
                ExternalNotFound(
                  __semantic_namespace,
                  Reference.Export.Named("DoesNotExist"),
                ),
              ),
              Reference.Namespace.of_internal("main"),
              Range.create((1, 10), (1, 21)),
            ),
          ],
          errors^,
        );
      }
    ),
    "syntax errors"
    >: (
      () => {
        let errors = ref([]);
        let argv = [|"knotc", "build"|];
        let (global, config) = process_build_cmd(__syntax_cwd, argv);

        Build.run(~report=_ => (@)(errors^) % (:=)(errors), global, config);

        Assert.compile_errors(
          [
            ParseError(
              ReservedKeyword("const"),
              __syntax_namespace,
              Range.create((1, 7), (1, 11)),
            ),
          ],
          errors^,
        );
      }
    ),
    "program errors"
    >: (
      () => {
        let errors = ref([]);
        let argv = [|"knotc", "build"|];
        let (global, config) = process_build_cmd(__program_cwd, argv);

        Build.run(~report=_ => (@)(errors^) % (:=)(errors), global, config);

        Assert.compile_errors(
          [
            /* InvalidModule(Reference.Namespace.of_internal("empty")), */
            FileNotFound("src/does_not_exist.kn"),
            /* FIXME: this should only be reported once */
            FileNotFound("src/does_not_exist.kn"),
          ],
          errors^,
        );
      }
    ),
  ];
