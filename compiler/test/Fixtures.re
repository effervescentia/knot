open Globals;
open KnotGenerate.Globals;

let all_tokens = [
  Plus,
  ForwardSlash,
  Plus,
  Minus,
  Asterisk,
  Space,
  Ampersand,
  ForwardSlash,
  LeftBrace,
  Space,
  Number(9),
  LeftParenthese,
  Number(3),
  LeftChevron,
  Number(2),
  Equals,
  Space,
  Space,
  Number(90),
  Lambda,
  Newline,
  String("BC"),
  Space,
  LeftBracket,
  Minus,
  Plus,
  Asterisk,
  RightChevron,
  ForwardSlash,
  RightBracket,
  GreaterThanOrEqual,
  Space,
  Number(300),
  Assign,
  DollarSign,
  Newline,
  Number(5),
  Space,
  Number(412),
  RightParenthese,
  VerticalBar,
  RightParenthese,
  Space,
  RightBrace,
  LineComment(" dth asd elkjqw"),
  Newline,
  JSXOpenEnd,
  String("123123"),
  Space,
  Equals,
  BlockComment("\n.a41#@\n"),
  Tilde,
  LogicalOr,
  LogicalAnd,
  LessThanOrEqual,
  Assign,
  JSXSelfClose,
  Newline,
  Newline,
  JSXSelfClose,
  LogicalOr,
  JSXOpenEnd,
  Space,
  LogicalAnd,
  Newline,
  Keyword(Main),
  Space,
  Keyword(Import),
  Space,
  Keyword(Const),
  Space,
  Keyword(Let),
  Space,
  Keyword(State),
  Space,
  Keyword(View),
  Space,
  Keyword(Func),
  Space,
  Keyword(If),
  Space,
  Keyword(Else),
  Space,
  Keyword(Get),
  Space,
  Keyword(Mut),
  Newline,
  Identifier("mainer"),
  Space,
  Identifier("importest"),
  Space,
  Identifier("constant"),
  Space,
  Identifier("letter"),
  Space,
  Identifier("stated"),
  Space,
  Identifier("viewing"),
  Space,
  Identifier("functor"),
  Space,
  Identifier("iffer"),
  Space,
  Identifier("elsern"),
  Space,
  Identifier("getting"),
  Space,
  Identifier("mutter"),
  Space,
  Identifier("igloo"),
  Space,
  Identifier("moron"),
];

let full_ast =
  Module(
    [Import("abc", [no_ctx(MainExport("ABC"))])],
    [
      Declaration(
        "numericConst",
        no_ctx(ConstDecl(no_ctx(NumericLit(8)))),
      ),
      Declaration(
        "additionConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              AddExpr(no_ctx(NumericLit(1)), no_ctx(NumericLit(10))),
            ),
          ),
        ),
      ),
      Declaration(
        "subtractionConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              SubExpr(no_ctx(NumericLit(8)), no_ctx(NumericLit(2))),
            ),
          ),
        ),
      ),
      Declaration(
        "multiplicationConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              MulExpr(no_ctx(NumericLit(2)), no_ctx(NumericLit(3))),
            ),
          ),
        ),
      ),
      Declaration(
        "divisionConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              DivExpr(no_ctx(NumericLit(4)), no_ctx(NumericLit(2))),
            ),
          ),
        ),
      ),
      Declaration(
        "stringConst",
        no_ctx(ConstDecl(no_ctx(StringLit("Hello, World!")))),
      ),
      Declaration(
        "trueConst",
        no_ctx(ConstDecl(no_ctx(BooleanLit(true)))),
      ),
      Declaration(
        "falseConst",
        no_ctx(ConstDecl(no_ctx(BooleanLit(false)))),
      ),
      Declaration(
        "lessThanConst",
        no_ctx(
          ConstDecl(
            no_ctx(LTExpr(no_ctx(NumericLit(7)), no_ctx(NumericLit(9)))),
          ),
        ),
      ),
      Declaration(
        "lessThanEqualConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              LTEExpr(no_ctx(NumericLit(8)), no_ctx(NumericLit(2))),
            ),
          ),
        ),
      ),
      Declaration(
        "greaterThanConst",
        no_ctx(
          ConstDecl(
            no_ctx(GTExpr(no_ctx(NumericLit(2)), no_ctx(NumericLit(4)))),
          ),
        ),
      ),
      Declaration(
        "greaterThanEqualConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              GTEExpr(no_ctx(NumericLit(9)), no_ctx(NumericLit(1))),
            ),
          ),
        ),
      ),
      Declaration(
        "closureConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              AddExpr(
                no_ctx(
                  MulExpr(no_ctx(NumericLit(3)), no_ctx(NumericLit(2))),
                ),
                no_ctx(
                  AddExpr(
                    no_ctx(NumericLit(1)),
                    no_ctx(
                      MulExpr(
                        no_ctx(
                          DivExpr(
                            no_ctx(NumericLit(6)),
                            no_ctx(
                              SubExpr(
                                no_ctx(NumericLit(2)),
                                no_ctx(NumericLit(5)),
                              ),
                            ),
                          ),
                        ),
                        no_ctx(NumericLit(3)),
                      ),
                    ),
                  ),
                ),
              ),
            ),
          ),
        ),
      ),
      Declaration(
        "dotAccessConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              Reference(
                no_ctx(
                  DotAccess(
                    no_ctx(DotAccess(no_ctx(Variable("a")), "b")),
                    "c",
                  ),
                ),
              ),
            ),
          ),
        ),
      ),
      Declaration(
        "executionConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              Reference(
                no_ctx(
                  Execution(
                    no_ctx(
                      DotAccess(
                        no_ctx(DotAccess(no_ctx(Variable("d")), "e")),
                        "f",
                      ),
                    ),
                    [
                      no_ctx(NumericLit(4)),
                      no_ctx(
                        Reference(
                          no_ctx(DotAccess(no_ctx(Variable("a")), "x")),
                        ),
                      ),
                      no_ctx(
                        MulExpr(
                          no_ctx(NumericLit(20)),
                          no_ctx(NumericLit(3)),
                        ),
                      ),
                      no_ctx(
                        Reference(
                          no_ctx(Execution(no_ctx(Variable("m")), [])),
                        ),
                      ),
                    ],
                  ),
                ),
              ),
            ),
          ),
        ),
      ),
      Declaration(
        "jsxConst",
        no_ctx(ConstDecl(no_ctx(JSX(Element("abc", [], [], []))))),
      ),
      Declaration(
        "jsxWithPropsConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              JSX(
                Element(
                  "def",
                  [],
                  [
                    (
                      "num",
                      no_ctx(
                        AddExpr(
                          no_ctx(NumericLit(8)),
                          no_ctx(NumericLit(9)),
                        ),
                      ),
                    ),
                    ("bool", no_ctx(BooleanLit(false))),
                    ("first", no_ctx(StringLit("look"))),
                    ("under", no_ctx(StringLit("there"))),
                  ],
                  [],
                ),
              ),
            ),
          ),
        ),
      ),
      Declaration(
        "nestedJSXConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              JSX(
                Element(
                  "parent",
                  [],
                  [("justMade", no_ctx(StringLit("you say")))],
                  [
                    Element(
                      "child",
                      [],
                      [
                        (
                          "variable",
                          no_ctx(Reference(no_ctx(Variable("variable")))),
                        ),
                      ],
                      [
                        Element(
                          "grandchild",
                          [],
                          [("under", no_ctx(StringLit("wear")))],
                          [],
                        ),
                      ],
                    ),
                    Element("sibling", [], [], []),
                  ],
                ),
              ),
            ),
          ),
        ),
      ),
      Declaration(
        "nestedExprJSXConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              JSX(
                Element(
                  "container",
                  [],
                  [],
                  [
                    EvalNode(
                      no_ctx(
                        AddExpr(
                          no_ctx(Reference(no_ctx(Variable("value")))),
                          no_ctx(NumericLit(20)),
                        ),
                      ),
                    ),
                    Element("middle", [], [], []),
                    EvalNode(
                      no_ctx(
                        AddExpr(
                          no_ctx(Reference(no_ctx(Variable("another")))),
                          no_ctx(StringLit("one")),
                        ),
                      ),
                    ),
                  ],
                ),
              ),
            ),
          ),
        ),
      ),
      Declaration(
        "fragmentJSXConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              JSX(
                Fragment([
                  Element("div", [], [], []),
                  Element("span", [], [], []),
                ]),
              ),
            ),
          ),
        ),
      ),
      Declaration(
        "compactFunc",
        no_ctx(
          FunctionDecl(
            [],
            [no_ctx(ExpressionStatement(no_ctx(NumericLit(4))))],
          ),
        ),
      ),
      Declaration(
        "compactExprFunc",
        no_ctx(
          FunctionDecl(
            [],
            [
              no_ctx(
                ExpressionStatement(
                  no_ctx(
                    AddExpr(
                      no_ctx(Reference(no_ctx(Variable("A")))),
                      no_ctx(Reference(no_ctx(Variable("B")))),
                    ),
                  ),
                ),
              ),
            ],
          ),
        ),
      ),
      Declaration(
        "multiExprFunc",
        no_ctx(
          FunctionDecl(
            [],
            [
              no_ctx(
                ExpressionStatement(
                  no_ctx(
                    AddExpr(
                      no_ctx(Reference(no_ctx(Variable("e")))),
                      no_ctx(Reference(no_ctx(Variable("f")))),
                    ),
                  ),
                ),
              ),
              no_ctx(
                ExpressionStatement(
                  no_ctx(Reference(no_ctx(Variable("j")))),
                ),
              ),
            ],
          ),
        ),
      ),
      Declaration(
        "paramFunc",
        no_ctx(
          FunctionDecl(
            [no_ctx(("a", None, None))],
            [
              no_ctx(
                ExpressionStatement(
                  no_ctx(Reference(no_ctx(Variable("a")))),
                ),
              ),
            ],
          ),
        ),
      ),
      Declaration("NoParamsState", no_ctx(StateDecl([], []))),
      Declaration("EmptyState", no_ctx(StateDecl([], []))),
      Declaration(
        "DefaultParamState",
        no_ctx(
          StateDecl(
            [no_ctx(("z", None, Some(no_ctx(NumericLit(30)))))],
            [],
          ),
        ),
      ),
      Declaration(
        "ComplexState",
        no_ctx(
          StateDecl(
            [],
            [
              ("a", no_ctx(`Property((Some(no_ctx("b")), None)))),
              ("_c", no_ctx(`Property((Some(no_ctx("d")), None)))),
              (
                "e",
                no_ctx(
                  `Getter((
                    [],
                    [no_ctx(ExpressionStatement(no_ctx(NumericLit(4))))],
                  )),
                ),
              ),
              (
                "f",
                no_ctx(
                  `Getter((
                    [],
                    [no_ctx(ExpressionStatement(no_ctx(NumericLit(5))))],
                  )),
                ),
              ),
              (
                "g",
                no_ctx(
                  `Getter((
                    [
                      no_ctx(("h", Some(no_ctx("j")), None)),
                      no_ctx(("k", None, Some(no_ctx(NumericLit(2))))),
                      no_ctx((
                        "l",
                        Some(no_ctx("m")),
                        Some(no_ctx(NumericLit(20))),
                      )),
                    ],
                    [no_ctx(ExpressionStatement(no_ctx(NumericLit(5))))],
                  )),
                ),
              ),
              (
                "n",
                no_ctx(
                  `Getter((
                    [],
                    [
                      no_ctx(ExpressionStatement(no_ctx(NumericLit(3)))),
                      no_ctx(
                        ExpressionStatement(
                          no_ctx(
                            AddExpr(
                              no_ctx(Reference(no_ctx(Variable("A")))),
                              no_ctx(Reference(no_ctx(Variable("m")))),
                            ),
                          ),
                        ),
                      ),
                    ],
                  )),
                ),
              ),
            ],
          ),
        ),
      ),
      Declaration("NoParamsView", no_ctx(ViewDecl(None, [], [], []))),
      Declaration(
        "ParamView",
        no_ctx(ViewDecl(None, [], [no_ctx(("m", None, None))], [])),
      ),
      Declaration(
        "TypedParamView",
        no_ctx(
          ViewDecl(
            None,
            [],
            [no_ctx(("a", Some(no_ctx("b")), None))],
            [],
          ),
        ),
      ),
      Declaration(
        "DefaultParamView",
        no_ctx(
          ViewDecl(
            None,
            [],
            [no_ctx(("a", None, Some(no_ctx(NumericLit(4)))))],
            [],
          ),
        ),
      ),
      Declaration(
        "MultiParamView",
        no_ctx(
          ViewDecl(
            None,
            [],
            [
              no_ctx(("m", Some(no_ctx("n")), None)),
              no_ctx((
                "a",
                Some(no_ctx("b")),
                Some(no_ctx(NumericLit(2))),
              )),
            ],
            [],
          ),
        ),
      ),
      Declaration(
        "InheritingView",
        no_ctx(
          ViewDecl(
            Some(no_ctx("SuperView")),
            [],
            [],
            [
              no_ctx(
                ExpressionStatement(
                  no_ctx(
                    AddExpr(
                      no_ctx(Reference(no_ctx(Variable("a")))),
                      no_ctx(Reference(no_ctx(Variable("b")))),
                    ),
                  ),
                ),
              ),
              no_ctx(ExpressionStatement(no_ctx(NumericLit(8)))),
            ],
          ),
        ),
      ),
      Declaration(
        "MixinView",
        no_ctx(ViewDecl(None, [no_ctx("MyMixin")], [], [])),
      ),
      Declaration(
        "InheritingMixinView",
        no_ctx(
          ViewDecl(
            Some(no_ctx("SuperView")),
            [no_ctx("MyMixin")],
            [],
            [],
          ),
        ),
      ),
      Declaration(
        "ComplexView",
        no_ctx(
          ViewDecl(
            Some(no_ctx("SuperView")),
            [no_ctx("MyMixin"), no_ctx("OtherMixin")],
            [],
            [
              no_ctx(
                ExpressionStatement(
                  no_ctx(
                    AddExpr(
                      no_ctx(Reference(no_ctx(Variable("e")))),
                      no_ctx(Reference(no_ctx(Variable("f")))),
                    ),
                  ),
                ),
              ),
            ],
          ),
        ),
      ),
      Declaration(
        "ClassStyle",
        no_ctx(
          StyleDecl(
            [],
            [
              (
                ClassKey("root"),
                [
                  (
                    no_ctx(Variable("fontSize")),
                    no_ctx(
                      Execution(
                        no_ctx(Variable("px")),
                        [no_ctx(NumericLit(20))],
                      ),
                    ),
                  ),
                  (
                    no_ctx(Variable("backgroundColor")),
                    no_ctx(Variable("red")),
                  ),
                ],
              ),
            ],
          ),
        ),
      ),
    ],
  );

let _with_export = (name, s) => s ++ Printf.sprintf("EXPORT %s;", name);
let _component_with_export = (name, s) =>
  Printf.sprintf(
    "var %s=$$knot_jsx$$.createComponent('%s',%s);",
    name,
    name,
    s,
  )
  |> _with_export(name);
let _var_with_export = (name, s) =>
  Printf.sprintf("var %s=%s;", name, s) |> _with_export(name);
let _expand_arg = (index, name) =>
  Printf.sprintf(
    "var %s=%s.arg(arguments,%n,'%s');",
    name,
    platform_plugin,
    index,
    name,
  );
let _expand_default_arg = (index, name) =>
  Printf.sprintf(
    "var %s=%s.arg(arguments,%n,'%s',%s);",
    name,
    platform_plugin,
    index,
    name,
  );

let full_generated =
  _var_with_export("numericConst", "8")
  ++ _var_with_export("additionConst", "(1+10)")
  ++ _var_with_export("subtractionConst", "(8-2)")
  ++ _var_with_export("multiplicationConst", "(2*3)")
  ++ _var_with_export("divisionConst", "(4/2)")
  ++ _var_with_export("stringConst", "'Hello, World!'")
  ++ _var_with_export("trueConst", "true")
  ++ _var_with_export("falseConst", "false")
  ++ _var_with_export("lessThanConst", "(7<9)")
  ++ _var_with_export("lessThanEqualConst", "(8<=2)")
  ++ _var_with_export("greaterThanConst", "(2>4)")
  ++ _var_with_export("greaterThanEqualConst", "(9>=1)")
  ++ _var_with_export("closureConst", "((3*2)+(1+((6/(2-5))*3)))")
  ++ _var_with_export("jsxConst", "$$knot_jsx$$.createElement('abc')")
  ++ _var_with_export(
       "jsxWithPropsConst",
       "$$knot_jsx$$.createElement('def',{num:(8+9),bool:false,first:'look',under:'there'})",
     )
  ++ _var_with_export("punnedVariable", "'something'")
  ++ _var_with_export(
       "nestedJSXConst",
       "$$knot_jsx$$.createElement('parent',{justMade:'you say'},"
       ++ /**/
          "$$knot_jsx$$.createElement('child',{punnedVariable:punnedVariable},"
       ++ /*  */ "$$knot_jsx$$.createElement('grandchild',{under:'wear'})"
       ++ /**/ "),"
       ++ /**/ "$$knot_jsx$$.createElement('sibling')"
       ++ ")",
     )
  ++ _var_with_export(
       "nestedExprJSXConst",
       "$$knot_jsx$$.createElement('container',null,"
       ++ /**/ "(numericConst+20),"
       ++ /**/ "$$knot_jsx$$.createElement('middle'),"
       ++ /**/ "(stringConst+'one')"
       ++ ")",
     )
  ++ _var_with_export(
       "fragmentJSXConst",
       "$$knot_jsx$$.createFragment("
       ++ /**/ "$$knot_jsx$$.createElement('div'),"
       ++ /**/ "$$knot_jsx$$.createElement('span')"
       ++ ")",
     )
  ++ _with_export("compactFunc", "function compactFunc(){return 4;}")
  ++ _with_export(
       "compactExprFunc",
       "function compactExprFunc(){return (numericConst+subtractionConst);}",
     )
  ++ _with_export(
       "multiExprFunc",
       "function multiExprFunc(){"
       ++ /**/ "(divisionConst+multiplicationConst);"
       ++ /**/ "return stringConst;"
       ++ "}",
     )
  ++ _with_export(
       "paramFunc",
       "function paramFunc(){"
       ++ /**/ _expand_arg(0, "a")
       ++ /**/ "return a;"
       ++ "}",
     )
  ++ _with_export(
       "NoParamsState",
       "function NoParamsState($_state_factory){"
       ++ /**/ "return {"
       ++ /*  */ "get:function(){"
       ++ /*    */ "return $_state_factory.build({});"
       ++ /*  */ "}"
       ++ /**/ "};"
       ++ "}",
     )
  ++ _with_export(
       "DefaultParamState",
       "function DefaultParamState($_state_factory){"
       ++ /**/ "var z=$$knot_platform$$.arg(arguments,0,'z',30);"
       ++ /**/ "return {"
       ++ /*  */ "get:function(){"
       ++ /*    */ "return $_state_factory.build({});"
       ++ /*  */ "}"
       ++ /**/ "};"
       ++ "}",
     )
  ++ _with_export(
       "ComplexState",
       "function ComplexState($_state_factory){"
       ++ /**/ "var $a=undefined;"
       ++ /**/ "var $setA=$_state_factory.mut("
       ++ /*  */ "\"setA\", "
       ++ /*  */ "(function (){"
       ++ /*    */ "var a1=$$knot_platform$$.arg(arguments,0,'a1');"
       ++ /*    */ "$a=a1;"
       ++ /*    */ "return;"
       ++ /*  */ "})"
       ++ /**/ ");"
       ++ /**/ "return {"
       ++ /*  */ "get:function(){"
       ++ /*    */ "return $_state_factory.build({a:$a,setA:$setA});"
       ++ /*  */ "}"
       ++ /**/ "};"
       ++ "}",
     )
  ++ _component_with_export(
       "TypedParamView",
       "function($_props){"
       ++ /**/ "var a=$$knot_platform$$.prop($_props,'a');"
       ++ "}",
     )
  ++ _component_with_export(
       "DefaultParamView",
       "function($_props){"
       ++ /**/ "var a=$$knot_platform$$.prop($_props,'a',4);"
       ++ "}",
     )
  ++ _component_with_export(
       "MultiParamView",
       "function($_props){"
       ++ /**/ "var m=$$knot_platform$$.prop($_props,'m');"
       ++ /**/ "var a=$$knot_platform$$.prop($_props,'a',2);"
       ++ "}",
     )
  ++ _with_export(
       "ClassStyle",
       "function ClassStyle(){" ++ "return {['.root']:{}};" ++ "}",
     );
