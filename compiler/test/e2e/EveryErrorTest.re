open Kore;

module Build = Executable.Build;

let __semantic_cwd = fixture("every_semantic_error");
let __syntax_cwd = fixture("every_syntax_error");
let __program_cwd = fixture("every_program_error");
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

        Build.run(~report=_ => (@)(errors^) % (:=)(errors), global, config);

        Assert.compile_errors(
          [
            ParseError(
              TypeError(
                NotFound(Reference.Identifier.of_string("not_found")),
              ),
              __semantic_namespace,
              Range.create((1, 28), (1, 36)),
            ),
            ParseError(
              TypeError(InvalidUnaryOperation(Not, Type.Valid(`String))),
              __semantic_namespace,
              Range.create((8, 31), (8, 38)),
            ),
            ParseError(
              TypeError(
                InvalidUnaryOperation(Positive, Type.Valid(`Boolean)),
              ),
              __semantic_namespace,
              Range.create((9, 36), (9, 39)),
            ),
            ParseError(
              TypeError(InvalidUnaryOperation(Negative, Type.Valid(`Nil))),
              __semantic_namespace,
              Range.create((10, 35), (10, 37)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  LogicalAnd,
                  Type.Valid(`String),
                  Type.Valid(`Nil),
                ),
              ),
              __semantic_namespace,
              Range.create((12, 30), (12, 44)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  LogicalOr,
                  Type.Valid(`Element),
                  Type.Valid(`Integer),
                ),
              ),
              __semantic_namespace,
              Range.create((13, 30), (13, 42)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  LessOrEqual,
                  Type.Valid(`Boolean),
                  Type.Valid(`String),
                ),
              ),
              __semantic_namespace,
              Range.create((14, 30), (14, 46)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  LessThan,
                  Type.Valid(`Nil),
                  Type.Valid(`Boolean),
                ),
              ),
              __semantic_namespace,
              Range.create((15, 29), (15, 38)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  GreaterOrEqual,
                  Type.Valid(
                    `Function((
                      [Valid(`String), Valid(`Boolean)],
                      Valid(`Nil),
                    )),
                  ),
                  Type.Valid(`Element),
                ),
              ),
              __semantic_namespace,
              Range.create((16, 30), (16, 57)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  GreaterThan,
                  Type.Valid(`String),
                  Type.Valid(`Boolean),
                ),
              ),
              __semantic_namespace,
              Range.create((17, 29), (17, 54)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  Equal,
                  Type.Valid(`Integer),
                  Type.Valid(`Float),
                ),
              ),
              __semantic_namespace,
              Range.create((18, 35), (18, 47)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  Unequal,
                  Type.Valid(`Nil),
                  Type.Valid(`String),
                ),
              ),
              __semantic_namespace,
              Range.create((19, 37), (19, 45)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  Add,
                  Type.Valid(`String),
                  Type.Valid(`String),
                ),
              ),
              __semantic_namespace,
              Range.create((20, 30), (20, 47)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  Subtract,
                  Type.Valid(`Boolean),
                  Type.Valid(`Boolean),
                ),
              ),
              __semantic_namespace,
              Range.create((21, 35), (21, 46)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  Divide,
                  Type.Valid(`Nil),
                  Type.Valid(`Nil),
                ),
              ),
              __semantic_namespace,
              Range.create((22, 33), (22, 40)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  Multiply,
                  Type.Valid(`String),
                  Type.Valid(`Integer),
                ),
              ),
              __semantic_namespace,
              Range.create((23, 35), (23, 47)),
            ),
            ParseError(
              TypeError(
                InvalidBinaryOperation(
                  Exponent,
                  Type.Valid(`Boolean),
                  Type.Valid(`Nil),
                ),
              ),
              __semantic_namespace,
              Range.create((24, 35), (24, 44)),
            ),
            ParseError(
              TypeError(
                InvalidJSXPrimitiveExpression(
                  Type.Valid(
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
              TypeError(InvalidJSXClassExpression(Type.Valid(`String))),
              __semantic_namespace,
              Range.create((28, 56), (28, 63)),
            ),
            ParseError(
              TypeError(
                InvalidJSXTag(
                  Reference.Identifier.of_string("NotView"),
                  Type.Valid(`Nil),
                  [],
                ),
              ),
              __semantic_namespace,
              Range.create((31, 35), (31, 41)),
            ),
            ParseError(
              TypeError(
                UnexpectedJSXAttribute(
                  "unexpected_attr",
                  Type.Valid(`Boolean),
                ),
              ),
              __semantic_namespace,
              Range.create((35, 48), (35, 67)),
            ),
            ParseError(
              TypeError(
                InvalidJSXAttribute(
                  "first_attr",
                  Type.Valid(`String),
                  Type.Valid(`Boolean),
                ),
              ),
              __semantic_namespace,
              Range.create((37, 45), (37, 59)),
            ),
            ParseError(
              TypeError(
                MissingJSXAttributes(
                  Reference.Identifier.of_string("TwoAttrsView"),
                  [
                    ("second_attr", Type.Valid(`Boolean)),
                    ("first_attr", Type.Valid(`String)),
                  ],
                ),
              ),
              __semantic_namespace,
              Range.create((39, 33), (39, 44)),
            ),
            ParseError(
              TypeError(InvalidDotAccess(Type.Valid(`Boolean), "property")),
              __semantic_namespace,
              Range.create((41, 44), (41, 51)),
            ),
            ParseError(
              TypeError(
                InvalidFunctionCall(
                  Type.Valid(`String),
                  [Type.Valid(`Integer), Type.Valid(`Float)],
                ),
              ),
              __semantic_namespace,
              Range.create((43, 46), (43, 58)),
            ),
            ParseError(
              TypeError(
                UntypedFunctionArgument(
                  Reference.Identifier.of_string("second_arg"),
                ),
              ),
              __semantic_namespace,
              Range.create((45, 47), (45, 56)),
            ),
            ParseError(
              TypeError(
                ExternalNotFound(
                  __semantic_namespace,
                  Reference.Export.Named(
                    Reference.Identifier.of_string("DoesNotExist"),
                  ),
                ),
              ),
              Reference.Namespace.of_internal("main"),
              Range.create((1, 10), (1, 21)),
            ),
            ParseError(
              TypeError(
                ExternalNotFound(
                  Reference.Namespace.of_internal("private"),
                  Reference.Export.Named(
                    Reference.Identifier.of_string("_HIDDEN"),
                  ),
                ),
              ),
              Reference.Namespace.of_internal("main"),
              Range.create((2, 10), (2, 16)),
            ),
            ParseError(
              TypeError(
                ExternalNotFound(
                  Reference.Namespace.of_internal("private"),
                  Reference.Export.Named(
                    Reference.Identifier.of_string("_MAIN_ONLY"),
                  ),
                ),
              ),
              Reference.Namespace.of_internal("main"),
              Range.create((3, 10), (3, 19)),
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
